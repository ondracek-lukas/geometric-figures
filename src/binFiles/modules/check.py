# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

module_help="""
Module check verifies some properties of figures,
it has only Python interface:
  isBoundaryConnected(figure, checkFaces=True)
  isBoundaryComplete(figure, checkFaces=True)
  isFigureConvex(figure)
  findComponets(figures, exclude=[])
    -divides list of figures of the same dimension
     into connected (by facets) components
    -exclude contains facets to be ignored

uses modules: algebra, objFigure, spaceCuts, [helpMod]

For more information see check.py.
"""

import objFigure
import spaceCuts
import algebra
import gf

try:
	import helpMod
	helpMod.addModule("check", module_help)
except ImportError:
	pass

def isBoundaryConnected(figure, checkFaces=True):
	if not isinstance(figure, objFigure.Figure):
		raise TypeError("objFigure.Figure needed")
	if checkFaces:
		for face in figure:
			if not isBoundaryConnected(face, False):
				return False
		return True;
	else:
		if figure.dim<2:
			return True
		if len(findComponents(figure.boundary))==1:
			return True
		else:
			return False

def isBoundaryComplete(figure, checkFaces=True):
	if not isinstance(figure, objFigure.Figure):
		raise TypeError("objFigure.Figure needed")
	if checkFaces:
		for face in figure:
			if not isBoundaryComplete(face, False):
				return False
		return True;
	else:
		oneParentEdges=set()
		twoParentsEdges=set()
		for facet in figure.boundary:
			for edge in facet.boundary:
				if edge in twoParentsEdges:
					return False
				if edge in oneParentEdges:
					oneParentEdges.remove(edge)
					twoParentsEdges.add(edge)
				else:
					oneParentEdges.add(edge)
		if oneParentEdges:
			return False
		else:
			return True


def isFigureConvex(figure):
	if not isBoundaryComplete(figure):
		return False
	innerPoint=algebra.vectAvg(*(v.position for v in figure if v.dim==0))
	facetsInnerPoints=dict()
	edgesParents=dict()
	for facet in figure.boundary:
		facetsInnerPoints[facet]=algebra.vectAvg(*(v.position for v in facet if v.dim==0))
		for edge in facet.boundary:
			if not edge in edgesParents:
				edgesParents[edge]=set()
			edgesParents[edge].add(facet)
	hyperplanes=spaceCuts.hyperplanesOfFigure(figure);
	for facet in figure.boundary:
		hyperplane=spaceCuts.hyperplaneOfFacet(facet, innerPoint)
		for edge in facet.boundary:
			facet2=[f for f in edgesParents[edge] if f!=facet][0]
			point=facetsInnerPoints[facet2]
			if hyperplane.orientedDistance(point)<0.0001:
				return False
	return True


def findComponents(figures, exclude=[]):
	for f in figures:
		f.component=None
		f.componentIndex=None
		f.componentRank=0
		for f2 in f.boundary:
			f2.componentRank=0
			f2.component=None
	def find(f):
		f2=f
		while f.component:
			f=f.component
		while f2.component:
			f3=f2.component
			f2.component=f
			f2=f3
		return f
	def union(parent, child):
		parent=find(parent)
		child=find(child)
		if child == parent:
			return
		if parent.componentRank>child.componentRank:
			child.component=parent
		elif parent.componentRank == child.componentRank:
			child.component=parent
			parent.componentRank+=1
		else:
			parent.component=child
		return
	for f in figures:
		for f2 in f.boundary:
			if not f2 in exclude:
				union(f, f2)
	components=[]
	for f in figures:
		g=find(f)
		if g.componentIndex == None:
			g.componentIndex=len(components)
			components.append([])
		components[g.componentIndex].append(f)
	for f in figures:
		del f.component
		del f.componentIndex
		del f.componentRank
		for f2 in f.boundary:
			f2.component=None
			del f2.component
			f2.componentRank=None
			del f2.componentRank
	return components


def commandIsConvex():
	figures=objFigure.fromGfFigure(gf.figureGet())
	for f in figures:
		if not isFigureConvex(f):
			gf.echo("The figure is not convex.")
			return
	cnt=len(figures)
	if len(figures)>1:
		gf.echo("All figures are convex.")
	else:
		gf.echo("The figure is convex.")

gf.addCommand("is convex", "check.commandIsConvex()")
