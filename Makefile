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

# ToDo:  change to separate targets
# ToDo:  Automatically create "run" and "reset" command lists
# ToDo:  Only print name if test files (silent success!)

# run all tests
run: all
	@echo StreamTest; ./tests/StreamTest | diff - tests/results/StreamTest.out.txt
	@echo DatagramTest; ./tests/DatagramTest | diff - tests/results/DatagramTest.out.txt
	@echo NodeMemoryTest; ./tests/NodeMemoryTest | diff - tests/results/NodeMemoryTest.out.txt
	@echo ConfigurationTest; ./tests/ConfigurationTest | diff - tests/results/ConfigurationTest.out.txt
	@echo CanMrrlcbTest; ./tests/CanMrrlcbTest | diff - tests/results/CanMrrlcbTest.out.txt

# reset the comparison files; only if sure!
reset: 
	@./tests/StreamTest > tests/results/StreamTest.out.txt
	@./tests/DatagramTest > tests/results/DatagramTest.out.txt
	@./tests/NodeMemoryTest > tests/results/NodeMemoryTest.out.txt
	@./tests/ConfigurationTest > tests/results/ConfigurationTest.out.txt
	@./tests/CanMrrlcbTest > tests/results/CanMrrlcbTest.out.txt
