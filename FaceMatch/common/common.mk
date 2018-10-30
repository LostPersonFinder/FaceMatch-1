# 2016-2017 (C) Eugene.Borovikov@NIH.gov
GCCVERSION := $(shell gcc -dumpversion)

#$(info GCCVERSION = $(GCCVERSION))

GCCVER := $(subst .,0,$(GCCVERSION))
GCCVER := $(shell expr `echo $(GCCVER)` | cut -b1-3)

#$(info GCCVER = $(GCCVER))

PRECC47 := $(shell echo $(GCCVER)\<407 | bc)
PostCC5 := $(shell echo $(GCCVER)\>=500 | bc)

ifeq ($(PRECC47), 1) # pre C++11 compatible
	CXX11 := -std=c++0x -Doverride=
else # assume 4.7 or higher
	CXX11 := -std=c++11
endif

ifeq ($(PostCC5), 1) # gcc 5.0 or higher
	CXX11 += -Wno-deprecated-declarations # suppress spurios warnings, e.g. deprecated auto_ptr in OpenCV-2.x
endif

RELEASE_FLAGS := -O3 

ifdef _DEBUG
	CFLAGS := -g -O0 -D_DEBUG -DTRACE=printf
	BUILD := Debug/
else ifdef _RELEASE
	CFLAGS := $(RELEASE_FLAGS)
	BUILD := Release/
else ifdef _PROFILE
	CFLAGS := -D_TIMING $(RELEASE_FLAGS)
	BUILD := Profile/
else
	BUILD := Default/
endif

COMMON_CFLAGS := $(CXX11) `pkg-config opencv --cflags`
CFLAGS += $(COMMON_CFLAGS) -fopenmp -I$(BUILD)
LFLAGS += `pkg-config opencv --libs` -fopenmp

SRC := $(wildcard *.cpp)
OBJ := $(addprefix $(BUILD),$(SRC:.cpp=.o))
AFN := stdafx
OBJ := $(filter-out $(BUILD)$(AFN).o,$(OBJ))
OBJ := $(filter-out $(BUILD)common.o,$(OBJ))
HDR := $(wildcard *.h)
PCX := .h.gch
PHD := $(addprefix $(BUILD),$(HDR:.h=.h.gch))
Bld := Debug Profile Release
bld := $(shell echo $(Bld) | tr A-Z a-z)

# empty space
EMP :=
SPC := $(EMP) $(EMP)

### common targets

.PHONY: all clean rebuild $(Bld)

all: $(BUILD).dir
.dir:
%.dir:
	mkdir -p $(@D); touch $(@D)
	
ALL: all $(Bld)

$(Bld):
	$(MAKE) $(shell echo _$@=1 | tr a-z A-Z)
$(bld):
	$(MAKE) $(shell echo _$@=1 | tr a-z A-Z)

rebuild: clean
	$(MAKE) all

CBld := $(addprefix clean, $(Bld))

cleanALL: clean $(CBld)

clean: $(BUILD).clean
.clean:
%.clean:
	rm -rf $(BUILD)

$(CBld):
	$(MAKE) $(shell echo _$(subst clean,,$@)=1 | tr a-z A-Z) clean

cleanTOTAL: cleanALL
	rm -rv `find . -type d -name Debug -o -name Release -o -name Profile`

