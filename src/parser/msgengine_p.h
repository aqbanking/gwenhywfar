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

#ifndef GWENHYWFAR_MSGENGINE_P_H
#define GWENHYWFAR_MSGENGINE_P_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/msgengine.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/inherit.h>


#define GWEN_MSGENGINE_CHARSTOESCAPE ":+\'"
#define GWEN_MSGENGINE_DEFAULT_GROUPDELIM ":"
#define GWEN_MSGENGINE_DEFAULT_DELIMITERS ":+\'"
#define GWEN_MSGENGINE_VARNAME_WIDTH  30
#define GWEN_MSGENGINE_TYPENAME_WIDTH 10

#define GWEN_MSGENGINE_SHOW_FLAGS_OPTIONAL 0x00010000
#define GWEN_MSGENGINE_TRUSTEDDATA_MAXPOS  32


struct GWEN__MSGENGINE {
  GWEN_INHERIT_ELEMENT(GWEN_MSGENGINE)
  GWEN_XMLNODE *defs;
  int ownDefs;
  char escapeChar;
  char *charsToEscape;
  char *delimiters;
  /*char *secMode; */
  /*unsigned int protocolVersion; */
  GWEN_DB_NODE *globalValues;

  GWEN_MSGENGINE_TYPEREAD_PTR typeReadPtr;
  GWEN_MSGENGINE_TYPEWRITE_PTR typeWritePtr;
  GWEN_MSGENGINE_TYPECHECK_PTR typeCheckPtr;

  GWEN_MSGENGINE_BINTYPEREAD_PTR binTypeReadPtr;
  GWEN_MSGENGINE_BINTYPEWRITE_PTR binTypeWritePtr;

  GWEN_MSGENGINE_GETCHARVALUE_PTR getCharValuePtr;
  GWEN_MSGENGINE_GETINTVALUE_PTR getIntValuePtr;

  GWEN_MSGENGINE_GETGLOBALVALUES_PTR getGlobalValuesPtr;

  GWEN_MSGENGINE_FREEDATA_PTR freeDataPtr;

  GWEN_MSGENGINE_TRUSTEDDATA *trustInfos;
  void *inheritorData;

  GWEN_TYPE_UINT32 usage;
};


int GWEN_MsgEngine__WriteValue(GWEN_MSGENGINE *e,
                               GWEN_BUFFER *gbuf,
                               GWEN_BUFFER *data,
                               GWEN_XMLNODE *node);

int GWEN_MsgEngine__WriteElement(GWEN_MSGENGINE *e,
                                 GWEN_BUFFER *gbuf,
                                 GWEN_XMLNODE *node,
                                 GWEN_XMLNODE *rnode,
                                 GWEN_DB_NODE *gr,
                                 int loopNr,
                                 int isOptional,
                                 GWEN_XMLNODE_PATH *nodePath);

int GWEN_MsgEngine__WriteGroup(GWEN_MSGENGINE *e,
                               GWEN_BUFFER *gbuf,
                               GWEN_XMLNODE *node,  /** format node */
                               GWEN_XMLNODE *rnode, /** reference node */
                               GWEN_DB_NODE *gr,
                               int groupIsOptional,
                               GWEN_XMLNODE_PATH *nodePath);

int GWEN_MsgEngine__ShowElement(GWEN_MSGENGINE *e,
                                const char *path,
                                GWEN_XMLNODE *node,
                                GWEN_STRINGLIST *sl,
                                GWEN_TYPE_UINT32 flags);

int GWEN_MsgEngine__ShowGroup(GWEN_MSGENGINE *e,
                              const char *path,
                              GWEN_XMLNODE *node,
                              GWEN_XMLNODE *rnode,
                              GWEN_STRINGLIST *sl,
                              GWEN_TYPE_UINT32 flags);

int GWEN_MsgEngine__ListElement(GWEN_MSGENGINE *e,
                                const char *path,
                                GWEN_XMLNODE *node,
                                GWEN_STRINGLIST *sl,
                                GWEN_XMLNODE *listNode,
                                GWEN_TYPE_UINT32 flags);

int GWEN_MsgEngine__ListGroup(GWEN_MSGENGINE *e,
                              const char *path,
                              GWEN_XMLNODE *node,
                              GWEN_XMLNODE *rnode,
                              GWEN_STRINGLIST *sl,
                              GWEN_XMLNODE *listNode,
                              GWEN_TYPE_UINT32 flags);

int GWEN_MsgEngine__ReadValue(GWEN_MSGENGINE *e,
                              GWEN_BUFFER *msgbuf,
                              GWEN_XMLNODE *node,
                              GWEN_XMLNODE *rnode,
                              GWEN_BUFFER *vbuf,
                              const char *delimiters,
                              GWEN_TYPE_UINT32 flags);

int GWEN_MsgEngine__ReadGroup(GWEN_MSGENGINE *e,
                              GWEN_BUFFER *msgbuf,
                              GWEN_XMLNODE *node,
                              GWEN_XMLNODE *rnode,
                              GWEN_DB_NODE *gr,
                              const char *delimiters,
                              GWEN_TYPE_UINT32 flags);

/**
 * @param dnode node of the element to transform
 */
const char *GWEN_MsgEngine__findInValues(GWEN_MSGENGINE *e,
                                         GWEN_XMLNODE *node,
                                         GWEN_XMLNODE *dnode,
                                         const char *name,
                                         unsigned int *datasize);

const char  *GWEN_MsgEngine__TransformValue(GWEN_MSGENGINE *e,
                                            const char *pvalue,
                                            GWEN_XMLNODE *node,
                                            GWEN_XMLNODE *dnode,
                                            unsigned int *datasize);
const char *GWEN_MsgEngine__SearchForValue(GWEN_MSGENGINE *e,
                                           GWEN_XMLNODE *node,
                                           GWEN_XMLNODE_PATH *nodePath,
                                           const char *name,
                                           unsigned int *datasize);
int GWEN_MsgEngine__IsCharTyp(GWEN_MSGENGINE *e, const char *type);
int GWEN_MsgEngine__IsIntTyp(GWEN_MSGENGINE *e, const char *type);
int GWEN_MsgEngine__IsBinTyp(GWEN_MSGENGINE *e, const char *type);

int GWEN_MsgEngine_GetHighestTrustLevel(GWEN_XMLNODE *node,
                                        GWEN_XMLNODE *refnode);

GWEN_DB_NODE *GWEN_MsgEngine__GetGlobalValues(GWEN_MSGENGINE *e);

GWEN_XMLNODE *GWEN_MsgEngine__GetGroup(GWEN_MSGENGINE *e,
                                       GWEN_XMLNODE *node,
                                       const char *t,
                                       int version,
                                       const char *pvalue);
GWEN_XMLNODE *GWEN_MsgEngine_GetGroup(GWEN_MSGENGINE *e,
                                      GWEN_XMLNODE *node,
                                      const GWEN_XMLNODE_PATH *nodePath,
                                      const char *t,
                                      int version,
                                      const char *pvalue);

int GWEN_MsgEngine__GetInline(GWEN_MSGENGINE *e,
                              GWEN_XMLNODE *node,
                              GWEN_BUFFER *mbuf);



struct GWEN_MSGENGINE_TRUSTEDDATA {
  GWEN_MSGENGINE_TRUSTEDDATA *next;
  char *data;
  unsigned int size;
  char *description;
  unsigned int trustLevel;
  char *replacement;
  unsigned int positions[GWEN_MSGENGINE_TRUSTEDDATA_MAXPOS];
  unsigned int posCount;
  unsigned int posPointer;
};



#endif

