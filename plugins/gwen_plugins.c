/***************************************************************************
    begin       : Sun Apr 13 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "gwen_plugins.h"

#include <gwenhywfar/plugin.h>



/* declarations of the plugin factory functions */

GWEN_PLUGIN *ct_ohbci_factory(GWEN_PLUGIN_MANAGER *pm,
			      const char *modName,
			      const char *fileName);


GWEN_PLUGIN *dbio_csv_factory(GWEN_PLUGIN_MANAGER *pm,
			      const char *modName,
			      const char *fileName);

GWEN_PLUGIN *dbio_olddb_factory(GWEN_PLUGIN_MANAGER *pm,
				const char *modName,
				const char *fileName);

GWEN_PLUGIN *dbio_xmldb_factory(GWEN_PLUGIN_MANAGER *pm,
				const char *modName,
				const char *fileName);




int GWEN_Plugins_Init() {
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_PLUGIN *p;

  pm=GWEN_PluginManager_FindPluginManager("ct");
  if (pm) {
#ifdef GWENHYWFAR_WITH_PLUGIN_CT_OHBCI
    p=ct_ohbci_factory(pm, "ohbci", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
  }

  pm=GWEN_PluginManager_FindPluginManager("dbio");
  if (pm) {
#ifdef GWENHYWFAR_WITH_PLUGIN_DBIO_CSV
    p=dbio_csv_factory(pm, "csv", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif

#ifdef GWENHYWFAR_WITH_PLUGIN_DBIO_OLDDB
    p=dbio_olddb_factory(pm, "olddb", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif

#ifdef GWENHYWFAR_WITH_PLUGIN_DBIO_XMLDB
    p=dbio_xmldb_factory(pm, "xmldb", NULL);
    if (p)
      GWEN_PluginManager_AddPlugin(pm, p);
#endif
  }

  return 0;
}

