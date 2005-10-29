/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Wed May 05 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/



#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "bio_netlayer_p.h"
#include <stdio.h>
#include <errno.h>
#include <openssl/bio.h>

#include <gwenhywfar/netlayer.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>



static BIO_METHOD bio_netlayer_methods={
  BIO_TYPE_SOCKET,
  "GWEN_NetLayer",
  BIO_netlayer_write,
  BIO_netlayer_read,
  BIO_netlayer_puts,
  NULL, /* gets */
  BIO_netlayer_ctrl,
  BIO_netlayer_create,
  BIO_netlayer_destroy,
  NULL, /* callback_ctrl */
};



BIO *BIO_netlayer_new(GWEN_NETLAYER *nl){
  BIO *bio;

  assert(nl);
  bio=BIO_new(&bio_netlayer_methods);
  if (bio==NULL)
    return NULL;

  bio->ptr=nl;
  GWEN_NetLayer_Attach(nl);

  return bio;
}



int BIO_netlayer_create(BIO *bio) {
  DBG_INFO(GWEN_LOGDOMAIN, "BIO method: Create");
  bio->init=1;
  bio->num=0;
  bio->flags=0;
  return 1;
}



int BIO_netlayer_destroy(BIO *bio){
  DBG_INFO(GWEN_LOGDOMAIN, "BIO method: Destroy");
  if (bio) {
    GWEN_NETLAYER *nl;

    nl=(GWEN_NETLAYER*) bio->ptr;
    if (bio->shutdown){
      /* disconnect layer */
      GWEN_NetLayer_Disconnect(nl);
      bio->init=0;
      bio->flags=0;
    }
    GWEN_NetLayer_free(nl); /* detach from netlayer */
    return 1;
  }
  else
    return 0;
}



int BIO_netlayer_read(BIO *bio, char *in, int inl){
  DBG_INFO(GWEN_LOGDOMAIN, "BIO method: Read(%d)", inl);
  if (in!=NULL) {
    GWEN_NETLAYER *nl;
    int bsize;
    int rv;

    nl=(GWEN_NETLAYER*) bio->ptr;
    BIO_clear_retry_flags(bio);
    if (nl==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "No netlayer in BIO");
      return -1;
    }

    bsize=inl;
    rv=GWEN_NetLayer_Read(nl, in, &bsize);
    if (rv==0) {
      return bsize;
    }
    else if (rv==1) {
      BIO_set_retry_read(bio);
      return -1;
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return -1;
    }
  }
  return 0;
}



int BIO_netlayer_write(BIO *bio, const char *out, int outl){
  DBG_INFO(GWEN_LOGDOMAIN, "BIO method: Write(%d)", outl);
  if (out!=NULL) {
    GWEN_NETLAYER *nl;
    int bsize;
    int rv;

    nl=(GWEN_NETLAYER*) bio->ptr;
    BIO_clear_retry_flags(bio);
    if (nl==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "No netlayer in BIO");
      return -1;
    }

    bsize=outl;
    rv=GWEN_NetLayer_Write(nl, out, &bsize);
    if (rv==0) {
      return bsize;
    }
    else if (rv==1) {
      BIO_set_retry_write(bio);
      return -1;
    }
    else {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      return -1;
    }
  }
  return 0;
}



long BIO_netlayer_ctrl(BIO *bio, int cmd, long num, void *ptr){
  long rv=1;

  DBG_INFO(GWEN_LOGDOMAIN, "BIO method: Ctl(%d, %li)", cmd, num);
  switch (cmd){

  case BIO_C_GET_FD:
    rv=-1;
    break;

  case BIO_CTRL_GET_CLOSE:
    rv=bio->shutdown;
    break;

  case BIO_CTRL_FLUSH:
    rv=1;
    break;

  case BIO_CTRL_SET_CLOSE:
    bio->shutdown=(int)num;
    break;

  case BIO_CTRL_RESET:
  case BIO_C_FILE_SEEK:
  case BIO_C_FILE_TELL:
  case BIO_CTRL_INFO:
  case BIO_C_SET_FD:
  case BIO_CTRL_PENDING:
  case BIO_CTRL_WPENDING:
  case BIO_CTRL_DUP:
  default:
    rv=0;
    break;
  }
  return rv;
}



int BIO_netlayer_puts(BIO *bp, const char *str){
  int l, rv;

  DBG_INFO(GWEN_LOGDOMAIN, "BIO method: Puts(\"%s\")", str);
  l=strlen(str);
  rv=BIO_netlayer_write(bp, str, l);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }
  return rv;
}




