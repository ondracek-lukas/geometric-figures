import gf
gf.figureOpen([[()]])

try:
	import figureInfo
	figureInfo.setNameDescPath('Point', '0-simplex.\n0-cube.', __file__)
except ImportError: pass