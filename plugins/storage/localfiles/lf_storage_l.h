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

#ifndef LF_STORAGE_L_H
#define LF_STORAGE_L_H

#include <gwenhywfar/st_storage_be.h>


GWENHYWFAR_EXPORT
GWEN_STO_STORAGE *LocalFilesStorage_new(const char *address);


#endif


