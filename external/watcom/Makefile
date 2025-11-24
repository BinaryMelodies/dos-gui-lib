
LIBDIR=../../bin/lib/win

all: $(LIBDIR)/win10s.o $(LIBDIR)/win10l.o $(LIBDIR)/win30s.o $(LIBDIR)/win30l.o

clean:
	rm -f *.o

distclean: clean
	rm -f *~

$(LIBDIR)/win30s.o: cstrtw16.asm
	mkdir -p `dirname $@`
	wasm -q `basename $< .asm` -bt=WINDOWS -ms -fo=$@ -i..

$(LIBDIR)/win30l.o: cstrtw16.asm
	mkdir -p `dirname $@`
	wasm -q `basename $< .asm` -bt=WINDOWS -ms -fo=$@ -i.. -ml

$(LIBDIR)/win10s.o: cstrtw16.asm
	mkdir -p `dirname $@`
	wasm -q `basename $< .asm` -bt=WINDOWS -ms -fo=$@ -d__WINDOWS1__=1 -i..

$(LIBDIR)/win10l.o: cstrtw16.asm
	mkdir -p `dirname $@`
	wasm -q `basename $< .asm` -bt=WINDOWS -ms -fo=$@ -d__WINDOWS1__=1 -i.. -ml

