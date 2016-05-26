# Geometric Figures  Copyright (C) 2015--2016  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

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
  countsToStr(counts)   -converts counts to string describing the figure
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
def onOpen():
	global name
	global description
	global filePath
	if snapshots and not snapshots.restoringInProgress():
		updateSnapshotState()
	gf.clear()
	printAll()
def onModify():
	if snapshots and snapshots.restoringInProgress():
		gf.clear()
		printAll()
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
			name=None
			description=None
			filePath=None
			modified=False
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
	cnts=[0,]*(figure.dim+1)
	for f in figure:
		cnts[f.dim]+=1
	return tuple(cnts)

def countsToStr(cnts):
	dim=len(cnts)-1
	if dim==0:
		return "free vertex"
	else:
		text=str(dim) + "D figure ("
		if dim>4:
			for i in range(dim-1, 3, -1):
				text+= str(cnts[i]) + " " + str(i) + "-faces, "
		if dim>3:
			text+= str(cnts[3]) + " cells, "
		if dim>2:
			text+= str(cnts[2]) + " faces, "
		if dim>1:
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
		cntsD=dict()
		groupLines=False
		for f in figures:
			cnts=counts(f)
			if cnts in cntsD:
				cntsD[cnts]+=1
				groupLines=True
			else:
				cntsD[cnts]=1
		for cnts in cntsD:
			if not groupLines:
				gf.echo("   "+countsToStr(cnts))
			elif cntsD[cnts]==1:
				gf.echo("      "+countsToStr(cnts))
			else:
				gf.echo(("%4d" % cntsD[cnts])+"x "+countsToStr(cnts))

gf.addCommand("info", "figureInfo.printAll()")

