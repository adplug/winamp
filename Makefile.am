SUBDIRS = src

BINARYDIST = $(srcdir)/README $(srcdir)/AUTHORS $(srcdir)/COPYING
BINARYBINS = $(srcdir)/NEWS src/.libs/in_adlib.dll

ZIP = zip
ZIPFLAGS = -j
ZIPFILE = $(PACKAGE)-$(VERSION).zip

binarydist:
	make
	$(ZIP) -l $(ZIPFLAGS) $(ZIPFILE) $(BINARYDIST)
	$(ZIP) $(ZIPFLAGS) $(ZIPFILE) $(BINARYBINS)
