Name: kdirstat
Summary: KDirStat - disk usage utility
Version: 1.7.8
Release: 1
Copyright: GPL
Group: X11/KDE/Development
Source: http://kdirstat.sourceforge.net/download/kdirstat-1.7.8-devel.tar.gz
Packager: Alexander Rawass <alexannika@users.sourceforge.net>
BuildRoot: /tmp/kd/kdirstat-1.7.8-devel
Prefix: /usr/local

%description
A du-tool that displays disk-usage information in a normal tree view
and with a treemap

%prep
rm -rf $RPM_BUILD_ROOT
%setup -n kdirstat-1.7.8-devel

%build
./configure --prefix=/usr/local
make

%install
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%{prefix}/share/apps/kdirstat/icons/locolor/16x16/actions/symlink.png
%{prefix}/share/apps/kdirstat/kdirstatui.rc
%{prefix}/share/applnk/Applications/kdirstat.desktop
%{prefix}/share/icons/locolor/32x32/apps/kdirstat.png
%{prefix}/share/icons/locolor/16x16/apps/kdirstat.png
%{prefix}/share/doc/HTML/en/kdirstat/index.docbook
%{prefix}/bin/kdirstat
