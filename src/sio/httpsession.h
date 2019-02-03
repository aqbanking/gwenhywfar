/***************************************************************************
    begin       : Fri Feb 15 2008
    copyright   : (C) 2008-2017 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_HTTP_SESSION_H
#define GWEN_HTTP_SESSION_H


#include <gwenhywfar/inherit.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GWEN_HTTP_SESSION GWEN_HTTP_SESSION;
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_HTTP_SESSION, GWENHYWFAR_API)

#ifdef __cplusplus
}
#endif


#include <gwenhywfar/url.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/syncio.h>


#ifndef NO_DEPRECATED_SYMBOLS
/**
 * This flag forces SSLv3 connections when in HTTPS mode.
 */
# define GWEN_HTTP_SESSION_FLAGS_FORCE_SSL3               0x00000001  /* deprecated, will be removed in a future release */
#endif // ifndef NO_DEPRECATED_SYMBOLS
#define GWEN_HTTP_SESSION_FLAGS_NO_CACHE                  0x00000002
#ifndef NO_DEPRECATED_SYMBOLS
# define GWEN_HTTP_SESSION_FLAGS_TLS_ONLY_SAFE_CIPHERS    0x00000004  /* deprecated, will be removed in a future release */
# define GWEN_HTTP_SESSION_FLAGS_TLS_FORCE_UNSAFE_CIPHERS 0x00000008  /* deprecated, will be removed in a future release */
#endif // ifndef NO_DEPRECATED_SYMBOLS

#define GWEN_HTTP_SESSION_FLAGS_TLS_IGN_PREMATURE_CLOSE   0x00000010

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup MOD_HTTP_SESSION HTTP Session
 *
 * This module provides support for exchanging a HTTP(s) request.
 */
/*@{*/

/** @name Contructor/Destructor
 *
 */
/*@{*/

GWENHYWFAR_API
GWEN_HTTP_SESSION *GWEN_HttpSession_new(const char *url, const char *defaultProto, int defaultPort);

GWENHYWFAR_API
void GWEN_HttpSession_Attach(GWEN_HTTP_SESSION *sess);

GWENHYWFAR_API
void GWEN_HttpSession_free(GWEN_HTTP_SESSION *sess);
/*@}*/



/** @name HTTP Setup Functions
 *
 * Functions of this groups should be called before @ref GWEN_HttpSession_Init
 * because the information conveyed via these functions is needed upon
 * initialisation.
 */
/*@{*/

GWENHYWFAR_API
uint32_t GWEN_HttpSession_GetFlags(const GWEN_HTTP_SESSION *sess);

GWENHYWFAR_API
void GWEN_HttpSession_SetFlags(GWEN_HTTP_SESSION *sess, uint32_t fl);

GWENHYWFAR_API
void GWEN_HttpSession_AddFlags(GWEN_HTTP_SESSION *sess, uint32_t fl);

GWENHYWFAR_API
void GWEN_HttpSession_SubFlags(GWEN_HTTP_SESSION *sess, uint32_t fl);

GWENHYWFAR_API
const char *GWEN_HttpSession_GetHttpUserAgent(const GWEN_HTTP_SESSION *sess);

GWENHYWFAR_API
void GWEN_HttpSession_SetHttpUserAgent(GWEN_HTTP_SESSION *sess, const char *s);

GWENHYWFAR_API
const char *GWEN_HttpSession_GetHttpContentType(const GWEN_HTTP_SESSION *sess);

GWENHYWFAR_API
void GWEN_HttpSession_SetHttpContentType(GWEN_HTTP_SESSION *sess, const char *s);


GWENHYWFAR_API
int GWEN_HttpSession_GetHttpVMajor(const GWEN_HTTP_SESSION *sess);

GWENHYWFAR_API
void GWEN_HttpSession_SetHttpVMajor(GWEN_HTTP_SESSION *sess, int i);

GWENHYWFAR_API
int GWEN_HttpSession_GetHttpVMinor(const GWEN_HTTP_SESSION *sess);

GWENHYWFAR_API
void GWEN_HttpSession_SetHttpVMinor(GWEN_HTTP_SESSION *sess, int i);
/*@}*/



/** @name Initialisation and Deinitialisation
 *
 */
/*@{*/
GWENHYWFAR_API
int GWEN_HttpSession_Init(GWEN_HTTP_SESSION *sess);

GWENHYWFAR_API
int GWEN_HttpSession_Fini(GWEN_HTTP_SESSION *sess);

/*@}*/



/** @name Sending and Receiving
 *
 */
/*@{*/

/**
 * This function connects to the server and then sends the given message.
 * The buffer given as argument to this function must only contain the
 * raw data (i.e. the HTTP body, no header).
 * @param sess http session object
 * @param httpCommand HTTP command to send (e.g. "GET", "POST")
 * @param buf pointer to the http body data to send
 * @param blen size of the http body data to send (might be 0)
 */
GWENHYWFAR_API
int GWEN_HttpSession_SendPacket(GWEN_HTTP_SESSION *sess,
                                const char *httpCommand,
                                const uint8_t *buf, uint32_t blen);

/**
 * This function receives a response packet from the server and closes
 * the connection. It expects the connection to be established by
 * @ref GWEN_HttpSession_SendPacket().
 */
GWENHYWFAR_API
int GWEN_HttpSession_RecvPacket(GWEN_HTTP_SESSION *sess, GWEN_BUFFER *buf);

GWENHYWFAR_API
int GWEN_HttpSession_RecvPacketToFile(GWEN_HTTP_SESSION *sess, const char *fname);

/**
 * Test-connect to the server. This function can be used to retrieve the SSL
 * certificate from a server as the cert exchange is part of the establishing of
 * a connection.
 * This function connects to the server and immediately disconnects.
 */
GWENHYWFAR_API
int GWEN_HttpSession_ConnectionTest(GWEN_HTTP_SESSION *sess);

/*@}*/



/** @name SyncIO Initialisation
 *
 * After creating a connection layer (consisting of a chain of GWEN_SYNCIO's) this callback is called.
 * AqBanking uses it to set the TLS-cert-checking callback for the TLS syncio part (@ref GWEN_SyncIo_Tls_SetCheckCertFn),
 * so whenever a TLS connection is established and a certificate received, that function is called.
 */
/*@{*/

typedef int GWENHYWFAR_CB(*GWEN_HTTPSESSION_INITSYNCIO_FN)(GWEN_HTTP_SESSION *sess, GWEN_SYNCIO *sio);
GWENHYWFAR_API GWEN_HTTPSESSION_INITSYNCIO_FN GWEN_HttpSession_SetInitSyncIoFn(GWEN_HTTP_SESSION *sess,
                                                                               GWEN_HTTPSESSION_INITSYNCIO_FN f);

/*@}*/



/*@}*/ /* defgroup */



#ifdef __cplusplus
}
#endif


#endif

