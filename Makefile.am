
if USE_NCURSES
gwenui_subdir = gwenui
else
gwenui_subdir = 
endif

SUBDIRS=admin doc m4 src gwenpp $(gwenui_subdir) plugins tools test po
SUBDIRS_DIST=admin doc m4 src gwenpp gwenui plugins tools test


AUTOMAKE_OPTIONS = foreign

EXTRA_DIST= config.rpath  \
 AUTHORS \
 COPYING \
 ChangeLog \
 Doxyfile.in \
 INSTALL \
 NEWS \
 README \
 README.W32 \
 README.DARWIN \
 README.SOLARIS \
 gwenhywfar.m4 \
 gwenhywfar.spec \
 gwenhywfar.spec.in \
 Makefile.cvs

bin_SCRIPTS = gwenhywfar-config
M4dir = $(datadir)/aclocal
M4_DATA = gwenhywfar.m4
ACLOCAL_AMFLAGS = -I m4


gwenhywfar-config: gwenhywfar-config.in
	echo "#! /bin/sh" >gwenhywfar-config && \
	echo 'dir="@prefix@"' >>gwenhywfar-config && \
	cat gwenhywfar-config.in >>gwenhywfar-config && \
	chmod a+x gwenhywfar-config

listdoc.h:
	admin/mklistdoc -v -I $(top_srcdir)/src/base @GWEN_HEADER_FILES@ >listdoc.h

srcdoc: Doxyfile listdoc.h
	doxygen

$(PACKAGE).tag: Doxyfile listdoc.h
	doxygen

install-srcdoc: $(PACKAGE).tag
	test -d "$(DESTDIR)@docpath@/$(PACKAGE)" || \
	  mkdir -p "$(DESTDIR)@docpath@/$(PACKAGE)"
	cp apidoc/* "$(DESTDIR)@docpath@/$(PACKAGE)"
	cp "$(PACKAGE).tag" "$(DESTDIR)@docpath@/"
	if test -x "$(DESTDIR)@docpath@/$(PACKAGE)/installdox"; then \
	  cd "$(DESTDIR)@docpath@/$(PACKAGE)" && \
	   ID_OPT="-q" ; \
	   for ff in ${DOXYGEN_TAGFILES}; do \
	    ID_OPT="$$ID_OPT -l `basename $$ff`@../`basename $$ff | $(SED) -e s/.tag//`";\
	   done ;\
	   ( cd "$(DESTDIR)@docpath@/$(PACKAGE)" && \
	      ./installdox -q $$ID_OPT \
	   ); \
	fi


rpm:    $(PACKAGE).spec dist
	rpmbuild="rpm" && \
	if [ `rpm --version | awk '{ print $$3 }'` > /dev/null ]; then rpmbuild="rpmbuild"; fi && \
	$$rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz

# Rule for using the Inno Setup compiler. The path to iscc.exe is set
# manually here for now.
ISCC = "C:\Programme\Inno Setup 4\iscc.exe"
setup: gwenhywfar.iss
	mkdir -p distribution
	make prefix="`pwd`/distribution/win32-tmp" install \
	 && $(ISCC) gwenhywfar.iss \
	 && rm -rf distribution/win32-tmp

# Extra rule for easy generation of Win32 archives with headers and
# DLLs. The helper directories are configured by the following
# variables.
win_buildroot=distribution
# The following two variables are both temporary directories and also
# the name of the resulting archive.
win_develdir=$(PACKAGE)-$(VERSION)-win32-devel
win_bindir=$(PACKAGE)-$(VERSION)-win32-bin
# The name of the actual DLL -- has to match the one in
# src/Makefile.am
win_LIBRARY_NAME = $(PACKAGE)32_$(GWENHYWFAR_SO_EFFECTIVE)
EXTRA_WIN_DIST=AUTHORS ChangeLog README README.W32 COPYING

# Target that uses the currently compiled result for installation
# (with the current ./configure settings)
win32:
	mkdir -p distribution
	make prefix="`pwd`/distribution/win32-tmp" install && \
	 ( cd distribution && mkdir -p win32-devel && \
	 cp -r win32-tmp/bin win32-devel/ && \
	 cp -r win32-tmp/include win32-devel/ && \
	 cp -r win32-tmp/share win32-devel && \
	 cp ../AUTHORS ../ChangeLog ../README ../README.W32 ../COPYING win32-devel/ && \
	 tar pczvf $(win_develdir).tgz win32-devel && \
	 rm -Rf win32-devel && \
	 rm -Rf win32-tmp )
	mkdir -p distribution/win32-bin && \
	 cp -r src/$(win_LIBRARY_NAME).dll distribution/win32-bin/ && \
	 cp AUTHORS ChangeLog README README.W32 COPYING distribution/win32-bin/ && \
	 strip distribution/win32-bin/*.dll && \
	 ( cd distribution && tar pczvf $(win_bindir).tgz win32-bin && \
	 rm -Rf win32-bin )

# Target that makes a full rebuild including ./configure
win32-cs-start: dist
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
	  && cp src/$(win_LIBRARY_NAME).dll ../../$(win_buildroot)/$(win_bindir) \
	  && cp src/$(win_LIBRARY_NAME).lib ../../$(win_buildroot)/$(win_develdir) \
	  && cd ../../$(win_buildroot) \
	  && cp -r "$$dc_install_base"/bin $(win_develdir) \
	  && cp -r "$$dc_install_base"/include $(win_develdir) \
	  && cp -r "$$dc_install_base"/share $(win_develdir) \
	  && cd .. \
	  && cp $(EXTRA_WIN_DIST) $(win_buildroot)/$(win_develdir) \
	  && cp $(EXTRA_WIN_DIST) $(win_buildroot)/$(win_bindir) \
	  && cd $(win_buildroot) \
	  && strip $(win_bindir)/*.dll \
	  && zip -r $(win_develdir).zip $(win_develdir) \
	  && zip -r $(win_bindir).zip $(win_bindir) \
	  && rm -Rf $(win_develdir) $(win_bindir)
	$(am__remove_distdir)

# Extra rule instead of "all" which creates the auto-generated headers
# *first* and only then starts "make all". This avoids duplicate
# compilation.
quick: config.h
	make -C src version.h types.h
	make -C src/os/$(GWEN_OS) system.h
	make

CLEANFILES = gwenhywfar-config
DISTCLEANFILES = gwenhywfar-config.in
