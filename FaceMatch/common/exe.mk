# 2016-2017 (C) Eugene.Borovikov@NIH.gov
CDN := common
CMN := ../$(CDN)

include $(CMN)/$(CDN).mk

CMB := $(CMN)/$(BUILD)
AFX := $(BUILD)$(AFN)$(PCX)
CFX := $(CMB)/$(CDN)$(PCX)

SOB := lib$(CDN).so
LIB := $(CMB)$(SOB)

TRG := $(BUILD)$(notdir $(CURDIR))
BRG := ../bin/$(TRG)

all: $(BUILD).dir $(TRG)
	cp $(TRG) ../bin/
	
$(TRG): $(LIB) $(OBJ)
	$(CXX) -o $@ $(OBJ) -L$(CMB) -l$(CDN) $(LFLAGS)

$(AFX): $(AFN).cpp $(CFX)
	$(CXX) $(CFLAGS) -I$(CMB) -I$(CMN) -c $< -o $@

$(CFX): $(LIB)

$(LIB):
	$(MAKE) -C $(CMN)

$(BUILD)%.o: %.cpp $(AFX)
	$(CXX) $(CFLAGS) -I$(CMN) -c $< -o $@

clean:
	rm -vf $(TRG) *.?ch *.o *~ $(BRG)

test: $(TRG)
	$(TRG) -test $(PRM)

testGPU:
	$(MAKE) test PRM=-GPU
