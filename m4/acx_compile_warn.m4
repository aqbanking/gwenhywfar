# Compile warning arguments to ./configure
# by Christian Stimming <stimming@tuhh.de> 2003-11-19

dnl ACX_COMPILE_WARN()
dnl Add arguments for compile warnings and debug options to ./configure.
dnl
AC_DEFUN([ACX_COMPILE_WARN],
[

dnl For enabling of debugging flags/code
AC_ARG_ENABLE(debug,
  [  --enable-debug          compile with debugging code enabled, default=no],
  [case "${enableval}" in
     yes)   CXXFLAGS="${CXXFLAGS} -g"
	    CFLAGS="${CFLAGS} -g"
	    LDFLAGS="${LDFLAGS} -g"
	    AC_DEFINE(DEBUG,1,[Define if you want debugging code enabled.]) ;;
     no) ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
   esac
  ], [
	# Default value if the argument was not given
	CXXFLAGS="${CXXFLAGS}"
	CFLAGS="${CFLAGS}"
	LDFLAGS="${LDFLAGS}"
])

dnl If this is gcc, then ...
if test ${GCC}x = yesx; then

  dnl Enable all warnings
  AC_ARG_ENABLE(warnings,
    [  --enable-warnings       enable compilation warnings, default=yes],
    [case "${enableval}" in
       yes) CXXFLAGS="${CXXFLAGS} -Wall"
	    CFLAGS="${CFLAGS} -Wall" ;;
       all) CXXFLAGS="${CXXFLAGS} -Wall -pedantic -ansi"
	    CFLAGS="${CFLAGS} -Wall -pedantic -ansi" ;;
       no) ;;
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-warnings) ;;
     esac
  ], [ 
     # Default value if the argument was not given
     CXXFLAGS="${CXXFLAGS} -Wall" 
     CFLAGS="${CFLAGS} -Wall" 
  ])

  dnl For gcc >= 3.4.x, specifically enable the new warning switch
  dnl -Wdeclaration-after-statement in order to preserve source code
  dnl compatibility to gcc 2.95 and other compilers.
  GCC_VERSION=`${CC} -dumpversion`
  if test `echo ${GCC_VERSION} | cut -d. -f1` -ge 3; then
     # This is gcc >= 3.x.x
     if test `echo ${GCC_VERSION} | cut -d. -f2` -ge 4; then
	# This is gcc >= 3.4.x
	CFLAGS="${CFLAGS} -Wdeclaration-after-statement"
     fi
  fi

  dnl For enabling error on warnings
  AC_ARG_ENABLE(error-on-warning,
    [  --enable-error-on-warning treat all compile warnings as errors, default=no],
    [case "${enableval}" in
       yes) CXXFLAGS="${CXXFLAGS} -Werror" 
	    CFLAGS="${CFLAGS} -Werror" ;;
       no) ;;
       *) AC_MSG_ERROR(bad value ${enableval} for --enable-error-on-warning) ;;
     esac
  ], [ 
     # Default value if the argument was not given: At least error out on implicit function declarations.
     CXXFLAGS="${CXXFLAGS} -Werror=implicit-function-declaration"
     CFLAGS="${CFLAGS} -Werror=implicit-function-declaration"
  ])
fi

])
