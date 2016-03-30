# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module saves and restores state of the compatible components (undo/redo functionality)

module_help="""
Module snapshots saves and restores state of the compatible components,
providing undo/redo functionality.
It records changes of the opened figures and possibly the state of other modules.

Commands:
  undo [<count>]  -goes <count> steps backwards in the history
  redo [<count>]  -goes <count> steps forwards in the history
  snapshots maxcount[=<count>] - gets/sets the maximum number of snapshots (defaults to 1000)

Python interface:
  undo(count)
  redo(count)
    - moves backwards/forwards in the history
  setState(name, state)
    - changes state of the named component to the given one
  getState(name, snapshotIndex=None)
    - returns current (or wanted) state of the named component, or None
  restoringInProgress()
    - returns whether restoring is in progress
  registerCallbackCreate(func)
  unregisterCallbackCreate(func)
    - (un)registers callback function func(), to be called on snapshot creation
    - it can be used to save state, whose change shouldn't cause snapshot creation
  registerCallbackRestore(func)
  unregisterCallbackRestore(func)
    - (un)registers callback function func(oldSnapshotIndex), to be called on restore
    - oldSnapshotIndex is valid only within the call of func, it can change
  setClearableProperty(name, clearable=True)
    - clearable property is set to None after every snapshot, the others are always inherited
  set_maxcount(count)
  get_maxcount()
    - gets/sets the maximum number of snapshots

uses modules: [helpMod]

For more information see snapshots.py
"""

try:
	import helpMod
	helpMod.addModule("snapshots", module_help)
	helpMod.addPage("undo", module_help)
	helpMod.addPage("redo", module_help)
	helpMod.addPage("snapshots", module_help)
except ImportError:
	pass

import gf
from time import time
snapshots=[dict()]
currentIndex=0

currentState=dict()
restoreCallbacks=set()
createCallbacks=set()
uncommittedChanges=False
restoring=False
clearableProperties=set()
maxCnt=1000


class NonexistentSnapshotError(LookupError):
	def __init__(self, msg):
		LookupError.__init__(self, msg)
	pass


def undo(cnt=1):
	global uncommittedChanges
	global currentIndex
	if uncommittedChanges:
		createSnapshot()
	for i in range(0, cnt):
		if currentIndex>0:
			restoreSnapshot(currentIndex-1)
		else:
			raise NonexistentSnapshotError("Already at oldest change")

def redo(cnt=1):
	global uncommittedChanges
	global currentIndex
	if uncommittedChanges:
		createSnapshot()
	for i in range(0, cnt):
		if currentIndex<len(snapshots)-1:
			restoreSnapshot(currentIndex+1)
		else:
			raise NonexistentSnapshotError("Already at newest change")

lastChangeTime=0
def setState(name, state):
	global lastChangeTime
	global uncommittedChanges
	global restoring
	if uncommittedChanges and time()-lastChangeTime>0.02:
		createSnapshot()
	callCallbacks=not uncommittedChanges
	uncommittedChanges=True
	lastChangeTime=time()
	try:
		if callCallbacks:
			for func in createCallbacks:
				func()
	finally:
		currentState[name]=state

def getState(name, snapshotIndex=None):
	if name=="index":
		if snapshotIndex==None:
			return currentIndex
		elif snapshotIndex<len(snapshots):
			return snapshotIndex
		else:
			return None
	else:
		try:
			if snapshotIndex==None:
				return currentState[name]
			else:
				return snapshots[snapshotIndex][name]
		except KeyError:
			return None

def restoringInProgress():
	return restoring


def setClearableProperty(name, clearable=True):
	global clearableProperties
	if clearable:
		clearableProperties.add(name)
	else:
		try: clearableProperties.remove(name)
		except KeyError: pass


def registerCallbackRestore(func):
	restoreCallbacks.add(func)

def unregisterCallbackRestore(func):
	restoreCallbacks.remove(func)

def registerCallbackCreate(func):
	createCallbacks.add(func)

def unregisterCallbackCreate(func):
	createCallbacks.remove(func)


def set_maxcount(newCnt):
	global maxCnt
	if newCnt<0:
		newCnt=0
	maxCnt=newCnt
	checkCnt()

def get_maxcount():
	global maxCnt
	return maxCnt
	

gf.addCommand("undo", "snapshots.undo(%)", -1)
gf.addCommand("redo", "snapshots.redo(%)", -1)
gf.addCommand("snapshots maxcount", "'  maxcount=' + str(snapshots.get_maxcount())")
gf.addCommand("snapshots maxcount=", "snapshots.set_maxcount(%)", 1, "-")


# Private methods

def checkCnt():
	global maxCnt
	global snapshots
	global currentIndex
	diff=len(snapshots)-maxCnt
	if diff>0:
		currentIndex=currentIndex-diff
		del snapshots[:diff]

def createSnapshot():
	global uncommittedChanges
	global currentIndex
	global currentState
	global snapshots
	del snapshots[currentIndex+1:]
	snapshots.append(currentState.copy())
	checkCnt()
	for key in clearableProperties:
		if key in currentState:
			del currentState[key]
	currentIndex=currentIndex+1
	uncommittedChanges=False

def restoreSnapshot(index):
	global currentIndex
	global currentState
	global snapshots
	global restoring
	try:
		restoring=True
		oldIndex=currentIndex
		currentIndex=index
		currentState=snapshots[index].copy()
		gf.set_convexhull(False)
		for func in restoreCallbacks:
			func(oldIndex)
	finally:
		restoring=False


# Handle figure changes

setClearableProperty("new")

def onNew():
	if restoringInProgress():
		return
	setState("new", True)
	setState("gfFigure", None)

def onOpen(path):
	if restoringInProgress():
		return
	setState("gfFigure", gf.figureGet())

def onModify():
	if restoringInProgress():
		return
	setState("gfFigure", gf.figureGet())

def onSnapshotCreate():
	setState("selectedVertex", gf.vertexSelected())

def onSnapshotRestore(oldIndex):
	newIndex=getState("index")
	preserveRotation = (abs(newIndex-oldIndex)==1 and not getState("new", max(oldIndex,newIndex)))
	gf.figureOpen(getState("gfFigure"), preserveRotation)
	selectedVertex=getState("selectedVertex")
	if selectedVertex>=0:
		gf.vertexSelect(selectedVertex)
	else:
		gf.vertexDeselect()

gf.registerCallback("new", onNew)
gf.registerCallback("open", onOpen)
gf.registerCallback("modified", onModify)
registerCallbackRestore(onSnapshotRestore)
registerCallbackCreate(onSnapshotCreate)

