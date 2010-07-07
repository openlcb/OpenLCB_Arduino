# Simple Makefile for tests directory

BINFILES := ConfigurationTest StreamTest DatagramTest CanMrrlcbTest NodeMemoryTest

include ../standard.mk

#ToDo:  Better way of finding -L, -l so can move this to standard.mk

$(foreach v,$(BINFILES),$(OBJDIR)/$(v)):
	$(CXX) -L$(OBJDIR)/../src/openlcb -lopenlcb -L$(OBJDIR)/../scaffold -lscaffold $@.o -o $@


