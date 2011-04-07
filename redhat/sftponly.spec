%define _version 1.0
%define _release 1

Summary: simple chroot() shell for sftp-only accounts
Name: sftponly
Version: %{_version}
Release: %{_release}
Group: System Environment/Shells
License: GPL
Source0: sftponly-%{_version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-root
URL: http://dozzie.jarowit.net/trac/wiki/sftponly
Vendor: Stanislaw Klekot <dozzie@jarowit.net>
Packager: Stanislaw Klekot <dozzie@jarowit.net>
Prefix: %{_prefix}

%description
Simple zero-administration shell for chroot() accounts which require sftp
access and nothing more.

Home page of sftponly is http://dozzie.jarowit.net/trac/wiki/sftponly

%prep
rm -rf $RPM_BUILD_DIR/sftponly-%{_version}
tar zxf %SOURCE0
cd sftponly-%{_version}

%build
cd sftponly-%{_version}
make all PREFIX=%{_prefix} SYSCONFDIR=%{_sysconfdir}

%install

cwd=${PWD}
cd sftponly-%{_version}
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf "$RPM_BUILD_ROOT"
make install DESTDIR="$RPM_BUILD_ROOT" \
  PREFIX=%{_prefix} \
  SYSCONFDIR=%{_sysconfdir}
mkdir -p $RPM_BUILD_ROOT%{_defaultdocdir}/sftponly-%{_version}
install -m 644 README \
  $RPM_BUILD_ROOT%{_defaultdocdir}/sftponly-%{_version}

%files
%attr(4755, root, root) %{_prefix}/bin/sftponly
%attr(755,  root, root) %{_prefix}/lib/sftponly/libchroot.so
%{_defaultdocdir}/sftponly-%{_version}/*

# %changelog
# no %changelog section
