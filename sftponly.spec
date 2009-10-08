%define _version 1.0
%define _release 1

%define _openssh_version 5.1p1
%define _openssh_libexec_dir /usr/libexec/openssh

Summary: simple chroot() shell for sftp-only accounts
Name: sftponly
Version: %{_version}
Release: %{_release}
Group: System Environment/Shells
License: GPL
Source0: sftponly-%{_version}.tar.gz
Source1: openssh-%{_openssh_version}.tar.gz
URL: http://dozzie.jarowit.net/code/sftponly.git
Vendor: Stanislaw Klekot <dozzie@jarowit.net>
Packager: Stanislaw Klekot <dozzie@jarowit.net>
Prefix: %{_prefix}

%description
Simple zero-administration shell for chroot() accounts which require sftp
access and nothing more.

Home page (git repository) of sftponly is
http://dozzie.jarowit.net/code/sftponly.git

%prep
rm -rf $RPM_BUILD_DIR/sftponly-%{_version}
tar zxf %SOURCE0
cd sftponly-%{_version}
tar zxf %SOURCE1

%build
cd sftponly-%{_version}
make sftponly SFTPSERVER_DIR=%{_openssh_libexec_dir}
touch openssh-%{_openssh_version}.tar.gz
(cd openssh-%{_openssh_version} && ./configure --prefix=%{_prefix})
make sftp-server.so

%install
mkdir -p $RPM_BUILD_ROOT%{_sbindir}
mkdir -p $RPM_BUILD_ROOT%{_openssh_libexec_dir}
mkdir -p $RPM_BUILD_ROOT%{_defaultdocdir}/sftponly-%{_version}

cd sftponly-%{_version}
install -m 755 -s sftponly       $RPM_BUILD_ROOT%{_sbindir}
install -m 755 -s sftp-server.so $RPM_BUILD_ROOT%{_openssh_libexec_dir}
install -m 644 README            $RPM_BUILD_ROOT%{_defaultdocdir}/sftponly-%{_version}

%files
%{_sbindir}/sftponly
%{_openssh_libexec_dir}/sftp-server.so
%{_defaultdocdir}/sftponly-%{_version}/*

# %changelog
# no %changelog section
