# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module allows creating duals of figures

module_help="""
Module duals creates duals of figures.

Commands:
  create dual  -creates dual of the opened figure
                using polar reciprocation

Python interface:
  createDual(figure)  -creates dual of the Figure object

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

def createDual(figure):
	if figure.dim<2:
		raise RuntimeError("Figure has to have at least 2 dimensions")
	figureElem = sorted(figure, key=attrgetter('dim'), reverse=True)[1:]
	dualFigure=Figure()
	for f in figureElem:
		f.dualCache=None
	for f in figureElem:
		if not f.dualCache: # create vertex of the dual figure using polar reciprocation
			vertsPos=[v.position for v in f if v.dim==0]
			ortBasis=algebra.orthonormalBasisFromPoints(vertsPos)
			dualVertPos=algebra.orthogonalizeVect(vertsPos[0], ortBasis)
			dualVertDist=algebra.vectLen(dualVertPos)
			if abs(dualVertDist) < 0.0001:
				raise RuntimeError("Facet passing through the origin")
			dualVertPos=algebra.vectMult(1/dualVertDist, dualVertPos)
			f.dualCache=Vertex(dualVertPos)
		for child in f.boundary:
			if not child.dualCache:
				child.dualCache=Figure()
			child.dualCache.addToBoundary(f.dualCache)
			if child.dim == 0:
				dualFigure.addToBoundary(child.dualCache)
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
		figureInfo.setNameDesc("Dual of " + name, None)

gf.addCommand("create dual", "duals.commandCreateDual()")
