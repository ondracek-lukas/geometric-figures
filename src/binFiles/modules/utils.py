# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

module_help="""
Module utils provides several utilities,
it has only Python interface:

  colorToTuple(color)            -converts color string to tuple
                                  (alpha,red,green,blue), each from 0 to 255
  tupleToColor(tuple)            -converts previous tuple to color string
  figuresMaxRadius(figures)      -the longest vector distance from the origin
  figuresScale(figures, factor)  -scales figures by given factor, in place

uses modules: algebra, objFigure, [helpMod]

For more information see utils.py
"""

import objFigure
from objFigure import figuresIterator
import algebra

try:
	import helpMod
	helpMod.addModule("utils", module_help)
except ImportError:
	pass

def colorToTuple(color):
	color=gf.normalizeColorAlpha(color)
	return tuple(ord(c) for c in color[1:].decode("hex"))
def tupleToColor(tupl):
	return "#{:02X}{:02X}{:02X}{:02X}".format(*tupl)

def figuresMaxRadius(figures):
	maxRadius=0
	for f in figuresIterator(figures):
		if f.dim==0:
			r=algebra.vectLen(f.position)
			if r>maxRadius:
				maxRadius=r
	return maxRadius

def figuresScale(figures, factor):
	for f in figuresIterator(figures):
		if f.dim==0:
			f.position=algebra.vectMult(factor, f.position)
