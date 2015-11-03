import gf
import math

gf.figureOpen([
	[
		(math.sqrt(2)/2, -math.sqrt(2)/2),
		(math.sqrt(2)/2, math.sqrt(2)/2),
		(-math.sqrt(2)/2, math.sqrt(2)/2),
		(-math.sqrt(2)/2, -math.sqrt(2)/2)
	], [[3, 0], [0, 1], [1, 2], [2, 3]], [[0, 1, 2, 3]]])

try:
	import figureInfo
	figureInfo.setNameDescPath('Square', '2-cube.\n2-orthoplex.', __file__)
except ImportError: pass
