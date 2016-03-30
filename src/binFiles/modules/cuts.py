# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module contains functions for cutting figures.
# It adds command :cut for direct access.

module_help="""
Module cuts allows cutting figures.

Commands:
  cut figure [<lastCoord>]     -cross-section by a hyperplane orthogonal to the last axis
  cut off [<lastCoord>]        -cuts off the nearer part of the figure det. by the hyperplane
    <lastCoord> = last coordinate of points in the hyperplane, defaults to 0
  cut vertices [<ratio>]       -cuts off vertices
  cut edges [<ratio>]          -cuts off edges
  cut faces [<dim> [<ratio>]]  -cuts off faces of given dimension (defaults to 2)
    <ratio> = d(face, hyperplane) : d(face, origin), defaults to 0.1

Python interface:
  cutFigure(figure, hyperplane)                    -returns both parts and section
  cutOff(figure, hyperplanes, showProgress)        -cuts off parts
  cutOffConvex(figure, hyperplanes, innerPoint)    -cuts off parts, faster than the previous
  cutOffFaces(figure, ratio, faces, showProgress)  -cuts off given faces
  cutOffFacesDim(figure, ratio, dim, showProgress) -cuts off all <dim>-faces

uses modules: algebra, objFigure, spaceCuts, check, [figureInfo], [helpMod]

For more information see cuts.py
"""

import itertools
from operator import attrgetter
import objFigure
from objFigure import Figure, Vertex, figuresIterator;
import algebra
from algebra import Hyperplane;
import gf
import spaceCuts
import check

try:
	import figureInfo
except ImportError:
	figureInfo=None

try:
	import helpMod
	helpMod.addModule("cuts", module_help)
	helpMod.addPage("cut", module_help)
except ImportError:
	pass

# Cuts Figure object with given Hyperplane,
# returns ([Figure negative_parts], [Figure section_parts], [Figure positive_parts])
def cutFigure(figure, hyperplane, reuseCache=False):
	if not reuseCache:
		for f in figure:
			f.cuttingCache=None

	if figure.cuttingCache: # Already computed
		return figure.cuttingCache
	def cache(left, section, right): # Caching before returning
		if reuseCache:
			figure.cuttingCache=left,section,right
		else: # Remove cache
			for f in figure:
				del f.cuttingCache
		return left, section, right

	if figure.dim == 0: # Vertex
		dist=hyperplane.orientedDistance(figure.position)
		if abs(dist)<0.00001:
			return cache([], [figure], [])
		elif dist<0:
			return cache([figure], [], [])
		else:
			return cache([], [], [figure])

	else: # Edge, face, ...
		left,middle,right = \
				zip(*map(lambda f: cutFigure(f, hyperplane, True), figure.boundary))
		left,middle,right = (reduce(lambda a,b: a+b, l) for l in (left,middle,right))

		if not right and not left: # All in the hyperplane
			return cache([], [figure], [])
		elif not right:            # Nothing right
			return cache([figure], middle, [])
		elif not left:             # Nothing left
			return cache([], middle, [figure])

		elif figure.dim == 1:      # Cutting edge
			diff=algebra.vectDiff(right[0].position, left[0].position)
			prod=algebra.dotProduct(diff, hyperplane.normal)
			dist=hyperplane.orientedDistance(left[0].position)
			vert=Vertex(algebra.vectSum(left[0].position, algebra.vectMult(-dist/prod, diff)))
			leftEdge=Figure([left[0], vert])
			rightEdge=Figure([right[0], vert])
			return cache([leftEdge], [vert], [rightEdge])

		else:                      # Cutting face (at least 2-dimensional)
			middle.sort(key=attrgetter('dim'), reverse=True)
			for f in figuresIterator(middle):
				f.used=False
			section=[]
			middle2=set()
			for f in middle:
				if not f.used:
					if f.dim == figure.dim-2:
						middle2.add(f)
					else:
						section.append(f)
					for f2 in f:
						f2.used=True
			for f in figuresIterator(middle):
				del f.used

			if True: # General case

				def findComponents(figures, exclude):
					for f in figures:
						f.group=None
						f.groupIndex=None
						f.groupRank=0
						for f2 in f.boundary:
							f2.groupRank=0
							f2.group=None
					def find(f):
						f2=f
						while f.group:
							f=f.group
						while f2.group:
							f3=f2.group
							f2.group=f
							f2=f3
						return f
					def union(parent, child):
						parent=find(parent)
						child=find(child)
						if child == parent:
							return
						if parent.groupRank>child.groupRank:
							child.group=parent
						elif parent.groupRank == child.groupRank:
							child.group=parent
							parent.groupRank+=1
						else:
							parent.group=child
						return
					for f in figures:
						for f2 in f.boundary:
							if not f2 in exclude:
								union(f, f2)
					groups=[]
					for f in figures:
						g=find(f)
						if g.groupIndex == None:
							g.groupIndex=len(groups)
							groups.append([])
						groups[g.groupIndex].append(f)
					for f in figures:
						del f.group
						del f.groupIndex
						del f.groupRank
						for f2 in f.boundary:
							f2.group=None
							del f2.group
							f2.groupRank=None
							del f2.groupRank
					return groups

				leftComp=findComponents(left, middle2)
				rightComp=findComponents(right, middle2)
				leftComp=[Figure(c) for c in leftComp]
				rightComp=[Figure(c) for c in rightComp]

				sectComp=[]
				for f in leftComp:
					comp=[set()]
					for f2 in f.boundary:
						comp[0] |= f2.boundary & middle2
					sectComp.append(comp)
					f.sect=comp
				# sectComp=[ [set(vertices)] ]
				for f in rightComp:
					f.sect=[]
				for c in sectComp:
					m=c.pop()
					for f in rightComp:
						comp=[set()]
						for f2 in f.boundary:
							comp[0] |= f2.boundary & m
						c.append(comp)
						f.sect.append(comp)
				# sectComp=[ [[set(vertices)]] ]
				for l in sectComp:
					for m in l:
						m.append(Figure(m.pop()))
						section.append(m[0])
				for f in leftComp+rightComp:
					for f2 in f.sect:
						f.addToBoundary(f2[0])
					del f.sect

			else: # Convex only

				newSect=Figure(middle2)
				leftComp=[Figure(left + [newSect])]
				rightComp=[Figure(right + [newSect])]
				section.append(newSect)

			return cache(leftComp, section, rightComp)


# Cuts off parts of the Figure object determined by the given Hyperplanes,
# iteratively calls cutFigure function, slow
# returns list of new figures
def cutOff(figure, hyperplanes, showProgress=False):
	figures=[figure]
	i=0
	for h in hyperplanes:
		i+=1
		figures2=[]
		for f in figures:
			figures2.extend(cutFigure(f, h)[0])
		figures=figures2
		if showProgress:
			gf.clear()
			gf.echo("Cutting figure, hyperplane " + str(i) + "/" + str(len(hyperplanes)) + ", press any key to abort...")
			if not gf.sleep(0):
				return [figure]
	if showProgress:
		gf.clear()
		gf.echo("Figure has been cut")
	return figures

# Cuts off parts of the Figure object determined by the given Hyperplanes,
# the figure has to be convex, the innerPoint has to lie in the new figure,
# much faster than cutOff function
# returns list of new figures (always one-item)
def cutOffConvex(figure, hyperplanes, innerPoint=None):
	hyperplanes=hyperplanes+spaceCuts.hyperplanesOfFigure(figure)
	return [spaceCuts.figureFromArea(hyperplanes, innerPoint)]


# Cuts off specified vertices/edges/faces of a figure,
# ratio = d(face, hyperplane) : d(face, origin)
def cutOffFaces(figure, ratio, faces):
	hyperplanes=[]
	innerPoint=algebra.vectAvg(*[v.position for v in figure if v.dim==0])
	for f in faces:
		points=[v.position for v in f if v.dim==0]
		basis=algebra.orthonormalBasisFromPoints(points)
		normal=algebra.orthogonalizeVect(algebra.vectDiff(points[0], innerPoint), basis)
		if algebra.vectLen(normal)<0.0001:
			raise RuntimeError("Cannot cut faces")
		hyperplanes.append(Hyperplane(normal, (1-ratio+algebra.dotProduct(normal, innerPoint)/algebra.vectLen(normal)**2)))
	return cutOffConvex(figure, hyperplanes, innerPoint)

# Cuts off faces of specified dimension of a figure,
# ratio = d(vertex, hyperplane) : d(vertex, origin)
def cutOffFacesDim(figure, ratio, dim):
	faces=[]
	for f in figure:
		if f.dim==dim:
			faces.append(f)
	if not faces:
		raise RuntimeError("No faces of the specified dimension found")
	return cutOffFaces(figure, ratio, faces)



def commandCutFaces(dim=2, ratio=0.1):
	if ratio>=1 or ratio<=0:
		raise RuntimeError("Wrong ratio")
	figures=objFigure.fromGfFigure(gf.figureGet())
	for f in figures:
		if not check.isFigureConvex(f):
			raise RuntimeError("The figure is not convex")
	if figureInfo:
		name, desc = figureInfo.getNameDesc()
	figures2=[]
	for f in figures:
		figures2.extend(cutOffFacesDim(f, ratio, dim))
	gf.figureOpen(objFigure.toGfFigure(figures2), True)
	gf.clear()
	if dim==0:
		name = name + " with vertices cut off"
	elif dim==1:
		name = name + " with edges cut off"
	else:
		name = name + " with " + str(dim) + "-faces cut off"

	if figureInfo:
		figureInfo.setNameDescPath(name, None)


def commandCutOff(lastCoordinate=0):
	gfFigure=gf.figureGet()
	if gfFigure:
		gfFigure[0]=[gf.posRotate(p) for p in gfFigure[0]]
	figures=objFigure.fromGfFigure(gfFigure)
	if not figures:
		raise RuntimeError("Nothing opened")
	dim=figures[0].spaceDim
	hyperplane=algebra.Hyperplane((0,)*(dim-1)+(1,), lastCoordinate)
	newFigures=[]
	for f in figures:
		newFigures.extend(cutFigure(f, hyperplane)[0])
	gfFigure=objFigure.toGfFigure(newFigures)
	if gfFigure:
		gfFigure[0]=[gf.posRotateBack(p) for p in gfFigure[0]]
	if figureInfo:
		name, desc=figureInfo.getNameDesc()
	gf.figureOpen(gfFigure, True)
	if figureInfo:
		figureInfo.setNameDescPath("Truncated "+name, None)

def commandCutFigure(lastCoordinate=0):
	gfFigure=gf.figureGet()
	gfFigure[0]=[gf.posRotate(p) for p in gfFigure[0]]
	figures=objFigure.fromGfFigure(gfFigure)
	if not figures:
		raise RuntimeError("Nothing opened")
	dim=figures[0].spaceDim
	if dim<=0:
		raise RuntimeError("The figure cannot be cut")
	hyperplane=algebra.Hyperplane((0,)*(dim-1)+(1,), lastCoordinate)
	newFigures=[]
	for f in figures:
		newFigures.extend(cutFigure(f, hyperplane)[1])
	gfFigure=objFigure.toGfFigure(newFigures)
	if gfFigure:
		gfFigure[0]=[p[:-1] for p in gfFigure[0]]
		del gfFigure[dim]
	if figureInfo:
		name, desc=figureInfo.getNameDesc()
	gf.figureOpen(gfFigure)
	if figureInfo:
		figureInfo.setNameDescPath("Cross-section of "+name, None)



gf.addCommand("cut vertices", "cuts.commandCutFaces(0)")
gf.addCommand("cut edges", "cuts.commandCutFaces(1)")
gf.addCommand("cut faces", "cuts.commandCutFaces()")
gf.addCommand("cut off", "cuts.commandCutOff()")
gf.addCommand("cut figure", "cuts.commandCutFigure()")

gf.addCommand("cut vertices ", "cuts.commandCutFaces(0,%)", -1, '-')
gf.addCommand("cut edges ", "cuts.commandCutFaces(1,%)", -1, '-')
gf.addCommand("cut faces ", "cuts.commandCutFaces(%)", -2, '-')
gf.addCommand("cut off ", "cuts.commandCutOff(%)", -1, '-')
gf.addCommand("cut figure ", "cuts.commandCutFigure(%)", -1, '-')
