# 5D hypercupe to be sourced from app

name="Penteract"
description="""
5-dimensional cube (5-cube).
One of the three regular 5-polytopes (5-dimensional analogy of Platonic solids).
Its facets are tesseracts.
"""

vertices=[]
for x in (-1,1):
	for y in (-1,1):
		for z in (-1,1):
			for a in (-1,1):
				for b in (-1,1):
					vertices.append((x,y,z,a,b))

import gfUtils
gfUtils.openConvexFromVertList(vertices, name, description, __file__)

del name
del description
del vertices
