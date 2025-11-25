
all:
	make -C 001
	make -C 002
	make -C 003

clean:
	make -C 001 clean
	make -C 002 clean
	make -C 003 clean

distclean: clean
	rm -rf *~
	make -C 001 distclean
	make -C 002 distclean
	make -C 003 distclean

.PHONY: all clean distclean

