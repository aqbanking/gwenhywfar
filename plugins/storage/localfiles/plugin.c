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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "lf_storage_l.h"
#include <gwenhywfar/st_plugin_be.h>



static GWEN_STO_STORAGE *LocalFilesPlugin_Factory(GWEN_PLUGIN *pl,
                                                  const char *address) {
  return LocalFilesStorage_new(address);
}



GWENHYWFAR_EXPORT
GWEN_PLUGIN *storage_localfiles_factory(GWEN_PLUGIN_MANAGER *pm,
                                        const char *modName,
                                        const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=GWEN_StoPlugin_new(pm, modName, fileName);
  assert(pl);

  GWEN_StoPlugin_SetFactoryFn(pl, LocalFilesPlugin_Factory);

  return pl;
}



