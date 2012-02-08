%define BINNAME qdcontour
Summary: qdcontour
Name: smartmet-%{BINNAME}
Version: 12.2.8
Release: 1.el5.fmi
License: FMI
Group: Development/Tools
URL: http://www.weatherproof.fi
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot-%(%{__id_u} -n)
BuildRequires: boost-devel >= 1.47
BuildRequires: freetype-devel
BuildRequires: glibc-devel
BuildRequires: libjpeg-devel
BuildRequires: libpng-devel
BuildRequires: libsmartmet-imagine >= 11.10.17-2
BuildRequires: libsmartmet-newbase >= 12.2.8-1
BuildRequires: libsmartmet-tron >= 11.7.20-1
BuildRequires: zlib-devel
Requires: freetype
Requires: libjpeg
Requires: libpng
Requires: zlib
Provides: qdcontour

%description
FMI qdcontour

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n %{BINNAME}
 
%build
make %{_smp_mflags}
make --quiet test

%install
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,0775)
%{_bindir}/qdcontour


%changelog
* Wed Feb  8 2012 mheiskan <mika.heiskanen@fmi.fi> - 12.2.8-1.el6.fmi
- New parameternames: IceSpeed, IceDirection
* Tue Feb  7 2012 mheiskan <mika.heiskanen@fmi.fi> - 12.2.7-1.el5.fmi
- New parametername Friction
- Fixed wind chill formula
* Fri Nov 25 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.11.25-1.el5.fmi
- fillarrow has no effect on meteorological arrows anymore
* Thu Nov 24 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.11.24-1.el5.fmi
- Upgraded to latest newbase which defines RadarBorder parameter
* Mon Oct 17 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.10.17-1.el5.fmi
- Upgraded to latest newbase
* Wed Oct  5 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.10.5-1.el5.fmi
- Upgraded to latest newbase
* Thu Sep 15 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.9.15-1.el5.fmi
- Returned graticule command which was lost in some cvs merge or something
* Wed Jul 20 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.7.20-1.el5.fmi
- Upgrade to boost 1.47
* Fri Jul  8 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.7.8-1.el5.fmi
- Recompiled to get new biomass parameter names into use
* Fri May 20 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.5.20-1.el6.fmi
- RHEL6 release
* Mon Mar 28 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.3.28-2.el5.fmi
- Flipped metorological arrows to get the flags on the correct side
- Fixed meteorological arrow speed values
* Thu Mar 24 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.5.20-1.el5.fmi
- Fixed meteorological arrows
- Upgrade to boost 1.46 and new base libraries
* Thu Mar 17 2011 keranen <pekka.keranen@geosaaga.fi> - 11.3.17-1.el6.fmi
- RHEL6 build with compilation fixes
* Wed Feb  2 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.2.2-1.el5.fmi
- Updated to latest newbase with new parameter names
* Tue Oct 12 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.10.12-1.el5.fmi
- timestampbackground is now by default 185,185,185,185
* Mon Jul  5 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.7.5-1.el5.fmi
- Upgrade to newbase 10.7.5-1
* Mon Jun  7 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.6.7-1.el5.fmi
- Output directories are now created automatically
- Now supports pkj projection via new newbase
* Wed Jan 20 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.1.20-1.el5.fmi
- Added the "overlay" command
* Fri Jan 15 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.1.15-1.el5.fmi
- Upgrade to boost 1.41
* Thu Jan  7 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.1.7-1.el5.fmi
- Fixed label rendering (imagine lib)
* Tue Jul 14 2009 mheiskan <mika.heiskanen@fmi.fi> - 9.7.14-1.el5.fmi
- Upgrade to boost 1.39
* Fri Jun 12 2009 mheiskan <mika.heiskanen@fmi.fi> - 9.6.12-1.el5.fmi
- Added datehour timestamp formatting for YLE
* Thu May 28 2009 mheiskan <mika.heiskanen@fmi.fi> - 9.5.28-1.el5.fmi
- Added roundarrow support
* Fri Jan 23 2009 mheiskan <mika.heiskanen@fmi.fi> - 9.1.23-1.el5.fmi
- Recompile for recognizing new newbase parameternames
* Tue Dec 30 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.12.30-1.el5.fmi
- Added safety checks for polar regions in wind arrow calculations
* Mon Dec 29 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.12.29-1.el5.fmi
- Fixed wind arrows to calculate north correctly
- Added graticule command
* Wed Oct  8 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.10.8-1.el5.fmi
- Bugfix in tron polygon builder
* Mon Sep 29 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.9.29-1.el5.fmi
- Newbase headers changed
* Wed Sep 24 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.9.24-1.el5.fmi
- Linked statically with boost 1.36
- Improved regression tests
* Mon May 19 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.5.19-1.el5.fmi
- Added contourlinewidth command
* Fri May 16 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.5.16-1.el5.fmi
- Fixed a memory leak caused by rounding errors in creating meteorological arrows
* Tue Mar 11 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.3.11-1.el5.fmi
- Linked with latest newbase including wind interpolation fixes
* Wed Jan 30 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.1.30-1.el5.fmi
- New roadmodel parameter names are now recognized
* Fri Dec 28 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.6-1.el5.fmi
- Linked with fixed newbase
* Fri Nov 30 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.5-1.el5.fmi
- Linked with newest libraries
* Mon Nov 19 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.4-1.el5.fmi
- Linked with newest libraries
* Thu Nov 15 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.3-1.el5.fmi
- Linked with newest libraries
* Fri Oct 19 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.2-1.el5.fmi
- Added "contourlabeltext" command.
* Mon Sep 24 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.1-4.el5.fmi
- Fixed "make depend".
* Fri Sep 14 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.1-3.el5.fmi
- Added "make tag" feature.
* Thu Sep 13 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.1-2.el5.fmi
- Improved make system.
* Thu Jun  7 2007 tervo <tervo@xodin.weatherproof.fi> - 
- Initial build.

