# Simple Makefile for local build

# CC_SUFFIX can be set to .cc or .cxx or .cpp
ifeq ($(CC_SUFFIX),)
	CC_SUFFIX := .cpp
endif	 
ifeq ($(HH_SUFFIX),)
	HH_SUFFIX := .hh
endif	 

# List of C++ source files
SRCLIST := $(wildcard *$(CC_SUFFIX))
# list of object (*.o) files to create
OBJLIST := $(foreach V,$(SRCLIST),$(V:$(CC_SUFFIX)=.o))

# Default target is clean and compile
default: clean compile link
	
compile: $(OBJLIST)

link: 
	$(CXX) -L../src/openlcb -lopenlcb -L../scaffold -lscaffold CanMrrlcbTest.o -o CanMrrlcbTest
	$(CXX) -L../src/openlcb -lopenlcb -L../scaffold -lscaffold DatagramTest.o -o DatagramTest
	
#subdirs:
#	cd openlcb; make

# Compile C++ file
%.o: %.cpp
	$(CXX) $(CCFLAGS) $(CPPFLAGS) -c -o $(*F).o $(<F)

clean:
	$(RM) *.o