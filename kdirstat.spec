#
# spec file for package kdirstat
#
# Copyright (c) 2006 SUSE LINUX Products GmbH, Nuernberg, Germany.
# This file and all modifications and additions to the pristine
# package are under the same license as the package itself.
#
# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

# norootforbuild

BuildRequires:	kdelibs3-devel

Name:		kdirstat
URL:		http://kdirstat.sourceforge.net
License:	GPL
Group:		Productivity/File utilities
Summary:	Graphical Directory Statistics for Used Disk Space
Version:	2.5.5
Release:	0
BuildRoot:	%{_tmppath}/%{name}-%{version}-build
Source0:	kdirstat-%{version}.tar.bz2

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

%if %suse_version < 1010
%suse_update_desktop_file %name Filesystem
%else
%suse_update_desktop_file -N "KDirStat" -G "Directory Statistics" %name Filesystem
%endif

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

