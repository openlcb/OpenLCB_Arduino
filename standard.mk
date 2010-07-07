# Simple Makefile fragment included by convention

# build subdirectory even when already present
#.PHONY: $(SUBDIRS)

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

#.PHONY: clean $(foreach v,$(SUBDIRS),$(v).clean)
clean:   $(foreach v,$(SUBDIRS),$(v).clean)
	@$(RM) *.o lib*.a $(BINFILES) 
	@$(RM) -rf $(OBJDIR)

%.clean:
	@make -C $(@:.clean=) clean CPPFLAGS="$(CPPFLAGS)" OBJDIRPREFIX="$(OBJDIR)"

$(OBJDIR):
	@mkdir -p $(OBJDIR)
	
# Compile general C++ file
%.o: %.cpp
	@$(CXX) $(CCFLAGS) $(CPPFLAGS) -c -o $(*F).o $(<F)

# compile everything
compile: $(OBJDIR) $(OBJLIST) $(foreach v,$(SUBDIRS),$(v).compile)
    
%.compile:
	@make -C $(@:.compile=) compile CPPFLAGS="$(CPPFLAGS)" OBJDIRPREFIX="$(OBJDIR)"

# link a library from all object files
# use libtool in preference to ar
AR := libtool
ARFLAGS := -static -o
%.a: $(OBJLIST)
	@$(AR) $(ARFLAGS) $@ $(OBJLIST)

# create libs
.PHONY: lib
lib: $(OBJDIR) $(LIBS) $(foreach v,$(SUBDIRS),$(v).lib) FORCE
    
%.lib:
	@make -C $(@:.lib=) lib CPPFLAGS="$(CPPFLAGS)" OBJDIRPREFIX="$(OBJDIR)"

# create bins
.PHONY: bin
bin: $(OBJDIR) $(BINFILES) $(foreach v,$(SUBDIRS),$(v).bin) FORCE
    
%.bin:
	@make -C $(@:.bin=) bin CPPFLAGS="$(CPPFLAGS)" OBJDIRPREFIX="$(OBJDIR)"

.PHONY: FORCE
FORCE:
	@# nothing

# Reset the default goal.
.DEFAULT_GOAL :=

