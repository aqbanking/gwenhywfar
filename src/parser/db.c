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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG

#include "db_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/path.h>
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/fslock.h>
#include <gwenhywfar/io_file.h>
#include <gwenhywfar/io_memory.h>
#include <gwenhywfar/iomanager.h>
#include <gwenhywfar/fastbuffer.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif


#define GWEN_DB_NODE_FLAGS_MASK_INTERNAL  0xf0000000
#define GWEN_DB_NODE_FLAGS_GROUP          0x80000000


GWEN_LIST_FUNCTIONS(GWEN_DB_NODE, GWEN_DB_Node)



GWEN_DB_NODE *GWEN_DB_Node_new(GWEN_DB_NODE_TYPE t){
  GWEN_DB_NODE *node;

  GWEN_NEW_OBJECT(GWEN_DB_NODE, node);
  GWEN_LIST_INIT(GWEN_DB_NODE, node);
  node->typ=t;
  return (GWEN_DB_NODE*)node;
}




GWEN_DB_NODE *GWEN_DB_ValueBin_new(const void *data,
                                   unsigned int datasize){
  GWEN_DB_NODE *n;

  n=GWEN_DB_Node_new(GWEN_DB_NodeType_ValueBin);
  if (datasize) {
    assert(data);
    n->dataSize=datasize;
    n->data.dataBin=(char*)GWEN_Memory_malloc(datasize);
    assert(n->data.dataBin);
    memmove(n->data.dataBin, data, datasize);
  }
  return n;
}



GWEN_DB_NODE *GWEN_DB_ValueInt_new(int data) {
  GWEN_DB_NODE *n;

  n=GWEN_DB_Node_new(GWEN_DB_NodeType_ValueInt);
  n->data.dataInt=data;
  return n;
}



GWEN_DB_NODE *GWEN_DB_ValueChar_new(const char *data) {
  GWEN_DB_NODE *n;

  n=GWEN_DB_Node_new(GWEN_DB_NodeType_ValueChar);
  if (data)
    n->data.dataChar=GWEN_Memory_strdup(data);
  else
    n->data.dataChar=GWEN_Memory_strdup("");
  return n;
}



GWEN_DB_NODE *GWEN_DB_ValuePtr_new(void *data) {
  GWEN_DB_NODE *n;

  n=GWEN_DB_Node_new(GWEN_DB_NodeType_ValuePtr);
  n->data.dataPtr=data;
  return n;
}



GWEN_DB_NODE *GWEN_DB_Group_new(const char *name){
  GWEN_DB_NODE *n;

  assert(name);
  n=GWEN_DB_Node_new(GWEN_DB_NodeType_Group);
  if (name)
    n->data.dataName=GWEN_Memory_strdup(name);
  else
    n->data.dataName=GWEN_Memory_strdup("");
  n->children=GWEN_DB_Node_List_new();
  return n;
}



GWEN_DB_NODE *GWEN_DB_Var_new(const char *name){
  GWEN_DB_NODE *n;

  assert(name);
  n=GWEN_DB_Node_new(GWEN_DB_NodeType_Var);
  if (name)
    n->data.dataName=GWEN_Memory_strdup(name);
  else
    n->data.dataName=GWEN_Memory_strdup("");
  n->children=GWEN_DB_Node_List_new();
  return n;
}



void GWEN_DB_Node_Append_UnDirty(GWEN_DB_NODE *parent,
				 GWEN_DB_NODE *n){
  assert(parent);
  assert(n);
  assert(parent!=n);

  assert(parent->children!=NULL);
  GWEN_DB_Node_List_Add(n, parent->children);

  n->parent=parent;
}



void GWEN_DB_Node_Append(GWEN_DB_NODE *parent,
                         GWEN_DB_NODE *n){
  GWEN_DB_Node_Append_UnDirty(parent, n);
  GWEN_DB_ModifyBranchFlagsUp(parent,
			      GWEN_DB_NODE_FLAGS_DIRTY,
			      GWEN_DB_NODE_FLAGS_DIRTY);
}



void GWEN_DB_Node_InsertUnDirty(GWEN_DB_NODE *parent,
				GWEN_DB_NODE *n){
  assert(parent);
  assert(n);
  assert(parent!=n);

  assert(parent->children!=NULL);
  GWEN_DB_Node_List_Insert(n, parent->children);

  n->parent=parent;
}



void GWEN_DB_Node_Insert(GWEN_DB_NODE *parent,
			 GWEN_DB_NODE *n){
  GWEN_DB_Node_InsertUnDirty(parent, n);
  GWEN_DB_ModifyBranchFlagsUp(parent,
			      GWEN_DB_NODE_FLAGS_DIRTY,
			      GWEN_DB_NODE_FLAGS_DIRTY);
}



void GWEN_DB_Node_Unlink_UnDirty(GWEN_DB_NODE *n) {
  GWEN_DB_NODE *parent;

  assert(n);
  parent=n->parent;
  if (!parent) {
    DBG_WARN(GWEN_LOGDOMAIN, "Node is not linked, nothing to do");
    return;
  }

  GWEN_DB_Node_List_Del(n);
  n->parent=NULL;
}



void GWEN_DB_Node_Unlink(GWEN_DB_NODE *n) {
  GWEN_DB_NODE *parent;

  assert(n);
  parent=n->parent;
  assert(parent);

  GWEN_DB_Node_Unlink_UnDirty(n);
  GWEN_DB_ModifyBranchFlagsUp(parent,
			      GWEN_DB_NODE_FLAGS_DIRTY,
                              GWEN_DB_NODE_FLAGS_DIRTY);
}



void GWEN_DB_Node_free(GWEN_DB_NODE *n){
  if (n) {
    GWEN_LIST_FINI(GWEN_DB_NODE, n);

    /* free children */
    if (n->children) {
      GWEN_DB_NODE *cn;

      cn=GWEN_DB_Node_List_First(n->children);
      while(cn) {
	GWEN_DB_NODE *ncn;

	ncn=GWEN_DB_Node_List_Next(cn);
	DBG_VERBOUS(GWEN_LOGDOMAIN, "Freeing child node");
	GWEN_DB_Node_free(cn);
	cn=ncn;
      }
    }

    /* free dynamic (allocated) data */
    switch(n->typ) {
    case GWEN_DB_NodeType_Group:
    case GWEN_DB_NodeType_Var:
      GWEN_Memory_dealloc(n->data.dataName);
      break;

    case GWEN_DB_NodeType_ValueChar:
      GWEN_Memory_dealloc(n->data.dataChar);
      break;
    case GWEN_DB_NodeType_ValueBin:
      GWEN_Memory_dealloc(n->data.dataBin);
      break;
    case GWEN_DB_NodeType_ValuePtr:
    case GWEN_DB_NodeType_ValueInt:
      break;
    default:
      DBG_WARN(GWEN_LOGDOMAIN, "Unknown node type (%d)", n->typ);
    }
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Freeing node itself");
    GWEN_FREE_OBJECT(n);
  }
}



GWEN_DB_NODE *GWEN_DB_Node_dup(const GWEN_DB_NODE *n){
  GWEN_DB_NODE *nn;

  switch(n->typ) {
  case GWEN_DB_NodeType_Group:
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Duplicating group \"%s\"",
		n->data.dataName);
    nn=GWEN_DB_Group_new(n->data.dataName);
    break;
  case GWEN_DB_NodeType_Var:
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Duplicating variable \"%s\"",
		n->data.dataName);
    nn=GWEN_DB_Var_new(n->data.dataName);
    break;
  case GWEN_DB_NodeType_ValueChar:
    nn=GWEN_DB_ValueChar_new(n->data.dataChar);
    break;
  case GWEN_DB_NodeType_ValueBin:
    nn=GWEN_DB_ValueBin_new(n->data.dataBin, n->dataSize);
    break;
  case GWEN_DB_NodeType_ValuePtr:
    nn=GWEN_DB_ValuePtr_new(n->data.dataPtr);
    break;
  case GWEN_DB_NodeType_ValueInt:
    nn=GWEN_DB_ValueInt_new(n->data.dataInt);
    break;
  default:
    DBG_WARN(GWEN_LOGDOMAIN, "Unknown node type (%d)", n->typ);
    nn=0;
  }

  /* duplicate all children and add them to the new node */
  if (nn) {
    const GWEN_DB_NODE *cn;

    cn=GWEN_DB_Node_List_First(n->children);
    while(cn) {
      GWEN_DB_NODE *ncn;

      /* duplicate child and add it */
      ncn=GWEN_DB_Node_dup(cn);
      if (!ncn) {
        GWEN_DB_Node_free(nn);
	return NULL;
      }
      GWEN_DB_Node_Append_UnDirty(nn, ncn);
      cn=GWEN_DB_Node_List_Next(cn);
    } /* while cn */
  }

  return nn;
}



void GWEN_DB_Group_free(GWEN_DB_NODE *n){
  GWEN_DB_Node_free(n);
}



GWEN_DB_NODE *GWEN_DB_Group_dup(const GWEN_DB_NODE *n){
  assert(n);
  if (n->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a group");
    return NULL;
  }
  return GWEN_DB_Node_dup(n);
}



GWEN_DB_NODE *GWEN_DB_GetFirstGroup(GWEN_DB_NODE *n){
  GWEN_DB_NODE *nn;

  assert(n);
  if (n->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a group");
    return NULL;
  }
  assert(n->children);
  nn=GWEN_DB_Node_List_First(n->children);
  while(nn) {
    if (nn->typ==GWEN_DB_NodeType_Group)
      break;
    nn=GWEN_DB_Node_List_Next(nn);
  } /* while node */
  return nn;
}



GWEN_DB_NODE *GWEN_DB_GetNextGroup(GWEN_DB_NODE *n){
  GWEN_DB_NODE *og;

  og=n;
  assert(n);
  if (n->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a group");
    return NULL;
  }
  n=GWEN_DB_Node_List_Next(n);
  while(n) {
    if (n->typ==GWEN_DB_NodeType_Group)
      break;
    n=GWEN_DB_Node_List_Next(n);
  } /* while node */
  return n;
}



GWEN_DB_NODE *GWEN_DB_GetFirstVar(GWEN_DB_NODE *n){
  GWEN_DB_NODE *nn;

  assert(n);
  if (n->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a group");
    return NULL;
  }
  assert(n->children);
  nn=GWEN_DB_Node_List_First(n->children);
  while(nn) {
    if (nn->typ==GWEN_DB_NodeType_Var)
      break;
    nn=GWEN_DB_Node_List_Next(nn);
  } /* while node */
  return nn;
}



GWEN_DB_NODE *GWEN_DB_GetNextVar(GWEN_DB_NODE *n){
  GWEN_DB_NODE *og;

  og=n;
  assert(n);
  if (n->typ!=GWEN_DB_NodeType_Var) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a variable");
    return NULL;
  }
  n=GWEN_DB_Node_List_Next(n);
  while(n) {
    if (n->typ==GWEN_DB_NodeType_Var)
      break;
    n=GWEN_DB_Node_List_Next(n);
  } /* while node */
  return n;
}



GWEN_DB_NODE *GWEN_DB_GetFirstValue(GWEN_DB_NODE *n){
  GWEN_DB_NODE *nn;

  assert(n);
  if (n->typ!=GWEN_DB_NodeType_Var) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a variable");
    return NULL;
  }
  assert(n->children);
  nn=GWEN_DB_Node_List_First(n->children);
  while(nn) {
    if (nn->typ>=GWEN_DB_NodeType_ValueChar &&
	nn->typ<GWEN_DB_NodeType_ValueLast) {
      break;
    }
    nn=GWEN_DB_Node_List_Next(nn);
  } /* while node */
  return nn;
}



GWEN_DB_NODE *GWEN_DB_GetNextValue(GWEN_DB_NODE *n){
  assert(n);
  if (n->typ<GWEN_DB_NodeType_ValueChar ||
      n->typ>=GWEN_DB_NodeType_ValueLast) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a value");
    return NULL;
  }

  n=GWEN_DB_Node_List_Next(n);
  while(n) {
    if (n->typ>=GWEN_DB_NodeType_ValueChar &&
	n->typ<GWEN_DB_NodeType_ValueLast) {
      break;
    }
    n=GWEN_DB_Node_List_Next(n);
  } /* while node */
  return n;
}



GWEN_DB_NODE_TYPE GWEN_DB_GetValueType(GWEN_DB_NODE *n){
  assert(n);
  if (n->typ>=GWEN_DB_NodeType_ValueChar &&
      n->typ<GWEN_DB_NodeType_ValueLast) {
    return n->typ;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a value");
    return GWEN_DB_NodeType_Unknown;
  }
}



const char *GWEN_DB_GetCharValueFromNode(const GWEN_DB_NODE *n){
  assert(n);
  if (n->typ!=GWEN_DB_NodeType_ValueChar) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a char value");
    return NULL;
  }
  return n->data.dataChar;
}



int GWEN_DB_SetCharValueInNode(GWEN_DB_NODE *n, const char *s) {
  assert(n);
  assert(s);

  if (n->typ!=GWEN_DB_NodeType_ValueChar) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a char value");
    return GWEN_ERROR_INVALID;
  }

  GWEN_Memory_dealloc(n->data.dataChar);
  n->data.dataChar=GWEN_Memory_strdup(s);
  return 0;
}



int GWEN_DB_GetIntValueFromNode(const GWEN_DB_NODE *n){
  const char *p;
  int res;

  assert(n);

  switch(n->typ) {
  case GWEN_DB_NodeType_ValueInt:
    return n->data.dataInt;
  case GWEN_DB_NodeType_ValueChar:
    p=n->data.dataChar;
    assert(p);
    if (sscanf(p, "%d", &res)!=1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "String in node is not an int value");
      return 0;
    }
    return res;

  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is neither char nor int value");
    return 0;
  }
}



const void *GWEN_DB_GetBinValueFromNode(const GWEN_DB_NODE *n,
                                        unsigned int *size){
  assert(n);

  if (n->typ!=GWEN_DB_NodeType_ValueBin) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a binary value");
    return NULL;
  }

  *size=n->dataSize;
  return n->data.dataBin;
}



GWEN_DB_NODE *GWEN_DB_FindGroup(GWEN_DB_NODE *n,
                                const char *name,
                                int idx) {
  GWEN_DB_NODE *nn;

  assert(n);
  assert(name);

  if (n->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a group");
    return NULL;
  }

  /* find existing node */
  assert(n->children);
  nn=GWEN_DB_Node_List_First(n->children);
  while(nn) {
    if (nn->typ==GWEN_DB_NodeType_Group) {
      if (strcasecmp(nn->data.dataName, name)==0) {
	if (!idx)
	  /* ok, group found, return it */
	  return nn;
	idx--;
      } /* if entry found */
    }
    nn=GWEN_DB_Node_List_Next(nn);
  } /* while node */

  return NULL;
}



GWEN_DB_NODE *GWEN_DB_FindVar(GWEN_DB_NODE *n,
                              const char *name,
                              int idx) {
  GWEN_DB_NODE *nn;

  assert(n);
  assert(name);

  if (n->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a group");
    return NULL;
  }

  /* find existing node */
  assert(n->children);
  nn=GWEN_DB_Node_List_First(n->children);
  while(nn) {
    if (nn->typ==GWEN_DB_NodeType_Var) {
      if (strcasecmp(nn->data.dataName, name)==0) {
	if (!idx)
	  /* ok, group found, return it */
	  return nn;
	idx--;
      } /* if entry found */
    }
    nn=GWEN_DB_Node_List_Next(nn);
  } /* while node */

  return NULL;
}







void* GWEN_DB_HandlePath(const char *entry,
                         void *data,
                         int idx,
                         uint32_t flags) {
  GWEN_DB_NODE *n;
  GWEN_DB_NODE *nn;

  n=(GWEN_DB_NODE*)data;

  /* check whether we are allowed to simply create the node */
  if (
      ((flags & GWEN_PATH_FLAGS_LAST) &&
       (((flags & GWEN_PATH_FLAGS_VARIABLE) &&
         (flags & GWEN_PATH_FLAGS_CREATE_VAR)) ||
        (!(flags & GWEN_PATH_FLAGS_VARIABLE) &&
         (flags & GWEN_PATH_FLAGS_CREATE_GROUP)))
      ) ||
      (
       !(flags & GWEN_PATH_FLAGS_LAST) &&
       (flags & GWEN_PATH_FLAGS_PATHCREATE))
     ) {
    /* simply create the new variable/group */
    if (idx!=0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Index is not 0, not creating %s[%d]",
               entry, idx);
      return 0;
    }
    if (flags & GWEN_PATH_FLAGS_VARIABLE) {
      DBG_VERBOUS(GWEN_LOGDOMAIN,
                  "Unconditionally creating variable \"%s\"", entry);
      nn=GWEN_DB_Var_new(entry);
      if (flags & GWEN_DB_FLAGS_INSERT)
        GWEN_DB_Node_Insert(n, nn);
      else
        GWEN_DB_Node_Append(n, nn);
      return nn;
    }
    else {
      DBG_VERBOUS(GWEN_LOGDOMAIN,
                  "Unconditionally creating group \"%s\"", entry);
      nn=GWEN_DB_Group_new(entry);
      if (flags & GWEN_DB_FLAGS_INSERT)
        GWEN_DB_Node_Insert(n, nn);
      else
        GWEN_DB_Node_Append(n, nn);
      return nn;
    }
  }

  /* find the node */
  if (flags & GWEN_PATH_FLAGS_VARIABLE) {
    nn=GWEN_DB_FindVar(n, entry, idx);
  }
  else {
    nn=GWEN_DB_FindGroup(n, entry, idx);
  }

  if (!nn) {
    /* node not found, check, if we are allowed to create it */
    if (
        (!(flags & GWEN_PATH_FLAGS_LAST) &&
         (flags & GWEN_PATH_FLAGS_PATHMUSTEXIST)) ||
        (flags & GWEN_PATH_FLAGS_NAMEMUSTEXIST)
       ) {
      if (flags & GWEN_PATH_FLAGS_VARIABLE) {
        DBG_VERBOUS(GWEN_LOGDOMAIN,
                    "Variable \"%s\" does not exist", entry);
      }
      else {
        DBG_VERBOUS(GWEN_LOGDOMAIN,
                    "Group \"%s\" does not exist", entry);
      }
      return 0;
    }
    /* create the new variable/group */
    if (idx!=0) {
      DBG_INFO(GWEN_LOGDOMAIN, "Index is not 0, not creating %s[%d]",
               entry, idx);
      return 0;
    }
    if (flags & GWEN_PATH_FLAGS_VARIABLE) {
      DBG_VERBOUS(GWEN_LOGDOMAIN,
                  "Variable \"%s\" not found, creating", entry);
      nn=GWEN_DB_Var_new(entry);
      if (flags & GWEN_DB_FLAGS_INSERT)
        GWEN_DB_Node_Insert(n, nn);
      else
        GWEN_DB_Node_Append(n, nn);
    }
    else {
      DBG_VERBOUS(GWEN_LOGDOMAIN,
                  "Group \"%s\" not found, creating", entry);
      nn=GWEN_DB_Group_new(entry);
      if (flags & GWEN_DB_FLAGS_INSERT)
        GWEN_DB_Node_Insert(n, nn);
      else
        GWEN_DB_Node_Append(n, nn);
    }
  } /* if node not found */
  else {
    /* node does exist, check whether this is ok */
    if (
        ((flags & GWEN_PATH_FLAGS_LAST) &
         (flags & GWEN_PATH_FLAGS_NAMEMUSTNOTEXIST)) ||
        (!(flags & GWEN_PATH_FLAGS_LAST) &
         (flags & GWEN_PATH_FLAGS_PATHMUSTNOTEXIST))
       ) {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Entry \"%s\" already exists", entry);
      return 0;
    }
  }

  return nn;
}



GWEN_DB_NODE *GWEN_DB_GetNode(GWEN_DB_NODE *n,
                              const char *path,
                              uint32_t flags){
  return (GWEN_DB_NODE*)GWEN_Path_HandleWithIdx(path,
                                                n,
                                                flags,
                                                GWEN_DB_HandlePath);
}



void GWEN_DB_ClearNode(GWEN_DB_NODE *n) {
  assert(n);
  if (n->children)
    GWEN_DB_Node_List_Clear(n->children);
}



GWEN_DB_NODE *GWEN_DB_GetValue(GWEN_DB_NODE *n,
			       const char *path,
			       int idx) {
  GWEN_DB_NODE *nn;

  /* find corresponding node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     GWEN_PATH_FLAGS_PATHMUSTEXIST |
		     GWEN_PATH_FLAGS_NAMEMUSTEXIST |
		     GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Path \"%s\" not found",
	      path);
    return 0;
  }

  /* find value */
  assert(nn->children);
  nn=GWEN_DB_Node_List_First(nn->children);
  while(nn) {
    if (nn->typ>=GWEN_DB_NodeType_ValueChar &&
	nn->typ<GWEN_DB_NodeType_ValueLast) {
      if (!idx)
	return nn;
      idx--;
    }
    nn=GWEN_DB_Node_List_Next(nn);
  }

  DBG_VERBOUS(GWEN_LOGDOMAIN, "No value[%d] for path \"%s\"",
	      idx, path);
  return NULL;
}



int GWEN_DB_DeleteVar(GWEN_DB_NODE *n,
		      const char *path) {
  GWEN_DB_NODE *nn;

  /* find corresponding node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     GWEN_PATH_FLAGS_PATHMUSTEXIST |
		     GWEN_PATH_FLAGS_NAMEMUSTEXIST |
		     GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Path \"%s\" not found",
	      path);
    return 1;
  }
  GWEN_DB_Node_Unlink(nn);
  GWEN_DB_Node_free(nn);
  return 0;
}



int GWEN_DB_DeleteGroup(GWEN_DB_NODE *n,
			const char *path) {
  GWEN_DB_NODE *nn;

  /* find corresponding node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     GWEN_PATH_FLAGS_PATHMUSTEXIST |
		     GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  if (!nn) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Path \"%s\" not found",
	      path);
    return 1;
  }
  GWEN_DB_Node_Unlink(nn);
  GWEN_DB_Node_free(nn);
  return 0;
}



int GWEN_DB_ClearGroup(GWEN_DB_NODE *n,
		       const char *path){
  assert(n);
  if (path) {
    GWEN_DB_NODE *nn;

    /* find corresponding node */
    nn=GWEN_DB_GetNode(n,
                       path,
                       GWEN_PATH_FLAGS_PATHMUSTEXIST |
                       GWEN_PATH_FLAGS_NAMEMUSTEXIST);
    if (!nn) {
      DBG_VERBOUS(GWEN_LOGDOMAIN, "Path \"%s\" not found",
                path);
      return 1;
    }
    GWEN_DB_ClearNode(nn);
  }
  else {
    GWEN_DB_ClearNode(n);
  }
  return 0;
}



const char *GWEN_DB_GetCharValue(GWEN_DB_NODE *n,
                                 const char *path,
                                 int idx,
				 const char *defVal){
  GWEN_DB_NODE *nn;

  nn=GWEN_DB_GetValue(n, path, idx);
  if (!nn){
    DBG_VERBOUS(GWEN_LOGDOMAIN,
		"Value for \"%s\" not found, returning default value",
		path);
    return defVal;
  }
  if (nn->typ!=GWEN_DB_NodeType_ValueChar) {
    /* bad type */
    DBG_VERBOUS(GWEN_LOGDOMAIN,
		"Bad type for path \"%s\", returning default value",
		path);
    return defVal;
  }
  return nn->data.dataChar;
}



int GWEN_DB_SetCharValue(GWEN_DB_NODE *n,
			 uint32_t flags,
			 const char *path,
			 const char *val){
  GWEN_DB_NODE *nn;
  GWEN_DB_NODE *nv;

  /* select/create node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     flags | GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Path \"%s\" not available",
	      path);
    return 1;
  }

  nv=GWEN_DB_ValueChar_new(val);

  /* delete contents of this variable if wanted */
  if (flags & GWEN_DB_FLAGS_OVERWRITE_VARS) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Clearing variable \"%s\"", path);
    GWEN_DB_ClearNode(nn);
  }

  /* add previously created value */
  if (flags & GWEN_DB_FLAGS_INSERT)
    GWEN_DB_Node_Insert(nn, nv);
  else
    GWEN_DB_Node_Append(nn, nv);
  DBG_VERBOUS(GWEN_LOGDOMAIN,
              "Added char value \"%s\" to variable \"%s\"", val, path);

  return 0;
}



int GWEN_DB_AddCharValue(GWEN_DB_NODE *n,
                         const char *path,
                         const char *val,
                         int senseCase,
                         int check){
  GWEN_DB_NODE *nn;
  GWEN_DB_NODE *nv;

  /* select/create node */
  nn=GWEN_DB_GetNode(n,
                     path,
                     GWEN_DB_FLAGS_DEFAULT | GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Path \"%s\" not available",
                path);
    return -1;
  }

  if (check) {
    nv=GWEN_DB_GetFirstValue(n);
    if (nv && nv->typ==GWEN_DB_NodeType_ValueChar) {
      int res;

      assert(nv->data.dataChar);
      if (senseCase)
	res=strcasecmp(nv->data.dataChar, val)==0;
      else
	res=strcmp(nv->data.dataChar, val)==0;
      if (res) {
	DBG_DEBUG(GWEN_LOGDOMAIN,
		  "Value \"%s\" of var \"%s\" already exists",
		  val, path);
	return 1;
      }
    }
  } /* if check */

  nv=GWEN_DB_ValueChar_new(val);
  GWEN_DB_Node_Append(nn, nv);
  DBG_VERBOUS(GWEN_LOGDOMAIN,
              "Added char value \"%s\" to variable \"%s\"", val, path);

  return 0;
}



int GWEN_DB_RemoveCharValue(GWEN_DB_NODE *n,
                            const char *path,
                            const char *val,
                            int senseCase){
  GWEN_DB_NODE *nn;
  GWEN_DB_NODE *nv;

  /* select/create node */
  nn=GWEN_DB_GetNode(n,
                     path,
                     GWEN_DB_FLAGS_DEFAULT | GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Path \"%s\" not available",
                path);
    return -1;
  }

  nv=GWEN_DB_GetFirstValue(n);
  if (nv && nv->typ==GWEN_DB_NodeType_ValueChar) {
    int res;

    assert(nv->data.dataChar);
    if (senseCase)
      res=strcasecmp(nv->data.dataChar, val)==0;
    else
      res=strcmp(nv->data.dataChar, val)==0;
    if (res) {
      DBG_DEBUG(GWEN_LOGDOMAIN,
		"Value \"%s\" of var \"%s\" already exists",
		val, path);
      GWEN_DB_Node_Unlink(nv);
      GWEN_DB_Node_free(nv);
      return 0;
    }
  }

  return 1;
}



int GWEN_DB_GetIntValue(GWEN_DB_NODE *n,
			const char *path,
			int idx,
			int defVal){
  GWEN_DB_NODE *nn;
  const char *p;
  int res;

  assert(n);
  nn=GWEN_DB_GetValue(n, path, idx);
  if (!nn){
    DBG_VERBOUS(GWEN_LOGDOMAIN,
		"Value[%d] for \"%s\" not found, returning default value",
		idx, path);
    return defVal;
  }

  switch(nn->typ) {
  case GWEN_DB_NodeType_ValueInt:
    return nn->data.dataInt;
  case GWEN_DB_NodeType_ValueChar:
    p=nn->data.dataChar;
    assert(p);
    if (sscanf(p, "%d", &res)!=1) {
      DBG_INFO(GWEN_LOGDOMAIN,
	       "String [%s] in node is not an int value", p);
      return defVal;
    }
    return res;

  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is neither char nor int value");
    return defVal;
  }
}



int GWEN_DB_SetIntValue(GWEN_DB_NODE *n,
			uint32_t flags,
			const char *path,
			int val){
  GWEN_DB_NODE *nn;
  GWEN_DB_NODE *nv;

  /* select/create node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     flags | GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Path \"%s\" not available",
	      path);
    return 1;
  }

  /* delete contents of this variable if wanted */
  if (flags & GWEN_DB_FLAGS_OVERWRITE_VARS) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Clearing variable \"%s\"", path);
    GWEN_DB_ClearNode(nn);
  }

  nv=GWEN_DB_ValueInt_new(val);
  if (flags & GWEN_DB_FLAGS_INSERT)
    GWEN_DB_Node_Insert(nn, nv);
  else
    GWEN_DB_Node_Append(nn, nv);
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Added int value \"%d\" to variable \"%s\"", val, path);
  return 0;
}



const void *GWEN_DB_GetBinValue(GWEN_DB_NODE *n,
                                const char *path,
                                int idx,
				const void *defVal,
                                unsigned int defValSize,
				unsigned int *returnValueSize){
  GWEN_DB_NODE *nn;

  assert(returnValueSize);
  nn=GWEN_DB_GetValue(n, path, idx);
  if (!nn){
    DBG_VERBOUS(GWEN_LOGDOMAIN,
		"Value for \"%s\" not found, returning default value",
		path);
    *returnValueSize=defValSize;
    return defVal;
  }
  if (nn->typ!=GWEN_DB_NodeType_ValueBin) {
    /* bad type */
    DBG_VERBOUS(GWEN_LOGDOMAIN,
		"Bad type for path \"%s\", returning default value",
		path);
    *returnValueSize=defValSize;
    return defVal;
  }
  *returnValueSize=nn->dataSize;
  return nn->data.dataBin;
}



int GWEN_DB_SetBinValue(GWEN_DB_NODE *n,
			uint32_t flags,
			const char *path,
                        const void *val,
			unsigned int valSize){
  GWEN_DB_NODE *nn;
  GWEN_DB_NODE *nv;

  assert(val);
  /* select/create node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     flags | GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Path \"%s\" not available",
                path);
    return 1;
  }

  /* delete contents of this variable if wanted */
  if (flags & GWEN_DB_FLAGS_OVERWRITE_VARS) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Clearing variable \"%s\"", path);
    GWEN_DB_ClearNode(nn);
  }

  nv=GWEN_DB_ValueBin_new(val, valSize);
  if (flags & GWEN_DB_FLAGS_INSERT)
    GWEN_DB_Node_Insert(nn, nv);
  else
    GWEN_DB_Node_Append(nn, nv);
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Added bin value to variable \"%s\"", path);
  return 0;
}



void *GWEN_DB_GetPtrValue(GWEN_DB_NODE *n,
                          const char *path,
                          int idx,
                          void *defVal){
  GWEN_DB_NODE *nn;

  nn=GWEN_DB_GetValue(n, path, idx);
  if (!nn){
    DBG_VERBOUS(GWEN_LOGDOMAIN,
		"Value for \"%s\" not found, returning default value",
		path);
    return defVal;
  }
  if (nn->typ!=GWEN_DB_NodeType_ValuePtr) {
    /* bad type */
    DBG_VERBOUS(GWEN_LOGDOMAIN,
		"Bad type for path \"%s\", returning default value",
		path);
    return defVal;
  }
  return nn->data.dataPtr;
}



int GWEN_DB_SetPtrValue(GWEN_DB_NODE *n,
                        uint32_t flags,
                        const char *path,
                        void *val){
  GWEN_DB_NODE *nn;
  GWEN_DB_NODE *nv;

  /* select/create node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     flags | GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Path \"%s\" not available",
	      path);
    return 1;
  }

  /* delete contents of this variable if wanted */
  if (flags & GWEN_DB_FLAGS_OVERWRITE_VARS) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Clearing variable \"%s\"", path);
    GWEN_DB_ClearNode(nn);
  }

  nv=GWEN_DB_ValuePtr_new(val);
  if (flags & GWEN_DB_FLAGS_INSERT)
    GWEN_DB_Node_Insert(nn, nv);
  else
    GWEN_DB_Node_Append(nn, nv);
  DBG_VERBOUS(GWEN_LOGDOMAIN, "Added ptr value to variable \"%s\"", path);

  return 0;
}
















GWEN_DB_NODE *GWEN_DB_GetGroup(GWEN_DB_NODE *n,
			       uint32_t flags,
			       const char *path) {
  GWEN_DB_NODE *nn;

  /* select/create node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     flags & ~GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Path \"%s\" not available",
	      path);
    return NULL;
  }

  /* delete contents of this variable if wanted */
  if (flags & GWEN_DB_FLAGS_OVERWRITE_GROUPS) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Clearing group \"%s\"", path);
    GWEN_DB_ClearNode(nn);
  }

  return nn;
}



const char *GWEN_DB_GroupName(GWEN_DB_NODE *n){
  assert(n);
  if (n->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a group");
    return NULL;
  }
  return n->data.dataName;
}



void GWEN_DB_Dump(GWEN_DB_NODE *n, FILE *f, int insert){
  if (n) {
    int i;

    for (i=0; i<insert; i++)
      fprintf(f, " ");

    /* dump dynamic (allocated) data */
    switch(n->typ) {
    case GWEN_DB_NodeType_Group:
      fprintf(f, "Group : \"%s\"\n", n->data.dataName);
      break;
    case GWEN_DB_NodeType_Var:
      fprintf(f, "Var   : \"%s\"\n", n->data.dataName);
      break;
    case GWEN_DB_NodeType_ValueChar:
      fprintf(f, "Value : \"%s\" (char)\n", n->data.dataChar);
      break;
    case GWEN_DB_NodeType_ValueInt:
      fprintf(f, "Value : %d (int)\n", n->data.dataInt);
      break;
    case GWEN_DB_NodeType_ValueBin: {
      char *buffer;

      buffer=(char*)GWEN_Memory_malloc((n->dataSize*2)+1);
      assert(buffer);
      if (GWEN_Text_ToHex(n->data.dataBin, n->dataSize,
			  buffer, (n->dataSize*2)+1)==0) {
	fprintf(f, "Value : %d bytes (bin)\n", n->dataSize);
      }
      else {
	fprintf(f, "Value : %s (bin)\n", buffer);
      }
      GWEN_Memory_dealloc(buffer);
      break;
    }
    case GWEN_DB_NodeType_ValuePtr:
      fprintf(f, "Value : %p (ptr)\n", n->data.dataPtr);
      break;
    default:
      fprintf(f, "[unknown node type %d]\n", n->typ);
    }

    /* dump children */
    if (n->children) {
      GWEN_DB_NODE *cn;

      cn=GWEN_DB_Node_List_First(n->children);
      while(cn) {
	GWEN_DB_Dump(cn, f, insert+4);
	cn=GWEN_DB_Node_List_Next(cn);
      }
    }
  }
  else {
    fprintf(f, "[no node]\n");
  }
}



int GWEN_DB_AddGroup(GWEN_DB_NODE *n, GWEN_DB_NODE *nn){
  assert(n);
  assert(nn);

  if (n->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Target node is not a group");
    return 0;
  }

  if (nn->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Source node is not a group");
    return 0;
  }

  GWEN_DB_Node_Append(n, nn);
  return 0;
}



int GWEN_DB_InsertGroup(GWEN_DB_NODE *n, GWEN_DB_NODE *nn){
  assert(n);
  assert(nn);

  if (n->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Target node is not a group");
    return 0;
  }

  if (nn->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Source node is not a group");
    return 0;
  }

  GWEN_DB_Node_Insert(n, nn);
  return 0;
}



int GWEN_DB_AddGroupChildren(GWEN_DB_NODE *n, GWEN_DB_NODE *nn){
  GWEN_DB_NODE *cpn;

  assert(n);
  assert(nn);

  if (n->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Target node is not a group");
    return -1;
  }

  if (nn->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Source node is not a group");
    GWEN_DB_Dump(nn, stderr, 1);
    return -1;
  }

  nn=GWEN_DB_Node_List_First(nn->children);
  while (nn) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Duplicating node");
    cpn=GWEN_DB_Node_dup(nn);
    GWEN_DB_Node_Append(n, cpn);
    nn=GWEN_DB_Node_List_Next(nn);
  } /* while */
  return 0;
}



void GWEN_DB_UnlinkGroup(GWEN_DB_NODE *n){
  assert(n);
  if (n->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a group");
    return;
  }
  GWEN_DB_Node_Unlink(n);
}


int GWEN_DB_VariableExists(GWEN_DB_NODE *n, const char *path){
  GWEN_DB_NODE *nn;

  /* find corresponding node */
  assert(n);
  nn=GWEN_DB_GetNode(n,
		     path,
		     GWEN_PATH_FLAGS_PATHMUSTEXIST |
		     GWEN_PATH_FLAGS_NAMEMUSTEXIST |
		     GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Path \"%s\" not found", path);
    return 0;
  }

  return 1;
}



int GWEN_DB_ValueExists(GWEN_DB_NODE *n,
                        const char *path,
                        unsigned int i){
  return (GWEN_DB_GetValue(n, path, i)!=0);
}



GWEN_DB_NODE_TYPE GWEN_DB_GetVariableType(GWEN_DB_NODE *n,
					  const char *p){
  GWEN_DB_NODE *nn;

  nn=GWEN_DB_FindVar(n, p, 0);
  if (!nn)
    return GWEN_DB_NodeType_Unknown;

  nn=GWEN_DB_GetFirstValue(nn);
  if (!nn)
    return GWEN_DB_NodeType_Unknown;
  return GWEN_DB_GetValueType(nn);
}



GWEN_DB_NODE_TYPE GWEN_DB_GetValueTypeByPath(GWEN_DB_NODE *n,
					     const char *path,
					     unsigned int i){
  GWEN_DB_NODE *nn;

  nn=GWEN_DB_GetValue(n, path, i);
  if (!nn)
    return GWEN_DB_NodeType_Unknown;
  return GWEN_DB_GetValueType(nn);
}


void GWEN_DB_GroupRename(GWEN_DB_NODE *n, const char *newname){
  assert(n);
  assert(newname);
  assert(n->typ==GWEN_DB_NodeType_Group);
  GWEN_Memory_dealloc(n->data.dataName);
  n->data.dataName=GWEN_Memory_strdup(newname);
}







int GWEN_DB_IsGroup(const GWEN_DB_NODE *n){
  assert(n);
  return n->typ==GWEN_DB_NodeType_Group;
}



int GWEN_DB_IsVariable(const GWEN_DB_NODE *n){
  assert(n);
  return n->typ==GWEN_DB_NodeType_Var;
}



int GWEN_DB_IsValue(const GWEN_DB_NODE *n){
  assert(n);
  return (n->typ>=GWEN_DB_NodeType_ValueChar &&
          n->typ>=GWEN_DB_NodeType_ValueLast);
}



void *GWEN_DB_Groups_Foreach(GWEN_DB_NODE *node, GWEN_DB_NODES_CB func,
                             void *user_data){
  GWEN_DB_NODE *iter;
  void *res;

  assert(node);
  assert(func);

  iter=GWEN_DB_GetFirstGroup(node);
  res=NULL;
  while(iter){
    res=(*func)(iter, user_data);
    if (res) {
      break;
    }
    iter=GWEN_DB_GetNextGroup(iter);
  }
  return res;
}



void *GWEN_DB_count_cb(GWEN_DB_NODE *node, void *user_data){
  unsigned int *a = user_data;
  ++(*a);
  return NULL;
}



unsigned int GWEN_DB_Groups_Count(const GWEN_DB_NODE *node){
  unsigned int res = 0;
  GWEN_DB_Groups_Foreach((GWEN_DB_NODE *)node, GWEN_DB_count_cb, &res);
  return res;
}



void *GWEN_DB_Variables_Foreach(GWEN_DB_NODE *node, GWEN_DB_NODES_CB func,
                                void *user_data){
  GWEN_DB_NODE *iter;
  void *res;

  assert(node);
  assert(func);

  iter=GWEN_DB_GetFirstVar(node);
  res=NULL;
  while(iter){
    res=(*func)(iter, user_data);
    if (res) {
      break;
    }
    iter=GWEN_DB_GetNextVar(iter);
  }
  return res;
}



unsigned int GWEN_DB_Variables_Count(const GWEN_DB_NODE *node){
  unsigned int res = 0;
  GWEN_DB_Variables_Foreach((GWEN_DB_NODE *)node, GWEN_DB_count_cb, &res);
  return res;
}



void *GWEN_DB_Values_Foreach(GWEN_DB_NODE *node, GWEN_DB_NODES_CB func,
                             void *user_data){
  GWEN_DB_NODE *iter;
  void *res;

  assert(node);
  assert(func);

  iter=GWEN_DB_GetFirstValue(node);
  res=NULL;
  while(iter){
    res=(*func)(iter, user_data);
    if (res) {
      break;
    }
    iter=GWEN_DB_GetNextValue(iter);
  }
  return res;
}



unsigned int GWEN_DB_Values_Count(const GWEN_DB_NODE *node){
  unsigned int res = 0;
  GWEN_DB_Values_Foreach((GWEN_DB_NODE *)node, GWEN_DB_count_cb, &res);
  return res;
}



uint32_t GWEN_DB_GetNodeFlags(const GWEN_DB_NODE *n){
  assert(n);
  return n->nodeFlags;
}



void GWEN_DB_SetNodeFlags(GWEN_DB_NODE *n,
                          uint32_t flags){
  assert(n);
  n->nodeFlags=flags;
}



void GWEN_DB_ModifyBranchFlagsUp(GWEN_DB_NODE *n,
				 uint32_t newflags,
				 uint32_t mask){
  uint32_t flags;

  assert(n);

  while(n) {
    flags=n->nodeFlags;
    flags=((flags^newflags)&(mask))^flags;
    n->nodeFlags=flags;
    n=n->parent;
  } /* while */
}



void GWEN_DB_ModifyBranchFlagsDown(GWEN_DB_NODE *n,
				   uint32_t newflags,
				   uint32_t mask){
  uint32_t flags;
  GWEN_DB_NODE *cn;

  assert(n);

  flags=n->nodeFlags;
  flags=((flags^newflags)&(mask))^flags;
  n->nodeFlags=flags;

  cn=GWEN_DB_Node_List_First(n->children);
  while(cn) {
    GWEN_DB_ModifyBranchFlagsDown(cn, newflags, mask);
    cn=GWEN_DB_Node_List_Next(cn);
  } /* while cn */
}



GWEN_DB_NODE *GWEN_DB_FindFirstGroup(GWEN_DB_NODE *n, const char *name){
  GWEN_DB_NODE *nn;

  assert(n);
  if (n->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a group");
    return NULL;
  }
  nn=GWEN_DB_Node_List_First(n->children);
  while(nn) {
    if ((nn->typ==GWEN_DB_NodeType_Group) &&
	(-1!=GWEN_Text_ComparePattern(nn->data.dataName, name, 0)))
      break;
    nn=GWEN_DB_Node_List_Next(nn);
  } /* while node */
  return nn;
}



GWEN_DB_NODE *GWEN_DB_FindNextGroup(GWEN_DB_NODE *n, const char *name){
  GWEN_DB_NODE *og;

  og=n;
  assert(n);
  if (n->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a group");
    return NULL;
  }
  n=GWEN_DB_GetNextGroup(n);
  while(n) {
    if (-1!=GWEN_Text_ComparePattern(n->data.dataName, name, 0))
      break;
    n=GWEN_DB_GetNextGroup(n);
  } /* while node */
  assert(n!=og);
  return n;
}



GWEN_DB_NODE *GWEN_DB_FindFirstVar(GWEN_DB_NODE *n, const char *name) {
  GWEN_DB_NODE *nn;

  assert(n);
  if (n->typ!=GWEN_DB_NodeType_Group) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a group");
    return NULL;
  }

  nn=GWEN_DB_Node_List_First(n->children);
  while(nn) {
    if ((nn->typ==GWEN_DB_NodeType_Var) &&
	(-1!=GWEN_Text_ComparePattern(nn->data.dataName, name, 0)))
      break;
    nn=GWEN_DB_Node_List_Next(nn);
  } /* while node */

  return nn;
}



GWEN_DB_NODE *GWEN_DB_FindNextVar(GWEN_DB_NODE *n, const char *name) {
  GWEN_DB_NODE *og;

  og=n;
  assert(n);
  if (n->typ!=GWEN_DB_NodeType_Var) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a variable");
    return NULL;
  }
  n=GWEN_DB_GetNextVar(n);
  while(n) {
    if (-1!=GWEN_Text_ComparePattern(n->data.dataName, name, 0))
      break;
    n=GWEN_DB_GetNextVar(n);
  } /* while node */
  assert(n!=og);
  return n;
}



const char *GWEN_DB_VariableName(GWEN_DB_NODE *n){
  assert(n);
  if (n->typ!=GWEN_DB_NodeType_Var) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Node is not a variable");
    return NULL;
  }
  return n->data.dataName;
}



void GWEN_DB_VariableRename(GWEN_DB_NODE *n, const char *newname){
  assert(n);
  assert(newname);
  assert(n->typ==GWEN_DB_NodeType_Var);
  GWEN_Memory_dealloc(n->data.dataName);
  n->data.dataName=GWEN_Memory_strdup(newname);
}






#include "dbrw.c"









