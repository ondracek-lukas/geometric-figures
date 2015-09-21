# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module manages information about opened figure

module_help="""
Module figureInfo shows some information about figure.

Commands:
  info  -shows information about figure

Some information is generated automatically, other can be set.


When opening figure from file <name>.dat,
metadata are read from file .<name>.txt,
the first line is the name, the others are description.

After figure is generated,
figureInfo.setNameDescPath(name, description, path) should be called.

In both cases information is printed.


Python interface:
  setNameDescPath(name, description, path)
  getNameDesc()
  getPath()
  counts(figure)        -returns counts of faces of Figure object
  printAll()            -prints information about opened figure

uses modules: objFigure, [helpMod]

For more information see figureInfo.py
"""

import gf
import os
import objFigure

try:
	import helpMod
	helpMod.addModule("figureinfo", module_help)
	helpMod.addPage("info", module_help)
except ImportError:
	pass

name=None
description=None
filePath=None
modified=False
def onNew():
	global name
	global description
	global filePath
	global modified
	name=None
	description=None
	filePath=None
	modified=False
def onOpen(path):
	global name
	global description
	global filePath
	if path and path[-4:]==".dat":
		filePath=gf.expandPath(path)
		parts=os.path.split(filePath)
		metaPath=os.path.join(parts[0], "." + parts[1][:-4] + ".txt")
		if os.access(metaPath, os.R_OK):
			with open(metaPath) as f:
				lines=f.readlines()
			if lines:
				name=lines.pop(0).rstrip()
				if len(lines)>0:
					description=str.join("", lines).rstrip()
		else:
			name=parts[1][:-4]
	gf.clear()
	printAll()
def onModify():
	global modified
	global name
	modified=True


gf.registerCallback("new", onNew)
gf.registerCallback("open", onOpen)
gf.registerCallback("modified", onModify)

def setNameDescPath(newName, newDescription, path=None):
	global name
	global description
	global modified
	global filePath
	name=newName
	if name:
		name=name.strip()
	description=newDescription
	if description:
		description=description.strip()
	filePath=path
	modified=False
	gf.clear()
	printAll()
def getNameDesc():
	n=name or "Unnamed"
	if modified:
		return n + ' (modified)', description
	else:
		return n, description
def getPath():
	return filePath

def counts(figure):
	cnts=[0]*(figure.dim+1)
	for f in figure:
		cnts[f.dim]+=1
	text=str(figure.dim) + "-dimensional figure ("
	if figure.dim>4:
		for i in range(figure.dim-1, 3, -1):
			text+= str(cnts[i]) + " " + str(i) + "-faces, "
	if figure.dim>3:
		text+= str(cnts[3]) + " cells, "
	if figure.dim>2:
		text+= str(cnts[2]) + " faces, "
	if figure.dim>1:
		text+= str(cnts[1]) + " edges, "
	text+= str(cnts[0]) + " vertices)"
	return text;

def printAll():
	figures=objFigure.fromGfFigure(gf.figureGet())
	if not figures:
		gf.echo("Nothing opened")
	else:
		if name:
			fileName=None
			if filePath:
				fileName=os.path.basename(filePath)
			n=name
			if fileName:
				n += ' (' + fileName + ')'
			if modified:
				n += ' (modified)'
			gf.echo(n)
		if description:
			gf.echo(str.join("\n", ["   "+s for s in description.split("\n")]))
		gf.echo("Content:")
		gf.echo(str.join("\n", ["   "+counts(f) for f in figures]))

gf.addCommand("info", "figureInfo.printAll()")

