SUBDIRS=doc src test m4

AUTOMAKE_OPTIONS = foreign

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
 gwenhywfar.spec \
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


rpm:    dist
	rpmbuild="rpm" && \
	if [ `rpm --version | awk '{ print $$3 }'` > 4 ]; then rpmbuild="rpmbuild"; fi && \
	$$rpmbuild -ta gwenhywfar-@VERSION@.tar.gz

win32:
	mkdir -p distribution
	make prefix="`pwd`/distribution/win32-tmp" install && \
	 cd distribution && mkdir -p win32-devel && \
	 cp -r win32-tmp/bin win32-devel/ && \
	 cp -r win32-tmp/include win32-devel/ && \
	 cp -r win32-tmp/share win32-devel && \
	 cp ../AUTHORS ../ChangeLog ../README ../README.W32 ../COPYING win32-devel/ && \
	 tar pczvf gwenhywfar-@VERSION@-win32-devel.tgz win32-devel && \
	 rm -Rf win32-devel && \
	 rm -Rf win32-tmp
	mkdir -p distribution/win32-bin && \
	 cp -r src/gwenhywfar32.dll distribution/win32-bin/ && \
	 cp ../AUTHORS ../ChangeLog ../README ../README.W32 ../COPYING win32-bin/ && \
	 strip distribution/win32-bin/*.dll && \
	 cd distribution && tar pczvf gwenhywfar-@VERSION@-win32-bin.tgz win32-bin && \
	 rm -Rf win32-bin

