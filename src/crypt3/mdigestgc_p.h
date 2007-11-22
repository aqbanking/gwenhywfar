/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: crypttoken.h 1113 2007-01-10 09:14:16Z martin $
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWENHYWFAR_MDIGESTGC_P_H
#define GWENHYWFAR_MDIGESTGC_P_H


#include "mdigest_be.h"


#include <gcrypt.h>



typedef struct GWEN_MDIGEST_GC GWEN_MDIGEST_GC;
struct GWEN_MDIGEST_GC {
  gcry_md_hd_t handle;
  int algo;
  unsigned int flags;
  int isOpen;
};


static GWEN_MDIGEST *GWEN_MDigest_Gc_new(GWEN_CRYPT_HASHALGOID a);
static GWENHYWFAR_CB void GWEN_Digest_Gc_freeData(void *bp, void *p);

static int GWEN_MDigest_Gc_Begin(GWEN_MDIGEST *md);
static int GWEN_MDigest_Gc_Update(GWEN_MDIGEST *md, const uint8_t *buf, unsigned int l);
static int GWEN_MDigest_Gc_End(GWEN_MDIGEST *md);






#endif


