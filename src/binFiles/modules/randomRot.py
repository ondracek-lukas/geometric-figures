# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This script provides function for random rotation of figures,
# it can be sourced or imported.

import gf
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
	minDelay=1000/gf.get_maxfps()
	while gf.sleep(minDelay):  # Repeat till user abort; repaint and wait at least 1ms before every course
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
