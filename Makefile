MINGW_PREFIX=x86_64-w64-mingw32.static.posix-
BUILD_DIR=build
LINUX_CXX=clang++

-include Makefile.local

ifndef LINUX_LINK
	LINUX_LINK=$(LINUX_CXX)
endif

ifneq (,$(findstring clang,$(LINUX_CXX)))
	CLANG=yes
	#LINUX_LINK+=-fuse-ld=lld
endif

_dummy := $(shell [ -d output ] || mkdir -p output)
_dummy := $(shell [ -d $(BUILD_DIR) ] || mkdir -p $(BUILD_DIR))
_dummy := $(shell [ -d $(BUILD_DIR)/imgui ] || mkdir -p $(BUILD_DIR)/imgui)
_dummy := $(shell [ -d $(BUILD_DIR)/keeperrl ] || mkdir -p $(BUILD_DIR)/keeperrl)

SHARED_SRC=keeperrl/fx_color keeperrl/fx_vec keeperrl/fx_math keeperrl/fx_curve keeperrl/clock \
		   keeperrl/fx_emission_source keeperrl/fx_particle_system keeperrl/fx_factory keeperrl/fx_manager \
		   keeperrl/fx_renderer keeperrl/fx_draw_buffers keeperrl/framebuffer keeperrl/opengl \
		   keeperrl/debug keeperrl/util keeperrl/directory_path keeperrl/file_path keeperrl/color keeperrl/texture \
		   imgui/imgui imgui/imgui_draw imgui/imgui_demo imgui_wrapper

PROGRAM_SRC=fx_tester

ALL_SRC=$(PROGRAM_SRC) $(SHARED_SRC) $(IMGUI_SRC)

LINUX_OBJECTS:=$(ALL_SRC:%=$(BUILD_DIR)/%.o)
#LINUX_DWO:=$(ALL_SRC:%=$(BUILD_DIR)/%.dwo)
MINGW_OBJECTS:=$(ALL_SRC:%=$(BUILD_DIR)/%_.o)

LINUX_LIB_OBJECTS:=$(LIBS_SRC:%=$(BUILD_DIR)/%.o)
MINGW_LIB_OBJECTS:=$(LIBS_SRC:%=$(BUILD_DIR)/%_.o)

LINUX_SHARED_OBJECTS:=$(SHARED_SRC:%=$(BUILD_DIR)/%.o)  $(LIBS_SRC:%=$(BUILD_DIR)/%.o)
MINGW_SHARED_OBJECTS:=$(SHARED_SRC:%=$(BUILD_DIR)/%_.o) $(LIBS_SRC:%=$(BUILD_DIR)/%_.o)

LINUX_PROGRAMS:=$(PROGRAM_SRC:%=%)
MINGW_PROGRAMS:=$(PROGRAM_SRC:%=%.exe)

all: $(LINUX_PROGRAMS) $(MINGW_PROGRAMS)

FWK_DIR=libfwk
include $(FWK_DIR)/Makefile.include

LIBS=-lSDL2_image
#$(shell $(LINUX_PKG_CONFIG) --libs $(LIBS))
#$(shell $(LINUX_PKG_CONFIG) --cflags $(LIBS))

LINUX_LIBS=$(LIBS) $(LINUX_FWK_LIBS) -lstdc++ -lm
MINGW_LIBS=$(LIBS) $(MINGW_FWK_LIBS)

INCLUDES=-Isrc/ -Ikeeperrl/ -Ikeeperrl/extern/ $(FWK_INCLUDES)


NICE_FLAGS=-Wall -Wextra -Woverloaded-virtual -Wnon-virtual-dtor -Werror=return-type -Werror=switch -Wimplicit-fallthrough\
		   -Wno-reorder -Wuninitialized -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter -Werror=format \
		   -Wparentheses -Wno-overloaded-virtual -Wno-undefined-inline #-Werror
FLAGS += -DFWK_TESTING_MODE -DSTX_NO_STD_VARIANT
ifdef CLANG
LINUX_FLAGS+=-Wconstant-conversion
LINUX_LIBS+=-pthread
else
NICE_FLAGS+=-Werror=aggressive-loop-optimizations
DISABLED_FLAGS=-Wno-unused-but-set-variable -Wno-strict-overflow
LINUX_LIBS+=-pthread
endif

FLAGS+=-std=c++1z -fno-omit-frame-pointer -pthread -fopenmp -Wall \
	   -ggdb $(NICE_FLAGS) $(DISABLED_FLAGS) $(INCLUDES)

LINUX_FLAGS=$(FLAGS) $(LINUX_FWK_FLAGS)
MINGW_FLAGS=$(FLAGS) $(MINGW_FWK_FLAGS)

LINUX_STRIP=strip

MINGW_CXX=$(MINGW_PREFIX)g++
MINGW_STRIP=$(MINGW_PREFIX)strip

PCH_FILE_SRC=src/pch.h

PCH_FILE_H=$(BUILD_DIR)/pch.h
PCH_FILE_GCH=$(BUILD_DIR)/pch.h.gch
PCH_FILE_PCH=$(BUILD_DIR)/pch.h.pch

ifdef CLANG
	PCH_INCLUDE=-include-pch $(PCH_FILE_PCH)
	PCH_FILE_MAIN=$(PCH_FILE_PCH)
else
	PCH_INCLUDE=-I$(BUILD_DIR) -include $(PCH_FILE_H)
	PCH_FILE_MAIN=$(PCH_FILE_GCH)
endif

$(PCH_FILE_H): $(PCH_FILE_SRC)
	cp $^ $@
$(PCH_FILE_MAIN): $(PCH_FILE_H)
	$(LINUX_CXX) -x c++-header -MMD $(LINUX_FLAGS) $(PCH_FILE_H) -o $@

#time -o stats.txt -a -f "%U $@" 
$(LINUX_OBJECTS): $(BUILD_DIR)/%.o:  src/%.cpp $(PCH_FILE_MAIN)
	$(LINUX_CXX) -MMD $(LINUX_FLAGS) $(FLAGS_$*) $(PCH_INCLUDE) -c src/$*.cpp -o $@

$(MINGW_OBJECTS): $(BUILD_DIR)/%_.o: src/%.cpp
	$(MINGW_CXX) -MMD $(MINGW_FLAGS) $(FLAGS_$*) -c src/$*.cpp -o $@

$(LINUX_PROGRAMS): %:     $(LINUX_SHARED_OBJECTS) $(BUILD_DIR)/%.o $(LINUX_FWK_LIB)
	$(LINUX_LINK) -o $@ $^ -Wl,--export-dynamic $(LIBS_$@) $(LINUX_LIBS)

$(MINGW_PROGRAMS): %.exe: $(MINGW_SHARED_OBJECTS) $(BUILD_DIR)/%_.o $(MINGW_FWK_LIB) 
	$(MINGW_CXX) -o $@ $^ $(LIBS_$*) $(MINGW_LIBS) -Wl,-subsystem,console
	$(MINGW_STRIP) $@

DEPS:=$(ALL_SRC:%=$(BUILD_DIR)/%.d) $(ALL_SRC:%=$(BUILD_DIR)/%_.d) $(PCH_FILE_H).d

clean:
	-rm -f $(LINUX_OBJECTS) $(LINUX_DWO) $(MINGW_OBJECTS) $(LINUX_PROGRAMS) $(MINGW_PROGRAMS) \
		   $(DEPS) $(PCH_FILE_GCH) $(PCH_FILE_PCH) $(PCH_FILE_H)
	-rmdir output
	find $(BUILD_DIR) -type d -empty -delete

full-clean: clean
	$(MAKE) -C $(FWK_DIR) clean

.PHONY: full-clean clean

-include $(DEPS)

