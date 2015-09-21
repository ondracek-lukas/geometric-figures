# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module provides some functions calling gf module

module_help="""
Module gfUtils provides some functions utilizing gf module,
it has only Python interface:

  openFileRelative(offset)  -opens file relatively to the opened one,
                             offset +/- 1 means next/previous file
  openConvexFromVertList(vertices, name, description, path)
                            -opens convex hull of the given list of vertices
                            -the other arguments are optional

uses modules: [figureInfo], [helpMod]

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
	files=[f for f in files if f[-3:]=='.py' or f[-4:]=='.dat']
	files.sort()
	try:
		i=files.index(currFile)
	except:
		i=-1
	i= (i+offset) % len(files)
	if files[i][-4:]=='.dat':
		gf.open(os.path.join(currDir, files[i]))
	else:
		gf.source(os.path.join(currDir, files[i]))

# Opens convex hull of the given list of coordinates of vertices
# The other arguments are forwarded to the figureInfo module
def openConvexFromVertList(vertices, name=None, description=None, path=None):
	if vertices:
		gfFigure=[vertices]+[[]]*len(vertices[0])
		convexhull=gf.get_convexhull()
		gf.set_convexhull(True)
		gf.figureOpen(gfFigure)
		gf.set_convexhull(convexhull)
	else:
		gf.close()
	if figureInfo:
		figureInfo.setNameDescPath(name, description, path)


