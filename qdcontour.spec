%define BINNAME qdcontour
Summary: qdcontour
Name: smartmet-%{BINNAME}
Version: 1.0.2
Release: 1.el5.fmi
License: FMI
Group: Development/Tools
URL: http://www.weatherproof.fi
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot-%(%{__id_u} -n)
BuildRequires: libsmartmet-newbase, libsmartmet-imagine, libsmartmet-tron, freetype-devel, libjpeg-devel, libpng-devel, zlib-devel, glibc-devel
Requires: freetype, libjpeg, libpng, zlib
Provides: qdcontour

%description
FMI qdcontour

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n %{BINNAME}
 
%build
make %{_smp_mflags} release

%install
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,0775)
%{_bindir}/qdcontour


%changelog
* Thu Jun  7 2007 tervo <tervo@xodin.weatherproof.fi> - 
- Initial build.

