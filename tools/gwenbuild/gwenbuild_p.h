/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_GWENBUILD_P_H
#define GWBUILD_GWENBUILD_P_H


#include "gwenbuild/gwenbuild.h"



struct GWENBUILD {
  int dummy; /* for now */

  char *toolNameCC;
  char *toolNameCXX;
  char *toolNameLD;
  char *toolNameAR;
  char *toolNameRANLIB;

  GWEN_STRINGLIST *buildFilenameList;

};


#endif
