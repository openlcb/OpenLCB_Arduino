# Simple Makefile fragment included by convention


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

clean:
	@$(RM) *.o lib*.a $(BINFILES)

# Compile general C++ file
%.o: %.cpp
	@$(CXX) $(CCFLAGS) $(CPPFLAGS) -c -o $(*F).o $(<F)

# compile everything
compile: $(OBJLIST)

# link a library from all object files
# use libtool in preference to ar
AR := libtool
ARFLAGS := -static -o
%.a: $(OBJLIST)
	@$(AR) $(ARFLAGS) $@ $(OBJLIST)

# Reset the default goal.
.DEFAULT_GOAL :=

# 
# Notes:
# 
# @ at the front of a command suppresses display unless -n is used
# 
# 
# The assumption is that a directory makes _either_
# one library, or executables; not both.
# 
