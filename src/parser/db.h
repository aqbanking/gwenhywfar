/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Tue Sep 09 2003
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


#ifndef GWENHYFWAR_DB_H
#define GWENHYFWAR_DB_H

#include <gwenhyfwar/gwenhyfwarapi.h>
#include <gwenhyfwar/path.h>
#include <gwenhyfwar/bufferedio.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GWEN_DB_LINE_MAXSIZE  1024

#define GWEN_DB_FLAGS_OVERWRITE_VARS         0x00010000
#define GWEN_DB_FLAGS_OVERWRITE_GROUPS       0x00020000

#define GWEN_DB_FLAGS_QUOTE_VARNAMES         0x00040000
#define GWEN_DB_FLAGS_QUOTE_VALUES           0x00080000
#define GWEN_DB_FLAGS_WRITE_SUBGROUPS        0x00100000

#define GWEN_DB_FLAGS_DEFAULT \
  GWEN_DB_FLAGS_QUOTE_VALUES | \
  GWEN_DB_FLAGS_WRITE_SUBGROUPS



typedef union GWEN_DB_NODE GWEN_DB_NODE;

typedef enum {
  GWEN_DB_VALUETYPE_UNKNOWN=0,
  GWEN_DB_VALUETYPE_CHAR,
  GWEN_DB_VALUETYPE_INT,
  GWEN_DB_VALUETYPE_BIN
} GWEN_DB_VALUETYPE;


GWEN_DB_NODE *GWEN_DB_Group_new(const char *name);
void GWEN_DB_Group_free(GWEN_DB_NODE *n);
GWEN_DB_NODE *GWEN_DB_Group_dup(GWEN_DB_NODE *n);


GWEN_DB_NODE *GWEN_DB_GetFirstGroup(GWEN_DB_NODE *n);
GWEN_DB_NODE *GWEN_DB_GetNextGroup(GWEN_DB_NODE *n);

GWEN_DB_NODE *GWEN_DB_GetFirstVar(GWEN_DB_NODE *n);
GWEN_DB_NODE *GWEN_DB_GetNextVar(GWEN_DB_NODE *n);

GWEN_DB_NODE *GWEN_DB_GetFirstValue(GWEN_DB_NODE *n);
GWEN_DB_NODE *GWEN_DB_GetNextValue(GWEN_DB_NODE *n);
GWEN_DB_VALUETYPE GWEN_DB_GetValueType(GWEN_DB_NODE *n);

const char *GWEN_DB_GetCharValueFromNode(GWEN_DB_NODE *n);
int GWEN_DB_GetIntValueFromNode(GWEN_DB_NODE *n);
const void *GWEN_DB_GetBinValueFromNode(GWEN_DB_NODE *n,
                                        unsigned int *size);


/**
 * @return 0 on success, !=0 on error
 */
int GWEN_DB_DeleteVar(GWEN_DB_NODE *n,
                      const char *path);
/**
 * @return 0 on success, !=0 on error
 */
int GWEN_DB_DeleteGroup(GWEN_DB_NODE *n,
                        const char *path);
/**
 * @return 0 on success, !=0 on error
 */
int GWEN_DB_ClearGroup(GWEN_DB_NODE *n,
                       const char *path);


const char *GWEN_DB_GetCharValue(GWEN_DB_NODE *n,
                                 const char *path,
                                 int idx,
                                 const char *defVal);
/**
 * @return 0 on success, !=0 on error
 */
int GWEN_DB_SetCharValue(GWEN_DB_NODE *n,
                         unsigned int flags,
                         const char *path,
                         const char *val);


int GWEN_DB_GetIntValue(GWEN_DB_NODE *n,
                        const char *path,
                        int idx,
                        int defVal);

/**
 * @return 0 on success, !=0 on error
 */
int GWEN_DB_SetIntValue(GWEN_DB_NODE *n,
                        unsigned int flags,
                        const char *path,
                        int val);


const void *GWEN_DB_GetBinValue(GWEN_DB_NODE *n,
                                const char *path,
                                int idx,
                                const void *defVal,
                                unsigned int defValSize,
                                unsigned int *returnValueSize);

/**
 * @return 0 on success, !=0 on error
 */
int GWEN_DB_SetBinValue(GWEN_DB_NODE *n,
			unsigned int flags,
			const char *path,
                        const void *val,
			unsigned int valSize);

GWEN_DB_NODE *GWEN_DB_GetGroup(GWEN_DB_NODE *n,
                               unsigned int flags,
                               const char *path);


const char *GWEN_DB_GroupName(GWEN_DB_NODE *n);


void GWEN_DB_Dump(GWEN_DB_NODE *n, FILE *f, int insert);

int GWEN_DB_AddGroup(GWEN_DB_NODE *n, GWEN_DB_NODE *nn);
int GWEN_DB_AddGroupChildren(GWEN_DB_NODE *n, GWEN_DB_NODE *nn);
void GWEN_DB_UnlinkGroup(GWEN_DB_NODE *n);

int GWEN_DB_ReadFromStream(GWEN_DB_NODE *n,
                           GWEN_BUFFEREDIO *bio,
                           unsigned int dbflags);
int GWEN_DB_ReadFile(GWEN_DB_NODE *n,
                     const char *fname,
                     unsigned int dbflags);

int GWEN_DB_WriteToStream(GWEN_DB_NODE *node,
                          GWEN_BUFFEREDIO *bio,
                          unsigned int dbflags);

int GWEN_DB_WriteFile(GWEN_DB_NODE *n,
                      const char *fname,
                      unsigned int dbflags);


#ifdef __cplusplus
}
#endif


#endif



