SUBDIRS=admin data m4 src plugins tools gui test po doc checks
DIST_SUBDIRS=admin data m4 src plugins tools gui test doc po checks


AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = config.rpath  \
 ChangeLog \
 ChangeLog.old \
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

$(gwenhywfar_symlinkdir)/gwenhywfar/version.h: $(top_builddir)/src/version.h
	make -C ` dirname $< ` ` basename $< `
	( cd $(gwenhywfar_symlinkdir)/gwenhywfar ; $(LN_S) -f ../../$< ` basename $@ ` )
$(gwenhywfar_symlinkdir)/gwenhywfar/types.h: $(top_builddir)/src/types.h
	make -C ` dirname $< ` ` basename $< `
	( cd $(gwenhywfar_symlinkdir)/gwenhywfar ; $(LN_S) -f ../../$< ` basename $@ ` )
$(gwenhywfar_symlinkdir)/gwenhywfar/system.h: $(top_builddir)/src/os/$(OS_TYPE)/system.h
	make -C ` dirname $< ` ` basename $< `
	( cd $(gwenhywfar_symlinkdir)/gwenhywfar ; $(LN_S) -f ../../$< ` basename $@ ` )

BUILT_SOURCES = $(gwenhywfar_symlinkdir)/gwenhywfar/version.h $(gwenhywfar_symlinkdir)/gwenhywfar/types.h $(gwenhywfar_symlinkdir)/gwenhywfar/system.h

gwenhywfar-config: gwenhywfar-config.in
	echo "#! /bin/sh" >gwenhywfar-config && \
	echo 'dir="@prefix@"' >>gwenhywfar-config && \
	cat gwenhywfar-config.in >>gwenhywfar-config && \
	chmod a+x gwenhywfar-config

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gwenhywfar.pc

listdoc.h:
	admin/mklistdoc -v -I $(top_srcdir)/src/base `find "$(top_builddir)/gwenhywfar3/gwenhywfar" -name "*.h"` >listdoc.h

srcdoc: Doxyfile listdoc.h
	doxygen

$(PACKAGE).tag: Doxyfile listdoc.h
	doxygen

install-srcdoc: $(PACKAGE).tag
	test -d "$(DESTDIR)@docpath@/$(PACKAGE)" || \
	  mkdir -p "$(DESTDIR)@docpath@/$(PACKAGE)"
	for f in apidoc/*; do \
	  cp $$f "$(DESTDIR)@docpath@/$(PACKAGE)/"; \
	done
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

# Rule for using the Inno Setup compiler. The path to iscc.exe is
# set manually here for now and can be overridden if you set an
# env variable with the same name.
ISCC = "C:\Program Files\Inno Setup 5\iscc.exe"
wintmpdir = distribution
setup: gwenhywfar.iss
	mkdir -p $(wintmpdir)
	make prefix="`pwd`/$(wintmpdir)/usr" \
	     WIN_PATH_WINDOWS_MINGW="`pwd`/$(wintmpdir)/usr/windows" \
	     install \
	 && $(ISCC) gwenhywfar.iss \
	 && rm -rf $(wintmpdir)


CLEANFILES = gwenhywfar-config
DISTCLEANFILES = gwenhywfar-config.in

# Rule for only building the source rpm
srpm: $(PACKAGE).spec dist
	rpmbuild="rpm" && \
	if [ `rpm --version | awk '{ print $$3 }'` > /dev/null ]; then rpmbuild="rpmbuild"; fi && \
	$$rpmbuild --nodeps -ts $(PACKAGE)-$(VERSION).tar.gz



if IF_MAKE_I18N_FILES

# We directly read that file list. For old make programs, you would
# set this to an empty variable, because they are only needed by the
# developers and release technicians.
I18NFILES=$(shell cat $(srcdir)/i18nsources)
DLGFILES=$(shell cat $(srcdir)/dlgsources)

# Current build rule:
sources:
	rm -f $(srcdir)/i18nsources $(srcdir)/dlgsources
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

static-sources:
	rm -f $(srcdir)/i18nsources $(srcdir)/dlgsources
	for d in src plugins; do \
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


po/$(PACKAGE).pot: i18nsources dlgsources $(I18NFILES)
	rm -f po/tmp*
	if test -n "$(I18NFILES)"; then \
	  $(XGETTEXT) -C -c -ki18n -ktr2i18n -kI18N -kI18S\
	    -kI18N_NOOP -ktranslate -kaliasLocale \
            -ktr -ktrUtf8 \
	    '--msgid-bugs-address=aqbanking-user@lists.aqbanking.de' \
	    -o po/tmppot \
	    $(I18NFILES) ; \
	fi
	for i in $(DLGFILES); do \
          po4a-gettextize -f xml -m $$i -p po/tmppot1 -o attributes=text && \
	  msghack -o po/tmppot2 --append po/tmppot po/tmppot1 && \
	  mv po/tmppot2 po/tmppot; \
	done
	mv po/tmppot po/$(PACKAGE).pot
	rm -f po/tmppot1


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
	  echo "Creating translation catalogs for language \"$$i\""; \
	  echo $(MSGFMT) -o $${filecat} $${filelingua} ; \
	  $(MSGFMT) -o $${filecat} $${filelingua} || touch $${filelingua}; \
	done

CLEANFILES += $(I18NCATALOGS)

install-mo-catalogs: catalogs
	@for i in $(ALL_LINGUAS); do \
	  filecat=$(top_builddir)/po/$$i.mo; \
          echo "Installing translation catalog for language \"$$i\""; \
          $(mkinstalldirs) "$(DESTDIR)$(localedir)/$$i/LC_MESSAGES"; \
          echo $(INSTALL_DATA) $${filecat} $(DESTDIR)$(localedir)/$$i/LC_MESSAGES/$(PACKAGE).mo; \
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

ChangeLog:
	git log --pretty="format:------------------------------------------------------------------%n%ai %an%n%s%n%b" >ChangeLog

dist-hook:
	rm -f ChangeLog 2>/dev/null
	$(MAKE) ChangeLog
