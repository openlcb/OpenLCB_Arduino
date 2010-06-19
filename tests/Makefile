# Simple Makefile for tests directory

include ../standard.mk

BINFILES := CanMrrlcbTest DatagramTest

default: clean compile $(BINFILES) 
	
DatagramTest:
	@$(CXX) -L../src/openlcb -lopenlcb -L../scaffold -lscaffold DatagramTest.o -o DatagramTest

CanMrrlcbTest:
	@$(CXX) -L../src/openlcb -lopenlcb -L../scaffold -lscaffold CanMrrlcbTest.o -o CanMrrlcbTest


