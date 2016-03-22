# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module allows to create convex figure from hyperplanes bordering it

module_help = """
Module spaceCuts creates convex figures from hyperplanes bordering them,
it has only Python interface:

  figureFromArea(hyperplanes, innerPoint)
    -creates objFigure containing innerPoint and being bounded by the hyperplanes
  hyperplaneOfFacet(facet, positivePoint)
    -creates hyperplane containing the given facet
    -optional positivePoint will have positive oriented distance from the hyperplane
  hyperplanesOfFigure(figure)
    -returns list of hyperplanes containing the facets of the figure

uses modules: algebra, duals, gfUtils, [helpMod]

For more information see spaceCuts.py
"""

import algebra
import duals
import gfUtils

try:
	import helpMod
	helpMod.addModule("spacecuts", module_help)
except ImportError:
	pass

class WrongAreaError(RuntimeError):
	def __init__(self):
		RuntimeError.__init__(self, "Inconsistent or infinite area")
	pass

def figureFromArea(hyperplanes, innerPoint):
	dualVertsPos=[duals.dualPointFromHyperplane(h, innerPoint) for h in hyperplanes]
	dualFigure=gfUtils.createConvexObjFigure(dualVertsPos)
	return duals.createDual(dualFigure, innerPoint)

def hyperplaneOfFacet(facet, positivePoint=None):
	vertsPos=[v.position for v in facet if v.dim==0]
	return algebra.hyperplaneFromPoints(vertsPos, positivePoint)

def hyperplanesOfFigure(figure):
	hyperplanes=[]
	vertsPos=[v.position for v in figure if v.dim == 0]
	innerPoint=algebra.vectAvg(*vertsPos)
	for facet in figure.boundary:
		hyperplanes.append(hyperplaneOfFacet(facet, innerPoint))
	return hyperplanes

