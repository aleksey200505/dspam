%define _man_dir %{_prefix}/man
%define _sh_man_dir %{_prefix}/share/man
#%define _init_dir /etc/rc.d/init.d

Summary: anti-spam solution
URL: http://dspam.nuclearelephant.org
Name: dspam
Version: 3.9.0
Release: 0.1
License: GPLv2
Provides: %{_bindir}/dspam
Packager: Jacob Leaver <jleaver@c-corp.net>
Vendor: C-Corp Centos
Group: Applications/Internet
Source0: http://dspam.nuclearelephant.com/sources/%{name}-%{version}-ALPHA2.tar.gz
Source1: dspam.init
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-ALPHA2-root
Requires: openldap
BuildRequires: gcc, sed, rpm >= 4, openldap-devel, mysql-devel, postgresql-devel sqlite-devel

%description
DSPAM is an open-source, freely available anti-spam solution
designed to combat unsolicited commercial email using advanced
statistical analysis.

%package mysql
Group: System Environment/Daemons
Summary: Mysql driver for DSPAM
Requires: dspam,mysql

%description mysql
Mysql driver for DSPAM

%package postgres
Group: System Environment/Daemons
Summary: Postresql driver for DSPAM
Requires: dspam,postgresql-libs

%description postgres
Postgresql driver for DSPAM

%package sqlite
Group: System Environment/Daemons
Summary: SQLite driver for DSPAM
Requires: dspam,sqlite

%description sqlite
SQLite driver for DSPAM

%package devel
Group: Development/Libraries
Summary: DPSAM resources for development
Requires: dspam

%description devel
DSPAM resources for development, including headers.

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
	--with-storage-driver=mysql_drv,pgsql_drv,sqlite3_drv,hash_drv \
#	--with-mysql-libraries=/usr/lib/mysql \
#	--with-mysql-includes=/usr/include/mysql \
        --with-dspam-mode=6755 \
        --enable-preferences-extension

make %{?_smp_mflags}

%install
make DESTDIR=%{buildroot} install

#%{__mkdir_p} $RPM_BUILD_ROOT{%_sysconfdir,%{_initrddir},%_bindir,%_includedir/dspam,%_man_dir/man1,%_man_dir/man3,%_sh_man_dir/man1,%_sh_man_dir/man3}

%{__install} -c %{_sourcedir}/dspam.init \
        ${RPM_BUILD_ROOT}%{_initrddir}/dspam
cp $RPM_BUILD_ROOT%{_sysconfdir}/dspam.conf \
        $RPM_BUILD_ROOT%{_sysconfdir}/dspam.conf.dist

%post
umask 022
chkconfig --add dspam
#chmod 755 %{_prefix}/share/doc/%{name}-%{version}

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-,root,root)
%config(noreplace) %{_sysconfdir}/dspam.conf
%{_sysconfdir}/dspam.conf.dist
{_initrddir}/dspam
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
%{_libdir}/libdspam*
%{_libdir}/libhash*
%{_libdir}/pkgconfig/dspam.pc
%_man_dir/man1/*
%_man_dir/man3/*
%doc CHANGELOG LICENSE README RELEASE.NOTES UPGRADING doc/*

%files mysql
%{_libdir}/libmysql*
%doc src/tools.mysql_drv/*sql

%files postgres
%{_libdir}/libpgsql*
%doc src/tools.pgsql_drv/*sql
%{_bindir}/dspam_pg2int8

%files sqlite
%{_libdir}/libsqlite*
%doc src/tools.sqlite_drv/*sql

%files devel
%{_includedir}/dspam/*

%changelog
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
