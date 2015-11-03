import gf
gf.figureOpen([
	[
		(0.5877852439880371, -0.80901700258255),
		(0.9510565400123596, 0.30901700258255005),
		(0, 1.0),
		(-0.9510565400123596, 0.30901700258255005),
		(-0.5877852439880371, -0.80901700258255)
	], [[3, 4], [4, 0], [0, 1], [1, 2], [2, 3]], [[0, 1, 2, 3, 4]]])

try:
	import figureInfo
	figureInfo.setNameDescPath('Regular Pentagon', None, __file__)
except ImportError: pass
