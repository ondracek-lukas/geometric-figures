# Geometric Figures  Copyright (C) 2015  Lukas Ondracek <ondracek.lukas@gmail.com>, see README file

module_help="""
Module helpMod provides help pages for other modules
and information about the default configuration.

To show the configuration info press ? or type :help config.
For help to imported modules type :help module <name>
(configuration info contains available names).

Python interface:
  addPage(name, content)    -creates new help page :help <name>
  addModule(name, content)  -creates new help page :help module <name>
                             and adds the module name to configuration info
  printPage(name)           -shows help page of given name

  configAdd(lines)          -adds lines to configuration info
  configPrint()             -prints the configuration info

For more information see helpMod.py
"""

import gf

help_pages={}
main_help_pages=set()

def addModule(name, content):
	addPage("module " + name, content)
	main_help_pages.add(name)

def addPage(name, content):
	global help_pages, main_help_pages
	gf.addCommand("help " + name, "helpMod.printPage('" + name + "')")
	help_pages[name]=content.strip("\n")

def printPage(name):
	help_page=help_pages[name]
	if not help_page:
		raise RuntimeException("help " + name + " not found")
	gf.clear()
	gf.printCentered(help_pages[name])


gf.map("?", "helpMod.configPrint()")
gf.addCommand("help config", "helpMod.configPrint()")
gf.echo("To see the configuration press ?")
config_readme=""

def configPrint():
	text = config_readme
	text += "\n\nFor further help see configuration file,"
	if main_help_pages:
		text += """
for help to imported modules type :help module <name>,
  <name> can be:""";
	i=0
	for s in sorted(main_help_pages):
		if i>0 and i % 6 == 0:
			text += "\n                "
		text+=" "+s+","
		i+=1
	text+="\nfor general help type :help."
	gf.printCentered(text)

def configAdd(lines):
	global config_readme
	if config_readme:
		config_readme+="\n"
	config_readme+=lines.strip("\n")


addModule("helpmod", module_help)
