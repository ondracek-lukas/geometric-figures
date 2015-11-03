import gf
import math

gf.figureOpen([
	[
		(math.sqrt(3)/2, -0.5),
		(0, 1.0),
		(-math.sqrt(3)/2, -0.5)
	], [[2, 0], [0, 1], [1, 2]], [[0, 1, 2]]])

try:
	import figureInfo
	figureInfo.setNameDescPath('Regular Triangle', '2-simplex.', __file__)
except ImportError: pass
