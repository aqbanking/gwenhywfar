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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Internationalization */
#ifdef ENABLE_NLS
# include <libintl.h>
# include <locale.h>
# define I18N(m) dgettext("gwenhywfar", m)
# define I18S(m) m
#else
# define I18N(m) m
# define I18S(m) m
#endif

#include "args.h"
#include <gwenhywfar/debug.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>



ARGUMENTS *Arguments_new() {
  ARGUMENTS *ar;

  ar=(ARGUMENTS*)malloc(sizeof(ARGUMENTS));
  assert(ar);
  memset(ar, 0, sizeof(ARGUMENTS));
  ar->verbous=0;
  ar->logLevel=GWEN_LoggerLevelNotice;
  ar->logType=GWEN_LoggerTypeConsole;
  ar->compact=0;
  ar->header=0;
  return ar;
}



FREEPARAM *FreeParam_new(const char *s) {
  FREEPARAM *fr;

  fr=(FREEPARAM*)malloc(sizeof(FREEPARAM));
  assert(fr);
  memset(fr, 0, sizeof(FREEPARAM));
  fr->param=s;
  return fr;
}


void FreeParam_free(FREEPARAM *fr) {
  if (fr)
    free(fr);
}



void Arguments_AddParam(ARGUMENTS *ar, const char *pr) {
  FREEPARAM *curr;
  FREEPARAM *nfp;

  DBG_ENTER;
  assert(ar);
  assert(pr);

  nfp=FreeParam_new(pr);

  curr=ar->params;
  if (!curr) {
    ar->params=nfp;
  }
  else {
    /* find last */
    while(curr->next) {
      curr=curr->next;
    } /* while */
    curr->next=nfp;
  }
  DBG_LEAVE;
}



void Arguments_free(ARGUMENTS *ar) {
  if (ar) {
    FREEPARAM *fr;
    FREEPARAM *next;

    fr=ar->params;
    while(fr) {
      next=fr->next;
      FreeParam_free(fr);
      fr=next;
    } /* while */
    free(ar);
  }
}



void usage(const char *prgname) {
  fprintf(stdout, "%s%s",
          k_PRG_VERSION_INFO "\n",
          I18N(
               "-v               verbous\n"
               "--logfile ARG    name of the logfile\n"
               "--logtype ARG    log type\n"
               "--loglevel ARG   log level\n"
               "-o ARG           name of output file (stdout if omitted)\n"
               "--compact        write a more compact file\n"
              )
         );
}



int checkArgs(ARGUMENTS *args, int argc, char **argv) {
  int i;

  i=1;
  while (i<argc){
    if (strcmp(argv[i],"--logfile")==0) {
      i++;
      if (i>=argc)
	return RETURNVALUE_PARAM;
      args->logFile=argv[i];
    }
    else if (strcmp(argv[i],"--logtype")==0) {
      i++;
      if (i>=argc)
        return RETURNVALUE_PARAM;
      args->logType=GWEN_Logger_Name2Logtype(argv[i]);
      if (args->logType==GWEN_LoggerTypeUnknown) {
        fprintf(stderr,
                I18N("Unknown log type \"%s\"\n"),
                argv[i]);
        return RETURNVALUE_PARAM;
      }
    }
    else if (strcmp(argv[i],"--loglevel")==0) {
      i++;
      if (i>=argc)
        return RETURNVALUE_PARAM;
      args->logLevel=GWEN_Logger_Name2Level(argv[i]);
      if (args->logLevel==GWEN_LoggerLevelUnknown) {
        fprintf(stderr,
                I18N("Unknown log level \"%s\"\n"),
                argv[i]);
        return RETURNVALUE_PARAM;
      }
    }
    else if (strcmp(argv[i],"-o")==0) {
      i++;
      if (i>=argc)
        return RETURNVALUE_PARAM;
      args->outputFile=argv[i];
    }
    else if (strcmp(argv[i],"--compact")==0) {
      args->compact=1;
    }
    else if (strcmp(argv[i],"--header")==0) {
      args->header=1;
    }
    else if (strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--help")==0) {
      usage(argv[0]);
      return -2;
    }
    else if (strcmp(argv[i],"-V")==0 || strcmp(argv[i],"--version")==0) {
      fprintf(stdout, k_PRG_VERSION_INFO);
      return -2;
    }
    else if (strcmp(argv[i],"-v")==0) {
      args->verbous=1;
    }
    else {
      // otherwise add param
      if (argv[i][0]=='-') {
        fprintf(stderr,I18N("Unknown option \"%s\"\n"),argv[i]);
        return RETURNVALUE_PARAM;
      }
      else
        Arguments_AddParam(args, argv[i]);
    }
    i++;
  } /* while */

  /* that's it */
  return 0;
}







