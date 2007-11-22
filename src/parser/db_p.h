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


#ifndef GWENHYWFAR_DB_P_H
#define GWENHYWFAR_DB_P_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/fastbuffer.h>



GWEN_LIST_FUNCTION_DEFS(GWEN_DB_NODE, GWEN_DB_Node)


typedef union GWEN_DB_NODE_VALUE_UNION GWEN_DB_NODE_VALUE_UNION;
union GWEN_DB_NODE_VALUE_UNION {
  char *dataName;
  char *dataChar;
  void *dataBin;
  int dataInt;
  void *dataPtr;
};


struct GWEN_DB_NODE {
  GWEN_LIST_ELEMENT(GWEN_DB_NODE)
  GWEN_DB_NODE *parent;
  GWEN_DB_NODE_LIST *children;
  GWEN_DB_NODE_TYPE typ;
  uint32_t nodeFlags;
  GWEN_DB_NODE_VALUE_UNION data;
  uint32_t dataSize;
};



static GWEN_DB_NODE *GWEN_DB_Node_new(GWEN_DB_NODE_TYPE t);
static void GWEN_DB_Node_free(GWEN_DB_NODE *n);



static GWEN_DB_NODE *GWEN_DB_ValueBin_new(const void *data,
					  unsigned int datasize);


static GWEN_DB_NODE *GWEN_DB_ValueInt_new(int data);

static GWEN_DB_NODE *GWEN_DB_ValueChar_new(const char *data);

static GWEN_DB_NODE *GWEN_DB_ValuePtr_new(void *data);


static GWEN_DB_NODE *GWEN_DB_Var_new(const char *name);


static GWEN_DB_NODE *GWEN_DB_Node_dup(const GWEN_DB_NODE *n);


static void GWEN_DB_Node_free(GWEN_DB_NODE *n);


static void GWEN_DB_Node_Append(GWEN_DB_NODE *parent,
				GWEN_DB_NODE *n);

static void GWEN_DB_Node_Insert(GWEN_DB_NODE *parent,
				GWEN_DB_NODE *n);

static void GWEN_DB_Node_Unlink(GWEN_DB_NODE *n);

/**
 * Used with GWEN_Path
 */

static void *GWEN_DB_HandlePath(const char *entry,
				void *data,
				int idx,
				uint32_t flags);


static GWEN_DB_NODE *GWEN_DB_FindGroup(GWEN_DB_NODE *n,
				       const char *name,
				       int idx);

static GWEN_DB_NODE *GWEN_DB_FindVar(GWEN_DB_NODE *n,
				     const char *name,
				     int idx);


static GWEN_DB_NODE *GWEN_DB_GetNode(GWEN_DB_NODE *n,
				     const char *path,
				     uint32_t flags);

static GWEN_DB_NODE *GWEN_DB_GetValue(GWEN_DB_NODE *n,
				      const char *path,
				      int idx);

static void GWEN_DB_ClearNode(GWEN_DB_NODE *n);

static void *GWEN_DB_count_cb(GWEN_DB_NODE *node, void *user_data);

static void GWEN_DB_Node_Append_UnDirty(GWEN_DB_NODE *parent,
					GWEN_DB_NODE *n);
static void GWEN_DB_Node_InsertUnDirty(GWEN_DB_NODE *parent,
				       GWEN_DB_NODE *n);
static void GWEN_DB_Node_Unlink_UnDirty(GWEN_DB_NODE *n);




static int GWEN_DB__ReadValues(GWEN_DB_NODE *n,
			       uint32_t dbflags,
			       const char *typeName,
			       const char *varName,
			       uint8_t *p);


static int GWEN_DB_WriteGroupToIoLayer(GWEN_DB_NODE *node,
				       GWEN_FAST_BUFFER *fb,
				       uint32_t dbflags,
				       int insert);


#endif



