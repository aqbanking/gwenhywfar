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
consumer_cmakedir = \
  $(libdir)/cmake/gwenhywfar-$(GWENHYWFAR_VERSION_MAJOR).$(GWENHYWFAR_VERSION_MINOR)

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

# make exports the CC that ./configure writes into the Makefile to
# the commands it runs, and CMake would read that CC. With MSVC, that
# CC is Automake's compile wrapper with flags, which CMake cannot use
# as a compiler, so we must unset CC and let CMake find the compiler
# by itself. However, under MSYS2, CMake typically finds MinGW's cc
# before cl, so we must force CC=cl whenever ./configure detects MSVC.
if IS_MSVC
consumer_cmake_cc = CC=cl
endif

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
# Under "make -j distcheck" on macOS, the make that CMake runs finds
# the -j settings of the outer make in MAKEFLAGS and tries to take
# part in its parallel build, which fails with "read jobs pipe:
# Resource temporarily unavailable". So we must unset MAKEFLAGS,
# MFLAGS and MAKELEVEL for the CMake step.
	$(consumer_env); unset CC MAKEFLAGS MFLAGS MAKELEVEL; \
	  $(consumer_cmake_cc) $(CMAKE) -S $(srcdir) -B consumer-build/cmake \
	    -Dgwenhywfar_DIR="$(consumer_cmakedir)" \
	    -DGWENHYWFAR_EXPECTED_VERSION="$(GWENHYWFAR_VERSION_STRING)" \
	    -DCONSUMER_GENERATED_DIR="$(abs_builddir)/$(consumer_gen)" && \
	  $(CMAKE) --build consumer-build/cmake && \
	  cd consumer-build/cmake && \
	  $(CTEST) -C Debug --output-on-failure

clean-local:
	rm -rf consumer-build
