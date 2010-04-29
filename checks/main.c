
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "checks.h"

#include <gwenhywfar/cgui.h>
#include <gwenhywfar/gui_be.h>




int checkCert(GWEN_GUI *gui,
	      const GWEN_SSLCERTDESCR *cert,
	      GWEN_SYNCIO *sio,
	      uint32_t guiid) {
  /* automatically accept any cert for non-interactive checks */
  return 0;
}



int main(int argc, char **argv) {
  int errs=0;
  GWEN_GUI *gui;

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetCheckCertFn(gui, checkCert);
  GWEN_Gui_SetGui(gui);

  //return check_io_tls();

#if 0
  if (check_db())
    errs++;

  if (check_io_file())
    errs++;

  if (check_io_buffered())
    errs++;

  if (check_io_socket())
    errs++;

  if (check_io_https())
    errs++;

  if (check_io_packets())
    errs++;

  if (check_syncio_tls())
    errs++;

#endif
  if (check_syncio_http())
    errs++;
  if (check_syncio_https())
    errs++;


  if (errs)
    return 2;

  return 0;
}


