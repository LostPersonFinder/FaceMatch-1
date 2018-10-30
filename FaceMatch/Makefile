include common/common.mk

LIB := common
BLIB := $(addsuffix .so,$(addprefix bin/lib,$(LIB)))
EXE := NearDupImgDetector FaceFinder ImageMatcher FaceMatcher
BEXE := $(addprefix bin/,$(EXE))

XOPT:= -f ../common/exe.mk -C
LOPT:= -C

CEXE := $(addsuffix _clean,$(EXE))
CLIB := $(addsuffix _clean,$(LIB))

TEXE := $(addprefix test,$(EXE))

.PHONY: all clean rebuild $(EXE) $(LIB)

all: $(EXE)

$(EXE): $(LIB)
	$(MAKE) $(XOPT) $@

$(LIB):
	$(MAKE) $(LOPT) $@

clean: $(CEXE) $(CLIB)
	rm -vf $(BEXE) $(BLIB)

$(CEXE):
	$(MAKE) $(XOPT) $(subst _,$(SPC),$@)

$(CLIB):
	$(MAKE) $(LOPT) $(subst _,$(SPC),$@)
	
test: testGPU
	# done $@

test%: bin/%
	cd bin; $(subst test,,$@) -test $(PRM)
	
testCPU:
	$(MAKE) $(TEXE)
	# done $@
	
testGPU: testCPU
	$(MAKE) $(TEXE) PRM=-GPU
	# done $@
