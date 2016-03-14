name="Cube"
description="""
One of the five Platonic solids (regular polyhedra).
Its faces are squared.
"""

import math
c=math.sqrt(1.0/3)
vertices=[]
for x in (-c, c):
	for y in (-c, c):
		for z in (-c, c):
			vertices.append((x,y,z))

import gfUtils
gfUtils.openConvexFromVertList(vertices, name, description, __file__)

	#del name
	#del description
	#del vertices
	#del c
