# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module provides some functions calling gf module

module_help="""
Module gfUtils provides some functions utilizing gf module,
it has only Python interface:

  openFileRelative(offset)
    -opens file relatively to the opened one,
     offset +/- 1 means next/previous file
  openConvexFromVertList(vertices, name, description, path)
    -opens convex hull of the given list of vertices
    -the other arguments are optional
  createConvexObjFigure(vertices)
    -creates convex hull of the given list of vertices
    -returns objFigure.Figure object

uses modules: objFigure, [figureInfo], [helpMod]

For more information see gfUtils.py
"""

import gf
import os

try:
	import helpMod
	helpMod.addModule("gfutils", module_help)
except ImportError:
	pass

try:
	import figureInfo
except ImportError:
	figureInfo=None

# Opens file relatively to the currently opened one in the same folder
# offset 1 means next file, offset -1 means previous file
def openFileRelative(offset):
	import figureInfo
	currPath=figureInfo.getPath()
	currDir=None
	currFile=None
	if currPath:
		currDir,currFile=os.path.split(currPath)
	if not currPath or not os.access(currDir, os.F_OK):
		currDir=gf.expandPath("%/figures")
	files=os.listdir(currDir)
	files=[f for f in files if f[-3:]=='.py']
	files.sort()
	try:
		i=files.index(currFile)
	except:
		i=-1
	i= (i+offset) % len(files)
	gf.open(os.path.join(currDir, files[i]))

# Opens convex hull of the given list of coordinates of vertices
# The other arguments are forwarded to the figureInfo module
def openConvexFromVertList(vertices, name=None, description=None, path=None):
	if vertices:
		gfFigure=[vertices]+[[]]*len(vertices[0])
		gfFigure=gf.figureConvexHullUpdate(gfFigure)
		gf.figureOpen(gfFigure)
	else:
		gf.close()
	if figureInfo:
		figureInfo.setNameDescPath(name, description, path)


def createConvexObjFigure(vertices):
	import objFigure
	if vertices:
		gfFigure=[vertices]+[[]]*len(vertices[0])
		gfFigure=gf.figureConvexHullUpdate(gfFigure)
		gfFigure[0]=vertices; # to avoid rounding, tuples will have the same hash as original
		figures=objFigure.fromGfFigure(gfFigure)
		if len(figures)>1:
			figures=[f for f in figures if f.dim>0]
		if len(figures)!=1:
			raise RuntimeError("Unspecified error occurred")
		return figures.pop()
	else:
		return None
