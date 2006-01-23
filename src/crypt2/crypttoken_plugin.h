/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef GWEN_CRYPTTOKEN_PLUGIN_H
#define GWEN_CRYPTTOKEN_PLUGIN_H

#include <gwenhywfar/keyspec.h>
#include <gwenhywfar/crypt.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/plugin.h>


#ifdef __cplusplus
extern "C" {
#endif


/** @name CryptToken_Plugin
 *
 */
/*@{*/

typedef GWEN_CRYPTTOKEN*
  (*GWEN_CRYPTTOKEN_PLUGIN_CREATETOKEN_FN)(GWEN_PLUGIN *pl,
                                           const char *subTypeName,
                                           const char *name);

typedef int
  (*GWEN_CRYPTTOKEN_PLUGIN_CHECKTOKEN_FN)(GWEN_PLUGIN *pl,
                                          GWEN_BUFFER *subTypeName,
                                          GWEN_BUFFER *name);



GWENHYWFAR_API
GWEN_PLUGIN *GWEN_CryptToken_Plugin_new(GWEN_PLUGIN_MANAGER *mgr,
                                        GWEN_CRYPTTOKEN_DEVICE devType,
                                        const char *typeName,
                                        const char *fileName);


GWENHYWFAR_API
GWEN_CRYPTTOKEN*
GWEN_CryptToken_Plugin_CreateToken(GWEN_PLUGIN *pl,
                                   const char *subTypeName,
                                   const char *name);

/**
 * <p>This function is used to let a crypt token plugin check whether it
 * supports a given plugin.</p>
 * <p>Initially the given buffers may contain some values to narrow the
 * search. For chip cards the <i>name</i> argument may contain the serial
 * number of the card (if known). For file based crypt tokens this argument
 * must contain the path to the file to check.</p>
 *
 * This function should return one of the following error codes:
 * <ul>
 *  <li>GWEN_SUCCESS: CryptToken is supported by this plugin, the buffers
 *   for typeName, subTypeName and name are updated accordingly</li>
 *  <li>GWEN_ERROR_CT_NOT_IMPLEMENTED: function not implmented</li>
 *  <li>GWEN_ERROR_CT_NOT_SUPPORTED: medium not supported by this plugin</li>
 *  <li>GWEN_ERROR_CT_BAD_NAME: Medium is supported but the name doesn't
 *      match that of the currently checked medium</li>
 *  <li>GWEN_ERROR_CT_IO_ERROR: any type of IO error occurred</li>
 *  <li>other codes as appropriate</li>
 * </ul>
 */
GWENHYWFAR_API
int GWEN_CryptToken_Plugin_CheckToken(GWEN_PLUGIN *pl,
                                      GWEN_BUFFER *subTypeName,
                                      GWEN_BUFFER *name);


GWENHYWFAR_API
GWEN_CRYPTTOKEN_DEVICE
  GWEN_CryptToken_Plugin_GetDeviceType(const GWEN_PLUGIN *pl);


GWENHYWFAR_API
void GWEN_CryptToken_Plugin_SetCreateTokenFn(GWEN_PLUGIN *pl,
                                             GWEN_CRYPTTOKEN_PLUGIN_CREATETOKEN_FN fn);

GWENHYWFAR_API
void GWEN_CryptToken_Plugin_SetCheckTokenFn(GWEN_PLUGIN *pl,
                                            GWEN_CRYPTTOKEN_PLUGIN_CHECKTOKEN_FN fn);


/*@}*/



/** @name CryptManager
 *
 */
/*@{*/
typedef int (*GWEN_CRYPTMANAGER_GETPIN_FN)(GWEN_PLUGIN_MANAGER *mgr,
                                           GWEN_CRYPTTOKEN *token,
                                           GWEN_CRYPTTOKEN_PINTYPE pt,
                                           GWEN_CRYPTTOKEN_PINENCODING pe,
                                           GWEN_TYPE_UINT32 flags,
                                           unsigned char *buffer,
                                           unsigned int minLength,
                                           unsigned int maxLength,
                                           unsigned int *pinLength);

typedef
  int (*GWEN_CRYPTMANAGER_SETPINSTATUS_FN)(GWEN_PLUGIN_MANAGER *mgr,
					   GWEN_CRYPTTOKEN *token,
					   GWEN_CRYPTTOKEN_PINTYPE pt,
					   GWEN_CRYPTTOKEN_PINENCODING pe,
					   GWEN_TYPE_UINT32 flags,
					   unsigned char *buffer,
					   unsigned int pinLength,
					   int isOk);

typedef int (*GWEN_CRYPTMANAGER_BEGIN_ENTER_PIN_FN)(GWEN_PLUGIN_MANAGER *mgr,
                                                    GWEN_CRYPTTOKEN *token,
                                                    GWEN_CRYPTTOKEN_PINTYPE pt);
typedef int (*GWEN_CRYPTMANAGER_END_ENTER_PIN_FN)(GWEN_PLUGIN_MANAGER *mgr,
                                                   GWEN_CRYPTTOKEN *token,
                                                   GWEN_CRYPTTOKEN_PINTYPE pt,
                                                   int ok);
typedef int (*GWEN_CRYPTMANAGER_INSERT_TOKEN_FN)(GWEN_PLUGIN_MANAGER *mgr,
                                                 GWEN_CRYPTTOKEN *token);
typedef int (*GWEN_CRYPTMANAGER_INSERT_CORRECT_TOKEN_FN)(GWEN_PLUGIN_MANAGER *mgr,
                                                         GWEN_CRYPTTOKEN *token);
typedef int (*GWEN_CRYPTMANAGER_SHOW_MESSAGE_FN)(GWEN_PLUGIN_MANAGER *mgr,
                                                 GWEN_CRYPTTOKEN *token,
                                                 const char *title,
                                                 const char *msg);




GWENHYWFAR_API
GWEN_PLUGIN_MANAGER *GWEN_CryptManager_new();

/**
 * This function tries to find a token plugin which is able to handle the
 * token given by the device type and name.
 */
GWENHYWFAR_API
int GWEN_CryptManager_CheckToken(GWEN_PLUGIN_MANAGER *cm,
                                 GWEN_CRYPTTOKEN_DEVICE devt,
                                 GWEN_BUFFER *typeName,
                                 GWEN_BUFFER *subTypeName,
                                 GWEN_BUFFER *tokenName);


GWENHYWFAR_API
void GWEN_CryptManager_SetGetPinFn(GWEN_PLUGIN_MANAGER *cm,
                                   GWEN_CRYPTMANAGER_GETPIN_FN fn);

GWENHYWFAR_API void
GWEN_CryptManager_SetSetPinStatusFn(GWEN_PLUGIN_MANAGER *cm,
				    GWEN_CRYPTMANAGER_SETPINSTATUS_FN fn);

GWENHYWFAR_API
void GWEN_CryptManager_SetBeginEnterPinFn(GWEN_PLUGIN_MANAGER *cm,
                                          GWEN_CRYPTMANAGER_BEGIN_ENTER_PIN_FN fn);

GWENHYWFAR_API
void GWEN_CryptManager_SetEndEnterPinFn(GWEN_PLUGIN_MANAGER *cm,
                                         GWEN_CRYPTMANAGER_END_ENTER_PIN_FN fn);

GWENHYWFAR_API
void GWEN_CryptManager_SetInsertTokenFn(GWEN_PLUGIN_MANAGER *cm,
                                        GWEN_CRYPTMANAGER_INSERT_TOKEN_FN fn);

GWENHYWFAR_API
void GWEN_CryptManager_SetInsertCorrectTokenFn(GWEN_PLUGIN_MANAGER *cm,
                                               GWEN_CRYPTMANAGER_INSERT_CORRECT_TOKEN_FN fn);

GWENHYWFAR_API
void GWEN_CryptManager_SetShowMessageFn(GWEN_PLUGIN_MANAGER *cm,
                                        GWEN_CRYPTMANAGER_SHOW_MESSAGE_FN fn);



GWENHYWFAR_API
int GWEN_CryptManager_GetPin(GWEN_PLUGIN_MANAGER *cm,
                             GWEN_CRYPTTOKEN *token,
                             GWEN_CRYPTTOKEN_PINTYPE pt,
                             GWEN_CRYPTTOKEN_PINENCODING pe,
                             GWEN_TYPE_UINT32 flags,
                             unsigned char *buffer,
                             unsigned int minLength,
                             unsigned int maxLength,
                             unsigned int *pinLength);

GWENHYWFAR_API
int GWEN_CryptManager_SetPinStatus(GWEN_PLUGIN_MANAGER *mgr,
				   GWEN_CRYPTTOKEN *token,
				   GWEN_CRYPTTOKEN_PINTYPE pt,
				   GWEN_CRYPTTOKEN_PINENCODING pe,
				   GWEN_TYPE_UINT32 flags,
				   unsigned char *buffer,
				   unsigned int pinLength,
				   int isOk);


GWENHYWFAR_API
int GWEN_CryptManager_BeginEnterPin(GWEN_PLUGIN_MANAGER *cm,
                                    GWEN_CRYPTTOKEN *token,
                                    GWEN_CRYPTTOKEN_PINTYPE pt);

GWENHYWFAR_API
int GWEN_CryptManager_EndEnterPin(GWEN_PLUGIN_MANAGER *cm,
                                  GWEN_CRYPTTOKEN *token,
                                  GWEN_CRYPTTOKEN_PINTYPE pt,
                                  int ok);

GWENHYWFAR_API
int GWEN_CryptManager_InsertToken(GWEN_PLUGIN_MANAGER *cm,
                                  GWEN_CRYPTTOKEN *token);

GWENHYWFAR_API
int GWEN_CryptManager_InsertCorrectToken(GWEN_PLUGIN_MANAGER *cm,
                                         GWEN_CRYPTTOKEN *token);

GWENHYWFAR_API
int GWEN_CryptManager_ShowMessage(GWEN_PLUGIN_MANAGER *cm,
                                  GWEN_CRYPTTOKEN *token,
                                  const char *title,
                                  const char *msg);

GWENHYWFAR_API
int GWEN_CryptManager_CheckToken(GWEN_PLUGIN_MANAGER *cm,
                                 GWEN_CRYPTTOKEN_DEVICE devt,
                                 GWEN_BUFFER *typeName,
                                 GWEN_BUFFER *subTypeName,
                                 GWEN_BUFFER *tokenName);

GWENHYWFAR_API
GWEN_PLUGIN_DESCRIPTION_LIST2*
  GWEN_CryptManager_GetPluginDescrs(GWEN_PLUGIN_MANAGER *pm,
                                    GWEN_CRYPTTOKEN_DEVICE devt);

/*@}*/


#ifdef __cplusplus
}
#endif



#endif /* GWEN_CRYPTTOKEN_PLUGIN_H */


