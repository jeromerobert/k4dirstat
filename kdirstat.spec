#
# spec file for package kdirstat (Version 2.4.2)
#
# Copyright (c) 2004 SuSE Linux AG, Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments via http://www.suse.de/feedback/
#

# norootforbuild
# neededforbuild  kde3-devel-packages

BuildRequires: aaa_base acl attr bash bind-utils bison bzip2 coreutils cpio cpp cracklib cvs cyrus-sasl db devs diffutils e2fsprogs file filesystem fillup findutils flex gawk gdbm-devel glibc glibc-devel glibc-locale gpm grep groff gzip info insserv kbd less libacl libattr libgcc libselinux libstdc++ libxcrypt m4 make man mktemp module-init-tools ncurses ncurses-devel net-tools netcfg openldap2-client openssl pam pam-modules patch permissions popt procinfo procps psmisc pwdutils rcs readline sed strace syslogd sysvinit tar tcpd texinfo timezone unzip util-linux vim zlib zlib-devel XFree86-Mesa XFree86-Mesa-devel XFree86-devel XFree86-libs arts arts-devel autoconf automake binutils expat fam fam-devel fontconfig fontconfig-devel freeglut freeglut-devel freetype2 freetype2-devel gcc gcc-c++ gdbm gettext glib2 glib2-devel gnome-filesystem kdelibs3 kdelibs3-devel libart_lgpl libart_lgpl-devel libidn libidn-devel libjpeg liblcms liblcms-devel libmng libmng-devel libpng libpng-devel libstdc++-devel libtiff libtool libxml2 libxml2-devel libxslt libxslt-devel openssl-devel pcre pcre-devel perl qt3 qt3-devel rpm update-desktop-files

Name:         kdirstat
URL:          http://kdirstat.sourceforge.net
License:      GPL
Group:        Productivity/File utilities
Summary:      Graphical Directory Statistics for Used Disk Space
Version:      2.4.2
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
update_admin

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
/opt/kde3/share/apps/kdirstat
/opt/kde3/share/applnk/*/kdirstat*
/opt/kde3/share/doc/HTML/*/kdirstat/
%dir /opt/kde3/share/icons/hicolor/16x16
%dir /opt/kde3/share/icons/hicolor/16x16/apps
%dir /opt/kde3/share/icons/hicolor/32x32
%dir /opt/kde3/share/icons/hicolor/32x32/apps
%dir /opt/kde3/share/icons/locolor/16x16/apps
%dir /opt/kde3/share/icons/locolor/32x32/apps
/opt/kde3/share/icons/??color/??x??/*/kdirstat*
%dir /opt/kde3/share/locale/de
%dir /opt/kde3/share/locale/de/LC_MESSAGES
%dir /opt/kde3/share/locale/fr
%dir /opt/kde3/share/locale/fr/LC_MESSAGES
%dir /opt/kde3/share/locale/hu
%dir /opt/kde3/share/locale/hu/LC_MESSAGES
%dir /opt/kde3/share/locale/ja
%dir /opt/kde3/share/locale/ja/LC_MESSAGES
/opt/kde3/share/locale/*/LC_MESSAGES/kdirstat.mo

%dir /opt/kde3/share/apps/kconf_update
/opt/kde3/share/apps/kconf_update/kdirstat.upd
/opt/kde3/share/apps/kconf_update/fix_move_to_trash_bin.pl

