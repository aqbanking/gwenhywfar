/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Nov 06 2003
 copyright   : (C) 2003 by Martin Preuss
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


#include "cryptssl_p.h"
#include "cryptssl_des_p.h"
#include "cryptssl_rsa_p.h"
#include "cryptssl_md5_p.h"
#include "cryptssl_rmd160_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/des.h>



GWEN_ERRORCODE GWEN_CryptImpl_Init(){
  GWEN_ERRORCODE err;
  GWEN_ERRORCODE lerr;

  err=0;

  DBG_INFO(GWEN_LOGDOMAIN, "Initializing OpenSSL");
  SSL_load_error_strings();
  SSL_library_init();

  /* register the various cryptkey types */
  DBG_INFO(GWEN_LOGDOMAIN, "Registering RSA");
  lerr=GWEN_CryptKeyRSA_Register();
  if (!GWEN_Error_IsOk(lerr)) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    err=lerr;
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Registering DES");
  lerr=GWEN_CryptKeyDES_Register();
  if (!GWEN_Error_IsOk(lerr)) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    err=lerr;
  }

  /* register the various MD types */
  DBG_INFO(GWEN_LOGDOMAIN, "Registering RMD160");
  lerr=GWEN_MdRmd160_Register();
  if (!GWEN_Error_IsOk(lerr)) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    err=lerr;
  }

  DBG_INFO(GWEN_LOGDOMAIN, "Registering MD5");
  lerr=GWEN_MdMd5_Register();
  if (!GWEN_Error_IsOk(lerr)) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    err=lerr;
  }

  return err;
}



GWEN_ERRORCODE GWEN_CryptImpl_Fini(){
  ERR_free_strings();
  return 0;
}



void GWEN_CryptImpl_Dump_Bignum(BIGNUM *bn, FILE *f, int indent) {
  unsigned char *bnbuf;

  bnbuf=(unsigned char*)malloc(BN_num_bytes(bn));
  BN_bn2bin(bn, bnbuf);
  GWEN_Text_DumpString((char*)bnbuf, BN_num_bytes(bn), f, indent);
  free(bnbuf);
}



int GWEN_Crypt_IsAvailable() {
  return 1;
}



int GWEN_CryptKey_FromPasswordSSL(const char *password,
                                  unsigned char *buffer,
                                  unsigned int bsize){
  des_cblock left, right;
  int i;

  if (bsize!=16) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer must be exact 16 bytes in length");
    return -1;
  }
  des_string_to_2keys(password, &left, &right);
  for (i=0; i<8; i++)
    *(buffer++)=left[i];
  for (i=0; i<8; i++)
    *(buffer++)=right[i];
  return 0;
}

















