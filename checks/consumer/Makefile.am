# "make installcheck" builds consumer.c against the installed Gwenhywfar
# and runs it: once with the flags from gwenhywfar-config, once with the
# flags from pkg-config, and once with find_package(gwenhywfar) of CMake.
# This checks the installed headers, library, gwenhywfar-config,
# gwenhywfar.pc and CMake package files.
#
# Each build also compiles the code that the installed typemaker and
# typemaker2 generate from consumer_type1.xml and consumer_type2.xml.
# This checks that the generated code does not use headers exclusive to
# Gwenhywfar's own build.

EXTRA_DIST = consumer.c consumer_type1.xml consumer_type2.xml CMakeLists.txt

CMAKE = cmake
CTEST = ctest

# The installed library and gwenhywfar.pc are not in the default search
# paths. Windows looks for DLLs in PATH.
consumer_env = \
  PKG_CONFIG_PATH="$(libdir)/pkgconfig$${PKG_CONFIG_PATH:+:$$PKG_CONFIG_PATH}" \
  LD_LIBRARY_PATH="$(libdir)$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH}" \
  PATH="$(bindir):$$PATH"; \
  export PKG_CONFIG_PATH LD_LIBRARY_PATH PATH

consumer_gen = consumer-build/gen
consumer_gen_sources = \
  $(consumer_gen)/consumer_type1.c $(consumer_gen)/consumer_type2.c

# Builds consumer.c and the generated code with the flags in $cflags and
# $libs as consumer-build/$name and runs it.
consumer_build = \
  $(CC) $(CFLAGS) $$cflags -I$(consumer_gen) \
    -o consumer-build/$$name$(EXEEXT) \
    $(srcdir)/consumer.c $(consumer_gen_sources) $(LDFLAGS) $$libs && \
  consumer-build/$$name$(EXEEXT)

installcheck-local:
	rm -rf consumer-build
	$(MKDIR_P) $(consumer_gen)
	@echo "Generating code with the installed typemaker and typemaker2"
	$(consumer_env); cd $(consumer_gen) && \
	  $(bindir)/typemaker $(abs_srcdir)/consumer_type1.xml && \
	  $(bindir)/typemaker2 $(abs_srcdir)/consumer_type2.xml
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
	  CC="$(CC)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" \
	    $(CMAKE) -S $(srcdir) -B consumer-build/cmake \
	      -DCMAKE_PREFIX_PATH="$(prefix)" \
	      -DGWENHYWFAR_EXPECTED_VERSION="$(GWENHYWFAR_VERSION_STRING)" \
	      -DCONSUMER_GENERATED_DIR="$(abs_builddir)/$(consumer_gen)" && \
	  $(CMAKE) --build consumer-build/cmake && \
	  cd consumer-build/cmake && \
	  $(CTEST) -C Debug --output-on-failure

clean-local:
	rm -rf consumer-build
