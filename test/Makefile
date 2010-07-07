# Simple Makefile for tests directory

BINFILES := ConfigurationTest StreamTest DatagramTest CanMrrlcbTest NodeMemoryTest

include ../standard.mk
	
OBJDIR := ${OBJDIRPREFIX}/test

CanMrrlcbTest:
	@$(CXX) -L../src/openlcb -lopenlcb -L../scaffold -lscaffold CanMrrlcbTest.o -o CanMrrlcbTest

DatagramTest:
	@$(CXX) -L../src/openlcb -lopenlcb -L../scaffold -lscaffold DatagramTest.o -o DatagramTest

StreamTest:
	@$(CXX) -L../src/openlcb -lopenlcb -L../scaffold -lscaffold StreamTest.o -o StreamTest

ConfigurationTest:
	@$(CXX) -L../src/openlcb -lopenlcb -L../scaffold -lscaffold ConfigurationTest.o -o ConfigurationTest

NodeMemoryTest:
	@$(CXX) -L../src/openlcb -lopenlcb -L../scaffold -lscaffold NodeMemoryTest.o -o NodeMemoryTest

