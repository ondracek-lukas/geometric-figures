# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module adds 3Dconnexion SpaceNavigator support
# to (Linux version of) Geometric Figures application,
# it is intended to be sourced from the application
# or imported in its configuration file.

# It adds command :spacenavigator (type without parameters for help)

# To allow access to the device, use the following udev rule
# (e.g. pasting the line in /etc/udev/rules.d/90-spacenavigator.rules):
#   KERNEL=="event[0-9]*", ATTRS{idVendor}=="046d", ATTRS{idProduct}=="c62[68]", MODE="0664", GROUP="plugdev", SYMLINK+="input/spacenavigator"
# It will create /dev/input/spacenavigator symlink to the device allowing user access

# The script can be modified for other input devices

import gf
import os
import struct
import threading
import time # gf.time can be used only in the main thread

if not 'spacenavigAxes' in globals():
	spacenavigDevice="/dev/input/spacenavigator"
	spacenavigLock=threading.Lock()
	spacenavigButtons=[]
	spacenavigLeftBtnFunc=0
	spacenavigRightBtnFunc=0
	if __name__=="__main__":
		spacenavigPrefix=""
	else:
		spacenavigPrefix=__name__+"."
spacenavigSens=0.01

def spacenavigator():
	global spacenavigAxes
	global spacenavigButtons
	global spacenavigLock
	if not 'spacenavigAxes' in globals():
		if not os.access(spacenavigDevice, os.R_OK):
			raise IOError("Device " + spacenavigDevice + " cannot be opened")
		spacenavigAxes=[0]*6
		def handler():
			global spacenavigAxes
			global spacenavigDevice
			global spacenavigButtons
			global spacenavigLock
			eventFormat="llHHi"
			eventSize=struct.calcsize(eventFormat)
			dev=open(spacenavigDevice, "rb")
			openTime=time.time()
			skipping=True
			event=dev.read(eventSize)
			while event:
				if skipping: # events that happened before opening the device
					if time.time()-openTime > 0.1:
						skipping=False
					event=dev.read(eventSize)
					continue
				(tv_sec, tv_usec, type, code, value)=struct.unpack(eventFormat, event)
				if type==2:
					with spacenavigLock:
						spacenavigAxes[code]+=value
				elif type==1 and value==1:
					spacenavigButtons.append(code)
				event=dev.read(eventSize)
		t=threading.Thread(target=handler, args=())
		t.daemon=True
		t.start()

	with spacenavigLock:
		spacenavigAxes=[0]*6
	delay=1000/gf.get_maxfps()
	while gf.sleep(delay):
		dim=gf.get_dimen()
		with spacenavigLock:
			(right, near, down, tiltNear, tiltLeft, rotRight)=spacenavigAxes
			spacenavigAxes=[0]*6
		if right or near or down or tiltNear or tiltLeft or rotRight:
			gf.clear();
		if dim>=2:
			if tiltLeft:
				gf.rotate(1, 2, spacenavigSens*tiltLeft)
		if dim>=3:
			if tiltNear: # ifs needed not to force repainting
				gf.rotate(2, 3, spacenavigSens*tiltNear)
			if rotRight:
				gf.rotate(1, 3, spacenavigSens*rotRight)
		if dim>=4:
			if right:
				gf.rotate(4, 1, spacenavigSens*right)
			if near:
				gf.rotate(4, 3, spacenavigSens*near)
			if down:
				gf.rotate(2, 4, spacenavigSens*down)

		while len(spacenavigButtons):
			gf.clear()
			if spacenavigButtons.pop()==256:
				if spacenavigLeftBtnFunc:
					spacenavigLeftBtnFunc()
			else:
				if spacenavigRightBtnFunc:
					spacenavigRightBtnFunc()

def spacenavigSetDevice(device):
	if 'spacenavigAxes' in globals():
		raise RuntimeError("Background thread already running, cannot change device")
	spacenavigDevice=device;
def spacenavigGetDevice():
	return spacenavigDevice

def spacenavigSetSens(sens):
	global spacenavigSens
	spacenavigSens=float(sens)
def spacenavigGetSens():
	return spacenavigSens

def spacenavigOn():
	gf.map("<idle>", spacenavigPrefix + "spacenavigator()")
def spacenavigOff():
	gf.map("<idle>")

def spacenavigHelp():
	gf.echo("""--- SpaceNavigator ---
  device=""" + spacenavigGetDevice() + """
  sensitivity=""" + str(spacenavigGetSens()) + """
Commands:
  spacenavigator option[=value]
  spacenavigator on              -sets idle event to spacenavigator
  spacenavigator off             -clears idle event
  spacenavigator help            -prints this help
For more information see spacenavigator.py""")
	gf.clearAfterCmd()

gf.removeCommands("spacenavigator")
gf.addCommand("spacenavigator", spacenavigPrefix + "spacenavigHelp()")
gf.addCommand("spacenavigator on", spacenavigPrefix + "spacenavigOn()")
gf.addCommand("spacenavigator off", spacenavigPrefix + "spacenavigOff()")
gf.addCommand("spacenavigator device=", spacenavigPrefix + "spacenavigSetDevice(%)", 1, "p")
gf.addCommand("spacenavigator device", "'  device=' + " + spacenavigPrefix + "spacenavigGetDevice()")
gf.addCommand("spacenavigator sensitivity", "'  sensitivity=' + str(" + spacenavigPrefix + "spacenavigGetSens())")
gf.addCommand("spacenavigator sensitivity=", spacenavigPrefix + "spacenavigSetSens(%)", 1, "-")

if os.access(spacenavigDevice, os.R_OK):
	spacenavigOn()
	gf.echo("Space navigator module loaded, device found")
elif not spacenavigPrefix:
		spacenavigHelp()
