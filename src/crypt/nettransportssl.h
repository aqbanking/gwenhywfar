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


#ifndef GWEN_NETTRANSPORTSSL_H
#define GWEN_NETTRANSPORTSSL_H

#include <gwenhywfar/nettransport.h>
#include <gwenhywfar/db.h>

#ifdef __cplusplus
extern "C" {
#endif


/** @name Password Functions
 *
 */
/*@{*/

/**
 * This is the prototype for the callback function which asks the user
 * for a password.
 * @param tr transport layer involved
 * @param buffer destination buffer for the password
 * @param num size of the password buffer
 * @param rwflag if 1 then the password is to be created (in this case
 * the function should let the user verify the password before writing
 * it into the buffer).
 */
GWENHYWFAR_API
typedef int (*GWEN_NETTRANSPORTSSL_GETPASSWD_FN)(GWEN_NETTRANSPORT *tr,
                                                 char *buffer, int num,
                                                 int rwflag);
/**
 * Sets the callback function which asks the user for a password.
 */
GWENHYWFAR_API
  void
  GWEN_NetTransportSSL_SetGetPasswordFn(GWEN_NETTRANSPORTSSL_GETPASSWD_FN fn);

/**
 * Returns the currently active callback function.
 */
GWENHYWFAR_API
  GWEN_NETTRANSPORTSSL_GETPASSWD_FN
  GWEN_NetTransportSSL_GetGetPasswordFn();
/*@}*/


/** @name Certificate Functions
 *
 */
/*@{*/
GWENHYWFAR_API
  typedef enum {
    /** error asking the user */
    GWEN_NetTransportSSL_AskAddCertResultError=0,
    /** user does not trust the given certificate */
    GWEN_NetTransportSSL_AskAddCertResultNo,
    /** user trusts the given certificate for this session only */
    GWEN_NetTransportSSL_AskAddCertResultTmp,
    /** user trusts the given certificate permanently */
    GWEN_NetTransportSSL_AskAddCertResultPerm
  } GWEN_NETTRANSPORTSSL_ASKADDCERT_RESULT;

/**
 * This is the prototype of the callback which asks the user whether he wants
 * to add the given certificate to the list of trusted ones.
 * @param tr transport layer via which the certificate has been received
 * @param cert a DB node containing information about the certificate
 */
GWENHYWFAR_API
  typedef GWEN_NETTRANSPORTSSL_ASKADDCERT_RESULT
  (*GWEN_NETTRANSPORTSSL_ASKADDCERT_FN)(GWEN_NETTRANSPORT *tr,
                                        GWEN_DB_NODE *cert);

/**
 * Sets the callback function for asking the user whether to accept a given
 * certificate.
 */
GWENHYWFAR_API
  void
  GWEN_NetTransportSSL_SetAskAddCertFn(GWEN_NETTRANSPORTSSL_ASKADDCERT_FN fn);

/**
 * Returns the currently active callback function.
 */
GWENHYWFAR_API
  GWEN_NETTRANSPORTSSL_ASKADDCERT_FN
  GWEN_NetTransportSSL_GetAskAddCertFn();

/*@}*/


/**
 * Creates a new SSL socket.
 * @param cadir name of a folder containing trusted certificates (or 0)
 * @param ownCertFile name of a file containing the certificate to be
 *   used (or 0)
 * @param secure if !=0 then the peer is forced to authenticate
 * @param takeOver if !=0 then the new object will take over the socket
 */
GWENHYWFAR_API
GWEN_NETTRANSPORT *GWEN_NetTransportSSL_new(GWEN_SOCKET *sk,
                                            const char *capath,
                                            const char *ownCertFile,
                                            const char *dhfile,
                                            int secure,
                                            int takeOver);

/** @name Cipher List
 *
 */
/*@{*/
/**
 * This function allows to set a list of allowed cipher algorithms.
 * It is a colon separated list. For OpenSSL you can get a list of
 * available ciphers via "openssl ciphers".
 */
GWENHYWFAR_API
  void GWEN_NetTransportSSL_SetCipherList(GWEN_NETTRANSPORT *tr,
                                          const char *ciphers);

/**
 * This function returns a list of supported ciphers. For each supported
 * cipher there is a group called "cipher" with the variables:
 * <ul>
 *  <li>name</li>
 *  <li>version</li>
 *  <li>description</li>
 * </ul>
 * You can use the <i>name</i>variables to create the string for
 * @ref GWEN_NetTransportSSL_SetCipherList.
 */
GWENHYWFAR_API
  GWEN_DB_NODE *GWEN_NetTransportSSL_GetCipherList();
/*@}*/


#ifdef __cplusplus
}
#endif


#endif /* GWEN_NETTRANSPORTSSL_H */


