Building Gwenhywfar with CMake
===============================

This document describes how to build Gwenhywfar using the CMake
build system.  CMake 3.28 or later is required.


Quick Start
-----------

  mkdir build && cd build
  cmake ..
  make -j$(nproc)
  make install

To install into a custom prefix:

  cmake -DCMAKE_INSTALL_PREFIX=/opt/gwenhywfar ..

To see all available options:

  cmake -LH ..


Required Dependencies
---------------------

The following libraries must be installed before building:

  - libgcrypt   >= 1.2.0   (cryptographic functions)
  - gpg-error              (error handling for libgcrypt)
  - GnuTLS     >= 2.9.8   (TLS/SSL support)
  - pthreads               (POSIX platforms only)

Optional dependencies (pulled in by specific options):

  - OpenSSL                (for gct-tool DES key support)
  - iconv                  (character set conversion)
  - gettext / libintl      (internationalization)
  - libxml2                (XML processing, requires WITH_LIBXML2_CODE=ON)
  - Doxygen                (API documentation, requires ENABLE_FULL_DOC=ON)

GUI backend dependencies (only needed when the backend is listed in GWEN_GUI_BACKENDS):

  - Qt5 Widgets+Core       (for qt5 backend)
  - Qt6 Widgets+Core       (for qt6 backend)
  - Qt4 QtCore+QtGui       (for qt4 backend)
  - GTK+ 2.0  >= 2.17.5   (for gtk2 backend)
  - GTK+ 3.0  >= 3.10.8   (for gtk3 backend)
  - FOX toolkit >= 1.6     (for fox16 backend)
  - Cocoa frameworks       (for cocoa backend, macOS only)


CMake Options
-------------

All options can be set on the cmake command line with -D, for example:

  cmake -DGWEN_GUI_BACKENDS="cpp;qt5" -DENABLE_SYSTEM_CERTS=OFF ..


### GUI Backend Selection

  GWEN_GUI_BACKENDS  (STRING, default: "cpp")

    Semicolon-separated list of GUI backends to build.  Valid values:

      cpp     C++ base backend (always built, even if omitted from the list)
      qt4     Qt 4 backend
      qt5     Qt 5 backend
      qt6     Qt 6 backend (uses the same sources as qt5)
      gtk2    GTK+ 2 backend
      gtk3    GTK+ 3 backend
      fox16   FOX 1.6 toolkit backend
      cocoa   Cocoa backend (macOS only)

    Notes:
      - The cpp backend is always included regardless of this setting.
      - qt5 and qt6 cannot be enabled simultaneously.
      - cocoa can only be used on macOS.

    Example:
      cmake -DGWEN_GUI_BACKENDS="cpp;qt5;gtk3" ..


### Plugin Selection

  WITH_PLUGINS_DBIO  (STRING, default: "csv;olddb;xmldb")

    Semicolon-separated list of DBIO plugins to build.  Valid values:
    csv, olddb, xmldb.

  WITH_PLUGINS_CT  (STRING, default: "ohbci")

    Semicolon-separated list of CryptoToken plugins to build.
    Valid values: ohbci.

  WITH_PLUGINS_CFGMGR  (STRING, default: "dir")

    Semicolon-separated list of ConfigManager plugins to build.
    Valid values: dir.


### Build Mode Options

  GWEN_SUBPROJECT  (BOOL, default: OFF)

    Set to ON when building Gwenhywfar as part of a larger project.
    Defines GWENHYWFAR_IS_SUBPROJECT in config.h.
    Equivalent to autotools --enable-gwen-subproject.

  ENABLE_TESTCODE  (BOOL, default: OFF)

    Compile additional test code into the library.
    Defines GWENHYWFAR_ENABLE_TESTCODE in config.h.
    Equivalent to autotools --enable-testcode.

  ENABLE_LOCAL_INSTALL  (BOOL, default: OFF)

    Use relative paths for plugin, data, and locale search directories
    instead of absolute install paths.  Useful for self-contained
    installations or relocatable packages.
    Equivalent to autotools --enable-local-install.

  ENABLE_RELEASE  (BOOL, default: OFF)

    Strip debug symbols from installed binaries.
    Equivalent to autotools --enable-release.

  ENABLE_PROFILING  (BOOL, default: OFF)

    Add -pg compiler and linker flags for profiling with gprof.
    Equivalent to autotools --enable-profiling.

  ENABLE_ERROR_ON_WARNING  (BOOL, default: OFF)

    Treat compiler warnings as errors.  Uses -Werror on GCC/Clang
    and /WX on MSVC.
    Equivalent to autotools --enable-error-on-warning.


### Feature Options

  ENABLE_SYSTEM_CERTS  (BOOL, default: ON)

    Use the operating system's trusted CA certificate store for TLS
    connections.  When OFF, applications must provide their own
    certificate bundle.
    Defines GWEN_TLS_USE_SYSTEM_CERTIFICATES in config.h.
    Equivalent to autotools --enable-system-certs.

  ENABLE_VARIADIC_MACROS  (BOOL, default: ON)

    Enable use of variadic macros in the source code.  Set to OFF for
    compilers with limited variadic macro support.  When OFF, defines
    NO_VARIADIC_MACROS in config.h.
    Equivalent to autotools --enable-variadic-macros.

  ENABLE_MEMTRACE  (BOOL, default: OFF)

    Enable memory allocation tracing for debugging.
    Defines GWEN_MEMORY_DEBUG in config.h.
    Equivalent to autotools --enable-memtrace.

  WITH_LIBXML2_CODE  (BOOL, default: OFF)

    Compile libxml2-dependent code.  Requires libxml-2.0 to be
    installed.  Defines GWEN_COMPILE_LIBXML2_CODE in config.h.
    Equivalent to autotools --with-libxml2-code.

  BUFFER_HARDLIMIT  (STRING, default: "(16*1024*1024)")

    Maximum buffer size expression used as a safety limit.
    Equivalent to autotools --with-buffer-hardlimit.


### Test and Documentation Options

  ENABLE_NETWORK_CHECKS  (BOOL, default: ON)

    Include network-dependent tests when running ctest.  Set to OFF
    to skip tests that require network connectivity.
    Equivalent to autotools --enable-network-checks.

  ENABLE_FULL_DOC  (BOOL, default: OFF)

    Generate API documentation using Doxygen.  Requires Doxygen to be
    installed.  Creates a "doc" build target.
    Equivalent to autotools --enable-full-doc.


### Search Directory Overrides

These options override the default search paths used at runtime to locate
plugins, data files, locale files, and configuration.  When not set, the
paths are derived from CMAKE_INSTALL_PREFIX.  When ENABLE_LOCAL_INSTALL
is ON or on Windows, relative paths are used by default.

  GWEN_PLUGIN_SEARCHDIR  (PATH, default: auto)

    Runtime search path for plugins.
    Default: <libdir>/gwenhywfar/plugins/<so_effective>

  GWEN_DATA_SEARCHDIR  (PATH, default: auto)

    Runtime search path for Gwenhywfar data files.
    Default: <datadir>/gwenhywfar

  GWEN_SYSDATA_SEARCHDIR  (PATH, default: auto)

    Runtime search path for system-wide data files.
    Default: <datadir>

  GWEN_LOCALE_SEARCHDIR  (PATH, default: auto)

    Runtime search path for locale/translation files.
    Default: <localedir>

  GWEN_CFG_SEARCHDIR  (PATH, default: auto)

    Runtime search path for configuration files.
    Default: <sysconfdir>


Build Examples
--------------

Minimal build (core library + cpp backend only):

  mkdir build && cd build
  cmake ..
  make -j$(nproc)

Build with Qt5 GUI backend:

  cmake -DGWEN_GUI_BACKENDS="cpp;qt5" ..

Build with multiple GUI backends:

  cmake -DGWEN_GUI_BACKENDS="cpp;qt5;gtk3" ..

Debug build with memory tracing:

  cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_MEMTRACE=ON ..

Release build with stripping:

  cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_RELEASE=ON ..

Install to custom prefix:

  cmake -DCMAKE_INSTALL_PREFIX=/opt/gwenhywfar ..
  make -j$(nproc)
  make install

Build with libxml2 support:

  cmake -DWITH_LIBXML2_CODE=ON ..

Generate API documentation:

  cmake -DENABLE_FULL_DOC=ON ..
  make doc


Running Tests
-------------

After building, run the test suite with:

  ctest

Or equivalently:

  make test

To skip network-dependent tests:

  cmake -DENABLE_NETWORK_CHECKS=OFF ..
  make -j$(nproc)
  ctest


Cross-Compilation Notes
-----------------------

For Windows cross-compilation with MinGW, use a CMake toolchain file:

  cmake -DCMAKE_TOOLCHAIN_FILE=<path-to-toolchain> ..

MSVC is also supported on Windows.  Open the project in Visual Studio
or use:

  cmake -G "Visual Studio 17 2022" ..


Differences from Autotools Build
--------------------------------

- The CMake build requires CMake 3.28+.
- GUI backends are selected via GWEN_GUI_BACKENDS (semicolon-separated
  list) instead of individual --with-gui-* flags.
- Plugin categories are selected via WITH_PLUGINS_DBIO, WITH_PLUGINS_CT,
  and WITH_PLUGINS_CFGMGR instead of individual --with-plugin-* flags.
- Template .in files are converted from the autotools originals at
  configure time; no duplicate CMake-specific .in files exist in the
  source tree.
- The dlgtest tool is currently disabled (references a removed UI
  subsystem).
