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
	@make -C test CPPFLAGS="$(CPPFLAGS)"

clean: FORCE 

FORCE:

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
