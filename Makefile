binaries := 
objects  := 
CPPFLAGS := -I$(PWD)/src/openlcb -I$(PWD)/scaffold $(CPPFLAGS)

# default target: clean & rebuild
all: clean scaffold src tests

# compile directories
src: FORCE
	@make -C src CPPFLAGS="$(CPPFLAGS)"

scaffold: FORCE
	@make -C scaffold CPPFLAGS="$(CPPFLAGS)"

tests: FORCE
	@make -C tests CPPFLAGS="$(CPPFLAGS)"

clean: FORCE 

FORCE:
