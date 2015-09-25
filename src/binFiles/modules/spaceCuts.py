# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module allows to create convex figure from hyperplanes bordering it

module_help = """
Module spaceCuts creates convex figures from hyperplanes bordering them.
it has only Python interface:

  figureFromArea(hyperplanes, innerPoint)
    -creates objFigure containing innerPoint and being bounded by the hyperplanes
    -if innerPoint is not given, it is calculated
     to has positive oriented distances from all hyperplanes
    -this currently needs no figure to be opened
     and during the process, some figures may be shown
     (will be improved)
  hyperplaneOfFacet(facet, positivePoint)
    -creates hyperplane containing the given facet
    -optional positivePoint will have positive oriented distance from the hyperplane

uses modules: algebra, duals, gfUtils, [helpMod]

For more information see spaceCuts.py
"""

import algebra
import duals
import gfUtils

try:
	import helpMod
	helpMod.addModule("spaceCuts", module_help)
except ImportError:
	pass

class WrongAreaError(RuntimeError):
	def __init__(self):
		RuntimeError.__init__(self, "Inconsistent or infinite area")
	pass

def figureFromArea(hyperplanes, innerPoint=None):
	if not innerPoint:
		innerPoint=(0,)*len(hyperplanes[0].normal)
		for h in hyperplanes:
			dist=h.orientedDistance(innerPoint)
			if dist<0:
				innerPoint=algebra.vectSum(innerPoint, algebra.vectMult(-dist, h.normal))
		pointDiff=(0,)*len(innerPoint)
		hplanesCnt=len(hyperplanes)
		for h in hyperplanes:
			dist=h.orientedDistance(innerPoint)
			pointDiff=algebra.vectSum(pointDiff, algebra.vectMult(-dist/hplanesCnt, h.normal))
		innerPoint=algebra.vectSum(innerPoint, pointDiff)
		for h in hyperplanes:
			if h.orientedDistance(innerPoint)<0.0001:
				raise WrongAreaError()
	dualVertsPos=[duals.dualPointFromHyperplane(h, innerPoint) for h in hyperplanes];
	dualFigure=gfUtils.createConvexObjFigure(dualVertsPos)
	return duals.createDual(dualFigure, innerPoint)

def hyperplaneOfFacet(facet, positivePoint=None):
	vertsPos=[v.position for v in facet if v.dim==0]
	return algebra.hyperplaneFromPoints(vertsPos, positivePoint)

def hyperplanesOfFigure(figure):
	hyperplanes=[]
	vertsPos=[v.position for v in figure if v.dim == 0]
	innerPoint=algebra.vectMult(1.0/len(vertsPos), algebra.vectSum(*vertsPos))
	for facet in figure.boundary:
		hyperplanes.append(hyperplaneOfFacet(facet, innerPoint))
	return hyperplanes

