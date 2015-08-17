#!/usr/bin/gawk -f 

# Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

# This script generates stringsData.c.tmp from stringsData folder (resp. from given files),
# all string resources will be then compiled into executable app

function warn(str){
	print "Warn: " str " in " FILENAME > "/dev/stderr"
}
function error(str){
	print "Error: " str " in " FILENAME > "/dev/stderr"
	exit(3);
}

BEGIN{
	print "// Generated by stringsData.awk"
	print "// vim: filetype=c"
	print
	contentCnt=0
}

BEGINFILE{
	FS=";"
}


(FS==";"){
	gsub(/\..*$/,"",FILENAME)
	printf "const char *stringsData_%s[]={", FILENAME
	for(i=1; i<=NF; i++) {
		printf "\"%s\",", $i
		content[contentCnt++]=$i
	}
	printf "\n\t\""
	FS=""
	files[FILENAME]=NF
	next
}

{
	gsub("\"", "\\\"")
	$0=gensub("@([[:upper:]]+)@","\" STRINGS_DATA_\\1 \"", "g")
	spaces=0
	for(i=1; i<=NF; i++)
		if($i==" ")
			spaces++;
		else{
			if(spaces){
				if(spaces==1)
					printf " "
				else
					printf "\\t\\%03o", spaces
				spaces=0
			}
			switch($i){
				case "\t":
					error("tab character found")
				default:
					printf "%s", $i
					break
			}
		}
	printf "\\n"
}

ENDFILE{
	if(FS==";")
		warn("nothing");
	else
		printf "\"};\n"
}

END{
	print "const char *stringsDataGet(char *section, char *name){"
	for (file in files)
		print "\tif(stringsEq(section,name,stringsData_" file "," files[file] ")) return stringsData_" file "[" files[file] "];"
	print "\treturn 0;"
	print "}"
	for (c in content) {
		split(content[c], arr, ":")
		if (content2[arr[1]])
			content2[arr[1]]=content2[arr[1]] ","
		content2[arr[1]]=content2[arr[1]] "\"" arr[2] "\""
	}
	for (section in content2) {
		print "const char * const stringsDataContent_"section"[] = {" content2[section] ", 0};"
	}
	print "const char * const*stringsDataGetContent(char *section) {"
	for (section in content2) {
		print "\tif (strcmp(section,\"" section "\")==0) {"
		print "\t\treturn stringsDataContent_"section";"
		print "\t}"
	}
	print "\treturn 0;"
	print "}"
}

# vim: filetype=awk
