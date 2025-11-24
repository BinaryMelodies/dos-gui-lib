
all:
	make -C external/watcom
	make -C src/lib
	make -C src/tests

clean:
	rm -rf bin obj
	make -C src/lib clean
	make -C src/tests clean

distclean: clean
	rm -rf *~
	make -C src/lib distclean
	make -C src/tests distclean

.PHONY: all clean distclean

