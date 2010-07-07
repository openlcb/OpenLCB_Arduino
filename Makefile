
TOPDIR := $(PWD)
OBJDIR := $(TOPDIR)/obj
CPPFLAGS := -I$(PWD)/src/openlcb -I$(PWD)/scaffold $(CPPFLAGS)

SUBDIRS = scaffold src test

include standard.mk

.PHONY: all bin lib compile clean

# default target: clean & rebuild
all: clean compile lib bin

# ToDo:  Automatically create "run" and "reset" command lists
# ToDo:  Move trailing / into OBJDIR, so null name works
# ToDo:  Handle dependency on header files, so "clean" not needed every time

# run all tests
run: all
	$(OBJDIR)/test/CanMrrlcbTest | diff - test/results/CanMrrlcbTest.out.txt || echo CanMrrlcbTest failed
	$(OBJDIR)/test/StreamTest | diff - test/results/StreamTest.out.txt || echo StreamTest failed 
	$(OBJDIR)/test/DatagramTest | diff - test/results/DatagramTest.out.txt || echo DatagramTest failed
	$(OBJDIR)/test/NodeMemoryTest | diff - test/results/NodeMemoryTest.out.txt || echo NodeMemoryTest failed
	$(OBJDIR)/test/ConfigurationTest | diff - test/results/ConfigurationTest.out.txt || echo ConfigurationTest failed

# reset the comparison files; only if sure!
#reset: 
#	@./test/StreamTest > test/results/StreamTest.out.txt
#	@./test/DatagramTest > test/results/DatagramTest.out.txt
#	@./test/NodeMemoryTest > test/results/NodeMemoryTest.out.txt
#	@./test/ConfigurationTest > test/results/ConfigurationTest.out.txt
#	@./test/CanMrrlcbTest > test/results/CanMrrlcbTest.out.txt
