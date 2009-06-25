#=========== General info ===============
%bcond_without mysql
%bcond_without postgresql
%bcond_without sqlite
%bcond_without hash

Summary: Anti-spam solution
URL: http://dspam.nuclearelephant.org
Name: dspam
Version: 3.9.0
Release: 0.1%{dist}
License: GPLv2
Provides: %{_bindir}/dspam
Vendor: C-Corp Centos
Group: Applications/Internet
Source0: http://dspam.nuclearelephant.com/sources/%{name}-%{version}-ALPHA2.tar.gz
#Source1: dspam.init
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-ALPHA2-root
Requires: openldap
BuildRequires: gcc
BuildRequires: sed
BuildRequires: rpm >= 4
BuildRequires: openldap-devel
BuildRequires: chrpath

%description
DSPAM is an open-source, freely available anti-spam solution
designed to combat unsolicited commercial email using advanced
statistical analysis.

%if 0%{?with_mysql}
%package mysql
Group: Applications/Databases
Summary: Mysql driver for DSPAM
Requires: %{name} = %{version}-%{release}
BuildRequires: mysql-devel
%description mysql
Mysql driver for DSPAM
%define ac_with_mysql mysql_drv,
%else
%define ac_with_mysql ""
%endif

%if 0%{?with_postgresql}
%package postgres
Group: Applications/Databases
Summary: Postresql driver for DSPAM
Requires: %{name} = %{version}-%{release}
Requires: postgresql-libs
BuildRequires: postgresql-devel
%description postgres
Postgresql driver for DSPAM
%define ac_with_postgresql pgsql_drv,
%else
%define ac_with_postgresql ""
%endif

%if 0%{?with_sqlite}
%package sqlite
Group: Applications/Databases
Summary: SQLite driver for DSPAM
Requires:%{name} = %{version}-%{release}
Requires: sqlite-devel
BuildRequires: sqlite-devel
%description sqlite
SQLite driver for DSPAM
%define ac_with_sqlite sqlite3_drv,
%else
%define ac_with_sqlite ""
%endif

%if 0%{?with_hash}
%package hash
Group: Applications/Databases
Summary: Hash driver for DSPAM
Requires: %{name} = %{version}-%{release}
%description hash
Hash driver for DSPAM
%define ac_with_hash hash_drv
%else
%define ac_with_hash ""
%endif

%package devel
Group: Development/Libraries
Summary: DPSAM resources for development
Requires: dspam
%description devel
DSPAM resources for development, including headers.

%package doc
Group: Development/Libraries
Summary: DPSAM documentation for uses
Requires: dspam
%description doc
DPSAM documentation for uses.

%prep

%setup -n %{name}-%{version}-ALPHA2

%build
umask 022

CFLAGS="%{optflags}" CXXFLAGS="%{optflags}" \
  %configure --enable-daemon \
	--enable-debug \
	--enable-syslog \
	--enable-virtual-users \
	--enable-long-usernames \
	--with-storage-driver=%{ac_with_mysql}%{ac_with_postgresql}%{ac_with_sqlite}%{ac_with_hash} \
%if 0%{?with_mysql}
	--with-mysql-libraries=/usr/lib64/mysql \
	--with-mysql-includes=/usr/include/mysql \
%endif
        --with-dspam-mode=6755 \
        --enable-preferences-extension

make

%install
make DESTDIR=%{buildroot} install

# let's try to get rid of rpath
chrpath --delete %{buildroot}%{_bindir}/{dspam,dspam_stats,dspam_clean,dspam_2sql,dspam_dump,dspam_merge,dspam_crc,dspam_admin}
chrpath --delete %{buildroot}%{_libdir}/dspam/{libhash_drv.so,libmysql_drv.so}
#%{__mkdir_p} $RPM_BUILD_ROOT{%_sysconfdir,%{_initrddir},%_bindir,%_includedir/dspam,%_man_dir/man1,%_man_dir/man3,%_sh_man_dir/man1,%_sh_man_dir/man3}

install -m 755 -d %{buildroot}%{_mandir}
install -m 755 -d %{buildroot}%{_initrddir}
install -m 755 %{_sourcedir}/dspam.init %{buildroot}%{_initrddir}/dspam

%post
/sbin/ldconfig
/sbin/chkconfig --add dspam
/sbin/chkconfig dspam off

%postun -p /sbin/ldconfig

%preun
/sbin/chkconfig --del dspam

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-,root,root)
%config(noreplace) %attr(0644,root,root) %{_sysconfdir}/dspam.conf
%{_initrddir}/%{name}
%{_bindir}/dspam
%{_bindir}/dspamc
%{_bindir}/dspam_dump
%{_bindir}/dspam_clean
%{_bindir}/dspam_crc
%{_bindir}/dspam_stats
%{_bindir}/dspam_merge
%{_bindir}/dspam_2sql
%{_bindir}/dspam_admin
%{_bindir}/dspam_logrotate
%{_bindir}/dspam_train
%{_bindir}/css*
%{_mandir}/man1/*
%{_mandir}/man3/*
%attr(-, root, root) %doc CHANGELOG LICENSE README RELEASE.NOTES UPGRADING

%files doc
%defattr(0644,root,root)
#%attr(0755, root, root)
%doc doc/*.txt

%if 0%{?with_mysql}
%files mysql
%{_libdir}/%{name}/libmysql*
%doc src/tools.mysql_drv/*sql
%endif

%if 0%{?with_postgresql}
%files postgres
%{_libdir}/%{name}/libpgsql*
%doc src/tools.pgsql_drv/*sql
%{_bindir}/dspam_pg2int8
%endif

%if 0%{?with_sqlite}
%files sqlite
%{_libdir}/dspam/libsqlite*
%exclude %{_libdir}/dspam/libsqlite*.a
%doc src/tools.sqlite_drv/*sql
%endif

%if 0%{?with_hash}
%files hash
%{_libdir}/%{name}/libhash*
%exclude %{_libdir}/dspam/libhash*.a
%endif

%files devel
%{_includedir}/dspam/*
%{_libdir}/libdspam*
%{_libdir}/pkgconfig/dspam.pc

%changelog
* Thu Jun 25 2009 Popkov Aleksey <aleksey@psniip.ru> 3.9.0-0.1
- Start adapted spec file for Fedora and CentOs
- Added of case self files to spec file.

* Thu Aug 4 2006 Jacob Leaver <jleaver@c-corp.net> 3.6.8-2
- Used make install with a basedir for the majority of instalation
- Updated to 3.6.8
- Configured for use in my mail system
- compiled for Centos 4.3
- made dspam.conf non-overwriteable
- split up drivers and devel into separate packages.
- disabled ldap as ldap and mysql seem to be incompatible?
* Thu Aug 25 2005 Tony Earnshaw <tonye@billy.demon.nl> 3.6.0-1
- Add docs and other useful stuff to '%doc'
* Wed Aug 24 2005 Tony Earnshaw <tonye@billy.demon.nl> 3.6.0-1
- dspam 3.6-beta1
* Wed Jun 22 2005 Tony Earnshaw <tonye@billy.demon.nl> 3.5.0-1
- Initial package 
