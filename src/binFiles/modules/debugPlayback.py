# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

import gf
from collections import deque

def sleep():
	return gf.sleep(300)
def redraw():
	#gfFigureTmp=[[f for f in layer if len(f)>1] for layer in gfFigure]
	#gfFigureTmp[2]=[f for f in gfFigureTmp[2] if reduce(lambda x,f2:x^gfFigureTmp
	tmpGfFigure=[]
	tmpMappingOldR=None
	for i in range(0, len(gfFigure)):
		tmpMapping=[]
		tmpMappingR=[]
		for j in range(0, len(gfFigure[i])):
			if (i==0 or len([k for k in gfFigure[i][j] if tmpMappingOldR[k]!=None])>1) and (i!=2 or reduce(lambda x,e:x^(2**gfFigure[1][e][0])^(2**gfFigure[1][e][1]), gfFigure[i][j], 0)==0):
				tmpMappingR.append(len(tmpMapping))
				tmpMapping.append(j)
			else:
				tmpMappingR.append(None)
		tmpGfFigure.append([gfFigure[i][j] for j in tmpMapping])
		if i>0:
			tmpGfFigure[i]=[[tmpMappingOldR[k] for k in f if tmpMappingOldR[k]!=None] for f in tmpGfFigure[i]]
		tmpMappingOldR=tmpMappingR

	gf.figureOpen(tmpGfFigure, True)
	return sleep()

def open(newGfFigure, byName):
	global gfFigure
	global figsByName
	global instructions
	gfFigure=newGfFigure
	figsByName=byName
	gf.figureOpen(gfFigure)
	gf.clear()
	sleep()
	instructions=deque()
	gf.map("<enter>", "debugPlayback.play()")


def attach(parent, child):
	instructions.append((attachNow, parent, child))
def detach(parent, child):
	instructions.append((detachNow, parent, child))
def rename(parent, child):
	instructions.append((renameNow, parent, child))

def step():
	try:
		inst=instructions.popleft()
	except IndexError: return False
	return inst[0](inst[1],inst[2])

def play():
	while step(): pass



def attachNow(parent, child):
	global gfFigure
	global figsByName
	try:
		parentTuple=figsByName[parent]
	except KeyError:
		parentDim=figsByName[child][0]+1
		parentTuple=(parentDim, len(gfFigure[parentDim]))
		figsByName[parent]=parentTuple
		gfFigure[parentDim].append([])
	gfFigure[figsByName[parent][0]][figsByName[parent][1]].append(figsByName[child][1])
	return redraw()

def detachNow(parent, child):
	global gfFigure
	global figsByName
	try:
		gfFigure[figsByName[parent][0]][figsByName[parent][1]].remove(figsByName[child][1])
		return redraw()
	except KeyError:
		pass
	except ValueError:
		pass
	return True

def renameNow(oldName, newName):
	fig=figsByName[oldName]
	del figsByName[oldName]
	figsByName[newName]=fig
	return True
