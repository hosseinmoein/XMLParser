## Hossein Moein
## March 24 2018

LOCAL_LIB_DIR = ../lib/$(BUILD_PLATFORM)
LOCAL_BIN_DIR = ../bin/$(BUILD_PLATFORM)
LOCAL_OBJ_DIR = ../obj/$(BUILD_PLATFORM)
LOCAL_INCLUDE_DIR = ../include
PROJECT_LIB_DIR = ../../../lib/$(BUILD_PLATFORM)
PROJECT_INCLUDE_DIR = ../../../include

# -----------------------------------------------------------------------------

SRCS = XMLParser.cc \
       XMLString.cc \
       XMLWriter.cc \
       xml_tester.cc

HEADERS = $(LOCAL_INCLUDE_DIR)/XMLNVPair.h \
          $(LOCAL_INCLUDE_DIR)/XMLParser.h \
          $(LOCAL_INCLUDE_DIR)/XMLString.h \
          $(LOCAL_INCLUDE_DIR)/XMLTreeNodes.h \
          $(LOCAL_INCLUDE_DIR)/XMLWriter.h

LIB_NAME = XMLParser
TARGET_LIB = $(LOCAL_LIB_DIR)/lib$(LIB_NAME).a

TARGETS = $(TARGET_LIB) $(LOCAL_BIN_DIR)/xml_tester

# -----------------------------------------------------------------------------

LFLAGS += -Bstatic -L$(LOCAL_LIB_DIR) -L$(PROJECT_LIB_DIR) -L$(XERCES_DIR)/lib

LIBS = $(LFLAGS) -l$(LIB_NAME) -lDMScu $(PLATFORM_LIBS)
INCLUDES += -I. -I$(LOCAL_INCLUDE_DIR) -I$(PROJECT_INCLUDE_DIR)
DEFINES = -D_REENTRANT -DDMS_INCLUDE_SOURCE \
          -DP_THREADS -D_POSIX_PTHREAD_SEMANTICS -DDMS_$(BUILD_DEFINE)__

# -----------------------------------------------------------------------------

# object file
#
LIB_OBJS = $(LOCAL_OBJ_DIR)/XMLParser.o \
           $(LOCAL_OBJ_DIR)/XMLString.o \
           $(LOCAL_OBJ_DIR)/XMLWriter.o

# -----------------------------------------------------------------------------

# set up C++ suffixes and relationship between .cc and .o files
#
.SUFFIXES: .cc

$(LOCAL_OBJ_DIR)/%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

.cc :
	$(CXX) $(CXXFLAGS) $< -o $@ -lm $(TLIB) -lg++

# -----------------------------------------------------------------------------

all: PRE_BUILD $(TARGETS)

PRE_BUILD:
	mkdir -p $(LOCAL_LIB_DIR)
	mkdir -p $(LOCAL_BIN_DIR)
	mkdir -p $(LOCAL_OBJ_DIR)
	mkdir -p $(PROJECT_LIB_DIR)
	mkdir -p $(PROJECT_INCLUDE_DIR)

$(TARGET_LIB): $(LIB_OBJS)
	ar -clrs $(TARGET_LIB) $(LIB_OBJS)

XML_TESTER_OBJ = $(LOCAL_OBJ_DIR)/xml_tester.o
$(LOCAL_BIN_DIR)/xml_tester: $(XML_TESTER_OBJ) $(HEADERS)
	$(CXX) -o $@ $(XML_TESTER_OBJ) $(LIBS)

# -----------------------------------------------------------------------------

depend:
	makedepend $(CXXFLAGS) -Y $(SRC)

clobber:
	rm -f $(LIB_OBJS) $(TARGETS) $(XML_TESTER_OBJ)

install_lib:
	cp -pf $(TARGET_LIB) $(PROJECT_LIB_DIR)/.

install_hdr:
	cp -pf $(HEADERS) $(PROJECT_INCLUDE_DIR)/.

# -----------------------------------------------------------------------------

## Local Variables:
## mode:Makefile
## tab-width:4
## End:
