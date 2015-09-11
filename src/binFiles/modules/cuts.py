# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module contains functions for cutting figures
# It adds command :cut for direct access (type without parameters for help)

import objFigure
from objFigure import Figure, Vertex, figuresIterator;
from operator import attrgetter
import algebra
from algebra import Hyperplane;

# Cuts Figure object with given Hyperplane,
# returns ([Figure negative_parts], [Figure section_parts], [Figure positive_parts])
def cutFigure(figure, hyperplane, reuseCache=False):
	if not reuseCache:
		for f in figure:
			f.cuttingCache=None

	if figure.cuttingCache: # Already computed
		return figure.cuttingCache
	def cache(left, section, right): # Caching before returning
		if reuseCache:
			figure.cuttingCache=left,section,right
		else: # Remove cache
			for f in figure:
				del f.cuttingCache
		return left, section, right

	if figure.dim == 0: # Vertex
		dist=hyperplane.orientedDistance(figure.position)
		if abs(dist)<0.00001:
			return cache([], [figure], [])
		elif dist<0:
			return cache([figure], [], [])
		else:
			return cache([], [], [figure])

	else: # Edge, face, ...
		left,middle,right = \
				zip(*map(lambda f: cutFigure(f, hyperplane, True), figure.boundary))
		left,middle,right = (reduce(lambda a,b: a+b, l) for l in (left,middle,right))

		if not right and not left: # All in the hyperplane
			return cache([], [figure], [])
		elif not right:            # Nothing right
			return cache([figure], middle, [])
		elif not left:             # Nothing left
			return cache([], middle, [figure])

		elif figure.dim == 1:      # Cutting edge
			diff=algebra.vectDiff(right[0].position, left[0].position)
			prod=algebra.dotProduct(diff, hyperplane.normal)
			dist=hyperplane.orientedDistance(left[0].position)
			vert=Vertex(algebra.vectSum(left[0].position, algebra.vectMult(-dist/prod, diff)))
			leftEdge=Figure([left[0], vert])
			rightEdge=Figure([right[0], vert])
			return cache([leftEdge], [vert], [rightEdge])

		else:                      # Cutting face (at least 2-dimensional)
			middle.sort(key=attrgetter('dim'), reverse=True)
			for f in figuresIterator(middle):
				f.used=False
			section=[]
			middle2=set()
			for f in middle:
				if not f.used:
					if f.dim == figure.dim-2:
						middle2.add(f)
					else:
						section.append(f)
					for f2 in f:
						f2.used=True
			for f in figuresIterator(middle):
				del f.used

			if True: # General case

				def findComponents(figures, exclude):
					for f in figures:
						f.group=None
						f.groupIndex=None
						for f2 in f.boundary:
							f2.group=None
					def union(parent, child):
						if child == parent:
							return
						if child.group:
							union(parent, child.group)
						child.group=parent
						return
					def find(f):
						while f.group:
							f=f.group
						return f
					for f in figures:
						for f2 in f.boundary:
							if not f2 in exclude:
								union(f, f2)
					groups=[]
					for f in figures:
						g=find(f)
						if g.groupIndex == None:
							g.groupIndex=len(groups)
							groups.append([])
						groups[g.groupIndex].append(f)
					for f in figures:
						del f.group
						del f.groupIndex
						for f2 in f.boundary:
							f2.group=None
							del f2.group
					return groups

				leftComp=findComponents(left, middle2)
				rightComp=findComponents(right, middle2)
				leftComp=[Figure(c) for c in leftComp]
				rightComp=[Figure(c) for c in rightComp]

				sectComp=[]
				for f in leftComp:
					comp=[set()]
					for f2 in f.boundary:
						comp[0] |= f2.boundary & middle2
					sectComp.append(comp)
					f.sect=comp
				# sectComp=[ [set(vertices)] ]
				for f in rightComp:
					f.sect=[]
				for c in sectComp:
					m=c.pop()
					for f in rightComp:
						comp=[set()]
						for f2 in f.boundary:
							comp[0] |= f2.boundary & m
						c.append(comp)
						f.sect.append(comp)
				# sectComp=[ [[set(vertices)]] ]
				for l in sectComp:
					for m in l:
						m.append(Figure(m.pop()))
						section.append(m[0])
				for f in leftComp+rightComp:
					for f2 in f.sect:
						f.addToBoundary(f2[0])
					del f.sect

			else: # Convex only

				newSect=Figure(middle2)
				leftComp=[Figure(left + [newSect])]
				rightComp=[Figure(right + [newSect])]
				section.append(newSect)

			return cache(leftComp, section, rightComp)


# Cuts off specified vertices (or all) of a figure,
# ratio = d(vertex, hyperplane) : d(vertex, origin)
def cutOffVertices(figure, ratio, vertices=None):
	if not vertices:
		vertices=[]
		for f in figure:
			if f.dim>0: continue
			vertices.append(f)
	hyperplanes=[Hyperplane(v.position, 1-ratio) for v in vertices]
	figures=[figure]
	for h in hyperplanes:
		figures2=[]
		for f in figures:
			figures2.extend(cutFigure(f, h)[0])
		figures=figures2
	return figures


import itertools
def commandCutVertices(ratio):
	figures=objFigure.fromGfFigure(gf.figureGet())
	figures2=[]
	for f in figures:
		figures2.extend(cutOffVertices(f, ratio))
	gf.figureOpen(objFigure.toGfFigure(figures2), True)

def commandCut():
	gf.echo("""--- cuts module ---
Commands:
  cut vertices <ratio>  -cuts off vertices, <ratio> = d(vertex, hyperplane) : d(vertex, origin)
For more information see cuts.py""")
	gf.clearAfterCmd()

import gf
gf.addCommand("cut vertices ", "cuts.commandCutVertices(%)", 1, '-')
gf.addCommand("cut", "cuts.commandCut()")
