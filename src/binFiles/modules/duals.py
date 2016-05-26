# Geometric Figures  Copyright (C) 2015--2016  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module allows creating duals of figures

module_help="""
Module duals creates duals of figures.

Commands:
  create dual  -creates dual of the opened figure
                using polar reciprocation

Python interface:
  createDual(figure, centerPoint)
    -creates dual of the Figure object
    -faces has origDualFace property pointing to original ones
  dualPointFromHyperplane(hyperplane, centerPoint)
    -creates dual point to the given hyperplane
     using polar reciprocation with the given center and radius 1

uses modules: algebra, check, objFigure, [figureInfo], [helpMod]

For more information see duals.py
"""

import objFigure
from objFigure import Figure, Vertex
import gf
from operator import attrgetter
import algebra
import check

try: import figureInfo
except ImportError: figureInfo=None

try:
	import helpMod
	helpMod.addModule("duals", module_help)
	helpMod.addPage("create dual", module_help)
except ImportError:
	pass

def dualPointFromHyperplane(hyperplane, centerPoint=None):
	if not centerPoint:
		centerPoint=(0,)*len(hyperplane.normal)
	dist=hyperplane.orientedDistance(centerPoint)
	if abs(dist) < 0.0001:
		raise RuntimeError("Hyperplane passing through the center")
	return algebra.vectSum(algebra.vectMult(-1/dist, hyperplane.normal), centerPoint)

def createDual(figure, centerPoint=None):
	if figure.dim<2:
		raise RuntimeError("Figure has to have at least 2 dimensions")
	dualFigure=Figure()
	objFigure.updateVerticesLists(figure)
	for f in figure:
		f.dualCache=None
	try:
		for f in figure:
			d=figure.dim-f.dim-1
			if d<0: continue
			if d==0: # create vertex of the dual figure using polar reciprocation
				vertsPos=[v.position for v in f.vertices]
				hyperplane=algebra.hyperplaneFromPoints(vertsPos)
				f.dualCache=Vertex(dualPointFromHyperplane(hyperplane, centerPoint))
			else:
				f.dualCache=Figure()
				f.dualCache.setDim(d,figure.spaceDim)
			f.dualCache.origDualFace=f
		for f in figure:
			if f.dim<figure.dim:
				for child in f.boundary:
					child.dualCache.addToBoundary(f.dualCache)
				if f.dim == 0:
					dualFigure.addToBoundary(f.dualCache)
	finally:
		for f in figure:
			del f.dualCache
	return dualFigure


def commandCreateDual():
	figures=objFigure.fromGfFigure(gf.figureGet())
	for f in figures:
		if not check.isFigureConvex(f):
			raise RuntimeError("The figure is not convex")
	if figureInfo:
		name, desc = figureInfo.getNameDesc()
	figures=[createDual(f) for f in figures]
	gf.figureOpen(objFigure.toGfFigure(figures), True)
	if figureInfo:
		figureInfo.setNameDescPath("Dual of " + name, None)

gf.addCommand("create dual", "duals.commandCreateDual()")
