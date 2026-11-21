
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "checks.h"

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/error.h>
#include <gwenhywfar/cgui.h>
#include <gwenhywfar/gui_be.h>


static int check_buffer_appendargs(void)
{
  struct {
    unsigned char pre[16];
    char raw[16];
    unsigned char post[16];
  } guarded;
  GWEN_BUFFER *bf;
  GWEN_BUFFER *dyn;
  char big[1400];
  int rv;
  unsigned int i;

  memset(guarded.pre, 0xA5, sizeof(guarded.pre));
  memset(guarded.raw, 0, sizeof(guarded.raw));
  memset(guarded.post, 0xA5, sizeof(guarded.post));

  bf=GWEN_Buffer_new(guarded.raw, (uint32_t)sizeof(guarded.raw), 0, 0);
  if (!bf)
    return 1;
  GWEN_Buffer_SubMode(bf, GWEN_BUFFER_MODE_DYNAMIC | GWEN_BUFFER_MODE_ABORT_ON_MEMFULL);

  rv=GWEN_Buffer_AppendArgs(bf, "ok:%d", 7);
  if (rv!=0 || strcmp(GWEN_Buffer_GetStart(bf), "ok:7")!=0 || GWEN_Buffer_GetUsedBytes(bf)!=4) {
    fprintf(stderr, "ERROR: Fixed buffer small AppendArgs failed\n");
    GWEN_Buffer_free(bf);
    return 1;
  }

  rv=GWEN_Buffer_AppendArgs(bf, "0123456789abcdef");
  if (rv!=GWEN_ERROR_BUFFER_OVERFLOW ||
      strcmp(GWEN_Buffer_GetStart(bf), "ok:7")!=0 ||
      GWEN_Buffer_GetUsedBytes(bf)!=4 ||
      GWEN_Buffer_GetPos(bf)!=4) {
    fprintf(stderr, "ERROR: Fixed buffer overflow did not fail cleanly\n");
    GWEN_Buffer_free(bf);
    return 1;
  }

  for (i=0; i<sizeof(guarded.pre); i++) {
    if (guarded.pre[i]!=0xA5 || guarded.post[i]!=0xA5) {
      fprintf(stderr, "ERROR: Canary corrupted in fixed buffer AppendArgs\n");
      GWEN_Buffer_free(bf);
      return 1;
    }
  }
  GWEN_Buffer_free(bf);

  dyn=GWEN_Buffer_new(0, 128, 0, 1);
  if (!dyn)
    return 1;
  GWEN_Buffer_SubMode(dyn, GWEN_BUFFER_MODE_ABORT_ON_MEMFULL);
  GWEN_Buffer_SetHardLimit(dyn, 128);
  for (i=0; i<32; i++) {
    rv=GWEN_Buffer_AppendArgs(dyn, "\\u%04x", 10);
    if (rv<0)
      break;
  }
  if (rv!=GWEN_ERROR_BUFFER_OVERFLOW || GWEN_Buffer_GetPos(dyn)>=128 ||
      GWEN_Buffer_GetUsedBytes(dyn)>=128) {
    fprintf(stderr, "ERROR: Hard-limited AppendArgs exceeded buffer bounds\n");
    GWEN_Buffer_free(dyn);
    return 1;
  }
  if (GWEN_Buffer_AllocRoom(dyn, (uint32_t)-1)!=GWEN_ERROR_BUFFER_OVERFLOW) {
    fprintf(stderr, "ERROR: AllocRoom failed to reject uint32_t wrap\n");
    GWEN_Buffer_free(dyn);
    return 1;
  }
  while (GWEN_Buffer_GetMaxUnsegmentedWrite(dyn)>0) {
    if (GWEN_Buffer_AppendByte(dyn, 'a')<0)
      break;
  }
  if (GWEN_Buffer_GetMaxUnsegmentedWrite(dyn)!=0) {
    fprintf(stderr, "ERROR: GetMaxUnsegmentedWrite failed on full buffer\n");
    GWEN_Buffer_free(dyn);
    return 1;
  }
  GWEN_Buffer_free(dyn);

  dyn=GWEN_Buffer_new(0, 64, 0, 1);
  if (!dyn)
    return 1;
  memset(big, 'x', sizeof(big)-1);
  big[sizeof(big)-1]='\0';
  rv=GWEN_Buffer_AppendArgs(dyn, "%s", big);
  if (rv!=0 || GWEN_Buffer_GetUsedBytes(dyn)!=sizeof(big)-1 ||
      strcmp(GWEN_Buffer_GetStart(dyn), big)!=0) {
    fprintf(stderr, "ERROR: Dynamic multi-KB AppendArgs failed\n");
    GWEN_Buffer_free(dyn);
    return 1;
  }
  GWEN_Buffer_free(dyn);

  return 0;
}




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
  if (check_buffer_appendargs())
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


