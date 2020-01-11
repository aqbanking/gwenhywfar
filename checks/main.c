
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "checks.h"

#include <gwenhywfar/cgui.h>
#include <gwenhywfar/gui_be.h>




int GWENHYWFAR_CB checkCert(GWEN_UNUSED GWEN_GUI *gui,
                            GWEN_UNUSED const GWEN_SSLCERTDESCR *cert,
                            GWEN_UNUSED GWEN_SYNCIO *sio,
                            GWEN_UNUSED uint32_t guiid)
{
  /* automatically accept any cert for non-interactive checks */
  return 0;
}



int main(GWEN_UNUSED int argc, GWEN_UNUSED char **argv)
{
  int errs=0;
  GWEN_GUI *gui;

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetCheckCertFn(gui, checkCert);
  GWEN_Gui_SetGui(gui);

  if (check_db())
    errs++;

#ifdef GWENHYWFAR_SKIP_NETWORK_CHECKS
  fprintf(stderr, "Skipping all checks that assume available network connectivity\n"
          "Run ./configure without --disable-network-checks to enable the skipped checks\n");
#else
  if (check_syncio_tls())
    errs++;

  if (check_syncio_http())
    errs++;
  if (check_syncio_https())
    errs++;
#endif


  if (errs)
    return 2;

  return 0;
}


