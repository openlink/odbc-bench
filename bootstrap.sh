#! /bin/sh
#
#  bootstrap.sh
#
#  $Id$
#
#  Generate Makefile.in and configure script for distribution
#
#  odbc-bench - a TPC-A and TPC-C like benchmark program for databases 
#  Copyright (C) 2000-2019 OpenLink Software <odbc-bench@openlinksw.com>
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

DIE=0

#
#  Check availability of build tools
#
echo "Checking build environment ..."

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`autoconf' installed on your system."
    echo
    DIE=1
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`automake' installed on your system."
    echo
    DIE=1
}

LIBTOOLIZE=
if test "x$LIBTOOLIZE" = "x"
then
	(libtoolize --version) > /dev/null 2>&1 && LIBTOOLIZE=libtoolize
fi
if test "x$LIBTOOLIZE" = "x"
then
	(glibtoolize --version) > /dev/null 2>&1 && LIBTOOLIZE=glibtoolize
fi
if test "x$LIBTOOLIZE" = "x"
then
    echo
    echo "**Error**: You must have \`libtool' installed on your system."
    echo
    DIE=1
fi

(gtk-config --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "*Warning*: You may need \`GTK' installed on your system."
    echo
}

(xml2-config --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "**Error**: You must have \`xml2' installed on your system."
    echo
    DIE=1
}

if test "$DIE" -eq 1
then
    echo
    echo "Please read the README.CVS file for a list of packages you need"
    echo "to install on your system before bootstrapping this project."
    echo
    echo "Bootstrap script aborting ..."
    exit 1
fi


#
#  Generate the build scripts
#
echo "Running $LIBTOOLIZE ..."
$LIBTOOLIZE --force --copy
if test $? -ne 0
then
    echo
    echo "Bootstrap script aborting ..."
    exit 1
fi

echo "Running aclocal ..."
aclocal -I admin
if test $? -ne 0
then
    echo
    echo "Bootstrap script aborting ..."
    exit 1
fi

echo "Running autoheader ..."
autoheader
if test $? -ne 0
then
    echo
    echo "Bootstrap script aborting ..."
    exit 1
fi

echo "Running automake ..."
automake --add-missing --copy --include-deps
if test $? -ne 0
then
    echo
    echo "Bootstrap script aborting ..."
    exit 1
fi

echo "Running autoconf ..."
autoconf
if test $? -ne 0
then
    echo
    echo "Bootstrap script aborting ..."
    exit 1
fi

echo
echo "Please check the INSTALL and README files for instructions to"
echo "configure, build and install odbc-bench on your system."
echo
echo "Certain build targets are only enabled in maintainer mode:"
echo
echo "    ./configure --enable-maintainer-mode ..."
echo
echo "Bootstrap script completed successfully."

exit 0
