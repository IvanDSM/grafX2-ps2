# see http://www.schaik.com/pngsuite/
URLBASE = http://www.schaik.com/pngsuite/
TARBALL = PngSuite-2017jul19.tgz
URL = $(URLBASE)$(TARBALL)
DESTDIR = pngsuite

GETURL = $(shell which wget || echo "$$(which curl) -O")
MKDIR = mkdir -p
TAR = tar

.PHONY:	all clean

all:	$(DESTDIR)/.ok

clean:
	$(RM) -r $(DESTDIR)
	$(RM) $(TARBALL)

$(DESTDIR)/.ok:	$(TARBALL)
	$(MKDIR) $(DESTDIR)
	cd $(DESTDIR) ; $(TAR) xzf ../$(TARBALL)
	touch $(DESTDIR)/.ok

$(TARBALL):
	$(GETURL) $(URL)
