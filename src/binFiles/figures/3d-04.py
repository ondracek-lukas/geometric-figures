import gf
gf.figureOpen([[(0.8164966106414795, 0.0, -0.5773502588272095), (-0.8164966106414795, 0.0, -0.5773502588272095), (0.0, 0.8164966106414795, 0.5773502588272095), (0.0, -0.8164966106414795, 0.5773502588272095)], [[0, 1], [0, 2], [1, 2], [0, 3], [1, 3], [2, 3]], [[0, 2, 1], [0, 4, 3], [1, 5, 3], [2, 5, 4]], [[0, 1, 2, 3]]])

try:
	import figureInfo
	figureInfo.setNameDescPath('Regular Tetrahedron', '3-simplex.\nOne of the five Platonic Solids (regular polyhedra).\nIts faces are triangular.', __file__)
except ImportError: pass
