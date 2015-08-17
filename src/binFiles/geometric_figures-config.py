# Configuration readme

gf.map("?", "gf.echo(config_readme) or gf.consoleClearAfterCmd()")
config_readme="--- Default configuration ---"
gf.echo("To see default configuration press ?")


# Figures:

def dim_ge(dim):
	return gf.get_dimen() >= dim

def openAndRot(name):
	gf.open("%/data/" + name + ".dat");
	gf.rotate(1 ,3, 30);
	gf.rotate(2 ,3, 20);
def sourceAndRot(name):
	gf.source("%/data/" + name + ".txt");
	gf.rotate(1 ,3, 30);
	gf.rotate(2 ,3, 20);

gf.map("<Esc>", "close")
 #0D:
gf.map("~", "o %/data/0d.dat")
 #1D:
gf.map("`", "o %/data/1d-2.dat")
 #2D:
gf.map("1", "o %/data/2d-3.dat")
 #3D:
gf.map("2", "openAndRot('3d-4')")
gf.map("3", "openAndRot('3d-6')")
gf.map("4", "openAndRot('3d-8')")
gf.map("5", "openAndRot('3d-12')")
gf.map("6", "openAndRot('3d-20')")
 #4D:
gf.map("7", "openAndRot('4d-5')")
gf.map("8", "openAndRot('4d-8')")
gf.map("9", "openAndRot('4d-16')")
gf.map("0", "openAndRot('4d-24')")
gf.map("-", "openAndRot('4d-120')")
gf.map("=", "openAndRot('4d-600')")
 #5D:
gf.map("+", "sourceAndRot('5d-10')")

config_readme+="""
Figures:
  0D: ~ point
  1D: ` abscissa
  2D: 1 triangle
  3D: 2 tetrahedron, 3 cube,      4 octahedron,            5 dodecahedron, 6 icosahedron
  4D: 7 pentachoron, 8 tesseract, 9 16-cell,    0 24-cell, - 120-cell,     = 600 cell
  5D:                + penteract (changes color settings, sets convexhull)"""


# Rotation:

 #2D:
gf.rmap("q", 1, 2)
gf.map("Q", "rot 1 2 15")
gf.rmap("w", 2, 1)
gf.map("W", "rot 2 1 15")

 #3D:
gf.rmap("a", 1, 3)
gf.map("A", "rot 1 3 15")
gf.rmap("s", 3, 1)
gf.map("S", "rot 3 1 15")
gf.rmap("z", 2, 3)
gf.map("Z", "rot 2 3 15")
gf.rmap("x", 3, 2)
gf.map("X", "rot 3 2 15")

 #4D:
gf.rmap("e", 3, 4)
gf.map("E", "rot 3 4 15")
gf.rmap("r", 4, 3)
gf.map("R", "rot 4 3 15")
gf.rmap("d", 1, 4)
gf.map("D", "rot 1 4 15")
gf.rmap("f", 4, 1)
gf.map("F", "rot 4 1 15")
gf.rmap("c", 2, 4)
gf.map("C", "rot 2 4 15")
gf.rmap("v", 4, 2)
gf.map("V", "rot 4 2 15")

config_readme+="""
Rotations: (use shift to rotate by 15 degrees)
  2D: q-w 12(xy)
  3D: a-s 13(xz), z-x 23(yz)
  4D: d-f 14(x4), c-v 24(y4), e-r 34(z4)"""


# Modification:

gf.map("p", "vertex previous")
gf.map("n", "vertex next")
gf.map("o", "vertex deselect")
gf.map("P", "vertex remove")
gf.map("N", "vertex add")
gf.map("h", "vertex move -0.2")
gf.map("l", "vertex move 0.2")
gf.map("j", "vertex move 0 -0.2")
gf.map("k", "vertex move 0 0.2")
gf.map("u", "vertex move 0 0 -0.2")
gf.map("i", "vertex move 0 0 0.2")
gf.map("m", "vertex move 0 0 0 -0.2")
gf.map(",", "vertex move 0 0 0 0.2")

config_readme+="""
Modifications:
  Remove/add vertex:    P-N
  Select previous/next: p-n
  Deselect:             o
  Move in an axis:      h-l 1(x), j-k 2(y), u-i 3(z), m-, 4"""


# Four small letters remain:

gf.map("b", "reset boundary")
gf.map("g", "reset rotation")
gf.map("t", "set noconvexhull")
gf.map("y", "set convexhull")

config_readme+="""
Settings:
  Reset boundary:         b
  Reset rotation:         g
  Convex hull generation: y begin, t stop"""


# Predefined color modes

def resetSpaceColor():
	if dim_ge(0):
		gf.set_spacecolor( 0, "gray");
	for dim in range(1, gf.get_dimen()+1):
		gf.set_spacecolor( dim, "transparent");
		gf.set_spacecolor(-dim, "transparent");
def setSpaceColor3D():
	if dim_ge(3):
		gf.set_spacecolor(-3, "blue")
		gf.set_spacecolor( 3, "yellow")
def setSpaceColor4D():
	if dim_ge(4):
		gf.set_spacecolor(-4, "red")
		gf.set_spacecolor( 4, "green")

def spacecolor3D():
	resetSpaceColor()
	setSpaceColor3D()
def spacecolor4D():
	resetSpaceColor()
	setSpaceColor4D()
def spacecolor3D4D():
	resetSpaceColor()
	setSpaceColor3D()
	setSpaceColor4D()

gf.map("<f1>", "reset colors")

gf.map("<f2>", "spacecolor3D()")   # 3D axis colored
gf.map("<f3>", "spacecolor4D()")   # 4D axis colored
gf.map("<f4>", "spacecolor3D4D()") # 3D and 4D axes colored

gf.map("<f5>", "set facecolor=0.2 gray")
gf.map("<f6>", "set facecolor=0.03 white")
gf.map("<f7>", "set facecolor=transparent")

config_readme+="""
Colors:
  Restore defaults: F1
  Colored axes:     F2 3(z),     F3 4,        F4 both 3(z) and 4
  Face color:       F5 20% gray, F6 3% white, F7 transparent"""


# Predefined edges and vertices sizes

def setSizes(edge, vert, selvert):
	gf.set_edgesize(edge)
	gf.set_vertsize(vert)
	gf.set_selvertsize(selvert)

gf.map("<f8>",  "setSizes(20, 20, 20)")
gf.map("<f9>",  "setSizes(10, 10, 10)")
gf.map("<f10>", "setSizes( 5,  5,  5)")
gf.map("<f11>", "setSizes( 2,  2,  2)")
gf.map("<f12>", "setSizes( 1,  1,  1)")

config_readme+="""
Size of vertices and edges: (in the origin)
  F8 20px, F9 10px, F10 5px, F11 2px, F12 1px"""


# End of configuration readme

config_readme+="""

For further help see configuration file,
for general help type :help."""
