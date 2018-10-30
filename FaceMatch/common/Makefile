# 2016-2017 (C) Eugene.Borovikov@NIH.gov
include common.mk

FMJI_BIN := libFaceMatchJavaInterface.so
FMJI_TRG := $(BUILD)$(FMJI_BIN)

FMJX_PATH := ../Tools/FaceMatchJavaInterface/FaceMatchJavaInterface
FMJX_SRC := fm_FaceMatchJavaInterface.cpp
FMJI_OBJ := $(addprefix $(BUILD),$(FMJX_SRC:.cpp=.o))

PCH := $(BUILD)common$(PCX)

BIN := libcommon.so
TRG := $(BUILD)$(BIN)

all: $(BUILD).dir common FaceMatchJavaInterface
	# done $@: $^

FaceMatchJavaInterface: $(FMJI_TRG)
	cp $(FMJI_TRG) ../bin/

common: $(TRG)
	cp $(TRG) ../bin/

$(FMJI_TRG): $(FMJI_OBJ) $(TRG)
	$(CXX) -shared -o $@ $(FMJI_OBJ) -L$(BUILD) -lcommon $(LFLAGS)

$(BUILD)%.o: $(FMJX_PATH)/%.cpp $(PCH)
	$(CXX) $(CFLAGS) -fPIC -I. -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux -I../FaceFinder -I../FaceMatcher -c $< -o $@

$(TRG): $(OBJ)
	$(CXX) -shared -o $@ $^ $(LFLAGS)

$(PCH): common.cpp *.h
	$(CXX) $(CFLAGS) -fPIC -c $< -o $@
	
$(BUILD)%.o: %.cpp $(PCH)
	$(CXX) $(CFLAGS) -fPIC -c $< -o $@

$(BUILD)%.o: %.cu %.cuh $(PCH) 
	nvcc -Xcompiler -fPIC -lineinfo -c $< -o $@ $(COMMON_CFLAGS)
 
clean:
	rm -vf ../bin/$(BIN) ../bin/$(FMJI_BIN) $(TRG) $(FMJI_TRG) $(addprefix $(BUILD),*.o *.?ch *~)
