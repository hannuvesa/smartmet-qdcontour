%define BINNAME qdcontour
Summary: qdcontour
Name: smartmet-%{BINNAME}
Version: 1.0.1
Release: 1.el5.fmi
License: FMI
Group: Development/Tools
URL: http://www.weatherproof.fi
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}
BuildPrereq: libsmartmet-newbase >= 1.0-1, libsmartmet-imagine >= 1.0-1, libsmartmet-tron => 1.0-1, libsmartmet-gpc >= 1.0-1, freetype,  libjpeg, libjpeg-devel, libpng-devel,   zlib, zlib-devel
#libpng10
Requires: libc.so.6()(64bit) libc.so.6(GLIBC_2.2.5)(64bit) libfreetype.so.6()(64bit) libgcc_s.so.1()(64bit) libgcc_s.so.1(GCC_3.0)(64bit) libjpeg.so.62()(64bit) libm.so.6()(64bit) libm.so.6(GLIBC_2.2.5)(64bit) libpng12.so.0()(64bit) libpng12.so.0(PNG12_0)(64bit) libstdc++.so.6()(64bit) libstdc++.so.6(CXXABI_1.3)(64bit) libstdc++.so.6(GLIBCXX_3.4)(64bit) libstdc++.so.6(GLIBCXX_3.4.5)(64bit) libz.so.1()(64bit) rtld(GNU_HASH)

%description
FMI qdcontour

%prep
rm -rf $RPM_BUILD_ROOT
mkdir $RPM_BUILD_ROOT

%setup -q -n %{BINNAME}
 
%build
make clean
make depend
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

