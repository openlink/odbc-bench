#
#  Makefile.am
# 
#  odbc-bench - a TPC-A and TPC-C like benchmark program for databases 
#  Copyright (C) 2000-2012 OpenLink Software <odbc-bench@openlinksw.com>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

AUTOMAKE_OPTIONS	= gnu 1.6
SUBDIRS			= lib src scripts contrib docs

EXTRA_DIST		= \
	README.CVS LICENSE \
	bootstrap.sh \
	odbcbenchrc \
	admin/gtk.m4 \
	mac/Makefile \
	mac/Images/VSplitterDisabled.png \
	mac/Images/VSplitter.png \
	mac/Images/HRidges.png \
	mac/Images/HRidgesDisabled.png \
	mac/Images/VRidges.png \
	mac/Images/HSplitter.png \
	mac/Images/VRidgesDisabled.png \
	mac/Images/HSplitterDisabled.png \
	mac/Resources/ReadMe-10.3.rtf \
	mac/Resources/ReadMe-10.4.rtf \
	mac/Resources/ReadMe-10.5.rtf \
	mac/Resources/Welcome.rtf \
	mac/Resources/License.rtf \
	mac/Resources/odbc-bench.info \
	mac/gen-installer.sh \
	mac/Installer/Description.plist \
	mac/Installer/Info.plist \
	mac/odbc-bench.xcode/project.pbxproj \
	mac/English.lproj/main.xib \
	mac/English.lproj/InfoPlist.strings \
	mac/Info.plist 


# ----------------------------------------------------------------------
#
#  Maintainers only
# 
# ----------------------------------------------------------------------

MAINTAINERCLEANFILES	= Makefile.in aclocal.m4 configure


if MAINTAINER_MODE

#
#  Create Linux RPM's
#

RPMFLAGS=--define="_topdir `pwd`/distrib"

linux-rpm:
	$(mkinstalldirs) distrib/SOURCES distrib/SRPMS distrib/SPECS
	$(mkinstalldirs) distrib/BUILD distrib/RPMS/i386
	$(MAKE) dist-gzip
	cp $(PACKAGE)-$(VERSION).tar.gz distrib/SOURCES
	rpmbuild $(RPMFLAGS) -ba contrib/RedHat/odbc-bench.spec
	rpmbuild $(RPMFLAGS) --clean --rmsource contrib/RedHat/odbc-bench.spec

#
#  Create a tar file containing the binaries and support files
#
binary-tar:
	$(mkinstalldirs) distrib @build@
	$(MAKE) install-strip prefix=`pwd`/@build@
	tar cvf distrib/@build@.tar @build@
	gzip -9vf distrib/@build@.tar
	rm -rf @build@

#
#  Create a OpenLink taz file containing the binaries and support files
#
openlink-taz:
	$(mkinstalldirs) distrib @build@
	$(MAKE) install-strip prefix=`pwd`/@build@
	(cd @build@; tar cvf ../distrib/@build@.tar .)
	compress -f distrib/@build@.tar
	mv distrib/@build@.tar.Z distrib/@build@.taz
	rm -rf @build@

#
#  Create a source snapshot package
#
snapshot:
	$(mkinstalldirs) distrib
	$(MAKE) dist-gzip distdir=distrib/$(PACKAGE)-`date +"%Y%m%d"`

#
#  Create an official release package
#
release:
	$(mkinstalldirs) distrib
	$(MAKE) distcheck 
	mv $(PACKAGE)-$(VERSION).tar.gz distrib

#
#  Generate ChangeLog
#
changelog:
	cvs2cl --branches --no-wrap --utc --hide-filenames --prune

endif
