SUBDIRS = src

BINARYDIST = $(srcdir)/README $(srcdir)/AUTHORS $(srcdir)/COPYING
BINARYBINS = $(srcdir)/NEWS src/.libs/in_adlib.dll

ZIP = zip
ZIPFLAGS = -j
ZIPFILE = $(PACKAGE)-$(VERSION).zip

MAKEPIMP = wine makepimp

bindistprep:
	make
	$(STRIP) src/.libs/in_adlib.dll

zipdist: bindistprep
	$(ZIP) -l $(ZIPFLAGS) $(ZIPFILE) $(BINARYDIST)
	$(ZIP) $(ZIPFLAGS) $(ZIPFILE) $(BINARYBINS)

pimpdist: bindistprep
	make
# MakePimp is unable to read from src/.libs, since .libs is an invalid
# path for a Windows program. Thus, we temporarily copy the file to a
# saner path.
	cp src/.libs/in_adlib.dll .
	$(MAKEPIMP) in_adlib.psc
	rm in_adlib.dll

binarydist: zipdist pimpdist
