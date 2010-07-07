
TOPDIR := $(PWD)
binaries := 
objects  := 
OBJDIR := $(TOPDIR)/lib
CPPFLAGS := -I$(PWD)/src/openlcb -I$(PWD)/scaffold $(CPPFLAGS)

SUBDIRS = scaffold src test

include standard.mk

.PHONY: all

# default target: clean & rebuild
all: clean compile lib bin test

# compile directories
.PHONY: clean compile lib bin test

#src: 
#	@make -C src CPPFLAGS="$(CPPFLAGS)" OBJDIRPREFIX="$(OBJDIR)"

#scaffold: 
#	@make -C scaffold CPPFLAGS="$(CPPFLAGS)" OBJDIRPREFIX="$(OBJDIR)"

#test: 
#	@make -C test CPPFLAGS="$(CPPFLAGS)" OBJDIRPREFIX="$(OBJDIR)"


# ToDo:  change to separate targets
# ToDo:  Automatically create "run" and "reset" command lists
# ToDo:  Only print name if test files (silent success!)

# run all tests
run: all
	@echo CanMrrlcbTest; ./test/CanMrrlcbTest | diff - test/results/CanMrrlcbTest.out.txt
	@echo StreamTest; ./test/StreamTest | diff - test/results/StreamTest.out.txt
	@echo DatagramTest; ./test/DatagramTest | diff - test/results/DatagramTest.out.txt
	@echo NodeMemoryTest; ./test/NodeMemoryTest | diff - test/results/NodeMemoryTest.out.txt
	@echo ConfigurationTest; ./test/ConfigurationTest | diff - test/results/ConfigurationTest.out.txt

# reset the comparison files; only if sure!
#reset: 
#	@./test/StreamTest > test/results/StreamTest.out.txt
#	@./test/DatagramTest > test/results/DatagramTest.out.txt
#	@./test/NodeMemoryTest > test/results/NodeMemoryTest.out.txt
#	@./test/ConfigurationTest > test/results/ConfigurationTest.out.txt
#	@./test/CanMrrlcbTest > test/results/CanMrrlcbTest.out.txt
