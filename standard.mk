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
OBJLIST := $(foreach V,$(SRCLIST),$(OBJDIR)/$(V:$(CC_SUFFIX)=.o))

#.PHONY: clean $(foreach v,$(SUBDIRS),$(v).clean)
clean:   $(foreach v,$(SUBDIRS),$(v).clean)
	@$(RM) *.o lib*.a $(BINFILES) 
	@$(RM) -rf $(OBJDIR)

%.clean:
	make -C $(@:.clean=) clean CPPFLAGS="$(CPPFLAGS)" OBJDIR="$(OBJDIR)/$(@:.clean=)"

$(OBJDIR):
	@mkdir -p $(OBJDIR)
	
# Compile general C++ file
$(OBJDIR)/%.o: %.cpp
	@$(CXX) $(CCFLAGS) $(CPPFLAGS) -c -o $(OBJDIR)/$(*F).o $(<F)

# compile everything
compile: $(OBJDIR) $(OBJLIST) $(foreach v,$(SUBDIRS),$(v).compile)
    
%.compile:
	make -C $(@:.compile=) compile CPPFLAGS="$(CPPFLAGS)" OBJDIR="$(OBJDIR)/$(@:.compile=)"

# link a library from all object files
# use libtool in preference to ar
AR := libtool
ARFLAGS := -static -o
$(OBJDIR)/%.a: $(OBJLIST)
	@$(AR) $(ARFLAGS) $@ $(OBJLIST)

# create libs
.PHONY: lib
lib: $(OBJDIR) $(OBJDIR)/$(LIB) $(foreach v,$(SUBDIRS),$(v).lib) FORCE
    
%.lib:
	make -C $(@:.lib=) lib CPPFLAGS="$(CPPFLAGS)" OBJDIR="$(OBJDIR)/$(@:.lib=)"

# create bins
.PHONY: bin
bin: $(OBJDIR) $(foreach v,$(SUBDIRS),$(v).bin) $(foreach v,$(BINFILES),$(OBJDIR)/$(v)) FORCE
    
%.bin: 
	make -C $(@:.bin=) bin CPPFLAGS="$(CPPFLAGS)" OBJDIR="$(OBJDIR)/$(@:.bin=)"

.PHONY: FORCE
FORCE:
	@# nothing

# Reset the default goal.
.DEFAULT_GOAL :=

