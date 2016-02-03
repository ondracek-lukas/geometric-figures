import gf
gf.figureOpen([[(-1.0, 0.0, 0.0), (1.0, 0.0, 0.0), (0.0, -1.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, -1.0), (0.0, 0.0, 1.0)], [[0, 2], [0, 4], [2, 4], [1, 2], [1, 4], [0, 5], [2, 5], [1, 5], [0, 3], [4, 3], [5, 3], [1, 3]], [[0, 2, 1], [3, 2, 4], [0, 6, 5], [3, 6, 7], [1, 9, 8], [5, 10, 8], [7, 10, 11], [4, 9, 11]], [[0, 1, 2, 3, 4, 5, 6, 7]]])

try:
	import figureInfo
	figureInfo.setNameDescPath('Regular Octahedron', 'One of the five Platonic Solids (regular polyhedra).\nIts faces are triangular.', __file__)
except ImportError: pass
