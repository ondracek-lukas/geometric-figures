# Geometric Figures	Copyright (C) 2015	Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module allows creating stellations of convex figures

module_help="""
Module stellation allows stellating figures.
Only finite ones are allowed.

Commands:
  stellate - creates stellated figures
           - ! application can freeze during generation !

Python interface:
  stellateFigure(figure)
    -creates stellated Figure object from the given one
    -it can last long time, application can freeze

uses modules: algebra, objFigure, spaceCuts, [figureInfo]

For more information see stellation.py
"""

import spaceCuts
import algebra
import objFigure
from operator import attrgetter
import gf
try: import figureInfo
except ImportError: figureInfo=None

try:
	import helpMod
	helpMod.addModule("stellation", module_help)
	helpMod.addPage("stellate", module_help)
except ImportError:
	pass

def stellateFigure(figure):
	for f in figure.boundary:
		for f2 in f:
			f2.bounds=[]
	for f in figure.boundary:
		for f2 in f:
			f2.bounds.append(f)
	vertsPos=[v.position for v in figure if v.dim == 0]
	innerPoint=algebra.vectMult(len(vertsPos), algebra.vectSum(*vertsPos))
	figByBoundary=dict()
	for f in sorted(figure, key=attrgetter("dim")):
		if f.dim == 0:
			pass
		else:
			figByBoundary[frozenset(f.boundary)]=f

	newFacets=[]
	for f in figure.boundary:
		facets=set()
		for f2 in f.boundary:
			facets=facets.union(f2.bounds) # to be in-place
		facets.remove(f)
		hyperplanes=[spaceCuts.hyperplaneOfFacet(f2, innerPoint) for f2 in facets]
		hyperplane=spaceCuts.hyperplaneOfFacet(f, innerPoint)
		hyperplanes.append(hyperplane.inverse())
		try:
			apexFigure=spaceCuts.figureFromArea(hyperplanes)
		except spaceCuts.WrongAreaError:
			raise RuntimeError("Infinite stellations are not supported")
		vertices=[v for v in f if v.dim == 0]
		apexVertices=[v for v in apexFigure if v.dim == 0]
		for f2 in sorted(apexFigure, key=attrgetter('dim')):
			f2.copy=None
			if f2.dim==0:
				for v in vertices: # slow, can be improved
					if algebra.pointsDist(v.position, f2.position) < 0.0001:
						f2.copy = v
						break
			else:
				boundary=set()
				shared=True
				for f3 in f2.boundary:
					boundary.add(f3.copy or f3)
					shared = shared and f3.copy
				f2.boundary=boundary
				if shared:
					f2.copy=figByBoundary[frozenset(boundary)]
				elif f2.dim==f.dim:
					newFacets.append(f2)
	return objFigure.Figure(newFacets)

def commandStellate():
	figures=objFigure.fromGfFigure(gf.figureGet())
	if figureInfo:
		name,desc=figureInfo.getNameDesc()
	figures=[stellateFigure(f) for f in figures]
	if name:
		name="Stellated " + name
	desc=None
	gf.figureOpen(objFigure.toGfFigure(figures))
	if figureInfo:
		figureInfo.setNameDescPath(name, desc)

gf.addCommand("stellate", "stellation.commandStellate()")
