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

typedef struct GWEN_DB_HEADER GWEN_DB_HEADER;
typedef struct GWEN_DB_GROUP GWEN_DB_GROUP;
typedef struct GWEN_DB_VAR GWEN_DB_VAR;

typedef struct GWEN_DB_VALUE_HEADER GWEN_DB_VALUE_HEADER;
typedef struct GWEN_DB_VALUE_CHAR GWEN_DB_VALUE_CHAR;
typedef struct GWEN_DB_VALUE_INT GWEN_DB_VALUE_INT;

typedef struct GWEN_DB_VALUE_BIN GWEN_DB_VALUE_BIN;
typedef union GWEN_DB_VALUE GWEN_DB_VALUE;

typedef enum {
  GWEN_DB_NODETYPE_UNKNOWN=0,
  GWEN_DB_NODETYPE_GROUP,
  GWEN_DB_NODETYPE_VAR,
  GWEN_DB_NODETYPE_VALUE
} GWEN_DB_NODETYPE;



struct GWEN_DB_HEADER {
  GWEN_DB_NODE *next;
  GWEN_DB_NODE *parent;
  GWEN_DB_NODE *child;
  GWEN_DB_NODETYPE typ;
  GWEN_TYPE_UINT32 nodeFlags;
};



struct GWEN_DB_GROUP {
  GWEN_DB_HEADER h;
  char *name;
};


struct GWEN_DB_VAR {
  GWEN_DB_HEADER h;
  char *name;
};



struct GWEN_DB_VALUE_HEADER {
  GWEN_DB_HEADER h;
  GWEN_DB_VALUETYPE typ;
};


struct GWEN_DB_VALUE_CHAR {
  GWEN_DB_VALUE_HEADER h;
  char *data;
};


struct GWEN_DB_VALUE_INT {
  GWEN_DB_VALUE_HEADER h;
  int data;
};


struct GWEN_DB_VALUE_BIN {
  GWEN_DB_VALUE_HEADER h;
  void *data;
  unsigned int dataSize;
};


union GWEN_DB_VALUE {
  GWEN_DB_VALUE_HEADER h;
  GWEN_DB_VALUE_CHAR c;
  GWEN_DB_VALUE_INT i;
  GWEN_DB_VALUE_BIN b;
};


union GWEN_DB_NODE {
  GWEN_DB_HEADER h;
  GWEN_DB_GROUP group;
  GWEN_DB_VAR var;
  GWEN_DB_VALUE val;
};



GWEN_DB_NODE *GWEN_DB_ValueBin_new(const void *data,
                                   unsigned int datasize);


GWEN_DB_NODE *GWEN_DB_ValueInt_new(int data);

GWEN_DB_NODE *GWEN_DB_ValueChar_new(const char *data);

GWEN_DB_NODE *GWEN_DB_Var_new(const char *name);


GWEN_DB_NODE *GWEN_DB_Node_dup(const GWEN_DB_NODE *n);


void GWEN_DB_Node_free(GWEN_DB_NODE *n);


void GWEN_DB_Node_Append(GWEN_DB_NODE *parent,
                         GWEN_DB_NODE *n);

void GWEN_DB_Node_Insert(GWEN_DB_NODE *parent,
                         GWEN_DB_NODE *n);

void GWEN_DB_Node_Unlink(GWEN_DB_NODE *n);

/**
 * Used with GWEN_Path
 */

void *GWEN_DB_HandlePath(const char *entry,
                         void *data,
                         GWEN_TYPE_UINT32 flags);


GWEN_DB_NODE *GWEN_DB_FindGroup(GWEN_DB_NODE *n,
                                const char *name);

GWEN_DB_NODE *GWEN_DB_FindVar(GWEN_DB_NODE *n,
                              const char *name);


GWEN_DB_NODE *GWEN_DB_GetNode(GWEN_DB_NODE *n,
                              const char *path,
                              GWEN_TYPE_UINT32 flags);

GWEN_DB_NODE *GWEN_DB_GetValue(GWEN_DB_NODE *n,
                               const char *path,
                               int idx);

void GWEN_DB_ClearNode(GWEN_DB_NODE *n);



int GWEN_DB_WriteGroupToStream(GWEN_DB_NODE *node,
                               GWEN_BUFFEREDIO *bio,
                               GWEN_TYPE_UINT32 dbflags,
                               int insert);


const char *GWEN_DB_GetCharValueFromNode(GWEN_DB_NODE *n);

int GWEN_DB_GetIntValueFromNode(GWEN_DB_NODE *n);

const void *GWEN_DB_GetBinValueFromNode(GWEN_DB_NODE *n,
                                        unsigned int *size);


static void *GWEN_DB_count_cb(GWEN_DB_NODE *node, void *user_data);

void GWEN_DB_Node_Append_UnDirty(GWEN_DB_NODE *parent,
                                 GWEN_DB_NODE *n);
void GWEN_DB_Node_InsertUnDirty(GWEN_DB_NODE *parent,
                                GWEN_DB_NODE *n);
void GWEN_DB_Node_Unlink_UnDirty(GWEN_DB_NODE *n);



#endif



