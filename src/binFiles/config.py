# Color aliases:

gf.addColorAlias("black",       "#000000");
gf.addColorAlias("white",       "#ffffff");
gf.addColorAlias("red",         "#ff0000");
gf.addColorAlias("green",       "#00ff00");
gf.addColorAlias("blue",        "#0000ff");
gf.addColorAlias("yellow",      "#ffff00");
gf.addColorAlias("cyan",        "#00ffff");
gf.addColorAlias("purple",      "#ff00ff");
gf.addColorAlias("gray",        "#808080");
gf.addColorAlias("transparent", "#00000000");
helpMod.configAdd("""
Named colors: black, white, red, green, blue, yellow, cyan, purple, gray, transparent
""")

# Figures:

 #0D:
gf.map("~", "open %/figures/0d.py")
 #1D:
gf.map("`", "open %/figures/1d-2.py")
 #2D:
gf.map("1", "open %/figures/2d-3.py")
 #3D:
gf.map("2", "open %/figures/3d-04.py")
gf.map("3", "open %/figures/3d-06.py")
gf.map("4", "open %/figures/3d-08.py")
gf.map("5", "open %/figures/3d-12.py")
gf.map("6", "open %/figures/3d-20.py")
 #4D:
gf.map("7", "open %/figures/4d-005.py")
gf.map("8", "open %/figures/4d-008.py")
gf.map("9", "open %/figures/4d-016.py")
gf.map("0", "open %/figures/4d-024.py")
gf.map("-", "open %/figures/4d-120.py")
gf.map("=", "open %/figures/4d-600.py")
 #5D:
gf.map("_", "open %/figures/5d-010.py")
gf.map("+", "open %/figures/5d-032.py")
 #Relative:
gf.map("<mouse3>", "gfUtils.openFileRelative(-1)")
gf.map("<mouse4>", "gfUtils.openFileRelative(1)")

helpMod.configAdd("""
Figures:
  0D: ~ point
  1D: ` abscissa
  2D: 1 triangle,      square,      pentagon,     hexagon
  3D: 2 tetrahedron, 3 cube,      4 octahedron,            5 dodecahedron, 6 icosahedron
  4D: 7 pentachoron, 8 tesseract, 9 16-cell,    0 24-cell, - 120-cell,     = 600-cell
  5D:                _ penteract, + 5-orthoplex
  Next/previous: mouse wheel
""")

# Rotation:

 #Random:
randomRot.map("<tab>")

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
gf.rmap("<mouse0x>", 3, 1)
gf.rmap("z", 2, 3)
gf.map("Z", "rot 2 3 15")
gf.rmap("x", 3, 2)
gf.map("X", "rot 3 2 15")
gf.rmap("<mouse0y>", 3, 2);

 #4D:
gf.rmap("e", 3, 4)
gf.map("E", "rot 3 4 15")
gf.rmap("r", 4, 3)
gf.map("R", "rot 4 3 15")
gf.rmap("d", 1, 4)
gf.map("D", "rot 1 4 15")
gf.rmap("f", 4, 1)
gf.map("F", "rot 4 1 15")
gf.rmap("<mouse2x>", 4, 1)
gf.rmap("c", 2, 4)
gf.map("C", "rot 2 4 15")
gf.rmap("v", 4, 2)
gf.map("V", "rot 4 2 15")
gf.rmap("<mouse2y>", 4, 2)

helpMod.configAdd("""
Rotations: (use shift to rotate by 15 degrees)
  2D: q-w 12(xy)
  3D: a-s 13(xz), z-x 23(yz), holding left mouse button
  4D: d-f 14(x4), c-v 24(y4), e-r 34(z4), holding right mouse button
Random rotation: TAB
""")

# Perspective setting:
def advanceCamposl(axis, by):
	if not gf.get_dimen() >= axis:
		return; # Silently suppress exception
	value=gf.get_camposl(axis)+by;
	if value>10:
		value=10;
	if value<0.14:
		value=0.14
	gf.set_camposl(axis, value);
	gf.clear();
	gf.echo("--- Setting camposl ---");
	for i in range(3, gf.get_dimen()+1):
		gf.echo("  camposl" + str(i) + "=" + str(gf.get_camposl(i)))
	gf.clearAfterCmd();
gf.map("<mouse1x>", "advanceCamposl(3, %/100.0)");
gf.map("<mouse1y>", "advanceCamposl(4, %/100.0)");

helpMod.configAdd("""
Perspective projection: holding middle mouse button - x camposl3, y camposl4
""")

# Modification:

gf.map("p", "vertex previous")
gf.map("n", "vertex next")
gf.map("o", "vertex deselect")
gf.map("<esc>", "vertex deselect")
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

gf.map("<bs>", "undo")
gf.map("<c-r>", "redo")

helpMod.configAdd("""
Modifications:
  Remove/add vertex:    P-N
  Select previous/next: p-n
  Deselect:             o/Esc
  Move in an axis:      h-l 1(x), j-k 2(y), u-i 3(z), m-, 4
  Undo, redo:           Backspace, Ctrl-r
""")


# Four small letters remain:

gf.map("b", "reset boundary")
gf.map("g", "reset rotation")
gf.map("t", "set noconvexhull")
gf.map("y", "set convexhull")

helpMod.configAdd("""
Settings:
  Reset boundary:         b
  Reset rotation:         g
  Convex hull generation: y start, t stop
""")


# Predefined color modes

def resetSpaceColor():
	dim=gf.get_dimen()
	if dim >= 0:
		gf.set_spacecolor( 0, "white");
	for dim in range(1, dim+1):
		gf.set_spacecolor( dim, "transparent");
		gf.set_spacecolor(-dim, "transparent");

def spacecolor3D():
	resetSpaceColor()
	gf.set_spacecolor(-3, "blue")
	gf.set_spacecolor( 3, "yellow")
def spacecolor3D4D():
	resetSpaceColor()
	gf.set_spacecolor(-3, "blue")
	gf.set_spacecolor( 3, "yellow")
	gf.set_spacecolor( 4, "red")
	gf.set_spacecolor(-4, "green")
def spacecolor4D():
	resetSpaceColor()
	gf.set_spacecolor( 4, "red")
	gf.set_spacecolor(-4, "green")
def spacecolor5D():
	resetSpaceColor()
	gf.set_spacecolor( 4, "red")
	gf.set_spacecolor(-4, "green")
	gf.set_spacecolor( 5, "blue")
	gf.set_spacecolor(-5, "yellow")

def defaultColors():
	dim=gf.get_dimen()
	if dim >= 5:
		spacecolor5D()
	elif dim == 4:
		spacecolor4D()
	else:
		resetSpaceColor()
	if dim >= 2:
		if dim <= 3:
			a=(1,0.75)[dim-2]
		else:
			a=0.5/dim
		b=1.0/(dim-1)
		gf.set_facecolor(utils.tupleToColor(tuple([int(255*a), int(128-b*128), int(128-b*64), int(127+b*128)])));

gf.map("<f1>", "reset colors")

gf.map("<f2>", "defaultColors()")  # 3D axis colored
gf.map("<f3>", "spacecolor4D()")   # 4D axis colored
gf.map("<f4>", "spacecolor3D4D()") # 3D and 4D axes colored

gf.map("<f5>", "set facecolor=" + utils.tupleToColor((int(255*0.2), 128, 128, 128)))
gf.map("<f6>", "set facecolor= " + utils.tupleToColor((int(255*0.03), 255, 255, 255)))
gf.map("<f7>", "set facecolor=transparent")

helpMod.configAdd("""
Colors:
  Restore defaults: F1 app colors, F2 config colors
  Colored axes:     F3 4,          F4 both 3(z) and 4
  Face color:       F5 20% gray,   F6 3% white, F7 transparent
""")


# Predefined edges and vertices sizes

def setSizes(edge, vert, selvert):
	gf.set_edgesize(edge)
	gf.set_vertsize(vert)
	gf.set_selvertsize(selvert)

gf.map("<f8>",  "setSizes(10,   10,   10  )")
gf.map("<f9>",  "setSizes( 5,    5,    5  )")
gf.map("<f10>", "setSizes( 2.5,  2.5,  2.5)")
gf.map("<f11>", "setSizes( 1,    1,    1  )")
gf.map("<f12>", "setSizes( 0.5,  0.5,  0.5)")

helpMod.configAdd("""
Size of vertices and edges: (in the origin)
  F8 20px, F9 10px, F10 5px, F11 2px, F12 1px
""")

# Default settings

gf.set_speed(60);
gf.set_vertsize(5);

def onNew():
	defaultColors()
	dim=gf.get_dimen()
	if dim >= 3:
		gf.set_campos(3,3.5)
		if dim >= 4:
			gf.set_campos(4,2)
			if dim >= 5:
				for i in range(5, dim+1):
					gf.set_campos(i,3)


def onOpen(path):
	if gf.get_dimen() >= 3:
		gf.rotate(1 ,3, 30)
		gf.rotate(2 ,3, 20)

gf.registerCallback("new", onNew)
gf.registerCallback("open", onOpen)
