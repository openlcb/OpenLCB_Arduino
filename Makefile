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

# run known tests; silent if no errors
run: all
	@./tests/ConfigurationTest | diff - tests/results/ConfigurationTest.out.txt
	@./tests/StreamTest | diff - tests/results/StreamTest.out.txt
	@./tests/DatagramTest | diff - tests/results/DatagramTest.out.txt
	@./tests/CanMrrlcbTest | diff - tests/results/CanMrrlcbTest.out.txt

# reset the comparison files; only if sure!
reset: 
	@./tests/ConfigurationTest > tests/results/ConfigurationTest.out.txt
	@./tests/StreamTest > tests/results/StreamTest.out.txt
	@./tests/DatagramTest > tests/results/DatagramTest.out.txt
	@./tests/CanMrrlcbTest > tests/results/CanMrrlcbTest.out.txt
