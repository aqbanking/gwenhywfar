# $Id$
# (c) 2004 Martin Preuss<martin@libchipcard.de>
# These functions retrieve some important paths


AC_DEFUN(AQ_WINDOZE_GETPATH, [
dnl IN:
dnl   - $1: type of path to get:
dnl         - windows: windows path
dnl         - system:  windows/system directory
dnl         - home:    users home directory
dnl   - $2: default value
dnl OUT:
dnl   - aq_windoze_path: path retrieved
dnl

AC_MSG_CHECKING([for windoze $1 path (program)])

rm -f conf.winpath
AC_TRY_RUN([
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main (){
  char buffer[260];
  FILE *f;

  buffer[0]=0;
  
  if (strlen("$2")) {
    if (strlen("$2")>=sizeof(buffer)) {
      printf("path is too long ($2)\n");
      exit(1);
    }
    strcpy(buffer, "$2");
  }
  else {
    if (strcasecmp("$1", "windows")==0) {
      GetWindowsDirectory(buffer, sizeof(buffer));
    }
    else if (strcasecmp("$1", "system")==0) {
      GetSystemDirectory(buffer, sizeof(buffer));
    }
    else if (strcasecmp("$1", "home")==0) {
      GetWindowsDirectory(buffer, sizeof(buffer));
    }
    else {
      printf("Unknown type \"$1\"\n");
      exit(1);
    }
  }
  
  f=fopen("conf.winpath", "w+");
  if (!f) {
    printf("Could not create file conf.winpath\n");
    exit(1);
  }
  fprintf(f, "%s", buffer);
  if (fclose(f)) {
   printf("Could not close file.\n");
   exit(1);
  }
  exit(0);
}
 ],
 [aq_windoze_path="`cat conf.winpath`"; AC_MSG_RESULT([$aq_windoze_path])],
 [AC_MSG_ERROR(Could not determine path for $1)],
 [aq_windoze_path="$2"; AC_MSG_RESULT([Crosscompiling, assuming $2])]
)
rm -f conf.winpath
])


AC_DEFUN(AQ_WINDOZE_GETPATH_MINGW, [
dnl IN:
dnl   - $1: type of path to get:
dnl         - windows: windows path
dnl         - system:  windows/system directory
dnl         - home:    users home directory
dnl   - $2: default value
dnl OUT:
dnl   - aq_windoze_path: path retrieved
dnl

AC_MSG_CHECKING([for windoze $1 path (mingw)])

rm -f conf.winpath
AC_TRY_RUN([
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main (){
  char buffer[260];
  char buffer2[260+2];
  char *p;
  char *tp;
  FILE *f;
  int lastWasSlash;

  buffer[0]=0;

  if (strlen("$2")) {
    if (strlen("$2")>=sizeof(buffer)) {
      printf("path is too long ($2)\n");
      exit(1);
    }
    strcpy(buffer, "$2");
  }
  else {
    if (strcasecmp("$1", "windows")==0) {
      GetWindowsDirectory(buffer, sizeof(buffer));
    }
    else if (strcasecmp("$1", "system")==0) {
      GetSystemDirectory(buffer, sizeof(buffer));
    }
    else if (strcasecmp("$1", "home")==0) {
      GetWindowsDirectory(buffer, sizeof(buffer));
    }
    else {
      printf("Unknown type \"$1\"\n");
      exit(1);
    }
  }
  

  /* create mingw path */
  tp=buffer2;
  p=buffer;
  if (strlen(buffer)>1) {
    if (buffer[1]==':') {
      *tp='/';
      tp++;
      *tp=buffer[0];
      tp++;
      p+=2;
    }
  }
  
  lastWasSlash=0;
  while(*p) {
    if (*p=='\\\\' || *p=='/') {
      if (!lastWasSlash) {
        *tp='/';
        tp++;
        lastWasSlash=1;
      }
    }
    else {
      lastWasSlash=0;
      *tp=*p;
      tp++;
    }
    p++;
  } /* while */
  *tp=0;
  
  f=fopen("conf.winpath", "w+");
  if (!f) {
    printf("Could not create file conf.winpath\n");
    exit(1);
  }
  fprintf(f, "%s", buffer2);
  if (fclose(f)) {
   printf("Could not close file.\n");
   exit(1);
  }
  exit(0);
}
 ],
 [aq_windoze_path=`cat conf.winpath`; AC_MSG_RESULT([$aq_windoze_path])],
 [AC_MSG_ERROR(Could not determine path for $1)],
 [aq_windoze_path="$2"; AC_MSG_RESULT([Crosscompiling, assuming $2])]
)
rm -f conf.winpath
])


AC_DEFUN(AQ_WINDOZE,[
dnl IN: 
dnl   - AC_CANONICAL_SYSTEM muste be called before
dnl OUT:
dnl   Variables (subst):
dnl     WIN_PATH_HOME          : path and name of the Windoze home folder
dnl     WIN_PATH_HOME_MINGW    : path and name of the Windoze home folder
dnl     WIN_PATH_WINDOWS       : path and name of the Windoze system folder
dnl     WIN_PATH_WINDOWS_MINGW : path and name of the Windoze system folder
dnl     WIN_PATH_SYSTEM        : path and name of the Windoze folder
dnl     WIN_PATH_SYSTEM_MINGW  : path and name of the Windoze folder
dnl   Defines:
dnl     WIN_PATH_HOME          : path and name of the Windoze home folder
dnl     WIN_PATH_WINDOWS       : path and name of the Windoze system folder
dnl     WIN_PATH_SYSTEM        : path and name of the Windoze folder

# presets
AC_ARG_WITH(home-path,    [  --with-home-path=DIR    specify the home directory for a user],
  [aq_windoze_path_home="$withval"])
AC_ARG_WITH(system-path,  [  --with-system-path=DIR  specify the system directory],
  [aq_windoze_path_system="$withval"])
AC_ARG_WITH(windows-path, [  --with-windows-path=DIR specify the windows directory],
  [aq_windoze_path_windows="$withval"])

AQ_WINDOZE_GETPATH(home, [$aq_windoze_path_home])
WIN_PATH_HOME="$aq_windoze_path"
AQ_WINDOZE_GETPATH_MINGW(home, [$aq_windoze_path_home])
WIN_PATH_HOME_MINGW="$aq_windoze_path"

AQ_WINDOZE_GETPATH(windows, [$aq_windoze_path_windows])
WIN_PATH_WINDOWS="$aq_windoze_path"
AQ_WINDOZE_GETPATH_MINGW(windows, [$aq_windoze_path_windows])
WIN_PATH_WINDOWS_MINGW="$aq_windoze_path"

AQ_WINDOZE_GETPATH(system, [$aq_windoze_path_system])
WIN_PATH_SYSTEM="$aq_windoze_path"
AQ_WINDOZE_GETPATH_MINGW(system, [$aq_windoze_path_system])
WIN_PATH_SYSTEM_MINGW="$aq_windoze_path"

AC_SUBST(WIN_PATH_HOME)
AC_DEFINE_UNQUOTED(WIN_PATH_HOME, "$WIN_PATH_HOME", [home path])
AC_SUBST(WIN_PATH_HOME_MINGW)
AC_SUBST(WIN_PATH_WINDOWS)
AC_DEFINE_UNQUOTED(WIN_PATH_WINDOWS, "$WIN_PATH_WINDOWS", [windows path])
AC_SUBST(WIN_PATH_WINDOWS_MINGW)
AC_SUBST(WIN_PATH_SYSTEM)
AC_DEFINE_UNQUOTED(WIN_PATH_SYSTEM, "$WIN_PATH_SYSTEM", [system path])
AC_SUBST(WIN_PATH_SYSTEM_MINGW)
])






