
# Top-level Makefile for OpenLCB C/C++ implementations

# Arduino sketch directories but not the OlcbStarLibraries directory
SUBDIRS := $(filter-out OlcbStarLibraries, $(foreach V,$(wildcard */Makefile),$(V:/Makefile=)))

%.all:
	make -C $(@:.all=) all 

# make sure Arduino sketches build OK first
all: $(foreach v,$(SUBDIRS),$(v).all)
	# make new OpenLCB C++ library (no tests exist)
	cd libraries/OpenLCB; make lib
	# make the OlcbStarLibraries directory, including tests
	cd OlcbStarLibraries; make

