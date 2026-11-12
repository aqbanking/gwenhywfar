# "make installcheck" builds consumer.c against the installed Gwenhywfar
# and runs it: once with the flags from gwenhywfar-config, once with the
# flags from pkg-config, and once with find_package(gwenhywfar) of CMake.
# This checks the installed headers, library, gwenhywfar-config,
# gwenhywfar.pc and CMake package files.

EXTRA_DIST = consumer.c CMakeLists.txt

CMAKE = cmake
CTEST = ctest

# The installed library and gwenhywfar.pc are not in the default search
# paths. Windows looks for DLLs in PATH.
consumer_env = \
  PKG_CONFIG_PATH="$(libdir)/pkgconfig$${PKG_CONFIG_PATH:+:$$PKG_CONFIG_PATH}" \
  LD_LIBRARY_PATH="$(libdir)$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH}" \
  PATH="$(bindir):$$PATH"; \
  export PKG_CONFIG_PATH LD_LIBRARY_PATH PATH

# Builds consumer.c with the flags in $cflags and $libs as
# consumer-build/$name and runs it.
consumer_build = \
  $(CC) $(CPPFLAGS) $(CFLAGS) $$cflags -o consumer-build/$$name$(EXEEXT) \
    $(srcdir)/consumer.c $(LDFLAGS) $$libs && \
  consumer-build/$$name$(EXEEXT)

installcheck-local:
	rm -rf consumer-build
	$(MKDIR_P) consumer-build
	@echo "Building against the installed files with gwenhywfar-config"
	$(consumer_env); name=config; \
	  cflags=`$(bindir)/gwenhywfar-config --includes`; \
	  libs=`$(bindir)/gwenhywfar-config --libraries`; \
	  $(consumer_build)
	@echo "Building against the installed files with pkg-config"
	$(consumer_env); name=pkgconfig; \
	  cflags=`$(PKG_CONFIG) --cflags gwenhywfar`; \
	  libs=`$(PKG_CONFIG) --libs gwenhywfar`; \
	  $(consumer_build)
	@echo "Building against the installed files with CMake"
	$(consumer_env); \
	  CC="$(CC)" CFLAGS="$(CPPFLAGS) $(CFLAGS)" LDFLAGS="$(LDFLAGS)" \
	    $(CMAKE) -S $(srcdir) -B consumer-build/cmake \
	      -DCMAKE_PREFIX_PATH="$(prefix)" \
	      -DGWENHYWFAR_EXPECTED_VERSION="$(GWENHYWFAR_VERSION_STRING)" && \
	  $(CMAKE) --build consumer-build/cmake && \
	  cd consumer-build/cmake && \
	  $(CTEST) -C Debug --output-on-failure

clean-local:
	rm -rf consumer-build
