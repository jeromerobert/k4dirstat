#
# spec file for package kdirstat (Version 2.4.0)
#
# Copyright (c) 2003 SuSE Linux AG, Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments via http://www.suse.de/feedback/
#

# neededforbuild  kde3-devel-packages
# usedforbuild    aaa_base acl attr bash bind9-utils bison coreutils cpio cpp cvs cyrus-sasl2 db devs diffutils e2fsprogs file filesystem fillup findutils flex gawk gdbm-devel glibc glibc-devel glibc-locale gpm grep groff gzip info insserv kbd less libacl libattr libgcc libstdc++ libxcrypt m4 make man mktemp modutils ncurses ncurses-devel net-tools netcfg pam pam-devel pam-modules patch permissions ps rcs readline sed sendmail shadow strace syslogd sysvinit tar texinfo timezone unzip util-linux vim zlib zlib-devel XFree86-devel XFree86-libs arts arts-devel autoconf automake binutils bzip2 cracklib expat fam fam-devel freetype2 freetype2-devel gcc gcc-c++ gdbm gettext kdelibs3 kdelibs3-devel libart_lgpl libart_lgpl-devel libjpeg liblcms liblcms-devel libmng libmng-devel libpng libpng-devel libstdc++-devel libtiff libtool libxml2 libxml2-devel libxslt libxslt-devel mesa mesa-devel mesaglu mesaglu-devel mesaglut mesaglut-devel mesasoft openssl openssl-devel perl qt3 qt3-devel rpm

Name:         kdirstat
License:      GPL
Group:        Productivity/File utilities
Summary:      Graphical directory statistics for used disk space
Version:      2.4.0
Release:      0
BuildRoot:    %{_tmppath}/%{name}-%{version}-build
Source0:      kdirstat-%{version}.tar.bz2

%description
KDirStat (for KDE Directory Statistics) is a utility program that sums up
disk usage for direcory trees - very much like the Unix 'du' command.
It also can help you clean up used space.

Authors:
--------
    Stefan Hundhammer <sh@suse.de>

%prep
%setup -q
. /etc/opt/kde3/common_options
update_admin

%build
. /etc/opt/kde3/common_options
./configure $configkde --disable-final
make

%install
make DESTDIR=$RPM_BUILD_ROOT install-strip

%files
%doc COPYING AUTHORS ChangeLog TODO INSTALL README 
/opt/kde3/bin/kdirstat
/opt/kde3/share/apps/kdirstat
/opt/kde3/share/applnk/*/kdirstat*
/opt/kde3/share/doc/HTML/*/kdirstat/
/opt/kde3/share/icons/??color/??x??/*/kdirstat*
/opt/kde3/share/locale/*/LC_MESSAGES/kdirstat.mo

