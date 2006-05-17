/***************************************************************************
 $RCSfile: account.h,v $
 -------------------
 cvs         : $Id: account.h,v 1.16 2006/02/22 19:38:54 aquamaniac Exp $
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef LF_TYPE_P_H
#define LF_TYPE_P_H


#include "lf_type_l.h"



typedef struct LOCALFILES_TYPE LOCALFILES_TYPE;
struct LOCALFILES_TYPE {
  char *baseFolder;
};

static void LocalFilesType_FreeData(void *bp, void *p);



#endif


