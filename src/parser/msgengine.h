/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Fri Jul 04 2003
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

#ifndef GWENHYFWAR_MSGENGINE_H
#define GWENHYFWAR_MSGENGINE_H

#include <chameleon/chameleonapi.h>
#include <chameleon/xml.h>
#include <chameleon/conf.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _MSGENGINE MSGENGINE;

#define MSGENGINE_SHOW_FLAGS_NOSET 0x0001
#define MSGENGINE_MAX_VALUE_LEN    8192


typedef int (*MSGENGINE_TYPECHECK_PTR)(MSGENGINE *e,
				       const char *value,
				       XMLNODE *node,
				       char escapeChar);

typedef int (*MSGENGINE_TYPEREAD_PTR)(MSGENGINE *e,
				      const char *msg,
				      unsigned int msgSize,
				      unsigned int *pos,
				      XMLNODE *node,
				      char *buffer,
				      int bufsize,
				      char escapeChar,
				      const char *delimiters);

/**
 * @return 0 on success, -1 on error, 1 if unknown type
 */
typedef int (*MSGENGINE_TYPEWRITE_PTR)(MSGENGINE *e,
				       char *buffer,
				       unsigned int size,
				       unsigned int *pos,
				       const char *value,
				       XMLNODE *node);


typedef int (*MSGENGINE_BINTYPEREAD_PTR)(MSGENGINE *e,
					 XMLNODE *node,
					 CONFIGGROUP *gr,
					 const char *data);
typedef int (*MSGENGINE_BINTYPEWRITE_PTR)(MSGENGINE *e,
					  XMLNODE *node,
					  CONFIGGROUP *gr,
					  char *buffer,
					  unsigned int bufsize);



typedef enum {
  MsgEngineFormatText=0
} MSGENGINE_FORMAT;


MSGENGINE *MsgEngine_new();
void MsgEngine_free(MSGENGINE *e);

void MsgEngine_SetFormat(MSGENGINE *e, MSGENGINE_FORMAT f);
MSGENGINE_FORMAT MsgEngine_GetFormat(MSGENGINE *e);

void MsgEngine_SetEscapeChar(MSGENGINE *e, char c);
char MsgEngine_GetEscapeChar(MSGENGINE *e);

void MsgEngine_SetCharsToEscape(MSGENGINE *e, const char *c);
const char *MsgEngine_GetCharsToEscape(MSGENGINE *e);

void MsgEngine_SetMode(MSGENGINE *e, const char *mode);
const char *MsgEngine_GetMode(MSGENGINE *e);

unsigned int MsgEngine_GetConfigMode(MSGENGINE *e);
void MsgEngine_SetConfigMode(MSGENGINE *e, unsigned int m);

XMLNODE *MsgEngine_GetDefinitions(MSGENGINE *e);
void MsgEngine_SetDefinitions(MSGENGINE *e, XMLNODE *n);

int MsgEngine_AddDefinitions(MSGENGINE *e,
			     XMLNODE *node);

void MsgEngine_SetTypeCheckFunction(MSGENGINE *e, MSGENGINE_TYPECHECK_PTR p);
MSGENGINE_TYPECHECK_PTR MsgEngine_GetTypeCheckFunction(MSGENGINE *e);

void MsgEngine_SetTypeReadFunction(MSGENGINE *e, MSGENGINE_TYPEREAD_PTR p);
MSGENGINE_TYPEREAD_PTR MsgEngine_GetTypeReadFunction(MSGENGINE *e);

void MsgEngine_SetTypeWriteFunction(MSGENGINE *e, MSGENGINE_TYPEWRITE_PTR p);
MSGENGINE_TYPEWRITE_PTR MsgEngine_GetTypeWriteFunction(MSGENGINE *e);


/** @name Handler for binary data
 *
 */
/*@{*/
void MsgEngine_SetBinTypeReadFunction(MSGENGINE *e,
				      MSGENGINE_BINTYPEREAD_PTR p);
MSGENGINE_BINTYPEREAD_PTR MsgEngine_GetBinTypeReadFunction(MSGENGINE *e);

void MsgEngine_SetBinTypeWriteFunction(MSGENGINE *e,
				       MSGENGINE_BINTYPEWRITE_PTR p);
MSGENGINE_BINTYPEWRITE_PTR MsgEngine_GetBinTypeWriteFunction(MSGENGINE *e);
/*@}*/


void *MsgEngine_GetInheritorData(MSGENGINE *e);
void MsgEngine_SetInheritorData(MSGENGINE *e, void *d);

XMLNODE *MsgEngine_FindGroupByProperty(MSGENGINE *e,
				       const char *pname,
				       int version,
				       const char *pvalue);

/**
 * Looks for a node of the given type.
 * Example: If type is "GROUP" then the node will be searched in
 * "<GROUPS>", and the tag name will be "<GROUPdef>".
 */
XMLNODE *MsgEngine_FindNodeByProperty(MSGENGINE *e,
				      const char *t,
				      const char *pname,
				      int version,
				      const char *pvalue);

/**
 * Set a global variable which will be used for "$"-Variables in description
 * files.
 */
int MsgEngine_SetValue(MSGENGINE *e,
		       const char *path,
		       const char *value);
int MsgEngine_SetIntValue(MSGENGINE *e,
			  const char *path,
			  int value);
const char *MsgEngine_GetValue(MSGENGINE *e,
			       const char *path,
			       const char *defValue);
int MsgEngine_GetIntValue(MSGENGINE *e,
			  const char *path,
			  int defValue);


int MsgEngine_CreateMessage(MSGENGINE *e,
			    const char *msgName,
                            int msgVersion,
			    char *buffer,
			    unsigned int size,
			    unsigned int *pos,
			    CONFIGGROUP *msgData);

int MsgEngine_CreateMessageFromNode(MSGENGINE *e,
				    XMLNODE *node,
				    char *buffer,
				    unsigned int size,
				    unsigned int *pos,
				    CONFIGGROUP *msgData);

int MsgEngine_ShowMessage(MSGENGINE *e,
                          const char *typ,
			  const char *msgName,
			  int msgVersion,
			  unsigned int flags);

int MsgEngine_ParseMessage(MSGENGINE *e,
			   XMLNODE *group,
			   const char *msg,
			   unsigned int msgSize,
			   unsigned int *pos,
			   CONFIGGROUP *gr);

#ifdef __cplusplus
}
#endif

#endif

