# Geometric Figures  Copyright (C) 2015  Lukáš Ondráček <ondracek.lukas@gmail.com>, see README file

.PHONY: compile package clean clean-tmp \
	compile-all package-all clean-all clean-tmp-all

LDFLAGS    = -lm
CFLAGS     = -std=gnu99 -Wimplicit-function-declaration
version    = $(shell cat VERSION)

ifndef debug
ifeq ($(findstring dev, $(version)), dev)
	debug=1
else
	debug=0
endif
endif
ifeq ($(debug), 1)
	CFLAGS  += -g -Wall -DCOREDUMP
endif

ifeq ($(arch), win32) # Windows
	CC       = i686-w64-mingw32-gcc
	CFLAGS  += -mwindows -DGLUT_DISABLE_ATEXIT_HACK -mthreads \
	           -I/usr/i686-w64-mingw32/include/python27 \
	           #-mconsole # uncomment to use console in windows (to see python scripts errors)
	LDFLAGS += -lfreeglut -lglu32 -lopengl32 -lpython27
	name     = geometric_figures.exe
	suff     = -win32
	pkg      = zip
	txtext   = .txt
	version += for Windows
else                  # Linux (native/32/64)
	LDFLAGS += -lglut -lGLU -lGL -lpython2.7
	CFLAGS  += -I/usr/include/python2.7
	name     = geometric_figures
	pkg      = tar.gz
endif
ifeq ($(arch), 32)    # Linux 32-bit
	CFLAGS  += -m32
	suff     = 32
	version += for Linux 32-bit
endif
ifeq ($(arch), 64)    # Linux 64-bit
	CFLAGS  += -m64
	suff     = 64
	version += for Linux 64-bit
endif
CFLAGS    += -D 'STRINGS_DATA_VERSION="$(version)"'

ifeq ($(debug), 1)
	binFiles=$(shell find src/binFiles -type f | sed -r 's=^src/binFiles/=bin$(suff)/=; s=/[A-Z]+$$=&$(txtext)=')
else
	binFiles=$(shell find src/binFiles -type f \! -name 'debug*' | sed -r 's=^src/binFiles/=bin$(suff)/=; s=/[A-Z]+$$=&$(txtext)=')
endif


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
obj$(suff)/%.o: src/%.c VERSION
	@mkdir -p obj$(suff)/
	$(CC) -c -o $@ $< $(CFLAGS)

bin$(suff)/$(name): $(shell ls src/*.c | sed 's=src/\(.*\)\.c=obj$(suff)/\1.o=')
	@# Link geometric figures (freeglut3-dev needed)
	@mkdir -p bin$(suff)/
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

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


src/stringsData.c.tmp: src/stringsData.awk src/stringsData src/stringsData/* VERSION
	@# Generate stringsData file
	cd src/stringsData/ && ../stringsData.awk -v version="$(version)" * > ../stringsData.c.tmp
src/scriptWrappers.h.tmp src/scriptWrappers.c.tmp: src/scriptWrappers.pl src/*.h
	@# Generate scriptWrappers
	cd src && ./scriptWrappers.pl *.h

