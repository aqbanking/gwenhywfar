/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "c_build.h"
#include "c_setup.h"
#include "utils.h"
#include "gwenbuild/buildctx/buildctx_xml.h"
#include "gwenbuild/buildctx/buildctx_run.h"

#include <gwenhywfar/debug.h>



int GWB_Build(GWEN_DB_NODE *dbArgs)
{
  GWB_BUILD_CONTEXT *buildCtx;
  int rv;
  int numThreads;
  const char *builderName;

  numThreads=GWEN_DB_GetIntValue(dbArgs, "jobs", 0, 1);
  builderName=GWEN_DB_GetCharValue(dbArgs, "builder", 0, NULL);

  if (GWB_Utils_BuildFilesChanged(".gwbuild.buildfiles")) {
    fprintf(stdout, "Build files changed, repeating last setup command.\n");
    rv=GWB_RepeatLastSetup(".gwbuild.args");
    if (rv<0) {
      DBG_INFO(NULL, "here");
      return rv;
    }
  }

  buildCtx=GWB_BuildCtx_ReadFromXmlFile(".gwbuild.ctx");
  if (buildCtx==NULL) {
    fprintf(stderr, "ERROR: Error reading build context from file.\n");
    return 2;
  }

  /* first build typemaker2 files */
  if (!(builderName && *builderName)) {
    rv=GWB_BuildCtx_Run(buildCtx, numThreads, 0, "tm2builder");
    if (rv<0) {
      fprintf(stderr, "ERROR: Error building typermaker2 files.\n");
      return 3;
    }
  }

  /* build the rest */
  rv=GWB_BuildCtx_Run(buildCtx, numThreads, 0, builderName);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error building builds.\n");
    return 3;
  }

  return 0;
}


