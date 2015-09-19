# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This script allows random rotation of figures.

module_help="""
Module randomRot allows random rotation of figures.

Commands:
  randomrot                -starts random rotation
  randomrot map [<key>]    -maps/unmaps random rotation to key
  randomrot [no]auto       -enables/disables automatic
                            random rotation after opening figure

Python interface:
  map(key)                      -maps/unmaps random rotation to key
  set_auto(bool_value)          -enables/disables automatic rotation
  randomRot(allowCameraMoving)  -rotation itself

uses modules: [helpMod]

For more information see randomRot.py
"""

import gf
import random
random.seed()

try:
	import helpMod
	helpMod.addModule("randomrot", module_help)
	helpMod.addPage("randomrot", module_help)
except ImportError:
	pass

mappedTo=None
def map(key):
	global mappedTo
	if mappedTo:
		gf.map(mappedTo)
	if key:
		gf.map(key, "randomRot.randomRot()");
	mappedTo=key

def schedule(path):
	def idle():
		gf.unregisterCallback("idle", idle)
		randomRot(False, False)
	gf.registerCallback("idle", idle)

auto=False
def set_auto(value):
	global auto
	if value:
		gf.registerCallback("open", schedule)
	else:
		gf.unregisterCallback("open", schedule)
	auto=value
def get_auto():
	return auto

set_auto(True)

gf.addCommand("randomrot auto", "randomRot.set_auto(True)")
gf.addCommand("randomrot noauto", "randomRot.set_auto(False)")
gf.addCommand("randomrot map", "randomRot.map(None)")
gf.addCommand("randomrot map ", "randomRot.map(%)", 1, "s")
gf.addCommand("randomrot", "randomRot.randomRot()")



# Randomly rotates opened figure in all existing axes, optionally moves with camera (resp. modifies perspective projections)
def randomRot(allowCameraMoving=False, printInfo=True):
	global mappedTo
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
	minDelay=1000/gf.get_maxfps()
	while gf.sleep(minDelay):  # Repeat till user abort; repaint and wait at least 1ms before every course
		t=gf.time() # Time measuring to deal with different sleeping time
		if printInfo:
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
				if printInfo:
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
				if printInfo:
					gf.echo("  camposl{}={:6.2f}" . format(i, camPosL[i]))
				gf.set_camposl(i, camPosL[i])

		lastTime=t
		if printInfo:
			if auto:
				gf.echo("Automatic rotation enabled")
			else:
				gf.echo("Automatic rotation disabled")
		if mappedTo:
			if printInfo:
				gf.clearAfterCmd("Press TAB to toggle camera moving or any other key to stop moving") # Remove only this line when user types command
			if allowCameraMoving:
				gf.map(mappedTo, "randomRot.randomRot(False)") # Tab always rotates without camera moving (default)
			else:
				gf.map(mappedTo, "randomRot.randomRot(gf.time()<" + str(gf.time()+100)+")") # Pressing tab in 100ms will rotate with camera moving
				                                                                            # If tab was used to abort rotation, the same press will resume it
		else:
			if printInfo:
				gf.clearAfterCmd("Press any key to stop moving")
