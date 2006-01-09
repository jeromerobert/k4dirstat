#
# spec file for package kdirstat 
#
# Copyright (c) 2005 SUSE LINUX Products GmbH, Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments via http://www.suse.de/feedback/
#

# norootforbuild
# neededforbuild  kde3-devel-packages

BuildRequires: aaa_base acl attr bash bind-utils bison bzip2 coreutils cpio cpp cracklib cvs cyrus-sasl db devs diffutils e2fsprogs file filesystem fillup findutils flex gawk gdbm-devel glibc glibc-devel glibc-locale gpm grep groff gzip info insserv klogd less libacl libattr libgcc libnscd libselinux libstdc++ libxcrypt libzio m4 make man mktemp module-init-tools ncurses ncurses-devel net-tools netcfg openldap2-client openssl pam pam-modules patch permissions popt procinfo procps psmisc pwdutils rcs readline sed strace syslogd sysvinit tar tcpd texinfo timezone unzip util-linux vim zlib zlib-devel arts arts-devel autoconf automake binutils expat fam fam-devel fontconfig fontconfig-devel freeglut freeglut-devel freetype2 freetype2-devel gcc gcc-c++ gdbm gettext glib2 glib2-devel gnome-filesystem jack jack-devel kdelibs3 kdelibs3-devel kdelibs3-doc libart_lgpl libart_lgpl-devel libgcrypt libgcrypt-devel libgpg-error libgpg-error-devel libidn libidn-devel libjpeg libjpeg-devel liblcms liblcms-devel libmng libmng-devel libpng libpng-devel libstdc++-devel libtiff libtiff-devel libtool libxml2 libxml2-devel libxslt libxslt-devel openssl-devel pcre pcre-devel perl python qt3 qt3-devel rpm unsermake update-desktop-files xorg-x11-Mesa xorg-x11-Mesa-devel xorg-x11-devel xorg-x11-libs

Name:         kdirstat
URL:          http://kdirstat.sourceforge.net
License:      GPL
Group:        Productivity/File utilities
Summary:      Graphical Directory Statistics for Used Disk Space
Version:      2.5.2
Release:      0
BuildRoot:    %{_tmppath}/%{name}-%{version}-build
Source0:      kdirstat-%{version}.tar.bz2

%description
KDirStat (KDE Directory Statistics) is a utility program that sums up
disk usage for directory trees - very much like the Unix 'du' command.
It can also help you clean up used space.



Authors:
--------
    Stefan Hundhammer <sh@suse.de>

%prep
%setup -q
. /etc/opt/kde3/common_options
update_admin --no-unsermake

%build
. /etc/opt/kde3/common_options
./configure $configkde --disable-final
make

%install
. /etc/opt/kde3/common_options
make DESTDIR=$RPM_BUILD_ROOT $INSTALL_TARGET
%suse_update_desktop_file %name Filesystem
%find_lang %name

%files -f %name.lang
%defattr(-,root,root)
%doc COPYING AUTHORS ChangeLog TODO README
/opt/kde3/bin/kdirstat
/opt/kde3/bin/kdirstat-cache-writer
/opt/kde3/share/apps/kdirstat
/opt/kde3/share/appl*/*/kdirstat*
/opt/kde3/share/doc/HTML/*/kdirstat/
%dir /opt/kde3/share/icons/hicolor/16x16
%dir /opt/kde3/share/icons/hicolor/16x16/apps
%dir /opt/kde3/share/icons/hicolor/32x32
%dir /opt/kde3/share/icons/hicolor/32x32/apps
%dir /opt/kde3/share/icons/locolor/16x16/apps
%dir /opt/kde3/share/icons/locolor/32x32/apps
/opt/kde3/share/icons/??color/??x??/*/kdirstat*
%dir /opt/kde3/share/apps/kconf_update
/opt/kde3/share/apps/kconf_update/kdirstat.upd
/opt/kde3/share/apps/kconf_update/fix_move_to_trash_bin.pl

