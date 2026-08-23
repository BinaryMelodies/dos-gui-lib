
# Template using GNU

OBJFILES=$(patsubst %.c,$(OBJDIR)/$(PLATFORM)/%$(LIBSUFFIX).o,$(notdir $(SRCFILES)))

$(BINDIR)/lib/$(PLATFORM)/libdtk$(LIBSUFFIX).a: $(OBJFILES) $(HDRFILES)
	mkdir -p `dirname $@`
	rm -f $@
	$(PREFIX)ar rcs $@ $(OBJFILES)

$(OBJDIR)/$(PLATFORM)/%$(LIBSUFFIX).o: $(SRCDIR)/%.c $(HDRFILES)
	mkdir -p `dirname $@`
	$(GCC) $(CFLAGS) $< -c -o$@ -I$(INCDIR) -I$(INCDIR)/$(USING)

$(OBJDIR)/$(PLATFORM)/%$(LIBSUFFIX).o: $(SRCDIR)/$(USING)/%.c $(HDRFILES)
	mkdir -p `dirname $@`
	$(GCC) $(CFLAGS) $< -c -o$@ -I$(INCDIR) -I$(INCDIR)/$(USING)

