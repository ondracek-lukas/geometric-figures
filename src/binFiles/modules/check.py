# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

module_help="""
Module check determines whether the figure is correctly closed
and whether it is convex, it has only Python interface:
  isFigureClosed(figure)
  isFigureConvex(figure)

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

def isFigureClosed(figure, checkFaces=True):
	if not isinstance(figure, objFigure.Figure):
		raise TypeError("objFigure.Figure needed")
	if checkFaces:
		for face in figure:
			if not isFigureClosed(face, False):
				return False
		return True;
	else:
		openEdges=set()
		closedEdges=set()
		for facet in figure.boundary:
			for edge in facet.boundary:
				if edge in closedEdges:
					return False
				if edge in openEdges:
					openEdges.remove(edge)
					closedEdges.add(edge)
				else:
					openEdges.add(edge)
		if openEdges:
			return False
		else:
			return True
	

def isFigureConvex(figure):
	if not isFigureClosed(figure):
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
