# $Id$
# (c) 2004 Martin Preuss<martin@libchipcard.de>
# These functions check for special types

AC_DEFUN([AQ_TYPE_UINT32],[
dnl PREREQUISITES:
dnl   none
dnl IN: 
dnl   - default type for crosscompiling
dnl OUT:
dnl   Variables:
dnl     aq_type_uint32
dnl   Defines:

AC_MSG_CHECKING(for UINT32 type)
rm -f conf.uint32
AC_TRY_RUN([
#include <stdlib.h>
#include <stdio.h>

int main (){
  const char *s;
  FILE *f;
  
  if (sizeof(unsigned int)==4)
    s="unsigned int";
  else if (sizeof(unsigned short)==4)
    s="unsigned short";
  else if (sizeof(unsigned long)==4)
    s="unsigned long";
  else if (sizeof(unsigned long long)==4)
    s="unsigned long long";
  else {
    printf("Could not determine type for UINT32");
    exit(1);
  }
  f=fopen("conf.uint32", "w+");
  if (!f) {
    printf("Could not create file conf.uint32\n");
    exit(1);
  }
  fprintf(f, "%s\n", s);
  if (fclose(f)) {
   printf("Could not close file.\n");
   exit(1);
  }
  exit(0);
}
 ],
 [aq_type_uint32=`cat conf.uint32`; AC_MSG_RESULT($aq_type_uint32)],
 [AC_MSG_ERROR(Could not determine type)],
 [aq_type_uint32="$1"; AC_MSG_RESULT([Crosscompiling, assuming $1])]
)
rm -f conf.uint32

])



AC_DEFUN([AQ_TYPE_UINT64],[
dnl PREREQUISITES:
dnl   none
dnl IN: 
dnl   - default type for crosscompiling
dnl OUT:
dnl   Variables:
dnl     aq_type_uint64
dnl   Defines:

AC_MSG_CHECKING(for UINT64 type)
rm -f conf.uint64
AC_TRY_RUN([
#include <stdlib.h>
#include <stdio.h>

int main (){
  const char *s;
  FILE *f;

#ifdef __GNUC__  
  if (sizeof(unsigned int)==8)
    s="unsigned int";
  else if (sizeof(unsigned short)==8)
    s="unsigned short";
  else if (sizeof(unsigned long)==8)
    s="unsigned long";
  else if (sizeof(unsigned long long)==8)
    s="unsigned long long";
  else {
    printf("Could not determine type for UINT64");
    exit(1);
  }
#else
# ifdef ULONG64
    s="ULONG64";
# else
    printf("Could not determine type for UINT64");
    exit(1);
# endif
#endif

  f=fopen("conf.uint64", "w+");
  if (!f) {
    printf("Could not create file conf.uint64\n");
    exit(1);
  }
  fprintf(f, "%s\n", s);
  if (fclose(f)) {
   printf("Could not close file.\n");
   exit(1);
  }
  exit(0);
}
 ],
 [aq_type_uint64=`cat conf.uint64`; AC_MSG_RESULT($aq_type_uint64)],
 [AC_MSG_ERROR(Could not determine type)],
 [aq_type_uint64="$1"; AC_MSG_RESULT([Crosscompiling, assuming $1])]
)
rm -f conf.uint64

])



