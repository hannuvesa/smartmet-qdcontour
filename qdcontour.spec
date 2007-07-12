Summary: qdcontour
Name: qdcontour
Version: 1.0
Release: 1
License: FMI
Group: Development/Tools
URL: http://www.weatherproof.fi
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}
Requires: newbase >= 1.0-1, imagine >= 1.0-1, tron => 1.0-1, gpc >= 1.0-1, freetype >= 2.1.4,  libjpeg, libjpeg-devel, libpng-devel >= 1.2.2, libpng10 => 1.0, zlib >= 1.1.4, zlib-devel >= 1.1.4

%description
FMI qdcontour

%prep
rm -rf $RPM_BUILD_ROOT
mkdir $RPM_BUILD_ROOT

%setup -q -n %{name}
 
%build
make clean
make depend
make %{_smp_mflags} 

%install
make install prefix="${RPM_BUILD_ROOT}"
mkdir -p ${RPM_BUILD_ROOT}/smartmet/src/c++/bin/qdcontour
cp -r . ${RPM_BUILD_ROOT}/smartmet/src/c++/bin/qdcontour

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,www,www,0775)
/smartmet/src/c++/bin/qdcontour
/usr/bin/qdcontour


%changelog
* Thu Jun  7 2007 tervo <tervo@xodin.weatherproof.fi> - 
- Initial build.

