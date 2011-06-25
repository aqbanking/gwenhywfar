/***************************************************************************
 begin       : Sat Jun 25 2011
 copyright   : (C) 2011 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GSA_GLOBALS_H
#define GSA_GLOBALS_H

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/args.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/misc2.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/ct.h>
#include <gwenhywfar/ctplugin.h>

#include <string.h>
#include <errno.h>

#define GSA_LOGDOMAIN "gsa"

#include "src/base/i18n_l.h"


int createArchive(GWEN_DB_NODE *dbArgs, int argc, char **argv);
int checkArchive(GWEN_DB_NODE *dbArgs, int argc, char **argv);
int listArchive(GWEN_DB_NODE *dbArgs, int argc, char **argv);
int add2Archive(GWEN_DB_NODE *dbArgs, int argc, char **argv);
int extractArchive(GWEN_DB_NODE *dbArgs, int argc, char **argv);



#endif /* GSA_GLOBALS_H */





