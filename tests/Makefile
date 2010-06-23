# Simple Makefile for tests directory

include ../standard.mk

BINFILES := ConfigurationTest StreamTest DatagramTest CanMrrlcbTest 

default: clean compile $(BINFILES) 
	
CanMrrlcbTest:
	@$(CXX) -L../src/openlcb -lopenlcb -L../scaffold -lscaffold CanMrrlcbTest.o -o CanMrrlcbTest

DatagramTest:
	@$(CXX) -L../src/openlcb -lopenlcb -L../scaffold -lscaffold DatagramTest.o -o DatagramTest

StreamTest:
	@$(CXX) -L../src/openlcb -lopenlcb -L../scaffold -lscaffold StreamTest.o -o StreamTest

ConfigurationTest:
	@$(CXX) -L../src/openlcb -lopenlcb -L../scaffold -lscaffold ConfigurationTest.o -o ConfigurationTest

