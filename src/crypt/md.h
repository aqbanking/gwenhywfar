/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Nov 13 2003
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


#ifndef GWENHYWFAR_MD_H
#define GWENHYWFAR_MD_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/error.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef struct GWEN_MD GWEN_MD;

typedef int (*GWEN_MD_BEGIN_FN)(GWEN_MD *md);
typedef int (*GWEN_MD_END_FN)(GWEN_MD *md);
typedef int (*GWEN_MD_UPDATE_FN)(GWEN_MD *md,
                                 const char *buf,
                                 unsigned int l);
typedef void (*GWEN_MD_FREEDATA_FN)(GWEN_MD *md);


GWENHYWFAR_API
GWEN_MD *GWEN_MD_Factory(const char *t);
GWENHYWFAR_API
void GWEN_MD_free(GWEN_MD *md);
GWENHYWFAR_API
unsigned char *GWEN_MD_GetDigestPtr(GWEN_MD *md);
GWENHYWFAR_API
unsigned int GWEN_MD_GetDigestSize(GWEN_MD *md);

GWENHYWFAR_API
int GWEN_MD_Begin(GWEN_MD *md);
GWENHYWFAR_API
int GWEN_MD_End(GWEN_MD *md);
GWENHYWFAR_API
int GWEN_MD_Update(GWEN_MD *md,
                   const char *buf,
                   unsigned int l);

/**
 * This is a conveniance function. It hashes a given data buffer using
 * the given hash algo.
 * @return 0 if ok, !=0 on error
 * @param algo algorithm name (like "RMD160", "MD5")
 * @param data pointer to the data to hash
 * @param dsize size of the data to hash
 * @param buffer pointer to a buffer for the hash
 * @param bsize pointer to a variable that initially holds the size of the
 * buffer. Upon return it will hold the size of the hash data.
 */
int GWEN_MD_Hash(const char *algo,
                 const char *data,
                 unsigned int dsize,
                 char *buffer,
                 unsigned int *bsize);


/** @name Functions for Inheriting Modules
 *
 * These function should not be called by applications. They are meant to
 * be called by modules implementing message digest code.
 */
/*@{*/
GWENHYWFAR_API
GWEN_MD *GWEN_MD_new(unsigned int size);
GWENHYWFAR_API
void GWEN_MD_SetBeginFn(GWEN_MD *md, GWEN_MD_BEGIN_FN fn);
GWENHYWFAR_API
void GWEN_MD_SetEndFn(GWEN_MD *md, GWEN_MD_END_FN fn);
GWENHYWFAR_API
void GWEN_MD_SetUpdateFn(GWEN_MD *md, GWEN_MD_UPDATE_FN fn);
GWENHYWFAR_API
void GWEN_MD_SetFreeDataFn(GWEN_MD *md, GWEN_MD_FREEDATA_FN fn);
GWENHYWFAR_API
void GWEN_MD_SetData(GWEN_MD *md, void *data);
GWENHYWFAR_API
void *GWEN_MD_GetData(GWEN_MD *md);
/*@}*/


typedef struct GWEN_MD_PROVIDER GWEN_MD_PROVIDER;

typedef GWEN_MD*
  (*GWEN_MDPROVIDER_NEWMD_FN)(GWEN_MD_PROVIDER *pr);

GWENHYWFAR_API
GWEN_MD_PROVIDER *GWEN_MdProvider_new();
GWENHYWFAR_API
void GWEN_MdProvider_free(GWEN_MD_PROVIDER *pr);

GWENHYWFAR_API
void GWEN_MdProvider_SetNewMdFn(GWEN_MD_PROVIDER *pr,
                                GWEN_MDPROVIDER_NEWMD_FN newMdFn);
GWENHYWFAR_API
void GWEN_MdProvider_SetName(GWEN_MD_PROVIDER *pr,
                             const char *name);

/**
 * On success this function takes over ownership of the given object.
 */
GWENHYWFAR_API
GWEN_ERRORCODE GWEN_MD_RegisterProvider(GWEN_MD_PROVIDER *pr);
GWENHYWFAR_API
GWEN_ERRORCODE GWEN_MD_UnregisterProvider(GWEN_MD_PROVIDER *pr);
GWENHYWFAR_API
void GWEN_MD_UnregisterAllProviders();

#ifdef __cplusplus
}
#endif



#endif





