import gf
gf.figureOpen([[(-1.0,), (1.0,)], [[0, 1]]])

try:
	import figureInfo
	figureInfo.setNameDescPath('Abscissa', '1-simplex.\n1-cube.\n1-orthoplex.', __file__)
except ImportError: pass