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

#ifndef XMLMERGE_ARGS_H
#define XMLMERGE_ARGS_H

#include <gwenhywfar/logger.h>


#define k_PRG "xmlmerge"
#define k_PRG_VERSION_INFO \
  "xmlmerge v0.1\n"\
  "(c) 2004 Martin Preuss<martin@libchipcard.de>\n"\
  "This program is free software licensed under GPL.\n"\
  "See COPYING for details.\n"


#define RETURNVALUE_PARAM   1
#define RETURNVALUE_SETUP   2
#define RETURNVALUE_NOSTART 3
#define RETURNVALUE_DEINIT  4
#define RETURNVALUE_HANGUP  9


typedef struct _S_PARAM FREEPARAM;
typedef struct _S_ARGS ARGUMENTS;

struct _S_PARAM {
  FREEPARAM  *next;
  const char *param;
};



struct _S_ARGS {
  FREEPARAM *params;
  int verbous;                  /* -v */
  char *logFile;                /* --logfile ARG */
  GWEN_LOGGER_LOGTYPE logType;  /* --logtype ARG */
  GWEN_LOGGER_LEVEL logLevel;   /* --loglevel ARG */
  const char *outputFile;       /* -o ARG */
  int compact;                  /* --compact */
  int header;                   /* --header */
};



ARGUMENTS *Arguments_new();
void Arguments_free(ARGUMENTS *ar);
int checkArgs(ARGUMENTS *args, int argc, char **argv);



#endif /* XMLMERGE_ARGS_H */
