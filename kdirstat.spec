#
# spec file for package kdirstat
# 
# Copyright  (c)  2002  SuSE GmbH  Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
# 
# please send bugfixes or comments to feedback@suse.de.
#

# neededforbuild  kde3-devel-packages
# usedforbuild    aaa_base aaa_dir autoconf automake base bash bindutil binutils bison bzip compress cpio cpp cracklib cyrus-sasl db devs diffutils docbook-dsssl-stylesheets docbook_3 e2fsprogs fam file fileutils findutils flex freetype2 freetype2-devel gawk gcc gdbm gdbm-devel gettext glibc glibc-devel gpm gpp gppshare grep groff gzip iso_ent jade_dsl kbd kdelibs kdelibs-artsd kdelibs-devel less libgpp libjpeg liblcms libmng libmng-devel libpng libtiff libtool libxml2 libxml2-devel libxslt libxslt-devel libz m4 make man mesa mesa-devel mesaglut mesaglut-devel mesasoft mktemp modutils ncurses ncurses-devel net-tools netcfg openssl openssl-devel pam pam-devel patch perl ps qt qt-devel qt-extensions rcs readline rpm sendmail sh-utils shadow sp sp-devel strace syslogd sysvinit texinfo textutils timezone unzip util-linux vim xdevel xf86 xf86glu xf86glu-devel xshared

Name:         kdirstat
Copyright:    GPL
Group:        X11/KDE/Utilities
Summary:      Graphical directory statistics for used disk space
Version:      2.3.3
Release:      0
Source0:      kdirstat-2.3.3.tar.bz2

%description
KDirStat (for KDE Directory Statistics) is a utility program that sums up
disk usage for direcory trees - very much like the Unix 'du' command.
It also can help you clean up used space.

Authors:
--------
    Stefan Hundhammer <sh@suse.de>

SuSE series: kde

%prep
%setup -n kdirstat-2.3.3

%build
CXXFLAGS="$CXXFLAGS -DNDEBUG -O2 " ./configure \
  --prefix=/opt/kde3 \
  --with-qt-dir=/usr/lib/qt3/
make

%install
make install-strip

%files
%doc COPYING AUTHORS ChangeLog TODO INSTALL README 
/opt/kde3/bin/kdirstat
/opt/kde3/share/apps/kdirstat
/opt/kde3/share/applnk/*/kdirstat*
/opt/kde3/share/doc/HTML/*/kdirstat/
/opt/kde3/share/icons/??color/??x??/*/kdirstat*
/opt/kde3/share/locale/*/LC_MESSAGES/kdirstat.mo
