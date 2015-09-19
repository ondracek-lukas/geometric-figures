# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

# This module adds 3Dconnexion SpaceNavigator support
# to (Linux version of) Geometric Figures application.

# To allow access to the device, use the following udev rule
# (e.g. pasting the line in /etc/udev/rules.d/90-spacenavigator.rules):
#   KERNEL=="event[0-9]*", ATTRS{idVendor}=="046d", ATTRS{idProduct}=="c62[68]", MODE="0664", GROUP="plugdev", SYMLINK+="input/spacenavigator"
# It will create /dev/input/spacenavigator symlink to the device allowing user access

# The script can be modified for other input devices

module_help="""
Module spaceNavigator adds 3d-mouse support.

Commands:
  spacenavigator                      -prints some information
  spacenavigator sensitivity=<value>  -gets/sets device sensitivity
  spacenavigator device=<path>        -gets/sets device path
  spacenavigator on                   -activates device
  spacenavigator off                  -deactivates device

Python interface:
  set_device(path)   -sets device path
  get_device()       -gets device path
  set_sens(value)    -sets device sensitivity
  get_sens()         -gets device sensitivity
  on()               -activates device
  off()              -deactivates device
  isActive()         -returns whether the device is active
  info()             -prints some information
  leftBtnFunc        -function called on left button press
  rightBtnFunc       -function called on right button press

uses modules: [gfUtils], [helpMod], [randomRot] (deactivates auto)

For more information see spaceNavigator.py
"""

import gf
import os
import struct
import threading
import time # gf.time can be used only in the main thread

try:
	import helpMod
	helpMod.addModule("spacenavigator", module_help)
	helpMod.addPage("spacenavigator", module_help)
except ImportError:
	pass

device="/dev/input/spacenavigator"
lock=threading.Lock()
buttons=[]
sensitivity=0.01
idleTime=0
axes=None
try:
	import gfUtils
	def leftBtnFunc():
		gfUtils.openFileRelative(-1)
	def rightBtnFunc():
		gfUtils.openFileRelative(1)
except ImportError:
	leftBtnFunc=None
	rightBtnFunc=None

def idle():
	global axes
	global buttons
	global lock
	global idleTime
	if not axes:
		if not os.access(device, os.R_OK):
			off()
			raise IOError("Device " + device + " cannot be opened")
		axes=[0]*6
		def handler():
			global axes
			global device
			global buttons
			global lock
			eventFormat="llHHi"
			eventSize=struct.calcsize(eventFormat)
			openTime=time.time()
			skipping=True
			try:
				dev=open(device, "rb")
				event=dev.read(eventSize)
			except IOError as ex:
				with lock:
					axes=None
					raise ex
			while event:
				if skipping: # events that happened before opening the device
					if time.time()-openTime > 0.1:
						skipping=False
					event=dev.read(eventSize)
					continue
				(tv_sec, tv_usec, type, code, value)=struct.unpack(eventFormat, event)
				if type==2:
					with lock:
						axes[code]+=value
				elif type==1 and value==1:
					buttons.append(code)
				try:
					event=dev.read(eventSize)
				except IOError as ex:
					with lock:
						axes=None
						raise ex
		t=threading.Thread(target=handler, args=())
		t.daemon=True
		t.start()

	currTime=gf.time()
	if currTime-idleTime>100:
		with lock:
			axes=[0]*6
		del buttons[:]
	else:
		dim=gf.get_dimen()
		with lock:
			if not axes:
				return;
			(right, near, down, tiltNear, tiltLeft, rotRight)=axes
			axes=[0]*6
		if dim>=2:
			if tiltLeft: # ifs needed not to force repainting
				gf.rotate(1, 2, sensitivity*tiltLeft)
		if dim>=3:
			if tiltNear:
				gf.rotate(2, 3, sensitivity*tiltNear)
			if rotRight:
				gf.rotate(1, 3, sensitivity*rotRight)
		if dim>=4:
			if right:
				gf.rotate(4, 1, sensitivity*right)
			if near:
				gf.rotate(4, 3, sensitivity*near)
			if down:
				gf.rotate(2, 4, sensitivity*down)

		while len(buttons):
			gf.clear()
			if buttons.pop()==256:
				if leftBtnFunc:
					leftBtnFunc()
			else:
				if rightBtnFunc:
					rightBtnFunc()
	idleTime=currTime

def set_device(device):
	if 'axes' in globals():
		raise RuntimeError("Background thread already running, cannot change device")
	device=device;
def get_device():
	return device

def set_sens(sens):
	global sensitivity
	sensitivity=float(sens)
def get_sens():
	return sensitivity

active=False
def on():
	global active
	gf.registerCallback("idle", idle);
	active=True
def off():
	global active
	gf.unregisterCallback("idle", idle);
	active=False
def is_active():
	return active

def info():
	gf.echo("""--- SpaceNavigator ---
  device=""" + get_device() + """
  sensitivity=""" + str(get_sens()))
	if active:
		gf.echo("The device is active")
	else:
		gf.echo("The device is inactive")
	gf.clearAfterCmd()

gf.removeCommands("spacenavigator")
gf.addCommand("spacenavigator", "spaceNavigator.info()")
gf.addCommand("spacenavigator on", "spaceNavigator.on()")
gf.addCommand("spacenavigator off", "spaceNavigator.off()")
gf.addCommand("spacenavigator device=", "spaceNavigator.set_device(%)", 1, "p")
gf.addCommand("spacenavigator device", "'  device=' + spaceNavigator.get_device()")
gf.addCommand("spacenavigator sensitivity", "'  sensitivity=' + str(spaceNavigator.get_sens())")
gf.addCommand("spacenavigator sensitivity=", "spaceNavigator.set_sens(%)", 1, "-")

if os.access(device, os.R_OK):
	on()
	try: # deactivate randomRot module, if exists
		import randomRot
		randomRot.set_auto(False)
	except ImportError:
		pass
	gf.echo("Space navigator module loaded, device found")
