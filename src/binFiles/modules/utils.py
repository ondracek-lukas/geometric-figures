# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

import gf
import objFigure
from objFigure import figuresIterator
import algebra

def figureInfo(figure):
	cnts=[0]*(figure.dim+1)
	for f in figure:
		cnts[f.dim]+=1
	text=str(figure.dim) + "-dimensional figure ("
	if figure.dim>3:
		for i in range(figure.dim-1, 1, -1):
			text+= str(cnts[i]) + " " + str(i) + "d-faces, "
	elif figure.dim==3:
		text+= str(cnts[2]) + " faces, "
	if figure.dim>1:
		text+= str(cnts[1]) + " edges, "
	text+= str(cnts[0]) + " vertices)"
	return text;

def commandInfo():
	figures=objFigure.fromGfFigure(gf.figureGet())
	if not figures:
		gf.echo("Nothing opened")
	else:
		gf.echo(str.join("\n", [figureInfo(f) for f in figures]))

gf.addCommand("info", "utils.commandInfo()")



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


