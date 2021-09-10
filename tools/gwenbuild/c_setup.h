/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GWBUILD_C_SETUP_H
#define GWBUILD_C_SETUP_H


#include <gwenhywfar/db.h>


int GWB_Setup(GWEN_DB_NODE *dbArgs);
int GWB_RepeatLastSetup(const char *fileName);




#endif

