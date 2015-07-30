# Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

.PHONY: compile package clean clean-tmp \
	compile-all package-all clean-all clean-tmp-all

link=-lglut -lGLU -lGL -lm
name=geometric_figures
pkg=tar.gz
version=$(shell cat VERSION)
ifeq ($(arch), 32)
	suff=32
	CFLAGS=-m32
	version=$(shell cat VERSION) for Linux 32-bit
endif
ifeq ($(arch), 64)
	suff=64
	CFLAGS=-m64
	version=$(shell cat VERSION) for Linux 64-bit
endif
ifeq ($(arch), win32)
	suff=-win32
	CC=i686-w64-mingw32-gcc
	CFLAGS=-mwindows -DGLUT_DISABLE_ATEXIT_HACK
	link=-lfreeglut -lglu32 -lopengl32 -lm
	name=geometric_figures.exe
	pkg=zip
	txtext=.txt
	version=$(shell cat VERSION) for Windows
endif
binFiles=$(shell find src/binFiles -type f | sed -r 's=^src/binFiles/=bin$(suff)/=; s=/[A-Z]+$$=&$(txtext)=')


compile: bin$(suff)/$(name) $(patsubst src/binFiles/%,bin$(suff)/%,$(binFiles))
package: pkg/geometric_figures$(suff).$(pkg)

clean: clean-tmp
	rm -rf bin$(suff)
	rm -f pkg/geometric_figures$(suff).$(pkg)
	rmdir pkg 2>/dev/null || true
clean-tmp:
	rm -rf src/*.tmp obj$(suff)

clean-all: clean-tmp-all
	rm -rf bin* pkg
clean-tmp-all:
	rm -rf src/*.tmp obj*
compile-all:
	make compile arch=32
	make compile arch=64
	make compile arch=win32
package-all:
	make package arch=32
	make package arch=64
	make package arch=win32


bin$(suff)/%: src/binFiles/%
	@mkdir -p $(@D)
	cp $< $@
ifeq ($(arch), win32)
	unix2dos $@
endif
bin$(suff)/%$(txtext): src/binFiles/%
	@mkdir -p $(@D)
	cp $< $@
ifeq ($(arch), win32)
	unix2dos $@
endif
bin$(suff)/README$(txtext): src/binFiles/README
	@mkdir -p $(@D)
	sed 's/@VERSION@/$(version)/' $< > $@
ifeq ($(arch), win32)
	unix2dos $@
endif

obj$(suff)/%.d: src/%.c
	@mkdir -p obj$(suff)/
	@echo Generating dependencies of $< to $@
	@{ { echo "obj$(suff)/$*.o:"; sed -nr 's=#include\s*"([^"]*)".*=src/\1=p' $<; } | tr "\n" " "; echo; } > $@

-include $(shell ls src/*.c | sed 's=src/\(.*\)\.c=obj$(suff)/\1.d=')
obj$(suff)/%.o: src/%.c
	@mkdir -p obj$(suff)/
	$(CC) -c -o $@ $< -std=gnu99 -Wimplicit-function-declaration $(CFLAGS) # -g -Wall

bin$(suff)/$(name): $(shell ls src/*.c | sed 's=src/\(.*\)\.c=obj$(suff)/\1.o=')
	# Link geometric figures (freeglut3-dev needed)
	@mkdir -p bin$(suff)/
	$(CC) -o $@ $^ $(link) $(CFLAGS) # -g

pkg/geometric_figures$(suff).tar.gz: compile
	@echo "Creating tar.gz package"
	@mkdir -p pkg
	@tmp=`mktemp -d`; \
	(cp -rL bin$(suff)/ $$tmp/geometric_figures && \
	cd $$tmp && \
	tar -czf geometric_figures$(suff).tar.gz geometric_figures); \
	mv $$tmp/geometric_figures$(suff).tar.gz pkg/; \
	rm -rf $$tmp

pkg/geometric_figures$(suff).zip: compile
	@echo "Creating zip package"
	@mkdir -p pkg
	@tmp=`mktemp -d`; \
	(cp -rL bin$(suff) $$tmp/geometric_figures && \
	cd $$tmp && \
	zip -r geometric_figures$(suff).zip geometric_figures); \
	mv $$tmp/geometric_figures$(suff).zip pkg/; \
	rm -rf $$tmp


src/consoleCmdSetMacrosDef.c.tmp: src/consoleCmdSetMacros.sh src/consoleCmdSet.c
	# Generate consoleCmdSet macros files
	src/consoleCmdSetMacros.sh write
src/consoleCmdSetMacrosUndef.c.tmp: src/consoleCmdSetMacros.sh src/consoleCmdSet.c
	# Generate consoleCmdSet macros files
	src/consoleCmdSetMacros.sh write
src/stringsData.c.tmp: src/stringsData.awk src/stringsData src/stringsData/*
	# Generate stringsData file
	export version
	cd src/stringsData/ && ../stringsData.awk -v version="$(version)" * > ../stringsData.c.tmp
