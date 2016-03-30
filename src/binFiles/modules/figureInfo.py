# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module manages information about opened figure

module_help="""
Module figureInfo shows some information about figure.

Commands:
  info  -shows information about figure

Some information is generated automatically, other can be set.

After figure is generated,
figureInfo.setNameDescPath(name, description, path) should be called,
the information is then printed.


Python interface:
  setNameDescPath(name, description, path)
  getNameDesc()
  getPath()
  counts(figure)        -returns counts of faces of Figure object
  printAll()            -prints information about opened figure

uses modules: objFigure, [snapshots], [helpMod]

For more information see figureInfo.py
"""

import gf
import os
import pprint
import objFigure

try: import snapshots
except ImportError: snapshots=None

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
	if snapshots and snapshots.restoringInProgress():
		return
	global name
	global description
	global filePath
	global modified
	name=None
	description=None
	filePath=None
	modified=False
	if snapshots:
		updateSnapshotState()
def onOpen(path):
	if snapshots and snapshots.restoringInProgress():
		return
	global name
	global description
	global filePath
	if snapshots:
		updateSnapshotState()
	gf.clear()
	printAll()
def onModify():
	if snapshots and snapshots.restoringInProgress():
		return
	global modified
	global name
	modified=True
	if snapshots:
		updateSnapshotState()
def onWrite(path):
	filePath=gf.expandPath(path)
	if os.access(filePath, os.W_OK):
		with open(filePath, "a") as f:
			f.write("""
try:
	import figureInfo
	figureInfo.setNameDescPath(""" + pprint.pformat(name) + ", " + pprint.pformat(description) + """, __file__)
except ImportError: pass""")


gf.registerCallback("new", onNew)
gf.registerCallback("open", onOpen)
gf.registerCallback("modified", onModify)
gf.registerCallback("write", onWrite)

if snapshots:
	def updateSnapshotState():
		snapshots.setState("figureInfo", (name, description, filePath, modified))
	def onSnapshotRestore(oldIndex):
		global name
		global description
		global filePath
		global modified
		state=snapshots.getState("figureInfo")
		if state:
			#(name, description, filePath, modified) = state
			name=state[0]
			description=state[1]
			filePath=state[2]
			modified=state[3]
		else:
			onNew()
		gf.clear()
		printAll()
	snapshots.registerCallbackRestore(onSnapshotRestore)

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
	if snapshots:
		updateSnapshotState()
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

