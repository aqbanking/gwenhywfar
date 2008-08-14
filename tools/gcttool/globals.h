/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: globals.h 923 2005-11-19 03:35:10Z aquamaniac $
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GCTTOOL_GLOBALS_H
#define GCTTOOL_GLOBALS_H

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

#define GCT_LOGDOMAIN "gcttool"

#include "src/base/i18n_l.h"


GWEN_CRYPT_TOKEN *getCryptToken(const char *ttype, const char *tname);


int createToken(GWEN_DB_NODE *dbArgs, int argc, char **argv);

int showUser(GWEN_DB_NODE *dbArgs, int argc, char **argv);

int showKey(GWEN_DB_NODE *dbArgs, int argc, char **argv);

int genKey(GWEN_DB_NODE *dbArgs, int argc, char **argv);

int updateToken(GWEN_DB_NODE *dbArgs, int argc, char **argv);

int setSignSeq(GWEN_DB_NODE *dbArgs, int argc, char **argv);

int setKey(GWEN_DB_NODE *dbArgs, int argc, char **argv);

int changePin(GWEN_DB_NODE *dbArgs, int argc, char **argv);


#endif /* GCTTOOL_GLOBALS_H */





