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
#include "cryptssl_rmd160_p.h"
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>



GWEN_ERRORCODE GWEN_CryptImpl_Init(){
  GWEN_ERRORCODE err;
  GWEN_ERRORCODE lerr;

  err=0;

  /* register the various cryptkey types */
  DBG_INFO(0, "Registering RSA");
  lerr=GWEN_CryptKeyRSA_Register();
  if (!GWEN_Error_IsOk(lerr)) {
    DBG_INFO(0, "here");
    err=lerr;
  }

  DBG_INFO(0, "Registering DES");
  lerr=GWEN_CryptKeyDES_Register();
  if (!GWEN_Error_IsOk(lerr)) {
    DBG_INFO(0, "here");
    err=lerr;
  }

  /* register the various MD types */
  DBG_INFO(0, "Registering RMD160");
  lerr=GWEN_MdRmd160_Register();
  if (!GWEN_Error_IsOk(lerr)) {
    DBG_INFO(0, "here");
    err=lerr;
  }

  return err;
}



GWEN_ERRORCODE GWEN_CryptImpl_Fini(){
  return 0;
}








