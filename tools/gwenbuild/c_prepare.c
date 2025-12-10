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


#include "c_prepare.h"
#include "c_setup.h"
#include "utils.h"
#include "gwenbuild/buildctx/buildctx_xml.h"
#include "gwenbuild/buildctx/buildctx_run.h"
#include "gwenbuild/filenames.h"

#include <gwenhywfar/debug.h>




int GWB_Prepare(GWEN_UNUSED GWEN_DB_NODE *dbArgs)
{
  GWB_BUILD_CONTEXT *buildCtx;
  int rv;

  if (GWB_Utils_BuildFilesChanged(GWBUILD_FILE_BUILDFILES)) {
    fprintf(stdout, "Build files changed, repeating last setup command.\n");
    rv=GWB_RepeatLastSetup(GWBUILD_FILE_ARGS);
    if (rv<0) {
      DBG_INFO(NULL, "here");
      return rv;
    }
  }

  buildCtx=GWB_BuildCtx_ReadFromXmlFile(GWBUILD_FILE_CTX);
  if (buildCtx==NULL) {
    fprintf(stderr, "ERROR: Error reading build context from file.\n");
    return 1;
  }

  /* prepare */
  rv=GWB_BuildCtx_Run(buildCtx, 10, 1, NULL);
  if (rv<0) {
    fprintf(stderr, "ERROR: Error preparing builds.\n");
    return 2;
  }

  rv=GWB_BuildCtx_Run(buildCtx, 10, 0, "tm2builder");
  if (rv<0) {
    fprintf(stderr, "ERROR: Error preparing builds.\n");
    return 2;
  }

  return 0;
}



