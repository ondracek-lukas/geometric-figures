# Configuration readme

gf.map("?", "gf.echo(config_readme) or gf.clearAfterCmd()")
gf.echo("To see default configuration press ?")
config_readme="--- Default configuration ---"

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
config_readme+="""
Named colors: black, white, red, green, blue, yellow, cyan, purple, gray, transparent"""

# Functions:

def dim_ge(dim):
	return gf.get_dimen() >= dim

def colorToTuple(color):
	color=gf.normalizeColorAlpha(color)
	return tuple(ord(c) for c in color[1:].decode("hex"))
def tupleToColor(tupl):
	return "#{:02X}{:02X}{:02X}{:02X}".format(*tupl)

import random
random.seed()

# Randomly rotates opened figure in all existing axes, optionally moves with camera (resp. modifies perspective projections)
def randomRot(allowCameraMoving):
	maxspeed=gf.get_speed()
	dim=gf.get_dimen()

	# Create two dimensional list of angular velocities (one for every pair of axes)
	velocities=[0]
	for i in range(1, dim+1):
		velocities=velocities+[[0]*i]

	if allowCameraMoving:
		# Create list of camera distances (in different levels of persp. proj.) and list of their current speeds
		camPosL=[0]*(dim+1)
		for i in range(3, dim+1):
			camPosL[i]=gf.get_camposl(i)
		camPosLAcc=[0]*(dim+1)

	lastTime=gf.time()
	while gf.sleep(1):  # Repeat till user abort; repaint and wait at least 1ms before every course
		t=gf.time() # Time measuring to deal with different sleeping time
		gf.clear(); # echo prints every line below the previous one, clearing is necessary
		gf.echo("--- Random rotation ---")

		# Rotate figure and write velocities to console
		for i in range(1, dim+1):
			for j in range(1, i):
				velocities[i][j]=velocities[i][j]+(random.random()-0.5)*maxspeed/20
				if velocities[i][j]>maxspeed/2:
					velocities[i][j]=maxspeed/2
				if velocities[i][j]<-maxspeed/2:
					velocities[i][j]=-maxspeed/2
				gf.echo("  {}{}: {:6.2f} deg/sec" . format(j, i, -velocities[i][j]));
				gf.rotate(i, j, velocities[i][j]*(t-lastTime)/1000)

		if allowCameraMoving:
			# Change camera distances and write new values to console
			for i in range(3, dim+1):
				camPosLAcc[i]=camPosLAcc[i]+(random.random()-0.5)/64
				if camPosLAcc[i]<0:
					if camPosLAcc[i]<-0.01:
						camPosLAcc[i]=-0.01
					camPosL[i]=camPosL[i]+(camPosL[i]-0.2)*camPosLAcc[i]
				if camPosLAcc[i]>0:
					if camPosLAcc[i]>0.01:
						camPosLAcc[i]=0.01
					camPosL[i]=camPosL[i]+(10-camPosL[i])*camPosLAcc[i]
				gf.echo("  camposl{}={:6.2f}" . format(i, camPosL[i]))
				gf.set_camposl(i, camPosL[i])

		gf.clearAfterCmd("Press TAB to toggle camera moving or any other key to stop moving"); # Remove only this line when user types command
		lastTime=t
	if allowCameraMoving:
		gf.map("<tab>", "randomRot(False)"); # Tab always rotates without camera moving (default)
	else:
		gf.map("<tab>", "randomRot(gf.time()<" + str(gf.time()+50)+")"); # Pressing tab in 50ms will rotate with camera moving
		                                                                 # If tab was used to abort rotation, the same press will resume it


# Figures:

import os;
lastLoaded="3d"
def loadRelative(offset):
	files=os.listdir(gf.expandPath("%/data"))
	files.sort()
	try:
		i=files.index(lastLoaded)
	except:
		i=-1
	i= (i+offset) % len(files)
	loadAndRot(files[i])

def loadAndRot(name):
	global lastLoaded
	gf.set_convexhull(False)
	if name.endswith(".py"):
		gf.source("%/data/" + name)
	else:
		gf.open("%/data/" + name);
	lastLoaded=name
	if dim_ge(3):
		gf.rotate(1 ,3, 30)
		gf.rotate(2 ,3, 20)
	defaultColors() # defined below
	gf.echo("File " + name + " loaded...")
	if gf.sleep(2000): # Wait 2s for user interrupt
		randomRot(False)

 #0D:
gf.map("~", "loadAndRot('0d.dat')")
 #1D:
gf.map("`", "loadAndRot('1d-2.dat')")
 #2D:
gf.map("1", "loadAndRot('2d-3.dat')")
 #3D:
gf.map("2", "loadAndRot('3d-04.dat')")
gf.map("3", "loadAndRot('3d-06.dat')")
gf.map("4", "loadAndRot('3d-08.dat')")
gf.map("5", "loadAndRot('3d-12.dat')")
gf.map("6", "loadAndRot('3d-20.dat')")
 #4D:
gf.map("7", "loadAndRot('4d-005.dat')")
gf.map("8", "loadAndRot('4d-008.dat')")
gf.map("9", "loadAndRot('4d-016.dat')")
gf.map("0", "loadAndRot('4d-024.dat')")
gf.map("-", "loadAndRot('4d-120.dat')")
gf.map("=", "loadAndRot('4d-600.dat')")
 #5D:
gf.map("_", "loadAndRot('5d-010.py')")
gf.map("+", "loadAndRot('5d-032.dat')")
 #Relative:
gf.map("<mouse3>", "loadRelative(-1)")
gf.map("<mouse4>", "loadRelative(1)")


config_readme+="""
Figures:
  0D: ~ point
  1D: ` abscissa
  2D: 1 triangle,      square,      pentagon,     hexagon
  3D: 2 tetrahedron, 3 cube,      4 octahedron,            5 dodecahedron, 6 icosahedron
  4D: 7 pentachoron, 8 tesseract, 9 16-cell,    0 24-cell, - 120-cell,     = 600 cell
  5D:                _ penteract, + 32-cell
  Next/previous: mouse wheel"""


# Rotation:

 #Random:
gf.map("<tab>", "randomRot(False)");

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

config_readme+="""
Rotations: (use shift to rotate by 15 degrees)
  2D: q-w 12(xy)
  3D: a-s 13(xz), z-x 23(yz), holding left mouse button
  4D: d-f 14(x4), c-v 24(y4), e-r 34(z4), holding right mouse button
Random rotation: TAB"""

# Perspective setting:
def advanceCamposl(axis, by):
	if not dim_ge(axis):
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

config_readme+="""
Perspective projection: holding middle mouse button - x camposl3, y camposl4"""

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

config_readme+="""
Modifications:
  Remove/add vertex:    P-N
  Select previous/next: p-n
  Deselect:             o/Esc
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
  Convex hull generation: y start, t stop"""


# Predefined color modes

def resetSpaceColor():
	if dim_ge(0):
		gf.set_spacecolor( 0, "white");
	for dim in range(1, gf.get_dimen()+1):
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
		gf.set_facecolor(tupleToColor(tuple([int(255*a), int(128-b*128), int(128-b*64), int(127+b*128)])));

gf.map("<f1>", "reset colors")

gf.map("<f2>", "defaultColors()")  # 3D axis colored
gf.map("<f3>", "spacecolor4D()")   # 4D axis colored
gf.map("<f4>", "spacecolor3D4D()") # 3D and 4D axes colored

gf.map("<f5>", "set facecolor=" + tupleToColor((int(255*0.2), 128, 128, 128)))
gf.map("<f6>", "set facecolor= " + tupleToColor((int(255*0.03), 255, 255, 255)))
gf.map("<f7>", "set facecolor=transparent")

config_readme+="""
Colors:
  Restore defaults: F1 app colors, F2 config colors
  Colored axes:     F3 4,          F4 both 3(z) and 4
  Face color:       F5 20% gray,   F6 3% white, F7 transparent"""


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

# Default settings

gf.set_speed(60);
gf.set_vertsize(10);


# End of configuration readme

config_readme+="""

For further help see configuration file,
for general help type :help."""
