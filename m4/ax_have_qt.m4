# ===========================================================================
#        https://www.gnu.org/software/autoconf-archive/ax_have_qt.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_HAVE_QT
#
# DESCRIPTION
#
#   Searches $PATH and queries qmake for Qt include files, libraries and Qt
#   binary utilities. The macro only supports Qt5 or later.
#
#   The following shell variable is set to either "yes" or "no":
#
#     have_qt
#
#   Additionally, the following variables are exported:
#
#     QT_CXXFLAGS
#     QT_LIBS
#     QT_MOC
#     QT_UIC
#     QT_RCC
#     QT_LRELEASE
#     QT_LUPDATE
#     QT_DIR
#     QMAKE
#
#   which respectively contain an "-I" flag pointing to the Qt include
#   directory, link flags necessary to link with Qt and X, the full path to
#   the meta object compiler and the user interface compiler both, and
#   finally the variable QTDIR as Qt likes to see it defined.
#
#   Example lines for Makefile.in:
#
#     CXXFLAGS = @QT_CXXFLAGS@
#     MOC      = @QT_MOC@
#
#   After the variables have been set, a trial compile and link is performed
#   to check the correct functioning of the meta object compiler. This test
#   may fail when the different detected elements stem from different
#   releases of the Qt framework. In that case, an error message is emitted
#   and configure stops.
#
#   No common variables such as $LIBS or $CFLAGS are polluted.
#
# LICENSE
#
#   Copyright (c) 2008 Bastiaan Veelo <Bastiaan@Veelo.net>
#   Copyright (c) 2014 Alex Henrie <alexhenrie24@gmail.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 27

AU_ALIAS([BNV_HAVE_QT], [AX_HAVE_QT])
AC_DEFUN([AX_HAVE_QT],
[
  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AC_PATH_X])
  AC_REQUIRE([AC_PATH_XTRA])

  # openSUSE leap 15.3 installs qmake-qt5, not qmake, for example.
  # Store the full name (like qmake-qt5) into QMAKE
  # and the specifier (like -qt5 or empty) into am_have_qt_qmexe_suff.
  AC_ARG_VAR([QMAKE],[Qt make tool])
  AC_CHECK_TOOLS([QMAKE],[qmake qmake-qt6 qmake6 qmake-qt5],[false])

  AC_ARG_WITH(qmake,
    [  --with-qmake=FILE    uses given qmake],
    [QMAKE="$withval"],
    []
  )


  AC_MSG_CHECKING(for Qt)
  am_have_qt_qmexe_suff=`echo $QMAKE | sed 's,^.*qmake,,'`
  # If we have Qt5 or later in the path, we're golden
  ver=`$QMAKE --version | grep -o "Qt version ."`

  if test "$ver" ">" "Qt version 4"; then
    QT_MAJOR_VERSION="5"
    have_qt=yes
    # This pro file dumps qmake's variables, but it only works on Qt 5 or later
    am_have_qt_dir=`mktemp -d`
    am_have_qt_pro="$am_have_qt_dir/test.pro"
    am_have_qt_stash="$am_have_qt_dir/.qmake.stash"
    am_have_qt_makefile="$am_have_qt_dir/Makefile"
    # http://qt-project.org/doc/qt-5/qmake-variable-reference.html#qt
    cat > $am_have_qt_pro << EOF
win32 {
    CONFIG -= debug_and_release
    CONFIG += release
}
qtHaveModule(axcontainer):       QT += axcontainer
qtHaveModule(axserver):          QT += axserver
qtHaveModule(concurrent):        QT += concurrent
qtHaveModule(core):              QT += core
qtHaveModule(dbus):              QT += dbus
qtHaveModule(declarative):       QT += declarative
qtHaveModule(designer):          QT += designer
qtHaveModule(gui):               QT += gui
qtHaveModule(help):              QT += help
qtHaveModule(multimedia):        QT += multimedia
qtHaveModule(multimediawidgets): QT += multimediawidgets
qtHaveModule(network):           QT += network
qtHaveModule(opengl):            QT += opengl
qtHaveModule(printsupport):      QT += printsupport
qtHaveModule(qml):               QT += qml
qtHaveModule(qmltest):           QT += qmltest
qtHaveModule(x11extras):         QT += x11extras
qtHaveModule(script):            QT += script
qtHaveModule(scripttools):       QT += scripttools
qtHaveModule(sensors):           QT += sensors
qtHaveModule(serialport):        QT += serialport
qtHaveModule(sql):               QT += sql
qtHaveModule(svg):               QT += svg
qtHaveModule(testlib):           QT += testlib
qtHaveModule(uitools):           QT += uitools
qtHaveModule(webkit):            QT += webkit
qtHaveModule(webkitwidgets):     QT += webkitwidgets
qtHaveModule(xml):               QT += xml
qtHaveModule(xmlpatterns):       QT += xmlpatterns
percent.target = %
percent.commands = @echo -n "\$(\$(@))\ "
QMAKE_EXTRA_TARGETS += percent
EOF
    $QMAKE $am_have_qt_pro -o $am_have_qt_makefile
    QT_CXXFLAGS=`cd $am_have_qt_dir; make -s -f $am_have_qt_makefile CXXFLAGS INCPATH`
    QT_LIBS=`cd $am_have_qt_dir; make -s -f $am_have_qt_makefile LIBS`
    rm $am_have_qt_pro $am_have_qt_stash $am_have_qt_makefile
    rmdir $am_have_qt_dir

    if test "$ver" ">" "Qt version 5"; then
      QT_MAJOR_VERSION="6"
      AC_CHECK_TOOLS([QTPATHS],[qtpaths-qt6 qtpaths6 qtpaths],[false])
    else
      AC_CHECK_TOOLS([QTPATHS],[qtpaths-qt5 qtpaths5 qtpaths],[false])
    fi

    ver=`$QTPATHS --version | cut -d' ' -f 2`
    if test "$ver" = "2.0"; then
      # Add QT_HOST_BINS and QT_HOST_LIBEXECS paths to PATH
      for var in QT_HOST_BINS QT_HOST_LIBEXECS; do
        PATH=$PATH:`$QTPATHS --query $var`
      done
    fi

    # Look for specific tools in $PATH
    AC_ARG_VAR([QT_MOC],[Qt moc tool])
    AC_PATH_PROG([QT_MOC],[moc$am_have_qt_qmexe_suff])
    if test "$QT_MOC" = ""; then
      AC_PATH_PROG([QT_MOC],[moc])
    fi

    AC_ARG_VAR([QT_UIC],[Qt uic tool])
    AC_PATH_PROG([QT_UIC],[uic$am_have_qt_qmexe_suff])
    if test "$QT_UIC" = ""; then
      AC_PATH_PROG([QT_UIC],[uic])
    fi

    AC_ARG_VAR([QT_RCC],[Qt rcc tool])
    AC_PATH_PROG([QT_RCC],[rcc$am_have_qt_qmexe_suff])
    if test "$QT_RCC" = ""; then
      AC_PATH_PROG([QT_RCC],[rcc])
    fi

    AC_ARG_VAR([QT_LRELEASE],[Qt lrelease tool])
    AC_PATH_PROG([QT_LRELEASE],[lrelease$am_have_qt_qmexe_suff])
    if test "$QT_LRELEASE" = ""; then
      AC_PATH_PROG([QT_LRELEASE],[lrelease])
    fi

    AC_ARG_VAR([QT_LUPDATE],[Qt lupdate tool])
    AC_PATH_PROG([QT_LUPDATE],[lupdate$am_have_qt_qmexe_suff])
    if test "$QT_LUPDATE" = ""; then
      AC_PATH_PROG([QT_LUPDATE],[lupdate])
    fi

    # Get Qt version from qmake
    QT_DIR=`$QMAKE --version | grep -o -E /.+`

    # All variables are defined, report the result
    AC_MSG_RESULT([$have_qt:
    QT_CXXFLAGS=$QT_CXXFLAGS
    QT_DIR=$QT_DIR
    QT_LIBS=$QT_LIBS
    QT_UIC=$QT_UIC
    QT_MOC=$QT_MOC
    QT_RCC=$QT_RCC
    QT_LRELEASE=$QT_LRELEASE
    QT_LUPDATE=$QT_LUPDATE
    QT_MAJOR_VERSION=$QT_MAJOR_VERSION])
  else
    # Qt was not found
    have_qt=no
    QT_CXXFLAGS=
    QT_DIR=
    QT_LIBS=
    QT_UIC=
    QT_MOC=
    QT_RCC=
    QT_LRELEASE=
    QT_LUPDATE=
    QT_MAJOR_VERSION=
    AC_MSG_RESULT($have_qt)
  fi
  AC_SUBST(QT_CXXFLAGS)
  AC_SUBST(QT_DIR)
  AC_SUBST(QT_LIBS)
  AC_SUBST(QT_MAJOR_VERSION)

  #### Being paranoid:
  if test x"$have_qt" = xyes; then
    AC_MSG_CHECKING(correct functioning of Qt installation)
    AC_CACHE_VAL(ax_cv_qt_test_result,
    [
      cat > ax_qt_test.h << EOF
#include <qobject.h>
class Test : public QObject
{
Q_OBJECT
public:
  Test() {}
  ~Test() {}
public slots:
  void receive() {}
signals:
  void send();
};
EOF

      cat > ax_qt_main.$ac_ext << EOF
#include "ax_qt_test.h"
#include <qapplication.h>
int main( int argc, char **argv )
{
  QApplication app( argc, argv );
  Test t;
  QObject::connect( &t, SIGNAL(send()), &t, SLOT(receive()) );
}
EOF

      ax_cv_qt_test_result="failure"
      ax_try_1="$QT_MOC ax_qt_test.h -o moc_ax_qt_test.$ac_ext >/dev/null 2>/dev/null"
      AC_TRY_EVAL(ax_try_1)
      if test x"$ac_status" != x0; then
        echo "$ax_err_1" >&AS_MESSAGE_LOG_FD
        echo "configure: could not run $QT_MOC on:" >&AS_MESSAGE_LOG_FD
        cat ax_qt_test.h >&AS_MESSAGE_LOG_FD
      else
        ax_try_2="$CXX $QT_CXXFLAGS -c $CXXFLAGS -o moc_ax_qt_test.o moc_ax_qt_test.$ac_ext >/dev/null 2>/dev/null"
        AC_TRY_EVAL(ax_try_2)
        if test x"$ac_status" != x0; then
          echo "$ax_err_2" >&AS_MESSAGE_LOG_FD
          echo "configure: could not compile:" >&AS_MESSAGE_LOG_FD
          cat moc_ax_qt_test.$ac_ext >&AS_MESSAGE_LOG_FD
        else
          ax_try_3="$CXX $QT_CXXFLAGS -c $CXXFLAGS -o ax_qt_main.o ax_qt_main.$ac_ext >/dev/null 2>/dev/null"
          AC_TRY_EVAL(ax_try_3)
          if test x"$ac_status" != x0; then
            echo "$ax_err_3" >&AS_MESSAGE_LOG_FD
            echo "configure: could not compile:" >&AS_MESSAGE_LOG_FD
            cat ax_qt_main.$ac_ext >&AS_MESSAGE_LOG_FD
          else
            ax_try_4="$CXX -o ax_qt_main ax_qt_main.o moc_ax_qt_test.o $QT_LIBS $LIBS >/dev/null 2>/dev/null"
            AC_TRY_EVAL(ax_try_4)
            if test x"$ac_status" != x0; then
              echo "$ax_err_4" >&AS_MESSAGE_LOG_FD
            else
              ax_cv_qt_test_result="success"
            fi
          fi
        fi
      fi
    ])dnl AC_CACHE_VAL ax_cv_qt_test_result
    AC_MSG_RESULT([$ax_cv_qt_test_result])
    if test x"$ax_cv_qt_test_result" = "xfailure"; then
      AC_MSG_ERROR([Failed to find matching components of a complete
                  Qt installation. Try using more options,
                  see ./configure --help.])
    fi

    rm -f ax_qt_test.h moc_ax_qt_test.$ac_ext moc_ax_qt_test.o \
          ax_qt_main.$ac_ext ax_qt_main.o ax_qt_main
  fi
])
