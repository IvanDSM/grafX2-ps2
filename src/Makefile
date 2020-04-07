#  Grafx2 - The Ultimate 256-color bitmap paint program
#  
#  Copyright 2018 Thomas Bernard
#  Copyright 2012 Franck Charlet
#  Copyright 2011 Pawel Góralski
#  Copyright 2009 Per Olofsson
#  Copyright 2008 Peter Gordon
#  Copyright 2008-2010 Yves Rizoud
#  Copyright 2007-2010 Adrien Destugues
#  Copyright 1996-2001 Sunset Design (Guillaume Dorme & Karl Maritaud)
#  
#  Grafx2 is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; version 2
#  of the License.
#  
#  Grafx2 is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with Grafx2; if not, see <http://www.gnu.org/licenses/>

# custom which function that takes 2 arguments to work around "which" commands
# that do not return FALSE (OS X 10.4).   Usage : $(call which,arg1,arg2)
which = $(shell P=`which "$(1)"` ; if [ "$$?" = "0" ] && [ -x "$$P" ]; then echo "$$P" ; else which "$(2)" ; fi)

# make V=1   to enable verbose building
V ?= 0

# Overridable defaults
  PREFIX ?= /usr/local
  exec_prefix = $(PREFIX)
  bindir = $(exec_prefix)/bin
  datarootdir = $(PREFIX)/share
  datadir = $(datarootdir)
  pixmapdir = $(datarootdir)/icons/hicolor/scalable/apps

# Compile with OPTIM=0 to disable gcc optimizations, to enable debug.
  STRIP = strip

# Detect GIT revision
ifeq (true,$(shell git rev-parse --is-inside-work-tree))
  GIT_REVISION = $(shell git rev-list --count 1af8c74f53110e349d8f0d19b14599281913f71f..)
  ifneq (,$(CI_COMMIT_REF_NAME))
    GIT_BRANCH = $(CI_COMMIT_REF_NAME)
  else
  ifneq (,$(TRAVIS_BRANCH))
    GIT_BRANCH = $(TRAVIS_BRANCH)
  else
    GIT_BRANCH = $(shell git rev-parse --abbrev-ref HEAD)
  endif
  endif
  ifneq (master,$(GIT_BRANCH))
    GIT_REVISION := $(GIT_REVISION)-$(GIT_BRANCH)
  endif
else
  # outside git repository : get version stored in version.c
  GIT_REVISION := $(shell cat version.c | cut -f 2 -d '"')
endif

  REVISION_CACHE = .revision.cache
  RES := $(shell if [ ! -f $(REVISION_CACHE) ] || [ "`cat $(REVISION_CACHE)`" != "$(GIT_REVISION)" ] ; then echo "$(GIT_REVISION)" > $(REVISION_CACHE) ; fi )

ifneq (,$(LABEL))
  LABEL_CACHE = .label.cache
  RES := $(shell if [ ! -f $(LABEL_CACHE) ] || [ "`cat $(LABEL_CACHE)`" != "$(LABEL)" ] ; then echo "$(LABEL)" > $(LABEL_CACHE) ; fi )
else
  # get label stored in pversion.c
  LABEL := $(shell cat pversion.c | cut -f 2 -d '"')
endif

  VERSIONTAG = $(subst wip.,wip,$(LABEL).$(GIT_REVISION))

  CFLAGS_CACHE = $(OBJDIR)/.cflags.cache

  RECOILVER = 5.0.0

# Video / input API used
  API = sdl

# At the moment support for TTF requires SDL or SDL2

# I can't build it with TTF on the PS2, sorry :(
NOTTF = 1

  ifneq (sdl,$(findstring sdl,$(API)))
    NOTTF = 1
  endif

### PLATFORM DETECTION AND CONFIGURATION ###

PLATFORMOBJ =

TAR = tar
# Note : --transform option was added in GNU tar version 1.15.91
TARTRANSFORM = --strip=1 --transform 's,^,grafx2/,g'

# default to gcc compiler
#ifeq (default,$(origin CC))
#  CC = gcc
#endif

#PS2 ee-gcc
CC = ee-gcc

PKG_CONFIG ?= pkg-config

# default for GENERATEDOCBIN
  GENERATEDOCBIN = ../bin/generatedoc
#For all other platforms, we can rely on uname
  PLATFORM = PS2

    # Finally, the default rules that work fine for most unix/gcc systems, linux and freebsd are tested.
    # Linux and FreeBSD specific (default rules)
    DELCOMMAND = rm -rf
    MKDIR = mkdir -p
    RMDIR = rmdir --ignore-fail-on-non-empty
    CP = cp
    ZIP = zip
    TARTRANSFORM = $(shell if $(TAR) --version | grep 'GNU tar' > /dev/null ; then \
                             echo "--strip=1 --transform 's,^,grafx2/,g'" ; else \
                             echo "-s '/\.\./grafx2/'" ; fi )
    PLATFORMFILES = ../share/grafx2/gfx2.png
        # Compiles a regular linux executable for the native platform
        BIN = ../bin/grafx2-$(API)
        TESTSBIN = ../bin/tests-$(API)
        COPT = -W -Wall -std=c99 -O3
          COPT += -I/usr/local/ps2dev/ps2sdk/common/include -I/usr/local/ps2dev/ps2sdk/ee/include -I/usr/local/ps2dev/ps2sdk/ports/include/SDL
        COPT += $(TTFCOPT) $(LUACOPT) $(JOYCOPT) -O$(OPTIM)
        COPT += -I/usr/local/ps2dev/ps2sdk/ports/include

        LOPT = -lm -lc -lgcc -lz
          LOPT += -L/usr/local/ps2dev/ps2sdk/ports/lib -lSDL_image -lsdl
        LOPT += $(TTFLOPT)
        LOPT += -lpng -lz
        LOPT += -ltiff 
        LOPT += $(LUALOPT)
        OBJDIR = ../obj/unix
        FCLOPT = -lfontconfig
        COPT += -DUSE_FC

### BUILD SETTINGS are set according to vars set in the platform selection, 
### the "overridable defaults", and environment variables set before launching make

#TrueType is optional: make NOTTF=1 to disable support and dependencies.
ifeq ($(NOTTF),1)
  TTFCOPT = -DNOTTF=1
  TTFLOPT =
  TTFLIBS =
  ifneq ($(API),win32)
    TTFLABEL = -nottf
  endif
else
 ifneq ($(PLATFORM),Darwin)
   TTFCOPT = 
   TTFLOPT =
   ifneq ($(WIN32CROSS),1)
     #TTFLOPT += -L$(PREFIX)/lib
   endif
   ifeq ($(API),sdl)
     TTFLOPT += -lSDL_ttf
     TTFLIBS = bin/SDL_ttf.dll
   else
     TTFLOPT += -lSDL2_ttf
     TTFLIBS = bin/SDL2_ttf.dll
   endif
   TTFLOPT += $(FCLOPT)
   TTFLIBS += bin/libfreetype-6.dll
   TTFLABEL = 
 endif
endif

# I disabled Lua for the PS2 build, maybe we could get it working after the proof of concept is building :)
NOLUA = 1

#Lua scripting is optional too
ifeq ($(NOLUA),1)
    LUACOPT =
    LUALOPT =
    LUALABEL = -nolua
else
    LUACOPT += -D__ENABLE_LUA__
    LUALABEL =
endif

#Recoil is optional [ps2 disabled]
    COPT += -DNORECOIL

OBJDIR := $(OBJDIR)-$(API)

  APIOBJ = sdlscreen.o
  COPT += -DUSE_SDL

  JOYCOPT = -DUSE_JOYSTICK

### And now for the real build rules ###

.PHONY : all debug release clean depend force install uninstall valgrind \
         doc doxygen htmldoc check

# This is the list of the objects we want to build. Dependancies are built by "make depend" automatically.
OBJS = main.o init.o graph.o $(APIOBJ) misc.o special.o \
       buttons.o palette.o help.o operatio.o pages.o \
       readline.o engine.o filesel.o fileseltools.o \
       op_c.o readini.o saveini.o \
       shade.o keyboard.o io.o version.o text.o SFont.o setup.o \
       pxsimple.o pxtall.o pxwide.o pxdouble.o pxtriple.o \
       pxtall2.o pxtall3.o pxwide2.o pxquad.o \
       windows.o brush.o realpath.o mountlist.o input.o hotkeys.o \
       transform.o pversion.o factory.o $(PLATFORMOBJ) \
       loadsave.o loadsavefuncs.o \
       pngformat.o motoformats.o stformats.o c64formats.o cpcformats.o \
       ifformat.o msxformats.o packbits.o giformat.o \
       fileformats.o miscfileformats.o libraw2crtc.o \
       brush_ops.o buttons_effects.o layers.o \
       oldies.o tiles.o colorred.o unicode.o gfx2surface.o \
       gfx2log.o gfx2mem.o tifformat.o c64load.o 6502.o
ifndef NORECOIL
OBJS += loadrecoil.o recoil.o
endif

TESTSOBJS = $(patsubst %.c,%.o,$(wildcard tests/*.c)) \
            miscfileformats.o fileformats.o oldies.o libraw2crtc.o \
            loadsavefuncs.o packbits.o tifformat.o c64load.o 6502.o \
            pngformat.o motoformats.o stformats.o c64formats.o cpcformats.o \
            ifformat.o msxformats.o giformat.o \
            op_c.o colorred.o \
            unicode.o fileseltools.o \
            io.o realpath.o version.o pversion.o \
            gfx2surface.o \
            gfx2log.o gfx2mem.o

OBJ = $(addprefix $(OBJDIR)/,$(OBJS))
TESTSOBJ = $(addprefix $(OBJDIR)/,$(TESTSOBJS))

DEP = $(patsubst %.o,%.d,$(OBJ) $(TESTSOBJ))

GENERATEDOCOBJ = $(addprefix $(OBJDIR)/,generatedoc.o hotkeys.o keyboard.o)

SKINS = skin_classic.png skin_modern.png skin_DPaint.png \
        font_Classic.png font_Fun.png font_Fairlight.png \
        font_Melon.png font_DPaint.png \
        skin_scenish.png font_Seen.png skin_Aurora.png
SKIN_FILES = $(addprefix ../share/grafx2/skins/,$(SKINS))
SKIN_FILES += $(wildcard ../share/grafx2/skins/unicode_*)

SCRIPTS_SAMPLES = codenetsend.lua
SCRIPTS_BRUSH = ApplyColor.lua Fisheye.lua GrayscaleAvg.lua \
                GrayscaleDesat.lua Halfsmooth.lua Waves.lua
SCRIPTS_DEMO = 3DPalette.lua Ellipse.lua FlipPicture.lua \
               SierpinskyCarpet.lua SierpinskyTriangle.lua \
               Spritesheet.lua
SCRIPTS_DEMO_BRUSH = Amigaball.lua ColorSphere.lua \
                     FindAA.lua Mandelbrot.lua
SCRIPTS_LIB = dawnbringer_lib.lua memory.lua
SCRIPTS_PAL = Desaturate.lua ExpandColors.lua FillColorCube.lua \
              InvertedRGB.lua Set3bit.lua Set6bit.lua \
              SetC64Palette.lua ShiftHue.lua
SCRIPTS_PIC = CellColourReducer.lua DrawGridIsometric.lua \
              DrawgridOrthogonal_Index.lua DrawGridOrthogonal_RGB.lua \
              GlassGridFilter.lua PaletteToPicture.lua Pic2isometric.lua \
              Rainbow-Dark2Bright.lua RemapImage2RGB.lua \
              RemapImage2RGB_ed.lua RemapImageTo3bitPal.lua \
              XBitColourXpaceFromPalette.lua Tiler.lua \
              FontConvert.lua
SCRIPTS_PIC_8BIT = ostro_oric.lua ostro_zx.lua \
                   lib/ostro_other.lua
SCRIPTS_PIC_THOMSON = bayer4_mo5.lua bayer4_to8.lua \
                      none_mo5.lua none_to8.lua none_to9.lua \
                      ostro_mo5.lua ostro_to7.lua ostro_to8.lua ostro_to9.lua \
                      lib/bayer.lua lib/color.lua lib/color_reduction.lua \
                      lib/convex_hull.lua lib/ostromoukhov.lua \
                      lib/thomson.lua
SCRIPT_FILES = $(addprefix ../share/grafx2/scripts/samples/, \
                 $(SCRIPTS_SAMPLES) \
                 $(addprefix brush/,$(SCRIPTS_BRUSH)) \
                 $(addprefix demo/,$(SCRIPTS_DEMO)) \
                 $(addprefix demo/brush/,$(SCRIPTS_DEMO_BRUSH)) \
                 $(addprefix libs/,$(SCRIPTS_LIB)) \
                 $(addprefix palette/,$(SCRIPTS_PAL)) \
                 $(addprefix picture/,$(SCRIPTS_PIC)) \
                 $(addprefix picture/others-8bit/,$(SCRIPTS_PIC_8BIT)) \
                 $(addprefix picture/thomson/,$(SCRIPTS_PIC_THOMSON)) )

FONTS = 8pxfont.png Tuffy.ttf PF_Arma_5__.png \
        PF_Easta_7_.png PF_Easta_7__.png PF_Ronda_7__.png \
        PF_Tempesta_5.png PF_Tempesta_5_.png PF_Tempesta_5__.png PF_Tempesta_5___.png \
        PF_Tempesta_7.png PF_Tempesta_7_.png PF_Tempesta_7__.png PF_Tempesta_7___.png \
        PF_Westa_7_.png PF_Westa_7__.png
FONT_FILES = $(addprefix ../share/grafx2/fonts/,$(FONTS))

DOC_FILES = $(wildcard ../doc/*.txt)
DOC_FILESRTF = ../doc/quickstart.rtf

# for silent/verbose building
ACTUAL_CC := $(CC)
CC_0 = @echo "$(if $(findstring .o,$@),Compiling $<,$(if $(findstring .d,$@),Building dependencies for $<,Linking $@))" ; $(ACTUAL_CC)
CC_1 = $(ACTUAL_CC)
CC = $(CC_$(V))

ifeq ($(V), 1)
CP += -v
AT =
else
AT = @
endif

DEPFLAGS = -MM -MG -MT $(patsubst %.d,%.o,$@) -MT $@
# -M : with system headers, -MM : without

ifeq ($(PLATFORM),Darwin)
all : $(MACAPPEXE)
$(MACAPPEXE) : $(BIN)
	rm -rf Grafx2.app
	mkdir -p Grafx2.app Grafx2.app/Contents Grafx2.app/Contents/Frameworks Grafx2.app/Contents/MacOS Grafx2.app/Contents/Resources Grafx2.app/Contents/Resources/scripts
	mkdir -p Grafx2.app/Contents/Resources/scripts/samples/brush
	mkdir -p Grafx2.app/Contents/Resources/scripts/samples/demo
	mkdir -p Grafx2.app/Contents/Resources/scripts/samples/libs
	mkdir -p Grafx2.app/Contents/Resources/scripts/samples/palette
	mkdir -p Grafx2.app/Contents/Resources/scripts/samples/picture
	echo 'APPL????' > Grafx2.app/Contents/PkgInfo
	cp Info.plist Grafx2.app/Contents
	cp Grafx2.icns Grafx2.app/Contents/Resources
	cp ../share/grafx2/gfx2.png Grafx2.app/Contents/Resources
	mkdir -p Grafx2.app/Contents/Resources/English.lproj
	iconv -f UTF-16BE English.lproj/InfoPlist.strings | sed -e 's/Grafx2 version [.0-9]*/Grafx2 version $(VERSIONTAG)/' | iconv -t UTF-16BE > Grafx2.app/Contents/Resources/English.lproj/InfoPlist.strings
	cp -r ../share/grafx2/fonts Grafx2.app/Contents/Resources
	cp -r ../share/grafx2/skins Grafx2.app/Contents/Resources
	cp -r ../share/grafx2/gfx2def.ini Grafx2.app/Contents/Resources
	cp -r ../share/grafx2/scripts/samples/brush Grafx2.app/Contents/Resources/scripts/samples
	cp -r ../share/grafx2/scripts/samples/demo Grafx2.app/Contents/Resources/scripts/samples
	cp -r ../share/grafx2/scripts/samples/libs Grafx2.app/Contents/Resources/scripts/samples
	cp -r ../share/grafx2/scripts/samples/palette Grafx2.app/Contents/Resources/scripts/samples
	cp -r ../share/grafx2/scripts/samples/picture Grafx2.app/Contents/Resources/scripts/samples
ifdef MACOSX_LION
	mkdir -p Grafx2.app/Contents/Frameworks/Lua.framework/Versions
	cp -Rp $(FWDIR)/Lua.framework/Versions/A Grafx2.app/Contents/Frameworks/Lua.framework/Versions
	mkdir -p Grafx2.app/Contents/Frameworks/libpng14.framework/Versions
	cp -Rp $(FWDIR)/libpng14.framework/Versions/1.4.8 Grafx2.app/Contents/Frameworks/libpng14.framework/Versions
endif
	if [ -d $(FWDIR)/SDL.framework/Versions ] ; then \
		mkdir -p Grafx2.app/Contents/Frameworks/SDL.framework/Versions ;\
		cp -Rp $(FWDIR)/SDL.framework/Versions/A Grafx2.app/Contents/Frameworks/SDL.framework/Versions ;\
	fi
	if [ -d $(FWDIR)/SDL_image.framework/Versions ] ; then \
		mkdir -p Grafx2.app/Contents/Frameworks/SDL_image.framework/Versions ;\
		cp -Rp $(FWDIR)/SDL_image.framework/Versions/A Grafx2.app/Contents/Frameworks/SDL_image.framework/Versions ;\
	fi
#	mkdir -p Grafx2.app/Contents/Frameworks/SDL_ttf.framework/Versions
#	cp -Rp $(FWDIR)/SDL_ttf.framework/Versions/A Grafx2.app/Contents/Frameworks/SDL_ttf.framework/Versions
    # remove those
	rm -fr Grafx2.app/Contents/Frameworks/SDL.framework/Versions/A/Headers
	rm -fr Grafx2.app/Contents/Frameworks/SDL.framework/Versions/A/Resources
	rm -fr Grafx2.app/Contents/Frameworks/SDL_image.framework/Versions/A/Headers
	rm -fr Grafx2.app/Contents/Frameworks/SDL_image.framework/Versions/A/Resources
ifdef MACOSX_LION
	rm -fr Grafx2.app/Contents/Frameworks/Lua.framework/Versions/A/Resources
	rm -fr Grafx2.app/Contents/Frameworks/Lua.framework/Versions/A/Headers
	rm -fr Grafx2.app/Contents/Frameworks/libpng14.framework/Versions/1.4.8/Resources
	rm -fr Grafx2.app/Contents/Frameworks/libpng14.framework/Versions/1.4.8/Headers
	rm -fr Grafx2.app/Contents/Frameworks/SDL_ttf.framework/Versions
endif
	cp $(BIN) $(MACAPPEXE)
	$(STRIP) -x -X -S $(MACAPPEXE)
	chmod +x $(MACAPPEXE)
	#tar cvzf grafx2-git$(GIT_REVISION)-macosx.tgz --exclude '.git' --exclude '*DS_Store*' Grafx2.app/*
else
all : $(BIN)
ifeq ($(ATARICROSS),1)
	$(STACK) -S 128k $(BIN)
	$(FIX_FLAGS) -S $(BIN)
else
ifeq ($(PLATFORM),FreeMiNT)  
	$(STACK) -S 128k $(BIN)
	$(FIX_FLAGS) -S $(BIN)
else
ifeq ($(SWITCH), 1)
	@nacptool --create "grafX2" "grafX2 team / romain337 (nxbuild)" "$(VERSIONTAG)" $(BIN).nacp
	@echo built ... $(BIN).nacp
	@elf2nro $(BIN) $(BIN).nro $(BIN).nacp --icon=../misc/switch/icon.jpg --nacp=$(BIN).nacp
	@echo built ... $(BIN).nro
endif
endif
endif
endif


debug : $(BIN)
ifeq ($(ATARICROSS),1)
	$(STACK) -S 128k $(BIN)
	$(FIX_FLAGS) -S $(BIN)
else
ifeq ($(PLATFORM),FreeMiNT)  
	$(STACK) -S 128k $(BIN)
	$(FIX_FLAGS) -S $(BIN)
else
ifeq ($(SWITCH), 1)
	@nacptool --create "grafX2" "grafX2 team / romain337 (nxbuild)" "$(VERSIONTAG)" $(BIN).nacp
	@echo built ... $(BIN).nacp
	@elf2nro $(BIN) $(BIN).nro $(BIN).nacp --icon=../misc/switch/icon.jpg --nacp=$(BIN).nacp
	@echo built ... $(BIN).nro
endif
endif
endif


# Make release will strip the executable to make it smaller but non-debugable
release : $(BIN)
	$(STRIP) $(BIN)
ifeq ($(SWITCH), 1)
		@nacptool --create "grafX2" "grafX2 team / romain337 (nxbuild)" "$(VERSIONTAG)" $(BIN).nacp
		@echo built ... $(BIN).nacp
		@elf2nro $(BIN) $(BIN).nro $(BIN).nacp --icon=../misc/switch/icon.jpg --nacp=$(BIN).nacp
		@echo built ... $(BIN).nro
endif

check:	$(TESTSBIN)
	$(TESTSBIN) --xml ../test-report.xml

# .tgz archive with source only files
SRCARCH = ../src-$(VERSIONTAG).tgz

$(SRCARCH):	$(wildcard *.c) $(wildcard *.cpp) $(wildcard *.h) Makefile \
	gfx2.ico Grafx2_Prefix.pch $(wildcard *.m) gfx2.rc grafx2.rdef
	$(AT)cd .. && $(TAR) czf $(SRCARCH:../%=%) $(addprefix src/,$^)

ifeq ($(PLATFORM),Darwin)
ziprelease: $(MACAPPEXE)
endif

# Create a zip archive ready for upload to the website, including binaries and sourcecode
ziprelease: release $(SRCARCH)
ifdef ATARICROSS
	$(eval TMP := $(shell mktemp -d))
	$(AT)$(MKDIR) $(TMP)/GRAFX2
	$(AT)$(CP) $(BIN) ../share/grafx2/gfx2def.ini ../share/grafx2/gfx2.png $(TMP)/GRAFX2
	$(AT)$(CP) $(SRCARCH) $(TMP)/GRAFX2/SRC$(firstword $(subst -, ,$(GIT_REVISION))).TGZ
	$(AT)$(MKDIR) $(TMP)/GRAFX2/DOC
	@for f in $(DOC_FILES) ; do \
	  n=$$(basename $$f .txt | sed -e 's/COMPILING/COMPILE/' -e 's/README-//' -e 's/SDL_image/SDL_img/' | tr "a-z." "A-Z_") ; \
	  $(CP) "$$f" "$(TMP)/GRAFX2/DOC/$$n.TXT" ; \
	done
	$(AT)$(MKDIR) $(TMP)/GRAFX2/FONTS
	@for f in $(FONT_FILES) ; do \
	  n=$$(basename $$f | sed -e 's/PF_\([a-zA-Z]\)[a-zA-Z]*_/PF\1/' | tr "a-z" "A-Z") ; \
	  $(CP) "$$f" "$(TMP)/GRAFX2/FONTS/$$n" ; \
	done
	$(AT)$(MKDIR) $(TMP)/GRAFX2/SKINS
	@for f in $(SKIN_FILES) ; do \
	  n=$$(basename $$f | sed -e 's/^\([a-z]\).*_/\1/' | tr "a-z" "A-Z") ; \
	  $(CP) "$$f" "$(TMP)/GRAFX2/SKINS/$$n" ; \
	done
	$(AT)$(MKDIR) $(TMP)/GRAFX2/SCRIPTS
	@for f in $(SCRIPT_FILES) ; do \
	  n="$$(basename $$f .lua | sed -e 's/ostro_/ostro/' -e 's/^\(.\{8\}\).*/\1/' | tr 'a-z' 'A-Z').LUA" ; \
	  d=$$(dirname $$f | sed -e 's#^../share/grafx2/scripts##' | tr "a-z" "A-Z") ; \
	  $(MKDIR) "$(TMP)/GRAFX2/SCRIPTS$$d" ; \
	  if [ -f "$(TMP)/GRAFX2/SCRIPTS$$d/$$n" ] ; then \
	    n=$$(echo $$n | sed -e 's/^\(.\{7\}\)./\1_/') ; \
	  fi ; \
	  $(CP) "$$f" "$(TMP)/GRAFX2/SCRIPTS$$d/$$n" ; \
	done
	$(AT)cd $(TMP) && \
	  $(ZIP) $(ZIPOPT) -k "$(CURDIR)/../grafx2-$(API)-$(VERSIONTAG)$(TTFLABEL)-$(PLATFORM).$(ZIP)" \
	    GRAFX2/$(notdir $(BIN)) GRAFX2/FONTS/*.* GRAFX2/SKINS/*.* GRAFX2/*.png
	$(AT)cd $(TMP) && \
	  $(ZIP) $(ZIPOPT) -k --to-crlf -r "$(CURDIR)/../grafx2-$(API)-$(VERSIONTAG)$(TTFLABEL)-$(PLATFORM).$(ZIP)" \
	    GRAFX2/gfx2def.ini GRAFX2/DOC/ GRAFX2/SCRIPTS/
	$(AT)$(RM) -r $(TMP)
else
ifdef SWITCH
	$(eval TMP := $(shell mktemp -d))
	$(AT)$(MKDIR) $(TMP)/switch/grafx2
	$(AT)$(CP) $(BIN).nro $(TMP)/switch/grafx2
	$(AT)$(CP) -R ../share/grafx2/fonts $(TMP)/switch/grafx2
	$(AT)$(CP) -R ../share/grafx2/skins $(TMP)/switch/grafx2
	$(AT)$(CP) ../misc/switch/gfx2def.ini $(TMP)/switch/grafx2
	cd .. && $(ZIP) $(ZIPOPT) "grafx2-$(API)-$(VERSIONTAG)$(TTFLABEL)-$(PLATFORM).$(ZIP)" \
	  $(DOC_FILES:../%=%) $(DOC_FILESRTF:../%=%) $(SRCARCH:../%=%)
	cd $(TMP) && \
	  $(ZIP) $(ZIPOPT) "$(CURDIR)/../grafx2-$(API)-$(VERSIONTAG)$(TTFLABEL)-$(PLATFORM).$(ZIP)" -r ./*
	$(AT)$(RM) -r $(TMP)
else
ifdef GP2XCROSS
	$(eval TMP := $(shell mktemp -d))
	$(AT)$(MKDIR) $(TMP)/data
	$(AT)$(CP) $(BIN) $(TMP)
	$(AT)$(CP) -R ../share/grafx2/* $(TMP)/data
	cd .. && $(ZIP) $(ZIPOPT) "grafx2-$(API)-$(VERSIONTAG)$(TTFLABEL)-$(PLATFORM).$(ZIP)" \
	  $(DOC_FILES:../%=%) $(DOC_FILESRTF:../%=%) $(SRCARCH:../%=%)
	cd $(TMP) && \
	  $(ZIP) $(ZIPOPT) "$(CURDIR)/../grafx2-$(API)-$(VERSIONTAG)$(TTFLABEL)-$(PLATFORM).$(ZIP)" \
	  $(BIN:../bin/%=%) $(SCRIPT_FILES:../share/grafx2/%=data/%) \
	  $(SKIN_FILES:../share/grafx2/%=data/%) $(FONT_FILES:../share/grafx2/%=data/%) \
	  data/gfx2def.ini
	$(AT)$(RM) -r $(TMP)
else
	cd .. && $(ZIP) $(ZIPOPT) "grafx2-$(API)-$(VERSIONTAG)$(TTFLABEL)-$(PLATFORM).$(ZIP)" \
	  $(BIN:../%=%) $(SCRIPT_FILES:../%=%) $(SKIN_FILES:../%=%) \
	  share/grafx2/gfx2.gif share/icons/grafx2.svg \
	  $(FONT_FILES:../%=%) $(SRCARCH:../%=%) $(PLATFORMFILES:../%=%) $(DOC_FILESRTF:../%=%)
ifdef WIN32CROSS
	cd .. && $(ZIP) $(ZIPOPT) --to-crlf "grafx2-$(API)-$(VERSIONTAG)$(TTFLABEL)-$(PLATFORM).$(ZIP)" \
	  $(DOC_FILES:../%=%) share/grafx2/gfx2def.ini
else
	cd .. && $(ZIP) $(ZIPOPT) "grafx2-$(API)-$(VERSIONTAG)$(TTFLABEL)-$(PLATFORM).$(ZIP)" \
	  $(DOC_FILES:../%=%) share/grafx2/gfx2def.ini
endif
ifeq ($(PLATFORM),Darwin)
	$(ZIP) $(ZIPOPT) -r ../$(APPZIPNAME) Grafx2.app
	cd .. && $(ZIP) $(ZIPOPT) $(APPZIPNAME) \
	  $(DOC_FILES:../%=%) $(DOC_FILESRTF:../%=%) $(SRCARCH:../%=%)
endif
endif
endif
endif
	$(AT)$(TAR) czf "../grafx2-$(VERSIONTAG)-src.tgz" $(TARTRANSFORM) \
	  ../src/*.c ../src/*.cpp ../src/*.h ../src/Makefile \
	  ../src/Grafx2_Prefix.pch ../src/*.m ../src/gfx2.rc ../src/gfx2.ico \
	  ../src/grafx2.rdef \
	  ../src/Grafx2.icns ../src/English.lproj/* ../src/Info.plist \
	  ../share/grafx2/gfx2def.ini $(SCRIPT_FILES) $(SKIN_FILES) \
	  ../share/grafx2/gfx2.gif ../share/grafx2/gfx2.png ../share/icons/grafx2.svg \
	  $(DOC_FILES) $(wildcard ../misc/*/*) $(FONT_FILES)
	$(AT)echo "$(VERSIONTAG)" > ../VERSION

$(BIN) : $(OBJ)
	@test -d ../bin || $(MKDIR) ../bin
	$(CC) $(OBJ) -o $@ $(LOPT) $(LDFLAGS) $(LDLIBS)
ifeq ($(PLATFORM),Haiku)
	rc -o $(OBJDIR)/grafx2.rsrc grafx2.rdef
	xres -o $(BIN) $(OBJDIR)/grafx2.rsrc
	mimeset -f $(BIN)
endif

$(TESTSBIN):	$(TESTSOBJ)
	@test -d ../bin || $(MKDIR) ../bin
	$(CC) $(TESTSOBJ) -o $@ $(LOPT) $(LDFLAGS) $(LDLIBS)


$(GENERATEDOCBIN): $(GENERATEDOCOBJ)
	@test -d ../bin || $(MKDIR) ../bin
	$(CC) $(GENERATEDOCOBJ) -o $(GENERATEDOCBIN) $(LOPT) $(LDFLAGS)

# GIT revision number
version.c:	$(REVISION_CACHE)
	echo "const char SVN_revision[]=\"$(GIT_REVISION)\";" > version.c

pversion.c:	$(LABEL_CACHE)
	echo "const char Program_version[]=\"$(LABEL)\";" > pversion.c

RES := $(shell $(MKDIR) $(OBJDIR) ; if [ ! -f $(CFLAGS_CACHE) ] || [ "`cat $(CFLAGS_CACHE)`" != "$(COPT) $(CFLAGS)" ] ; then echo "$(COPT) $(CFLAGS)" > $(CFLAGS_CACHE) ; fi )

ifndef NORECOIL
recoil.c:	../3rdparty/recoil-$(RECOILVER)/recoil.c
	if [ -f $< ] ; then $(CP) $< $@ ; fi
	if [ -f $(subst .c,.h,$<) ] ; then $(CP) $(subst .c,.h,$< $@) ; fi

recoil.h:	recoil.c

$(OBJDIR)/loadrecoil.o:	recoil.c recoil.h

../3rdparty/recoil-$(RECOILVER)/recoil.c:
	if [ -d ../3rdparty ] ; then $(MAKE) -C ../3rdparty recoil ; fi

$(OBJDIR)/recoil.o:
endif

6502.h:	6502.c

6502.c:	../3rdparty/6502/sources/6502.c
	if [ -f $< ] ; then $(CP) $< $@ ; fi
	if [ -f ../3rdparty/6502/API/emulation/CPU/6502.h ] ; then $(CP) ../3rdparty/6502/API/emulation/CPU/6502.h 6502.h ; fi

../3rdparty/6502/sources/6502.c:
	if [ -d ../3rdparty ] ; then $(MAKE) -C ../3rdparty 6502 ; fi

$(OBJDIR)/6502.o:	CFLAGS += -DCPU_6502_STATIC -DCPU_6502_USE_LOCAL_HEADER -DCPU_6502_DEPENDENCIES_H=\"6502types.h\"

$(OBJDIR)/6502.d:	CFLAGS += -DCPU_6502_STATIC -DCPU_6502_USE_LOCAL_HEADER -DCPU_6502_DEPENDENCIES_H=\"6502types.h\"

$(OBJ):	$(CFLAGS_CACHE)

$(OBJDIR)/%.o : %.c
	@$(MKDIR) -p $(@D)
	$(CC) $(COPT) $(CFLAGS) -c $*.c -o $(OBJDIR)/$*.o

$(OBJDIR)/%.o : %.m
	@$(MKDIR) -p $(@D)
	$(CC) $(COPT) -c $*.m -o $(OBJDIR)/$*.o

depend:	$(DEP)

$(OBJDIR)/%.d :	%.c
	@$(MKDIR) -p $(@D)
	$(CC) $(COPT) $(CFLAGS) $(DEPFLAGS) -o $@ $<

# update the gfx2.rc Windows ressource file
comma := ,

gfx2.rc: gfx2.ico $(REVISION_CACHE) $(LABEL_CACHE)
	sed -e 's/\(VERSION [ ]*\)\(.*\)/\1$(subst .,$(comma),$(subst wip,,$(LABEL))),0,$(firstword $(subst -, ,$(GIT_REVISION)))/' \
	    -e 's/\(Version", "\)\(.*\)"/\1$(subst wip.,wip,$(LABEL).$(GIT_REVISION))"/' $@ > $@.tmp
	if ! diff -q -i $@ $@.tmp ; then mv $@.tmp $@ ; else rm $@.tmp ; fi

# update the grafx2.rdef Haiku description file
grafx2.rdef:	$(REVISION_CACHE) $(LABEL_CACHE)
	sed -e 's/\(middle.*=[ ]*\)\([0-9]*\)/\1$(shell echo $(subst wip, ,$(LABEL)) | cut -d. -f 2)/' \
	    -e 's/\(minor.*=[ ]*\)\([0-9]*\)/\1$(firstword $(subst -, ,$(GIT_REVISION)))/' \
	    -e 's/\(variety.*=[ ]*\)\([A-Z0-9_]*\)/\1$(if $(findstring wip,$(LABEL)),B_APPV_BETA,B_APPV_FINAL)/' $@ > $@.tmp
	if ! diff -q -i $@ $@.tmp ; then mv $@.tmp $@ ; else rm $@.tmp ; fi

# Link the icons to the program under windows
$(OBJDIR)/winres.o : gfx2.rc gfx2.ico
	$(WINDRES) -i $< -o $(OBJDIR)/winres.o

# Compile the C++ file needed in Haiku to use the API
$(OBJDIR)/haiku.o : haiku.cpp
	g++ -c haiku.cpp -o $(OBJDIR)/haiku.o $(COPT) -Wno-multichar

clean :
	$(DELCOMMAND) $(OBJ) $(DEP)
	$(DELCOMMAND) $(TESTSOBJ)
	$(DELCOMMAND) $(BIN) $(TESTSBIN)
	if [ -d ../3rdparty ] ; then $(DELCOMMAND) recoil.c recoil.h ; fi

ifneq ($(PLATFORM),amiga-vbcc)
# Linux installation of the program
install : $(BIN)
	# Create dirs
	test -d $(DESTDIR)$(bindir) || $(MKDIR) $(DESTDIR)$(bindir)
	test -d $(DESTDIR)$(datadir)/grafx2 || $(MKDIR) $(DESTDIR)$(datadir)/grafx2
	test -d $(DESTDIR)$(datadir)/grafx2/fonts || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/fonts
	test -d $(DESTDIR)$(datadir)/grafx2/skins || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/skins
	test -d $(DESTDIR)$(datadir)/grafx2/scripts || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/scripts
	test -d $(DESTDIR)$(datadir)/grafx2/scripts/libs || $(MKDIR) $(DESTDIR)$(datadir)/grafx2/scripts/libs
	test -d $(DESTDIR)$(datadir)/applications || $(MKDIR) $(DESTDIR)$(datadir)/applications
	test -d $(DESTDIR)$(datadir)/metainfo || $(MKDIR) $(DESTDIR)$(datadir)/metainfo
	test -d $(DESTDIR)$(pixmapdir) || $(MKDIR) $(DESTDIR)$(pixmapdir)
	
	# Copy files
	$(CP) $(BIN) $(DESTDIR)$(bindir)
	$(CP) ../share/grafx2/gfx2def.ini $(DESTDIR)$(datadir)/grafx2/
	$(CP) ../share/grafx2/gfx2.gif $(DESTDIR)$(datadir)/grafx2/
	$(CP) ../share/grafx2/gfx2.png $(DESTDIR)$(datadir)/grafx2/
	$(CP) ../share/grafx2/fonts/* $(DESTDIR)$(datadir)/grafx2/fonts/
	$(CP) $(SKIN_FILES) $(DESTDIR)$(datadir)/grafx2/skins/
	for f in $(SCRIPT_FILES:../share/%=%); do \
	  test -d $(DESTDIR)$(datadir)/`dirname $$f` || $(MKDIR) $(DESTDIR)$(datadir)/`dirname $$f` ; \
	  $(CP) "../share/$$f" "$(DESTDIR)$(datadir)/$$f" ; \
	done
	# Icon and desktop file for debian
	$(CP) ../misc/unix/grafx2.desktop $(DESTDIR)$(datadir)/applications/
	$(CP) ../misc/unix/grafx2.appdata.xml $(DESTDIR)$(datadir)/metainfo/
	$(CP) ../misc/unix/grafx2.xpm $(DESTDIR)$(pixmapdir)
	$(CP) ../share/icons/grafx2.svg $(DESTDIR)$(pixmapdir)
	@echo Install complete
  
# Linux uninstallation of the program
uninstall :
	$(DELCOMMAND) $(DESTDIR)$(bindir)/grafx2
	$(DELCOMMAND) $(DESTDIR)$(datadir)/grafx2/gfx2def.ini
	$(DELCOMMAND) $(DESTDIR)$(datadir)/grafx2/gfx2.gif
	$(DELCOMMAND) $(DESTDIR)$(datadir)/grafx2/fonts/*
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/fonts),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/fonts,)
	$(DELCOMMAND) $(SKIN_FILES:../share%=$(DESTDIR)$(datadir)%)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/skins),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/skins,)
	$(DELCOMMAND) $(SCRIPT_FILES:../share%=$(DESTDIR)$(datadir)%)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/samples/picture),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples/picture,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/samples/palette),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples/palette,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/samples/libs),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples/libs,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/samples/demo/brush),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples/demo/brush,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/samples/demo),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples/demo,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/samples/brush),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples/brush,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/samples),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/samples,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts/libs),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts/libs,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2/scripts),$(RMDIR) $(DESTDIR)$(datadir)/grafx2/scripts,)
	$(if $(wildcard $(DESTDIR)$(datadir)/grafx2),$(RMDIR) $(DESTDIR)$(datadir)/grafx2,)
	# Icon and desktop file for debian
	$(DELCOMMAND) $(DESTDIR)$(datadir)/applications/grafx2.desktop
	$(DELCOMMAND) $(DESTDIR)$(datadir)/metainfo/grafx2.appdata.xml
	$(DELCOMMAND) $(DESTDIR)$(pixmapdir)/grafx2.xpm
	$(DELCOMMAND) $(DESTDIR)$(pixmapdir)/grafx2.svg
	@echo Uninstall complete

endif

valgrind: $(BIN)
	valgrind --suppressions=../valgrind.grafx2.suppr --leak-check=full --show-leak-kinds=all --num-callers=20 $(BIN) -v

doc:	doxygen

doxygen:
	$(MAKE) -C ../tools doxygen

htmldoc:	$(GENERATEDOCBIN)
	$(MKDIR) ../doc/html
	$(CP) $(filter ../share/grafx2/skins/skin%.png,$(SKIN_FILES)) ../doc/html
	$(GENERATEDOCBIN) ../doc/html

print-%:
	@echo "$* = $($*)"

ifneq ($(MAKECMDGOALS),clean)
-include $(DEP)
endif
