SUBDIRS=admin m4 src plugins tools test po doc
SUBDIRS_DIST=admin m4 src plugins tools test doc po


AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = \
 README.W32 \
 README.DARWIN \
 README.SOLARIS \
 RELEASES \
 gwenhywfar.spec \
 Makefile.cvs \
 i18nsources

bin_SCRIPTS = gwenhywfar-config
M4dir = $(datadir)/aclocal
dist_M4_DATA = gwenhywfar.m4
ACLOCAL_AMFLAGS = -I m4


gwenhywfar-config: gwenhywfar-config.in
	echo "#! /bin/sh" >gwenhywfar-config && \
	echo 'dir="@prefix@"' >>gwenhywfar-config && \
	cat gwenhywfar-config.in >>gwenhywfar-config && \
	chmod a+x gwenhywfar-config

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gwenhywfar.pc

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
ISCC = "C:\Programme\Inno Setup 5\iscc.exe"
setup: gwenhywfar.iss
	mkdir -p distribution
	make prefix="`pwd`/distribution/win32-tmp" \
	     WIN_PATH_WINDOWS_MINGW="`pwd`/distribution/win32-tmp/windows" \
	     install \
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
	make -C po stamp-po
	make

CLEANFILES = gwenhywfar-config
DISTCLEANFILES = gwenhywfar-config.in

# Rule for only building the source rpm
srpm: $(PACKAGE).spec dist
	rpmbuild="rpm" && \
	if [ `rpm --version | awk '{ print $$3 }'` > /dev/null ]; then rpmbuild="rpmbuild"; fi && \
	$$rpmbuild --nodeps -ts $(PACKAGE)-$(VERSION).tar.gz


i18nsources: sources


if USE_I18N

# We directly read that file list. For old make programs, you would
# set this to an empty variable, because they are only needed by the
# developers and release technicians.
I18NFILES=$(shell cat $(srcdir)/i18nsources)
# Current build rule:
sources:
	rm -f $(srcdir)/i18nsources
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done
$(srcdir)/i18nsources:
	make sources

# Could also be solved this way: Read the I18NFILES variable from file
#include $(srcdir)/i18nsources
# (Needs the following build rule:)
#sources:
#	rm -f $(srcdir)/i18nsources
#	make -C $(top_srcdir)/src sources
#	mv $(srcdir)/i18nsources $(srcdir)/i18nsources.tmp
#	echo "I18NFILES = \\" > $(srcdir)/i18nsources
#	$(SED) 's!\(^.*$$\)!$$(top_srcdir)/\1 \\!' \
#	  $(srcdir)/i18nsources.tmp >> $(srcdir)/i18nsources
#	echo '$$(top_srcdir)/config.h' >> $(srcdir)/i18nsources
#	rm -f $(srcdir)/i18nsources.tmp
#	make Makefile

# The list of available languages (makes file name transformations much easier)
ALL_LINGUAS = $(basename $(notdir $(shell ls $(srcdir)/po/*.po)))
# For old make programs, the locales would directly be written here:
#ALL_LINGUAS = de

# List of po files. Only necessary in the dependency of the catalogs
# rule. For old make programs, you would simpy set this to an empty
# variable.
I18NLINGUAS = $(foreach lang,$(ALL_LINGUAS),$(srcdir)/po/$(lang).po)

# List of mo files. Only necessary for cleanup. For old make programs,
# you would simply set this to an empty variable.
I18NCATALOGS = $(foreach lang,$(ALL_LINGUAS),$(top_builddir)/po/$(lang).mo)
I18NCATALOGS += $(foreach lang,$(ALL_LINGUAS),$(top_builddir)/po/$(lang).qm)

po/$(PACKAGE).pot: $(I18NFILES)
	if test -n "$(I18NFILES)"; then \
	  $(XGETTEXT) -C -c -ki18n -ktr2i18n -kI18N \
	    -kI18N_NOOP -ktranslate -kaliasLocale \
            -ktr -ktrUtf8 \
	    '--msgid-bugs-address=openhbci-general@lists.sourceforge.net' \
	    -o po/$(PACKAGE).pot \
	    $(I18NFILES); \
	fi

# This rule is called automatically at install time. However, it is
# necessary only for the translators, so maybe it shouldn't be called
# at install time.
merge: po/$(PACKAGE).pot
	@catalogs=$(I18NLINGUAS); \
	for cat in $$catalogs; do \
	echo msgmerge -o $$cat.new $$cat ./po/$(PACKAGE).pot ; \
	msgmerge -o $$cat.new $$cat ./po/$(PACKAGE).pot ; \
	if test -s $$cat.new; then \
	  grep -v "\"POT-Creation" $$cat.new > $$cat.new.2 ; \
	  grep -v "\"POT-Creation" $$cat >> $$cat.new.1; \
	  if diff $$cat.new.1 $$cat.new.2; then \
		echo "  $$cat unchanged; leaving existing po file untouched."; \
		rm $$cat.new;  \
	  else  \
		mv $$cat.new $$cat ; \
	fi; \
	rm -f $$cat.new.1 $$cat.new.2 ;\
	fi ;\
	done

catalogs: $(I18NLINGUAS)
	@for i in $(ALL_LINGUAS); do \
	  filelingua=$(srcdir)/po/$$i.po; \
	  filecat=$(top_builddir)/po/$$i.mo; \
	  fileqm=$(top_builddir)/po/$$i.qm; \
	  echo "Creating translation catalogs for language \"$$i\""; \
	  $(MSGFMT) -o $${filecat} $${filelingua} || touch $${filelingua}; \
	done



localedir = $(datadir)/locale
install-mo-catalogs: catalogs
	@for i in $(ALL_LINGUAS); do \
	  filecat=$(top_builddir)/po/$$i.mo; \
	  fileqm=$(top_builddir)/po/$$i.qm; \
          echo "Installing translation catalog for language \"$$i\""; \
          $(mkinstalldirs) "$(DESTDIR)$(localedir)/$$i/LC_MESSAGES"; \
          $(INSTALL_DATA) $${filecat} $(DESTDIR)$(localedir)/$$i/LC_MESSAGES/$(PACKAGE).mo; \
        done
uninstall-mo-catalogs:
	@$(NORMAL_UNINSTALL)
	@for i in $(ALL_LINGUAS); do \
	  echo " rm -f '$(DESTDIR)$(localedir)/$$i/LC_MESSAGES/$(PACKAGE).mo'"; \
	  rm -f "$(DESTDIR)$(localedir)/$$i/LC_MESSAGES/$(PACKAGE).mo"; \
	done

install-data-local: install-mo-catalogs
uninstall-local: uninstall-mo-catalogs


endif


