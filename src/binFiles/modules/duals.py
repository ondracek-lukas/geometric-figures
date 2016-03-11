# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module allows creating duals of figures

module_help="""
Module duals creates duals of figures.

Commands:
  create dual  -creates dual of the opened figure
                using polar reciprocation

Python interface:
  createDual(figure)
    -creates dual of the Figure object
  dualPointFromHyperplane(hyperplane, centerPoint)
    -creates dual point to the given hyperplane
     using polar reciprocation with the given center and radius 1

using modules: algebra, objFigure, [figureInfo]

For more information see duals.py
"""

import objFigure
from objFigure import Figure, Vertex
import gf
from operator import attrgetter
import algebra
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
	figureElem = sorted(figure, key=attrgetter('dim'), reverse=True)[1:]
	dualFigure=Figure()
	for f in figureElem:
		f.dualCache=None
	try:
		for f in figureElem:
			if not f.dualCache: # create vertex of the dual figure using polar reciprocation
				vertsPos=[v.position for v in f if v.dim==0]
				hyperplane=algebra.hyperplaneFromPoints(vertsPos)
				f.dualCache=Vertex(dualPointFromHyperplane(hyperplane, centerPoint))
			for child in f.boundary:
				if not child.dualCache:
					child.dualCache=Figure()
				child.dualCache.addToBoundary(f.dualCache)
				if child.dim == 0:
					dualFigure.addToBoundary(child.dualCache)
	finally:
		for f in figureElem:
			del f.dualCache
	return dualFigure


def commandCreateDual():
	figures=objFigure.fromGfFigure(gf.figureGet())
	if figureInfo:
		name, desc = figureInfo.getNameDesc()
	figures=[createDual(f) for f in figures]
	gf.figureOpen(objFigure.toGfFigure(figures), True)
	if figureInfo:
		figureInfo.setNameDescPath("Dual of " + name, None)

gf.addCommand("create dual", "duals.commandCreateDual()")
