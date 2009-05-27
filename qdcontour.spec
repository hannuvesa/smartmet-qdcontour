%define BINNAME qdcontour
Summary: qdcontour
Name: smartmet-%{BINNAME}
Version: 9.5.28
Release: 1.el5.fmi
License: FMI
Group: Development/Tools
URL: http://www.weatherproof.fi
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot-%(%{__id_u} -n)
BuildRequires: freetype-devel
BuildRequires: glibc-devel
BuildRequires: libjpeg-devel
BuildRequires: libpng-devel
BuildRequires: libsmartmet-imagine >= 9.4.6-1
BuildRequires: libsmartmet-newbase >= 9.4.21-1
BuildRequires: libsmartmet-tron >= 8.10.23-1
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

