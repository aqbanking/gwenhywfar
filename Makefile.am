SUBDIRS=doc src m4 plugins tools test
#DIST_SUBDIRS=test


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


rpm:    $(PACKAGE).spec dist
	rpmbuild="rpm" && \
	if [ `rpm --version | awk '{ print $$3 }'` > /dev/null ]; then rpmbuild="rpmbuild"; fi && \
	$$rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz

# Extra rule for easy generation of Win32 archives with headers and
# DLLs. The helper directories are configured by the following
# variables.
win_buildroot=distribution
win_develdir=$(PACKAGE)-$(VERSION)-win32-devel
win_bindir=$(PACKAGE)-$(VERSION)-win32-bin
EXTRA_WIN_DIST=AUTHORS ChangeLog README README.W32 COPYING
win32:
	mkdir -p distribution
	make prefix="`pwd`/distribution/win32-tmp" install && \
	 ( cd distribution && mkdir -p win32-devel && \
	 cp -r win32-tmp/bin win32-devel/ && \
	 cp -r win32-tmp/include win32-devel/ && \
	 cp -r win32-tmp/share win32-devel && \
	 cp ../AUTHORS ../ChangeLog ../README ../README.W32 ../COPYING win32-devel/ && \
	 tar pczvf gwenhywfar-@VERSION@-win32-devel.tgz win32-devel && \
	 rm -Rf win32-devel && \
	 rm -Rf win32-tmp )
	mkdir -p distribution/win32-bin && \
	 cp -r src/gwenhywfar32.dll distribution/win32-bin/ && \
	 cp AUTHORS ChangeLog README README.W32 COPYING distribution/win32-bin/ && \
	 strip distribution/win32-bin/*.dll && \
	 ( cd distribution && tar pczvf gwenhywfar-@VERSION@-win32-bin.tgz win32-bin && \
	 rm -Rf win32-bin )

win32-cs: dist
	$(am__remove_distdir)
	GZIP=$(GZIP_ENV) gunzip -c $(distdir).tar.gz | $(AMTAR) xf -
	chmod -R a-w $(distdir); chmod a+w $(distdir)
	mkdir $(distdir)/=build
	mkdir $(distdir)/=inst
	mkdir -p $(win_buildroot)
	mkdir -p $(win_buildroot)/$(win_develdir)
	mkdir -p $(win_buildroot)/$(win_bindir)
	chmod a-w $(distdir)
	dc_install_base=`$(am__cd) $(distdir)/=inst && pwd` \
	  && dc_destdir="$${TMPDIR-/tmp}/am-dc-$$$$/" \
	  && cd $(distdir)/=build \
	  && ../configure --srcdir=.. --prefix="$$dc_install_base" \
	    $(DISTCHECK_CONFIGURE_FLAGS) \
	  && $(MAKE) $(AM_MAKEFLAGS) \
	  && $(MAKE) $(AM_MAKEFLAGS) check \
	  && $(MAKE) $(AM_MAKEFLAGS) install \
	  && cp src/$(PACKAGE)32.dll ../../$(win_buildroot)/$(win_bindir) \
	  && cp src/$(PACKAGE)32.lib ../../$(win_buildroot)/$(win_develdir) \
	  && cd ../../$(win_buildroot) \
	  && cp -r "$$dc_install_base"/bin $(win_develdir) \
	  && cp -r "$$dc_install_base"/include $(win_develdir) \
	  && cp -r "$$dc_install_base"/share $(win_develdir) \
	  && cd .. \
	  && cp $(EXTRA_WIN_DIST) $(win_buildroot)/$(win_develdir) \
	  && cp $(EXTRA_WIN_DIST) $(win_buildroot)/$(win_bindir) \
	  && cd $(win_buildroot) \
	  && strip $(win_bindir)/*.dll \
	  && zip -9r $(win_develdir).zip $(win_develdir) \
	  && zip -9r $(win_bindir).zip $(win_bindir) \
	  && rm -Rf $(win_develdir) $(win_bindir)
	$(am__remove_distdir)

# Extra rule instead of "all" which creates the auto-generated headers
# *first* and only then starts "make all". This avoids duplicate
# compilation.
quick: config.h
	make -C src version.h types.h
	make -C src/os/$(GWEN_OS) system.h
	make

CLEANFILES = gwennet-config 
DISTCLEANFILES = gwennet-config.in

