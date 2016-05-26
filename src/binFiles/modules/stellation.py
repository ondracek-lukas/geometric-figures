# Geometric Figures  Copyright (C) 2015--2016  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

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

uses modules: algebra, objFigure, spaceCuts, [figureInfo], [helpMod]

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
	objFigure.updateVerticesLists(figure)
	objFigure.updateParentsLists(figure)
	innerPoint=algebra.vectAvg(*[v.position for v in figure.vertices])

	for f in figure.boundary:
		f.hyperplane=spaceCuts.hyperplaneOfFacet(f, innerPoint);
		f.hyperplane.origFacet=f

	newFacets=[]
	for f in figure.boundary:
		facets=set()
		for f2 in f.boundary:
			facets.update(f2.parents)
		facets.remove(f)
		hyperplanes=[f2.hyperplane for f2 in facets]
		hyperplane=f.hyperplane.inverse()
		hyperplane.origFacet=f
		apexPoint=algebra.vectAvg(*[v.position for v in f.vertices])
		dist=float('inf')
		for h in hyperplanes:
			p=algebra.dotProduct(hyperplane.normal, h.normal)
			if p<-0.0001:
				d=-h.orientedDistance(apexPoint)/p
				if d<0.0001:
					raise RuntimeError("The figure is not convex")
				elif d<dist:
					dist=d
		if dist == float('inf'):
			raise RuntimeError("Infinite stellations are not supported")

		hyperplanes.append(hyperplane)
		apexInnerPoint=algebra.vectSum(apexPoint, algebra.vectMult(dist/2.0, hyperplane.normal))
		apexFigure=spaceCuts.figureFromArea(hyperplanes, apexInnerPoint)

		apexBaseFacet=[f3 for f3 in apexFigure.boundary if f3.origHypp.origFacet==f][0]
		for f3 in apexFigure:
			f3.origFace=None
		apexFigure.rmFromBoundary(apexBaseFacet)
		for apexFacet in apexFigure.boundary:
			apexRidge=(apexFacet.boundary & apexBaseFacet.boundary).pop()
			apexRidge.origFace=(apexFacet.origHypp.origFacet.boundary & f.boundary).pop()
		objFigure.updateParentsLists(apexBaseFacet)
		queue=[]
		for apexRidge in apexBaseFacet.boundary:
			queue.extend(apexRidge.boundary)
		while queue:
			apexFace=queue.pop(0)
			if apexFace.origFace: continue
			origP1=apexFace.parents[0].origFace
			origP2=apexFace.parents[1].origFace
			apexFace.origFace=(origP1.boundary & origP2.boundary).pop()
			queue.extend(apexFace.boundary)
		for f3 in apexBaseFacet:
			del f3.parents
		queue=[apexFigure]
		apexFigure.mark=True
		while queue:
			apexFace=queue.pop(0)
			if not apexFace.mark: continue
			apexFace.mark=None
			for apexFace2 in list(apexFace.boundary):
				if apexFace2.origFace:
					apexFace.boundary.remove(apexFace2)
					apexFace.boundary.add(apexFace2.origFace)
				else:
					apexFace2.mark=True
					queue.append(apexFace2)
		newFacets.extend(apexFigure.boundary)

	return objFigure.Figure(newFacets)

def commandStellate():
	figures=objFigure.fromGfFigure(gf.figureGet())
	if figureInfo:
		name,desc=figureInfo.getNameDesc()
	figures=[stellateFigure(f) for f in figures]
	if name:
		name="Stellated " + name
	desc=None
	gf.figureOpen(objFigure.toGfFigure(figures), True)
	if figureInfo:
		figureInfo.setNameDescPath(name, desc)

gf.addCommand("stellate", "stellation.commandStellate()")
