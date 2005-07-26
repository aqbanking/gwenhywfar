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


#ifndef GWEN_CRYPTTOKEN_FILE_H
#define GWEN_CRYPTTOKEN_FILE_H

#include <gwenhywfar/crypttoken.h>
#include <gwenhywfar/crypttoken_user.h>


/**
 * <p>A crypt token file may contain mutliple user contexts.</p>
 * <p>Every user context contains the local signature sequence counter,
 * 4 keys and a GWEN_CRYPTTOKEN_USER.</p>
 * <p>The key ids must be chosen according to the following table:</p>
 * <table>
 * <tr><th>Key id</th><th>Description</th></tr>
 * <tr><td>XXXXXX01</td><td>LocalSignKey</td></tr>
 * <tr><td>XXXXXX02</td><td>LocalCryptKey</td></tr>
 * <tr><td>XXXXXX03</td><td>RemoteSignKey</td></tr>
 * <tr><td>XXXXXX04</td><td>RemoteCryptKey</td></tr>
 * </table>
 * <p>This table shows that the lower 8 bits are predefined. The other 24 bits
 * can be freely chosen by the implementation of the plugin.</p>
 */


typedef struct GWEN_CT_FILE_CONTEXT GWEN_CT_FILE_CONTEXT;

GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_CT_FILE_CONTEXT, GWENHYWFAR_API)
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_CT_FILE_CONTEXT,
                            GWEN_CryptTokenFile_Context,
                            GWENHYWFAR_API)
GWEN_LIST2_FUNCTION_LIB_DEFS(GWEN_CT_FILE_CONTEXT,
                             GWEN_CryptTokenFile_Context,
                             GWENHYWFAR_API)



GWEN_CT_FILE_CONTEXT *GWEN_CryptTokenFile_Context_new();
void GWEN_CryptTokenFile_Context_free(GWEN_CT_FILE_CONTEXT *fc);

unsigned int
  GWEN_CryptTokenFile_Context_GetLocalSignSeq(const GWEN_CT_FILE_CONTEXT *fc);
void GWEN_CryptTokenFile_Context_SetLocalSignSeq(GWEN_CT_FILE_CONTEXT *fc,
                                         unsigned int i);

unsigned int
  GWEN_CryptTokenFile_Context_GetRemoteSignSeq(const GWEN_CT_FILE_CONTEXT *fc);
void GWEN_CryptTokenFile_Context_SetRemoteSignSeq(GWEN_CT_FILE_CONTEXT *fc,
                                          unsigned int i);

GWEN_CRYPTKEY*
  GWEN_CryptTokenFile_Context_GetLocalSignKey(const GWEN_CT_FILE_CONTEXT *fc);
void GWEN_CryptTokenFile_Context_SetLocalSignKey(GWEN_CT_FILE_CONTEXT *fc,
                                         GWEN_CRYPTKEY *key);

GWEN_CRYPTKEY*
  GWEN_CryptTokenFile_Context_GetLocalCryptKey(const GWEN_CT_FILE_CONTEXT *fc);
void GWEN_CryptTokenFile_Context_SetLocalCryptKey(GWEN_CT_FILE_CONTEXT *fc,
                                          GWEN_CRYPTKEY *key);

GWEN_CRYPTKEY*
  GWEN_CryptTokenFile_Context_GetRemoteSignKey(const GWEN_CT_FILE_CONTEXT *fc);
void GWEN_CryptTokenFile_Context_SetRemoteSignKey(GWEN_CT_FILE_CONTEXT *fc,
                                          GWEN_CRYPTKEY *key);

GWEN_CRYPTKEY*
  GWEN_CryptTokenFile_Context_GetRemoteCryptKey(const GWEN_CT_FILE_CONTEXT *fc);
void GWEN_CryptTokenFile_Context_SetRemoteCryptKey(GWEN_CT_FILE_CONTEXT *fc,
                                                   GWEN_CRYPTKEY *key);

GWEN_CRYPTTOKEN_USER*
  GWEN_CryptTokenFile_Context_GetUser(const GWEN_CT_FILE_CONTEXT *fc);
void GWEN_CryptTokenFile_Context_SetUser(GWEN_CT_FILE_CONTEXT *fc,
                                         GWEN_CRYPTTOKEN_USER *u);





typedef struct GWEN_CRYPTTOKEN_FILE GWEN_CRYPTTOKEN_FILE;


typedef int (*GWEN_CRYPTTOKEN_FILE_READ_FN)(GWEN_CRYPTTOKEN *ct,
                                            int fd);

typedef int (*GWEN_CRYPTTOKEN_FILE_WRITE_FN)(GWEN_CRYPTTOKEN *ct,
                                             int fd);


GWEN_CRYPTTOKEN *GWEN_CryptTokenFile_new(GWEN_PLUGIN_MANAGER *pm,
                                         const char *typeName,
                                         const char *subTypeName,
                                         const char *name);


void GWEN_CryptTokenFile_SetReadFn(GWEN_CRYPTTOKEN *ct,
                                   GWEN_CRYPTTOKEN_FILE_READ_FN fn);
void GWEN_CryptTokenFile_SetWriteFn(GWEN_CRYPTTOKEN *ct,
                                    GWEN_CRYPTTOKEN_FILE_WRITE_FN fn);


GWEN_CT_FILE_CONTEXT_LIST*
  GWEN_CryptTokenFile_GetFileContextList(GWEN_CRYPTTOKEN *ct);
void GWEN_CryptTokenFile_AddFileContext(GWEN_CRYPTTOKEN *ct,
                                        GWEN_CT_FILE_CONTEXT *fc);
void GWEN_CryptTokenFile_ClearFileContextList(GWEN_CRYPTTOKEN *ct);




#endif /* GWEN_CRYPTTOKEN_FILE_H */

