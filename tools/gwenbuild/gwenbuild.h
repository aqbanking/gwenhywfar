/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_GWENBUILD_H
#define GWBUILD_GWENBUILD_H



typedef enum {
  GWBUILD_TargetType_Invalid=-1,
  GWBUILD_TargetType_None=0,
  GWBUILD_TargetType_InstallLibrary,
  GWBUILD_TargetType_ConvenienceLibrary,
  GWBUILD_TargetType_Program,
  GWBUILD_TargetType_Objects,
} GWBUILD_TARGETTYPE;


#endif
