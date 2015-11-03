import gf
gf.figureOpen([[(0.25, 0.322748601436615, 0.4564354717731476, -0.7905694246292114), (0.25, 0.322748601436615, 0.4564354717731476, 0.7905694246292114), (0.25, 0.322748601436615, -0.9128709435462952, 0.0), (0.25, -0.9682458639144897, 0.0, 0.0), (-1.0, 0.0, 0.0, 0.0)], [[0, 1], [0, 2], [1, 2], [0, 3], [1, 3], [2, 3], [0, 4], [1, 4], [2, 4], [3, 4]], [[0, 2, 1], [0, 4, 3], [1, 5, 3], [2, 5, 4], [0, 7, 6], [1, 8, 6], [2, 8, 7], [3, 9, 6], [4, 9, 7], [5, 9, 8]], [[0, 1, 2, 3], [0, 4, 5, 6], [1, 4, 7, 8], [2, 5, 7, 9], [3, 6, 8, 9]], [[0, 1, 2, 3, 4]]])

try:
	import figureInfo
	figureInfo.setNameDescPath('Regular Pentachoron', '4-dimensional tetrahedron (4-simplex).\nOne of the six regular polychorons (4-dimensional analogy of Platonic solids).\nIts cells are tetrahedral.', __file__)
except ImportError: pass