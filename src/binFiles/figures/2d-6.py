import gf
gf.figureOpen([
	[
		(0.5, -0.8660253882408142),
		(1.0, 0),
		(0.5, 0.8660253882408142),
		(-0.5, 0.8660253882408142),
		(-1.0, 0),
		(-0.5, -0.8660253882408142)
	], [[4, 5], [5, 0], [0, 1], [1, 2], [2, 3], [3, 4]], [[0, 1, 2, 3, 4, 5]]])

try:
	import figureInfo
	figureInfo.setNameDescPath('Regular Hexagon', None, __file__)
except ImportError: pass
