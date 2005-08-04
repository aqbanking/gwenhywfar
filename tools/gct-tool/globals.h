/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GCT_TOOL_GLOBALS_H
#define GCT_TOOL_GLOBALS_H

#define DEBUG_GCT_TOOL 1

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/args.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/misc2.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/crypttoken.h>

#include <string.h>
#include <errno.h>

#define GCT_LOGDOMAIN "gct-tool"


#define I18N(x) x


int signFiles(GWEN_DB_NODE *dbArgs, int argc, char **argv);

int createToken(GWEN_DB_NODE *dbArgs, int argc, char **argv);

int showCtx(GWEN_DB_NODE *dbArgs, int argc, char **argv);

int readKey(GWEN_DB_NODE *dbArgs, int argc, char **argv);

int showUser(GWEN_DB_NODE *dbArgs, int argc, char **argv);

int changePin(GWEN_DB_NODE *dbArgs, int argc, char **argv);

int checkToken(GWEN_DB_NODE *dbArgs, int argc, char **argv);

int disableKey(GWEN_DB_NODE *dbArgs, int argc, char **argv);


#endif /* GCT_TOOL_GLOBALS_H */


