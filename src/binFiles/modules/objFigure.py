# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module provides objectively oriented access to figures
# and converting methods from/to gf.figure structure


# Iterator over the boundaries of (iterable of) figures
# Only one iterator can be active at a time
def figuresIterator(figures):
	markObj=object()
	for figure in figures:
		figure.markObj=markObj
		for f in figure:
			yield f


# Takes gf.figure structure, returns set of figures
def fromGfFigure(gfFigure):
	figures=[[]]
	for i in range(0, len(gfFigure[0])):
		figures[0].append(Vertex(gfFigure[0][i], i))
	for dim in range(1, len(gfFigure)):
		figures.append([])
		for i in range(0, len(gfFigure[dim])):
			figures[dim].append(Figure(map(lambda j: figures[dim-1][j], gfFigure[dim][i]), i))
	freeFigures=set()
	for lists in figures:
		for fig in lists:
			if not fig.bounds:
				freeFigures.add(fig)
	return freeFigures


# Takes iterable of figures, returns gf.figure structure
def toGfFigure(figures):
	if not figures:
		return None
	def gfStructure(figure, gfFigure):
		if figure.gfIndex!=None: return
		figure.gfIndex=len(gfFigure[figure.dim])
		gfFigure[figure.dim].append(figure)
		for child in figure.boundary:
			gfStructure(child, gfFigure)
	for f in figuresIterator(figures):
		f.gfIndex=None
	gfFigure=map(list, [[]]*(figures[0].spaceDim+1));
	for figure in figures:
		gfStructure(figure, gfFigure)
	gfFigure[0]=map(lambda f: f.position, gfFigure[0])
	for dim in range(1, len(gfFigure)):
		for i in range(0, len(gfFigure[dim])):
			gfFigure[dim][i]=map(lambda f: f.gfIndex, gfFigure[dim][i].boundary)
	return gfFigure;


class Figure:

	# Creates Figure from the (iterable of) figures bounding it
	def __init__(self, boundary=None, gfIndex=None):
		self.boundary=set() # childs
		self.bounds=set()   # parents
		self.gfIndex=gfIndex
		self.spaceDim=None
		self.dim=None
		self.mark=None
		self.markObj=None
		if boundary:
			for fig in boundary:
				self.addToBoundary(fig)


	def addToBoundary(self, child):
		if self.dim == None and child.dim != None:
			self.setDim(child.dim+1, child.spaceDim)
		elif self.dim != None:
			child.setDim(self.dim-1, self.spaceDim)
		self.boundary.add(child)
		child.bounds.add(self)
	def rmFromBoundary(self, child):
		self.boundary.discard(child)
		child.bounds.discard(self)


	# Gets every figure from boudary (of all dimensions) just once
	# Only one iterator can be active at a time
	def __iter__(self):
		if self.markObj:
			markObj=self.markObj
			self.markObj=None
		else:
			markObj=object()
		if self.mark == markObj: return
		self.mark=markObj
		yield self
		for child in self.boundary:
			child.markObj=markObj
			for f in child:
				yield f


	def setDim(self, dim, spaceDim): # internal method
		if self.dim != None:
			if self.dim != dim or self.spaceDim != spaceDim:
				raise RuntimeError("Wrong number of dimensions")
		elif dim<1:
			raise RuntimeError("Wrong number of dimensions")
		else:
			self.dim=dim
			self.spaceDim=spaceDim
			for parent in self.bounds:
				parent.setDim(dim+1, spaceDim)
			for child in self.boundary:
				child.setDim(dim-1, spaceDim)


class Vertex(Figure):

	# Creates Vertex from its coordinates
	def __init__(self, position, gfIndex=None):
		Figure.__init__(self, None, gfIndex)
		self.position=tuple(position)
		self.boundary=frozenset()
		self.spaceDim=len(position)
		self.dim=0
