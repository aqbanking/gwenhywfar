SUBDIRS=doc src test m4

EXTRA_DIST= \
 AUTHORS \
 COPYING \
 ChangeLog \
 Doxyfile.in \
 INSTALL \
 NEWS \
 README \
 README.W32 \
 gwenhywfar.m4 \
 gwenhywfar.spec.in 

bin_SCRIPTS = gwenhywfar-config
M4dir = $(datadir)/aclocal
M4_DATA = gwenhywfar.m4
ACLOCAL_AMFLAGS = -I m4


gwenhywfar-config: gwenhywfar-config.in
	echo "#! /bin/sh" >gwenhywfar-config && \
	echo 'dir="@prefix@"' >>gwenhywfar-config && \
	cat gwenhywfar-config.in >>gwenhywfar-config && \
	chmod a+x gwenhywfar-config

srcdoc: Doxyfile
	doxygen


rpm: gwenhywfar.spec
	@echo "Please use one of the targets below:"
	@echo " rpm-suse81     - build RPMs for SuSE 8.1"
	@echo " rpm-suse82     - build RPMs for SuSE 8.2"
	@echo " rpm-suse90     - build RPMs for SuSE 9.0"
	@echo " rpm-redhat80   - build RPMs for Redhat 8.0"
	@echo " rpm-redhat90   - build RPMs for Redhat 9.0"
	@echo " rpm-mandrake90 - build RPMs for Mandrake 9.0"

rpm-suse81: gwenhywfar.spec
	rpm -bb gwenhywfar.spec
	mkdir -p distribution
	cp /usr/src/packages/RPMS/i386/gwenhywfar-@rpm_version@-@rpm_release@.i386.rpm \
           `pwd`/distribution/gwenhywfar-@rpm_version@-@rpm_release@.i586.suse81.rpm
	cp /usr/src/packages/RPMS/i386/gwenhywfar-devel-@rpm_version@-@rpm_release@.i386.rpm \
           `pwd`/distribution/gwenhywfar-devel-@rpm_version@-@rpm_release@.i586.suse81.rpm

rpm-suse82: gwenhywfar.spec
	rpm -bb gwenhywfar.spec
	mkdir -p distribution
	cp /usr/src/packages/RPMS/i386/gwenhywfar-@rpm_version@-@rpm_release@.i386.rpm \
           `pwd`/distribution/gwenhywfar-@rpm_version@-@rpm_release@.i586.suse82.rpm
	cp /usr/src/packages/RPMS/i386/gwenhywfar-devel-@rpm_version@-@rpm_release@.i386.rpm \
           `pwd`/distribution/gwenhywfar-devel-@rpm_version@-@rpm_release@.i586.suse82.rpm

rpm-suse90: gwenhywfar.spec
	rpmbuild -bb gwenhywfar.spec
	mkdir -p distribution
	cp /usr/src/packages/RPMS/i586/gwenhywfar-@rpm_version@-@rpm_release@.i586.rpm \
           `pwd`/distribution/gwenhywfar-@rpm_version@-@rpm_release@.i586.suse90.rpm
	cp /usr/src/packages/RPMS/i586/gwenhywfar-devel-@rpm_version@-@rpm_release@.i586.rpm \
           `pwd`/distribution/gwenhywfar-devel-@rpm_version@-@rpm_release@.i586.suse90.rpm

rpm-redhat80: gwenhywfar.spec
	rpmbuild -ba gwenhywfar.spec
	mkdir -p distribution
	cp /usr/src/redhat/RPMS/i386/gwenhywfar-@rpm_version@-@rpm_release@.i386.rpm \
           `pwd`/distribution/gwenhywfar-@rpm_version@-@rpm_release@.i586.redhat80.rpm
	cp /usr/src/redhat/RPMS/i386/gwenhywfar-devel-@rpm_version@-@rpm_release@.i386.rpm \
           `pwd`/distribution/gwenhywfar-devel-@rpm_version@-@rpm_release@.i586.redhat80.rpm

rpm-redhat90: gwenhywfar.spec
	rpmbuild -ba gwenhywfar.spec
	mkdir -p distribution
	cp /usr/src/redhat/RPMS/i386/gwenhywfar-@rpm_version@-@rpm_release@.i386.rpm \
           `pwd`/distribution/gwenhywfar-@rpm_version@-@rpm_release@.i586.redhat90.rpm
	cp /usr/src/redhat/RPMS/i386/gwenhywfar-devel-@rpm_version@-@rpm_release@.i386.rpm \
           `pwd`/distribution/gwenhywfar-devel-@rpm_version@-@rpm_release@.i586.redhat90.rpm

rpm-mandrake90: gwenhywfar.spec
	rpmbuild -ba gwenhywfar.spec
	mkdir -p distribution
	cp /usr/src/RPM/RPMS/i586/gwenhywfar-@rpm_version@-@rpm_release@.i586.rpm \
           `pwd`/distribution/gwenhywfar-@rpm_version@-@rpm_release@.i586.mandrake90.rpm
	cp /usr/src/RPM/RPMS/i586/gwenhywfar-devel-@rpm_version@-@rpm_release@.i586.rpm \
           `pwd`/distribution/gwenhywfar-devel-@rpm_version@-@rpm_release@.i586.mandrake90.rpm

win32:
	mkdir -p distribution
	make prefix="`pwd`/distribution/win32-tmp" install && \
	 cd distribution && mkdir -p win32-devel && \
	 cp -r win32-tmp/bin win32-devel/ && \
	 cp -r win32-tmp/include win32-devel/ && \
	 cp -r win32-tmp/share win32-devel && \
	 cp ../README.W32 win32-devel/ && \
	 tar pczvf gwenhywfar-@rpm_version@-win32-devel.tgz win32-devel && \
	 rm -Rf win32-devel && \
	 rm -Rf win32-tmp
	mkdir -p distribution/win32-bin && \
	 cp -r src/gwenhywfar32.dll distribution/win32-bin/ && \
	 cp ../README.W32 win32-bin/ && \
	 strip distribution/win32-bin/*.dll && \
	 cd distribution && tar pczvf gwenhywfar-@rpm_version@-win32-bin.tgz win32-bin && \
	 rm -Rf win32-bin

