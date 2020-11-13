SYS	?= unix

all:
	(cd src/; $(MAKE) -f sys/$(SYS)/Makefile)

clean distclean:
	(cd src/; $(MAKE) -f sys/$(SYS)/Makefile $@)
