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
#include <unistd.h>




GWEN_DB_NODE *GWEN_DB_ValueBin_new(const void *data,
                                    unsigned int datasize){
  GWEN_DB_VALUE_BIN *v;

  GWEN_NEW_OBJECT(GWEN_DB_VALUE_BIN, v);
  v->h.h.typ=GWEN_DB_NODETYPE_VALUE;
  v->h.typ=GWEN_DB_VALUETYPE_BIN;
  if (datasize) {
    assert(data);
    v->dataSize=datasize;
    v->data=(char*)malloc(datasize);
    assert(v->data);
    memmove(v->data, data, datasize);
  }
  return (GWEN_DB_NODE*)v;
}



GWEN_DB_NODE *GWEN_DB_ValueInt_new(int data) {
  GWEN_DB_VALUE_INT *v;

  GWEN_NEW_OBJECT(GWEN_DB_VALUE_INT, v);
  v->h.h.typ=GWEN_DB_NODETYPE_VALUE;
  v->h.typ=GWEN_DB_VALUETYPE_INT;
  v->data=data;
  return (GWEN_DB_NODE*)v;
}



GWEN_DB_NODE *GWEN_DB_ValueChar_new(const char *data) {
  GWEN_DB_VALUE_CHAR *v;

  GWEN_NEW_OBJECT(GWEN_DB_VALUE_CHAR, v);
  v->h.h.typ=GWEN_DB_NODETYPE_VALUE;
  v->h.typ=GWEN_DB_VALUETYPE_CHAR;
  if (data)
    v->data=strdup(data);
  else
    v->data=strdup("");
  return (GWEN_DB_NODE*)v;
}



GWEN_DB_NODE *GWEN_DB_Group_new(const char *name){
  GWEN_DB_GROUP *node;

  assert(name);
  GWEN_NEW_OBJECT(GWEN_DB_GROUP, node);
  node->h.typ=GWEN_DB_NODETYPE_GROUP;
  node->name=strdup(name);
  return (GWEN_DB_NODE*)node;
}



GWEN_DB_NODE *GWEN_DB_Var_new(const char *name){
  GWEN_DB_VAR *node;

  assert(name);
  GWEN_NEW_OBJECT(GWEN_DB_VAR, node);
  node->h.typ=GWEN_DB_NODETYPE_VAR;
  node->name=strdup(name);
  return (GWEN_DB_NODE*)node;
}


void GWEN_DB_Node_Append_UnDirty(GWEN_DB_NODE *parent,
				 GWEN_DB_NODE *n){
  GWEN_DB_NODE *curr;

  assert(parent);
  assert(n);

  curr=parent->h.child;
  if (!curr) {
    parent->h.child=n;
  }
  else {
    while(curr->h.next) {
      curr=curr->h.next;
    }
    curr->h.next=n;
  }
  n->h.parent=parent;
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
  GWEN_DB_NODE *curr;

  assert(parent);
  assert(n);

  curr=parent->h.child;
  if (!curr) {
    parent->h.child=n;
  }
  else {
    n->h.next=parent->h.child;
    parent->h.child=n;
  }
  n->h.parent=parent;
}



void GWEN_DB_Node_Insert(GWEN_DB_NODE *parent,
			 GWEN_DB_NODE *n){
  GWEN_DB_Node_InsertUnDirty(parent, n);
  GWEN_DB_ModifyBranchFlagsUp(parent,
			      GWEN_DB_NODE_FLAGS_DIRTY,
			      GWEN_DB_NODE_FLAGS_DIRTY);
}


void GWEN_DB_Node_Unlink_UnDirty(GWEN_DB_NODE *n) {
  GWEN_DB_NODE *curr;
  GWEN_DB_NODE *parent;

  assert(n);
  parent=n->h.parent;
  assert(parent);

  curr=parent->h.child;
  if (curr) {
    if (curr==n) {
      parent->h.child=curr->h.next;
    }
    else {
      while(curr->h.next!=n) {
        curr=curr->h.next;
      }
      if (curr)
	curr->h.next=n->h.next;
    }
  }
  n->h.next=0;
  n->h.parent=0;
}



void GWEN_DB_Node_Unlink(GWEN_DB_NODE *n) {
  GWEN_DB_NODE *parent;

  assert(n);
  parent=n->h.parent;
  assert(parent);

  GWEN_DB_Node_Unlink_UnDirty(n);
  GWEN_DB_ModifyBranchFlagsUp(parent,
			      GWEN_DB_NODE_FLAGS_DIRTY,
			      GWEN_DB_NODE_FLAGS_DIRTY);
}


void GWEN_DB_Node_free(GWEN_DB_NODE *n){
  if (n) {
    GWEN_DB_NODE *cn;

    /* free children */
    cn=n->h.child;
    while(cn) {
      GWEN_DB_NODE *ncn;

      ncn=cn->h.next;
      DBG_VERBOUS(0, "Freeing child node");
      GWEN_DB_Node_free(cn);
      cn=ncn;
    }

    /* free dynamic (allocated) data */
    switch(n->h.typ) {
    case GWEN_DB_NODETYPE_GROUP:
      DBG_VERBOUS(0, "Freeing dynamic data of group \"%s\"", n->group.name);
      free(n->group.name);
      break;

    case GWEN_DB_NODETYPE_VAR:
      DBG_VERBOUS(0, "Freeing dynamic data of var \"%s\"", n->var.name);
      free(n->var.name);
      break;

    case GWEN_DB_NODETYPE_VALUE:
      switch(n->val.h.typ) {
      case GWEN_DB_VALUETYPE_CHAR:
        DBG_VERBOUS(0, "Freeing dynamic data of char value");
        free(n->val.c.data);
        break;

      case GWEN_DB_VALUETYPE_INT:
        /* no dynamic data, nothing to do */
        DBG_VERBOUS(0, "Freeing dynamic data of int value");
        break;

      case GWEN_DB_VALUETYPE_BIN:
        DBG_VERBOUS(0, "Freeing dynamic data of bin value");
        free(n->val.b.data);
        break;

      default:
        DBG_WARN(0, "Unknown value type (%d)", n->val.h.typ);
      }
      break;

    default:
      DBG_WARN(0, "Unknown node type (%d)", n->h.typ);
    }
    DBG_VERBOUS(0, "Freeing node itself");
    GWEN_FREE_OBJECT(n);
  }
}


GWEN_DB_NODE *GWEN_DB_Node_dup(const GWEN_DB_NODE *n){
  GWEN_DB_NODE *nn;

  switch(n->h.typ) {
  case GWEN_DB_NODETYPE_GROUP:
    DBG_VERBOUS(0, "Duplicating group \"%s\"", n->group.name);
    nn=GWEN_DB_Group_new(n->group.name);
    break;

  case GWEN_DB_NODETYPE_VAR:
    DBG_VERBOUS(0, "Duplicating variable \"%s\"", n->var.name);
    nn=GWEN_DB_Var_new(n->var.name);
    break;

  case GWEN_DB_NODETYPE_VALUE:
    switch(n->val.h.typ) {
    case GWEN_DB_VALUETYPE_CHAR:
      nn=GWEN_DB_ValueChar_new(n->val.c.data);
      break;

    case GWEN_DB_VALUETYPE_INT:
      nn=GWEN_DB_ValueInt_new(n->val.i.data);
      break;

    case GWEN_DB_VALUETYPE_BIN:
      nn=GWEN_DB_ValueBin_new(n->val.b.data,
                              n->val.b.dataSize);
      break;

    default:
      DBG_WARN(0, "Unknown value type (%d)", n->val.h.typ);
      nn=0;
    }
    break;

  default:
    DBG_WARN(0, "Unknown node type (%d)", n->h.typ);
    nn=0;
  }

  /* duplicate all children and add them to the new node */
  if (nn) {
    const GWEN_DB_NODE *cn;

    cn=n->h.child;
    while(cn) {
      GWEN_DB_NODE *ncn;

      /* duplicate child and add it */
      ncn=GWEN_DB_Node_dup(cn);
      if (!ncn) {
        GWEN_DB_Node_free(nn);
        return 0;
      }
      GWEN_DB_Node_Append_UnDirty(nn, ncn);
      cn=cn->h.next;
    } /* while cn */
  }

  return nn;
}



void GWEN_DB_Group_free(GWEN_DB_NODE *n){
  GWEN_DB_Node_free(n);
}



GWEN_DB_NODE *GWEN_DB_Group_dup(const GWEN_DB_NODE *n){
  assert(n);
  if (n->h.typ!=GWEN_DB_NODETYPE_GROUP) {
    DBG_ERROR(0, "Node is not a group");
    return 0;
  }
  return GWEN_DB_Node_dup(n);
}



GWEN_DB_NODE *GWEN_DB_GetFirstGroup(GWEN_DB_NODE *n){
  GWEN_DB_NODE *nn;

  assert(n);
  if (n->h.typ!=GWEN_DB_NODETYPE_GROUP) {
    DBG_ERROR(0, "Node is not a group");
    return 0;
  }
  nn=n->h.child;
  while(nn) {
    if (nn->h.typ==GWEN_DB_NODETYPE_GROUP)
      break;
    nn=nn->h.next;
  } /* while node */
  return nn;
}



GWEN_DB_NODE *GWEN_DB_GetNextGroup(GWEN_DB_NODE *n){
  GWEN_DB_NODE *og;

  og=n;
  assert(n);
  if (n->h.typ!=GWEN_DB_NODETYPE_GROUP) {
    DBG_ERROR(0, "Node is not a group");
    return 0;
  }
  n=n->h.next;
  while(n) {
    if (n->h.typ==GWEN_DB_NODETYPE_GROUP)
      break;
    n=n->h.next;
  } /* while node */
  assert(n!=og);
  return n;
}



GWEN_DB_NODE *GWEN_DB_GetFirstVar(GWEN_DB_NODE *n){
  GWEN_DB_NODE *nn;

  assert(n);
  if (n->h.typ!=GWEN_DB_NODETYPE_GROUP) {
    DBG_ERROR(0, "Node is not a group");
    return 0;
  }
  nn=n->h.child;
  while(nn) {
    if (nn->h.typ==GWEN_DB_NODETYPE_VAR)
      break;
    nn=nn->h.next;
  } /* while node */
  return nn;
}



GWEN_DB_NODE *GWEN_DB_GetNextVar(GWEN_DB_NODE *n){
  assert(n);
  if (n->h.typ!=GWEN_DB_NODETYPE_VAR) {
    DBG_ERROR(0, "Node is not a variable");
    return 0;
  }
  n=n->h.next;
  while(n) {
    if (n->h.typ==GWEN_DB_NODETYPE_VAR)
      break;
    n=n->h.next;
  } /* while node */
  return n;
}



GWEN_DB_NODE *GWEN_DB_GetFirstValue(GWEN_DB_NODE *n){
  GWEN_DB_NODE *nn;

  assert(n);
  if (n->h.typ!=GWEN_DB_NODETYPE_VAR) {
    DBG_ERROR(0, "Node is not a variable");
    return 0;
  }
  nn=n->h.child;
  while(nn) {
    if (nn->h.typ==GWEN_DB_NODETYPE_VALUE)
      break;
    nn=nn->h.next;
  } /* while node */
  return nn;
}



GWEN_DB_NODE *GWEN_DB_GetNextValue(GWEN_DB_NODE *n){
  assert(n);
  if (n->h.typ!=GWEN_DB_NODETYPE_VALUE) {
    DBG_ERROR(0, "Node is not a value");
    return 0;
  }
  n=n->h.next;
  while(n) {
    if (n->h.typ==GWEN_DB_NODETYPE_VALUE)
      break;
    n=n->h.next;
  } /* while node */
  return n;
}



GWEN_DB_VALUETYPE GWEN_DB_GetValueType(GWEN_DB_NODE *n){
  assert(n);
  if (n->h.typ!=GWEN_DB_NODETYPE_VALUE) {
    DBG_ERROR(0, "Node is not a value");
    return GWEN_DB_VALUETYPE_UNKNOWN;
  }
  return n->val.h.typ;
}



const char *GWEN_DB_GetCharValueFromNode(GWEN_DB_NODE *n){
  assert(n);
  if (n->h.typ!=GWEN_DB_NODETYPE_VALUE) {
    DBG_ERROR(0, "Node is not a value");
    return 0;
  }
  if (n->val.h.typ!=GWEN_DB_VALUETYPE_CHAR) {
    DBG_ERROR(0, "Node is not a char value");
    return 0;
  }
  return n->val.c.data;
}



int GWEN_DB_GetIntValueFromNode(GWEN_DB_NODE *n){
  assert(n);
  if (n->h.typ!=GWEN_DB_NODETYPE_VALUE) {
    DBG_ERROR(0, "Node is not a value");
    return 0;
  }

  if (n->val.h.typ==GWEN_DB_VALUETYPE_CHAR) {
    /* try to convert the char value into an integer */
    const char *p;
    int res;

    p=GWEN_DB_GetCharValueFromNode(n);
    assert(p);
    if (sscanf(p, "%d", &res)!=1) {
      DBG_ERROR(0, "Node is not an int value");
      return 0;
    }
    return res;
  }
  else if (n->val.h.typ!=GWEN_DB_VALUETYPE_INT) {
    DBG_ERROR(0, "Node is not a char or int value");
    return 0;
  }
  return n->val.i.data;
}



const void *GWEN_DB_GetBinValueFromNode(GWEN_DB_NODE *n,
                                        unsigned int *size){
  assert(n);
  if (n->h.typ!=GWEN_DB_NODETYPE_VALUE) {
    DBG_ERROR(0, "Node is not a value");
    return 0;
  }

  if (n->val.h.typ!=GWEN_DB_VALUETYPE_BIN) {
    DBG_ERROR(0, "Node is not a binary value");
    return 0;
  }

  *size=n->val.b.dataSize;
  return n->val.b.data;
}



GWEN_DB_NODE *GWEN_DB_FindGroup(GWEN_DB_NODE *n,
                                const char *name) {
  GWEN_DB_NODE *nn;

  assert(n);
  assert(name);

  /* find existing node */
  nn=n->h.child;
  while(nn) {
    if (nn->h.typ==GWEN_DB_NODETYPE_GROUP) {
      if (strcasecmp(nn->group.name, name)==0) {
        /* ok, group found, return it */
        return nn;
      } /* if entry found */
    }
    nn=nn->h.next;
  } /* while child */

  return nn;
}



GWEN_DB_NODE *GWEN_DB_FindVar(GWEN_DB_NODE *n,
                              const char *name) {
  GWEN_DB_NODE *nn;

  assert(n);
  assert(name);

  /* find existing node */
  nn=n->h.child;
  while(nn) {
    if (nn->h.typ==GWEN_DB_NODETYPE_VAR) {
      if (strcasecmp(nn->var.name, name)==0) {
        /* ok, group found, return it */
        return nn;
      } /* if entry found */
    }
    nn=nn->h.next;
  } /* while child */

  return nn;
}





void* GWEN_DB_HandlePath(const char *entry,
                         void *data,
                         GWEN_TYPE_UINT32 flags) {
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
    if (flags & GWEN_PATH_FLAGS_VARIABLE) {
      DBG_VERBOUS(0, "Unconditionally creating variable \"%s\"", entry);
      nn=GWEN_DB_Var_new(entry);
      GWEN_DB_Node_Append(n, nn);
      return nn;
    }
    else {
      DBG_VERBOUS(0, "Unconditionally creating group \"%s\"", entry);
      nn=GWEN_DB_Group_new(entry);
      GWEN_DB_Node_Append(n, nn);
      return nn;
    }
  }

  /* find the node */
  if (flags & GWEN_PATH_FLAGS_VARIABLE) {
    nn=GWEN_DB_FindVar(n, entry);
  }
  else {
    nn=GWEN_DB_FindGroup(n, entry);
  }

  if (!nn) {
    /* node not found, check, if we are allowed to create it */
    if (
        (!(flags & GWEN_PATH_FLAGS_LAST) &&
         (flags & GWEN_PATH_FLAGS_PATHMUSTEXIST)) ||
        (flags & GWEN_PATH_FLAGS_NAMEMUSTEXIST)
       ) {
      if (flags & GWEN_PATH_FLAGS_VARIABLE) {
        DBG_VERBOUS(0, "Variable \"%s\" does not exist", entry);
      }
      else {
        DBG_VERBOUS(0, "Group \"%s\" does not exist", entry);
      }
      return 0;
    }
    /* create the new variable/group */
    if (flags & GWEN_PATH_FLAGS_VARIABLE) {
      DBG_VERBOUS(0, "Variable \"%s\" not found, creating", entry);
      nn=GWEN_DB_Var_new(entry);
      GWEN_DB_Node_Append(n, nn);
    }
    else {
      DBG_VERBOUS(0, "Group \"%s\" not found, creating", entry);
      nn=GWEN_DB_Group_new(entry);
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
      DBG_VERBOUS(0, "Entry \"%s\" already exists", entry);
      return 0;
    }
  }

  return nn;
}



GWEN_DB_NODE *GWEN_DB_GetNode(GWEN_DB_NODE *n,
                              const char *path,
                              GWEN_TYPE_UINT32 flags){
  return (GWEN_DB_NODE*)GWEN_Path_Handle(path,
                                         n,
                                         flags,
                                         GWEN_DB_HandlePath);
}



void GWEN_DB_ClearNode(GWEN_DB_NODE *n) {
  GWEN_DB_NODE *nn;

  assert(n);
  while ((nn=n->h.child)) {
    GWEN_DB_Node_Unlink(nn);
    GWEN_DB_Node_free(nn);
  } /* while */
}



GWEN_DB_NODE *GWEN_DB_GetValue(GWEN_DB_NODE *n,
			       const char *path,
			       int idx) {
  GWEN_DB_NODE *nn;
  int i;

  /* find corresponding node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     GWEN_PATH_FLAGS_PATHMUSTEXIST |
		     GWEN_PATH_FLAGS_NAMEMUSTEXIST |
		     GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_VERBOUS(0, "Path \"%s\" not found",
	      path);
    return 0;
  }

  /* find value */
  nn=nn->h.child;
  i=0;
  while (nn) {
    if (nn->h.typ==GWEN_DB_NODETYPE_VALUE) {
      /* value found */
      if (i==idx) {
	return nn;
      }
      else
	i++;
    }
    nn=nn->h.next;
  } /* while */
  DBG_VERBOUS(0, "No value[%d] for path \"%s\"",
	    idx, path);
  return 0;
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
    DBG_VERBOUS(0, "Path \"%s\" not found",
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
    DBG_VERBOUS(0, "Path \"%s\" not found",
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
      DBG_VERBOUS(0, "Path \"%s\" not found",
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
    DBG_VERBOUS(0, "Value for \"%s\" not found, returning default value",
	      path);
    return defVal;
  }
  if (nn->val.h.typ!=GWEN_DB_VALUETYPE_CHAR) {
    /* bad type */
    DBG_VERBOUS(0, "Bad type for path \"%s\", returning default value",
	      path);
    return defVal;
  }
  return nn->val.c.data;
}



int GWEN_DB_SetCharValue(GWEN_DB_NODE *n,
			 GWEN_TYPE_UINT32 flags,
			 const char *path,
			 const char *val){
  GWEN_DB_NODE *nn;
  GWEN_DB_NODE *nv;

  /* select/create node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     flags | GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_VERBOUS(0, "Path \"%s\" not available",
	      path);
    return 1;
  }

  /* delete contents of this variable if wanted */
  if (flags & GWEN_DB_FLAGS_OVERWRITE_VARS) {
    DBG_VERBOUS(0, "Clearing variable \"%s\"", path);
    GWEN_DB_ClearNode(nn);
  }

  nv=GWEN_DB_ValueChar_new(val);
  GWEN_DB_Node_Append(nn, nv);
  DBG_VERBOUS(0, "Added char value \"%s\" to variable \"%s\"", val, path);

  return 0;
}



int GWEN_DB_GetIntValue(GWEN_DB_NODE *n,
			const char *path,
			int idx,
			int defVal){
  GWEN_DB_NODE *nn;

  nn=GWEN_DB_GetValue(n, path, idx);
  if (!nn){
    DBG_VERBOUS(0, "Value[%d] for \"%s\" not found, returning default value",
	      idx, path);
    return defVal;
  }
  if (nn->val.h.typ==GWEN_DB_VALUETYPE_CHAR) {
    /* try to convert the char value into an integer */
    const char *p;
    int res;

    DBG_VERBOUS(0, "Converting char value to int");
    p=GWEN_DB_GetCharValueFromNode(nn);
    assert(p);
    if (sscanf(p, "%d", &res)!=1) {
      DBG_ERROR(0, "Value[%d] of \"%s\" is not an int value",
		idx, path);
      return defVal;
    }
    return res;
  }
  else if (nn->val.h.typ!=GWEN_DB_VALUETYPE_INT) {
    DBG_VERBOUS(0, "Value[%d] of \"%s\" is not an int or char value",
	      idx, path);
    return defVal;
  }
  DBG_VERBOUS(0, "Returning value from node (\"%s\":%d)",
            path, idx);
  return nn->val.i.data;
}



int GWEN_DB_SetIntValue(GWEN_DB_NODE *n,
			GWEN_TYPE_UINT32 flags,
			const char *path,
			int val){
  GWEN_DB_NODE *nn;
  GWEN_DB_NODE *nv;

  /* select/create node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     flags | GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_VERBOUS(0, "Path \"%s\" not available",
	      path);
    return 1;
  }

  /* delete contents of this variable if wanted */
  if (flags & GWEN_DB_FLAGS_OVERWRITE_VARS) {
    DBG_VERBOUS(0, "Clearing variable \"%s\"", path);
    GWEN_DB_ClearNode(nn);
  }

  nv=GWEN_DB_ValueInt_new(val);
  GWEN_DB_Node_Append(nn, nv);
  DBG_VERBOUS(0, "Added int value \%d\" to variable \"%s\"", val, path);
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
    DBG_VERBOUS(0, "Value for \"%s\" not found, returning default value",
	      path);
    *returnValueSize=defValSize;
    return defVal;
  }
  if (nn->val.h.typ!=GWEN_DB_VALUETYPE_BIN) {
    /* bad type */
    DBG_VERBOUS(0, "Bad type for path \"%s\", returning default value",
	      path);
    *returnValueSize=defValSize;
    return defVal;
  }
  *returnValueSize=nn->val.b.dataSize;
  return nn->val.b.data;
}



int GWEN_DB_SetBinValue(GWEN_DB_NODE *n,
			GWEN_TYPE_UINT32 flags,
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
    DBG_VERBOUS(0, "Path \"%s\" not available",
                path);
    return 1;
  }

  /* delete contents of this variable if wanted */
  if (flags & GWEN_DB_FLAGS_OVERWRITE_VARS) {
    DBG_VERBOUS(0, "Clearing variable \"%s\"", path);
    GWEN_DB_ClearNode(nn);
  }

  nv=GWEN_DB_ValueBin_new(val, valSize);
  GWEN_DB_Node_Append(nn, nv);
  DBG_VERBOUS(0, "Added bin value to variable \"%s\"", path);
  return 0;
}



GWEN_DB_NODE *GWEN_DB_GetGroup(GWEN_DB_NODE *n,
			       GWEN_TYPE_UINT32 flags,
			       const char *path) {
  GWEN_DB_NODE *nn;

  /* select/create node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     flags & ~GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_VERBOUS(0, "Path \"%s\" not available",
	      path);
    return 0;
  }

  /* delete contents of this variable if wanted */
  if (flags & GWEN_DB_FLAGS_OVERWRITE_GROUPS) {
    DBG_VERBOUS(0, "Clearing group \"%s\"", path);
    GWEN_DB_ClearNode(nn);
  }

  return nn;
}



const char *GWEN_DB_GroupName(GWEN_DB_NODE *n){
  assert(n);
  if (n->h.typ!=GWEN_DB_NODETYPE_GROUP) {
    DBG_ERROR(0, "Node is not a group");
    return 0;
  }
  return n->group.name;
}



void GWEN_DB_Dump(GWEN_DB_NODE *n, FILE *f, int insert){
  if (n) {
    GWEN_DB_NODE *cn;
    int i;

    for (i=0; i<insert; i++)
      fprintf(f, " ");

    /* dump dynamic (allocated) data */
    switch(n->h.typ) {
    case GWEN_DB_NODETYPE_GROUP:
      fprintf(f, "Group : \"%s\"\n", n->group.name);
      break;

    case GWEN_DB_NODETYPE_VAR:
      fprintf(f, "Var   : \"%s\"\n", n->var.name);
      break;

    case GWEN_DB_NODETYPE_VALUE:
      switch(n->val.h.typ) {
      case GWEN_DB_VALUETYPE_CHAR:
        fprintf(f, "Value : \"%s\" (char)\n", n->val.c.data);
        break;

      case GWEN_DB_VALUETYPE_INT:
        /* no dynamic data, nothing to do */
        fprintf(f, "Value : %d (int)\n", n->val.i.data);
        break;

      case GWEN_DB_VALUETYPE_BIN:
        if (1) {
          char *buffer;

          buffer=(char*)malloc((n->val.b.dataSize*2)+1);
          assert(buffer);
          if (GWEN_Text_ToHex(n->val.b.data, n->val.b.dataSize,
                              buffer, (n->val.b.dataSize*2)+1)==0) {
            fprintf(f, "Value : %d bytes (bin)\n", n->val.b.dataSize);
          }
          else {
            fprintf(f, "Value : %s (bin)\n", buffer);
          }
          free(buffer);
        }
        break;

      default:
        fprintf(f, "Value : [unknown type]\n");
      }
      break;

    default:
      fprintf(f, "[unknown node type %d]\n", n->h.typ);
    }

    /* dump children */
    cn=n->h.child;
    while(cn) {
      GWEN_DB_Dump(cn, f, insert+4);
      cn=cn->h.next;
    }
  }
  else {
    fprintf(f, "[no node]\n");
  }
}



int GWEN_DB_ReadFromStream(GWEN_DB_NODE *n,
                           GWEN_BUFFEREDIO *bio,
                           GWEN_TYPE_UINT32 dbflags) {
  GWEN_ERRORCODE err;
  GWEN_DB_NODE *currGrp;
  GWEN_DB_NODE *currVar;
  char linebuf[GWEN_DB_LINE_MAXSIZE];
  char wbuf[256];
  char *p;
  const char *pos;
  int lineno;
  GWEN_DB_VALUETYPE vt;
  int depth;

  currGrp=n;
  currVar=0;
  lineno=0;
  depth=0;

  while(!GWEN_BufferedIO_CheckEOF(bio)) {
    int isVar;
    int isVal;

    isVar=0;
    isVal=0;
    vt=GWEN_DB_VALUETYPE_CHAR;
    pos=linebuf;

    /* read next line */
    lineno++;
    err=GWEN_BufferedIO_ReadLine(bio, linebuf, sizeof(linebuf)-1);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR(0, "Error in line %d", lineno);
      return -1;
    }

    /* eventually stop if empty line */
    if (!*pos && (dbflags&GWEN_DB_FLAGS_STOP_ON_EMPTY_LINE)) {
      return 0;
    }

    /* skip blanks */
    while(*pos && isspace(*pos))
      pos++;

    if (!*pos || *pos=='#') {
      DBG_VERBOUS(0, "Line %d is empty", lineno);
    }
    else {
      if (*pos=='}') {
        /* end of current group */
        if (depth<1) {
          DBG_ERROR(0, "Extra \"}\" in line %d, pos %d",
                    lineno, pos-linebuf+1);
          return -1;
        }
        assert(currGrp->h.parent);
        currGrp=currGrp->h.parent;
        currVar=0;
        depth--;
        *pos++;
        while(*pos && isspace(*pos))
          pos++;
      }
      else if (*pos=='#') {
        /* comment only line */
        DBG_VERBOUS(0, "Comment-only line");
      }
      else {
        if (*pos==',') {
          /* continuing a variable on the next line */
          pos++;
          isVal=1;
        }

        if (!isVal) {
          /* get first word */
          *wbuf=(char)0;
          p=GWEN_Text_GetWord(pos,
                              (dbflags&GWEN_DB_FLAGS_USE_COLON)?
                              "}{: #,":"}{= #,",
                              wbuf,
                              sizeof(wbuf)-1,
                              GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS |
                              GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS |
                              GWEN_TEXT_FLAGS_DEL_QUOTES |
                              GWEN_TEXT_FLAGS_CHECK_BACKSLASH,
                              &pos);
          if (!p || !*wbuf) {
            DBG_ERROR(0, "Error in line %d, pos %d",
                      lineno, pos-linebuf+1);
            return -1;
          }

          /* skip blanks */
          while(*pos && isspace(*pos))
            pos++;

          /* actually did read a word */
          if (*pos=='{') {
            /* found a group definition */
            GWEN_DB_NODE *tmpn;

            pos++;
            DBG_VERBOUS(0, "Found group \"%s\"", wbuf);
            tmpn=GWEN_DB_GetGroup(currGrp, dbflags, wbuf);
            if (!tmpn) {
              DBG_ERROR(0, "Error in line %d, pos %d (no group)",
                        lineno, pos-linebuf+1);
              return -1;
            }
            currGrp=tmpn;
            currVar=0;
            depth++;
          }
          else if (*pos==((dbflags&GWEN_DB_FLAGS_USE_COLON)?':':'=')) {
            /* found a variable */
            DBG_VERBOUS(0, "Found a variable \"%s\", handle it later", wbuf);
            isVar=1;
          }
          else {
            /* found another word, so the previous word is a type specifier */
            isVar=1;
            DBG_VERBOUS(0, "Found a type specifier \"%s\"", wbuf);
            if (strcasecmp(p, "int")==0)
              vt=GWEN_DB_VALUETYPE_INT;
            else if (strcasecmp(p, "bin")==0)
              vt=GWEN_DB_VALUETYPE_BIN;
            else if (strcasecmp(p, "char")==0)
              vt=GWEN_DB_VALUETYPE_CHAR;
            else {
              DBG_WARN(0, "Unknown type \"%s\", assuming \"char\"", p);
              vt=GWEN_DB_VALUETYPE_CHAR;
            }
            /* get the variable name */
            wbuf[0]=(char)0;
            p=GWEN_Text_GetWord(pos,
                                (dbflags&GWEN_DB_FLAGS_USE_COLON)?
                                "}{: #":"}{= #",
                                wbuf,
                                sizeof(wbuf)-1,
                                GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS |
                                GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS |
                                GWEN_TEXT_FLAGS_DEL_QUOTES |
                                GWEN_TEXT_FLAGS_CHECK_BACKSLASH,
                                &pos);
            if (!p || !*wbuf) {
              DBG_ERROR(0, "Error in line %d, pos %d",
                        lineno, pos-linebuf+1);
              return -1;
            }
            /* skip blanks */
            while(*pos && isspace(*pos))
              pos++;
            if (*pos!=((dbflags&GWEN_DB_FLAGS_USE_COLON)?':':'=')) {
              DBG_ERROR(0, "Expected \"=\" in line %d at %d",
                        lineno, pos-linebuf+1);
              return -1;
            }
            isVar=1;
          } /* if two words */
        } /* if !isVal */

        if (isVar) {
          DBG_VERBOUS(0, "Creating variable \"%s\"", wbuf);
          pos++;
          currVar=GWEN_DB_GetNode(currGrp,
                                  wbuf,
                                  dbflags | GWEN_PATH_FLAGS_VARIABLE);
          /* next word is a value */
          isVal=1;
        } /* isVar */

        if (isVal) {
          GWEN_DB_NODE *tmpn;
          int value;

          if (!currVar) {
            DBG_ERROR(0, "Error in line %d, pos %d (no var)",
                      lineno, pos-linebuf+1);
            return -1;
          }

          while (*pos) {
            DBG_VERBOUS(0, "Reading value");
            /* get next value */
            p=GWEN_Text_GetWord(pos,
                                (dbflags&GWEN_DB_FLAGS_USE_COLON)?
                                "}{:#,":"}{=#,",
                                wbuf,
                                sizeof(wbuf)-1,
                                GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS |
                                GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS |
                                GWEN_TEXT_FLAGS_DEL_QUOTES |
                                GWEN_TEXT_FLAGS_CHECK_BACKSLASH,
                                &pos);
            /* if (!p || !*wbuf) { */
            if (!p) {
              DBG_DEBUG(0, "Line %d, pos %d: no word",
                        lineno, pos-linebuf+1);
              break;
            }
            if (!*wbuf) {
              DBG_INFO(0, "Line %d, pos %d: no word",
                       lineno, pos-linebuf+1);
            }
            DBG_VERBOUS(0, "Creating value \"%s\"", wbuf);

            /* set value */
            switch(vt) {
            case GWEN_DB_VALUETYPE_CHAR:
              tmpn=GWEN_DB_ValueChar_new(wbuf);
              break;
            case GWEN_DB_VALUETYPE_INT:
              if (sscanf(wbuf, "%d", &value)!=1) {
                DBG_ERROR(0, "Error in line %d, pos %d (no integer)",
                          lineno, pos-linebuf+1);
                return -1;
              }
              tmpn=GWEN_DB_ValueInt_new(value);
              break;
            case GWEN_DB_VALUETYPE_BIN: {
              char binbuffer[128];
              int binsize;

              binsize=GWEN_Text_FromHex(wbuf, binbuffer, sizeof(binbuffer));
              if (binsize<0) {
                DBG_ERROR(0, "Error in line %d, pos %d (no binary)",
                          lineno, pos-linebuf+1);
                return -1;
              }
              if (binsize)
                tmpn=GWEN_DB_ValueBin_new(binbuffer, binsize);
              else {
                tmpn=0;
                DBG_WARN(0, "Line %d, pos %d: empty binary value",
                         lineno, pos-linebuf+1);
              }
              break;
            }
            default:
              DBG_ERROR(0, "Error in line %d, pos %d (bad type)",
                        lineno, pos-linebuf+1);
              return -1;
            } /* switch */
            if (tmpn)
              GWEN_DB_Node_Append(currVar, tmpn);

            /* skip blanks */
            while(*pos && isspace(*pos))
              pos++;

            if (*pos!=',')
              break;
            pos++;
          } /* while *pos */
        } /* if isVal */

        while(*pos && isspace(*pos))
          pos++;
        if (*pos) {
          if (*pos=='}') {
            if (depth<1) {
              DBG_ERROR(0, "Extra \"}\" in line %d, pos %d",
                        lineno, pos-linebuf+1);
            }
            assert(currGrp->h.parent);
            currGrp=currGrp->h.parent;
            currVar=0;
            depth--;
            *pos++;
            while(*pos && isspace(*pos))
              pos++;
          }
          if (*pos && *pos!='#') {
            DBG_ERROR(0, "Extra character in line %d, pos %d",
                      lineno, pos-linebuf+1);
            return -1;
          }
        }
      }
    } /* if line is not empty */
  } /* while */

  if (depth) {
    DBG_ERROR(0, "Unbalanced groups (missing %d \"}\" at end of file)",
              depth);
    return -1;
  }
  return 0;
}



int GWEN_DB_ReadFile(GWEN_DB_NODE *n,
                     const char *fname,
                     GWEN_TYPE_UINT32 dbflags) {
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;
  int fd;
  int rv;

  fd=open(fname, O_RDONLY);
  if (fd==-1) {
    DBG_ERROR(0, "Error opening file \"%s\": %s",
              fname,
              strerror(errno));
    return -1;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);
  rv=GWEN_DB_ReadFromStream(n, bio, dbflags);
  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    GWEN_BufferedIO_free(bio);
    return -1;
  }
  GWEN_BufferedIO_free(bio);
  return rv;
}



int GWEN_DB_AddGroup(GWEN_DB_NODE *n, GWEN_DB_NODE *nn){
  assert(n);
  assert(nn);

  if (n->h.typ!=GWEN_DB_NODETYPE_GROUP) {
    DBG_ERROR(0, "Target node is not a group");
    return 0;
  }

  if (nn->h.typ!=GWEN_DB_NODETYPE_GROUP) {
    DBG_ERROR(0, "Sorce node is not a group");
    return 0;
  }

  GWEN_DB_Node_Append(n, nn);
  return 0;
}



int GWEN_DB_AddGroupChildren(GWEN_DB_NODE *n, GWEN_DB_NODE *nn){
  GWEN_DB_NODE *cpn;

  assert(n);
  assert(nn);

  if (n->h.typ!=GWEN_DB_NODETYPE_GROUP) {
    DBG_ERROR(0, "Target node is not a group");
    return -1;
  }

  if (nn->h.typ!=GWEN_DB_NODETYPE_GROUP) {
    DBG_ERROR(0, "Source node is not a group");
    GWEN_DB_Dump(nn, stderr, 1);
    return -1;
  }

  nn=nn->h.child;
  while (nn) {
    DBG_VERBOUS(0, "Duplicating node");
    cpn=GWEN_DB_Node_dup(nn);
    GWEN_DB_Node_Append(n, cpn);
    nn=nn->h.next;
  } /* while */
  return 0;
}



void GWEN_DB_UnlinkGroup(GWEN_DB_NODE *n){
  assert(n);
  if (n->h.typ!=GWEN_DB_NODETYPE_GROUP) {
    DBG_ERROR(0, "Node is not a group");
    return;
  }
  GWEN_DB_Node_Unlink(n);
}



int GWEN_DB_WriteGroupToStream(GWEN_DB_NODE *node,
			       GWEN_BUFFEREDIO *bio,
			       GWEN_TYPE_UINT32 dbflags,
			       int insert) {
  GWEN_DB_NODE *n;
  GWEN_DB_NODE *cn;
  int i;
  GWEN_ERRORCODE err;
  int lastWasVar;

  lastWasVar=0;

  n=node->h.child;
  while(n) {
    DBG_VERBOUS(0, "Writing node");
    switch(n->h.typ) {
    case GWEN_DB_NODETYPE_GROUP:
      if (dbflags & GWEN_DB_FLAGS_WRITE_SUBGROUPS) {
        if (dbflags & GWEN_DB_FLAGS_ADD_GROUP_NEWLINES) {
          if (lastWasVar) {
            /* only insert newline if the last one before this group was a
             * variable */
            err=GWEN_BufferedIO_WriteLine(bio, "");
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "called from here");
              return 1;
            }
          }
        }

        /* indend */
        if (dbflags & GWEN_DB_FLAGS_INDEND) {
          for (i=0; i<insert; i++) {
            err=GWEN_BufferedIO_WriteChar(bio, ' ');
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "called from here");
              return 1;
            }
          } /* for */
        } /* if indend */
        err=GWEN_BufferedIO_Write(bio, n->group.name);
	if (!GWEN_Error_IsOk(err)) {
	  DBG_INFO(0, "called from here");
	  return 1;
	}
	err=GWEN_BufferedIO_WriteLine(bio, " {");
	if (!GWEN_Error_IsOk(err)) {
	  DBG_INFO(0, "called from here");
	  return 1;
	}
	if (GWEN_DB_WriteGroupToStream(n, bio, dbflags, insert+2))
          return 1;

        /* indend */
        if (dbflags & GWEN_DB_FLAGS_INDEND) {
          for (i=0; i<insert; i++) {
            err=GWEN_BufferedIO_WriteChar(bio, ' ');
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "called from here");
              return 1;
            }
          } /* for */
        } /* if indend */

        if (dbflags & GWEN_DB_FLAGS_DETAILED_GROUPS) {
          err=GWEN_BufferedIO_Write(bio, "} # ");
          if (!GWEN_Error_IsOk(err)) {
            DBG_INFO(0, "called from here");
            return 1;
          }
          err=GWEN_BufferedIO_WriteLine(bio, n->group.name);
          if (!GWEN_Error_IsOk(err)) {
            DBG_INFO(0, "called from here");
            return 1;
          }
        } /* if detailed groups */
        else {
          err=GWEN_BufferedIO_WriteLine(bio, "}");
          if (!GWEN_Error_IsOk(err)) {
            DBG_INFO(0, "called from here");
            return 1;
          }
        }
        if (dbflags & GWEN_DB_FLAGS_ADD_GROUP_NEWLINES) {
          if (n->h.next) {
            /* only insert newline if a group is following on the same level
             */
            err=GWEN_BufferedIO_WriteLine(bio, "");
            if (!GWEN_Error_IsOk(err)) {
              DBG_INFO(0, "called from here");
              return 1;
            }
          }
        }
      }
      lastWasVar=0;
      break;

    case GWEN_DB_NODETYPE_VAR:
      cn=n->h.child;
      if (cn) {
	char *typname;
	int namewritten;
	int values;

	typname=0;
	namewritten=0;
	values=0;
	while(cn) {
          char numbuffer[32];
          char *binbuffer;
          unsigned int bbsize;
	  const char *pvalue;

          pvalue=0;
          binbuffer=0;
	  switch(cn->h.typ) {
	  case GWEN_DB_NODETYPE_VALUE:
	    switch(cn->val.h.typ) {
	    case GWEN_DB_VALUETYPE_CHAR:
	      typname="char ";
	      pvalue=cn->val.c.data;
	      break;

	    case GWEN_DB_VALUETYPE_INT:
	      typname="int  ";
	      if (GWEN_Text_NumToString(cn->val.i.data,
					numbuffer,
					sizeof(numbuffer)-1,
					0)<1) {
		DBG_ERROR(0, "Error writing numeric value");
		return 1;
	      }
	      pvalue=numbuffer;
	      break;

            case GWEN_DB_VALUETYPE_BIN:
              bbsize=cn->val.b.dataSize*2+1;
              binbuffer=(char*)malloc(bbsize);
              assert(binbuffer);
              typname="bin  ";
              if (!GWEN_Text_ToHex(cn->val.b.data,
                                   cn->val.b.dataSize,
                                   binbuffer,
                                   bbsize)) {
                DBG_ERROR(0, "Error writing binary value");
                return 1;
              }
              pvalue=binbuffer;
              break;

            default:
              DBG_WARN(0, "Unknown value type (%d)\n", n->val.h.typ);
              break;
	    } /* switch value type */

	    if (!namewritten) {
	      /* write name */
              /* indend */
              if (dbflags & GWEN_DB_FLAGS_INDEND) {
                for (i=0; i<insert; i++) {
                  err=GWEN_BufferedIO_WriteChar(bio, ' ');
                  if (!GWEN_Error_IsOk(err)) {
                    DBG_INFO(0, "called from here");
                    free(binbuffer);
                    return 1;
                  }
                } /* for */
              } /* if indend */
              if (!(dbflags & GWEN_DB_FLAGS_OMIT_TYPES)) {
                err=GWEN_BufferedIO_Write(bio, typname);
                if (!GWEN_Error_IsOk(err)) {
                  DBG_INFO(0, "called from here");
                  free(binbuffer);
                  return 1;
                }
              }
	      if (dbflags & GWEN_DB_FLAGS_QUOTE_VARNAMES) {
		err=GWEN_BufferedIO_Write(bio, "\"");
		if (!GWEN_Error_IsOk(err)) {
		  DBG_INFO(0, "called from here");
                  free(binbuffer);
		  return 1;
		}
	      }
	      err=GWEN_BufferedIO_Write(bio, n->var.name);
	      if (!GWEN_Error_IsOk(err)) {
		DBG_INFO(0, "called from here");
                free(binbuffer);
		return 1;
	      }
	      if (dbflags & GWEN_DB_FLAGS_QUOTE_VARNAMES) {
		err=GWEN_BufferedIO_Write(bio, "\"");
		if (!GWEN_Error_IsOk(err)) {
		  DBG_INFO(0, "called from here");
                  free(binbuffer);
		  return 1;
		}
              }
              err=GWEN_BufferedIO_Write(bio,
                                        ((dbflags&GWEN_DB_FLAGS_USE_COLON)?
                                         ":":"="));
	      if (!GWEN_Error_IsOk(err)) {
		DBG_INFO(0, "called from here");
                free(binbuffer);
                return 1;
	      }
	      namewritten=1;
	    } /* if !namewritten */

	    if (values) {
	      err=GWEN_BufferedIO_Write(bio, ", ");
	      if (!GWEN_Error_IsOk(err)) {
		DBG_INFO(0, "called from here");
                free(binbuffer);
		return 1;
	      }
	    }
	    values++;
	    if (dbflags & GWEN_DB_FLAGS_QUOTE_VALUES) {
	      err=GWEN_BufferedIO_Write(bio, "\"");
	      if (!GWEN_Error_IsOk(err)) {
		DBG_INFO(0, "called from here");
                free(binbuffer);
		return 1;
	      }
	    }
	    err=GWEN_BufferedIO_Write(bio, pvalue);
	    if (!GWEN_Error_IsOk(err)) {
	      DBG_INFO(0, "called from here");
              free(binbuffer);
	      return 1;
	    }
	    if (dbflags & GWEN_DB_FLAGS_QUOTE_VALUES) {
	      err=GWEN_BufferedIO_Write(bio, "\"");
	      if (!GWEN_Error_IsOk(err)) {
		DBG_INFO(0, "called from here");
                free(binbuffer);
		return 1;
	      }
	    }

	    break;
	  default:
	    break;
	  } /* switch */

          free(binbuffer);
	  cn=cn->h.next;
	} /* while cn */
	err=GWEN_BufferedIO_WriteLine(bio, "");
	if (!GWEN_Error_IsOk(err)) {
	  DBG_INFO(0, "called from here");
	  return 1;
	}
      } /* if children */
      lastWasVar=1;
      break;

    default:
      DBG_WARN(0, "[unhandled node type %d]\n", n->h.typ);
    } /* switch */

    n=n->h.next;
  } /* while */
  return 0;
}



int GWEN_DB_WriteToStream(GWEN_DB_NODE *node,
			  GWEN_BUFFEREDIO *bio,
			  GWEN_TYPE_UINT32 dbflags) {
  return GWEN_DB_WriteGroupToStream(node, bio, dbflags, 0);
}



int GWEN_DB_WriteFile(GWEN_DB_NODE *n,
                      const char *fname,
		      GWEN_TYPE_UINT32 dbflags){
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;
  int fd;
  int rv;

  if (dbflags & GWEN_DB_FLAGS_APPEND_FILE)
    fd=open(fname, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  else
    fd=open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd==-1) {
    DBG_ERROR(0, "Error opening file \"%s\": %s",
              fname,
              strerror(errno));
    return -1;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetWriteBuffer(bio, 0, 1024);
  rv=GWEN_DB_WriteToStream(n, bio, dbflags);
  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    GWEN_BufferedIO_free(bio);
    return -1;
  }
  GWEN_BufferedIO_free(bio);
  return rv;
}



int GWEN_DB_VariableExists(GWEN_DB_NODE *n,
                           const char *path){
  return (GWEN_DB_FindVar(n, path)!=0);
}



int GWEN_DB_ValueExists(GWEN_DB_NODE *n,
                        const char *path,
                        unsigned int i){
  return (GWEN_DB_GetValue(n, path, i)!=0);
}



GWEN_DB_VALUETYPE GWEN_DB_GetVariableType(GWEN_DB_NODE *n,
                                          const char *p){
  GWEN_DB_NODE *nn;

  nn=GWEN_DB_FindVar(n, p);
  if (!nn)
    return GWEN_DB_VALUETYPE_UNKNOWN;
  nn=GWEN_DB_GetFirstValue(nn);
  if (!nn)
    return GWEN_DB_VALUETYPE_UNKNOWN;
  return GWEN_DB_GetValueType(nn);
}



GWEN_DB_VALUETYPE GWEN_DB_GetValueTypeByPath(GWEN_DB_NODE *n,
                                             const char *path,
                                             unsigned int i){
  GWEN_DB_NODE *nn;

  nn=GWEN_DB_GetValue(n, path, i);
  if (!nn)
    return GWEN_DB_VALUETYPE_UNKNOWN;
  return GWEN_DB_GetValueType(nn);
}


void GWEN_DB_GroupRename(GWEN_DB_NODE *n, const char *newname){
  assert(n);
  assert(newname);
  assert(n->h.typ==GWEN_DB_NODETYPE_GROUP);
  free(n->group.name);
  n->group.name=strdup(newname);
}







int GWEN_DB_IsGroup(const GWEN_DB_NODE *n){
  assert(n);
  return n->h.typ==GWEN_DB_NODETYPE_GROUP;
}



int GWEN_DB_IsVariable(const GWEN_DB_NODE *n){
  assert(n);
  return n->h.typ==GWEN_DB_NODETYPE_VAR;
}



int GWEN_DB_IsValue(const GWEN_DB_NODE *n){
  assert(n);
  return n->h.typ==GWEN_DB_NODETYPE_VALUE;
}



void *GWEN_DB_Groups_Foreach(GWEN_DB_NODE *node, GWEN_DB_NODES_CB func,
                             void *user_data){
  GWEN_DB_NODE *iter;
  void *res;

  assert(node);
  assert(func);

  iter = GWEN_DB_GetFirstGroup(node);
  res = NULL;
  while(iter){
    res = (*func)(iter, user_data);
    if (res) {
      break;
    }
    iter = GWEN_DB_GetNextGroup(iter);
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

  iter = GWEN_DB_GetFirstVar(node);
  res = NULL;
  while(iter){
    res = (*func)(iter, user_data);
    if (res) {
      break;
    }
    iter = GWEN_DB_GetNextVar(iter);
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

  iter = GWEN_DB_GetFirstValue(node);
  res = NULL;
  while(iter){
    res = (*func)(iter, user_data);
    if (res) {
      break;
    }
    iter = GWEN_DB_GetNextValue(iter);
  }
  return res;
}



unsigned int GWEN_DB_Values_Count(const GWEN_DB_NODE *node){
  unsigned int res = 0;
  GWEN_DB_Values_Foreach((GWEN_DB_NODE *)node, GWEN_DB_count_cb, &res);
  return res;
}



GWEN_TYPE_UINT32 GWEN_DB_GetNodeFlags(const GWEN_DB_NODE *n){
  assert(n);
  return n->h.nodeFlags;
}



void GWEN_DB_SetNodeFlags(GWEN_DB_NODE *n,
                          GWEN_TYPE_UINT32 flags){
  assert(n);
  n->h.nodeFlags=flags;
}



void GWEN_DB_ModifyBranchFlagsUp(GWEN_DB_NODE *n,
				 GWEN_TYPE_UINT32 newflags,
				 GWEN_TYPE_UINT32 mask){
  GWEN_TYPE_UINT32 flags;

  assert(n);

  while(n) {
    flags=n->h.nodeFlags;
    flags=((flags^newflags)&(mask))^flags;
    n->h.nodeFlags=flags;
    n=n->h.parent;
  } /* while */
}



void GWEN_DB_ModifyBranchFlagsDown(GWEN_DB_NODE *n,
				   GWEN_TYPE_UINT32 newflags,
				   GWEN_TYPE_UINT32 mask){
  GWEN_TYPE_UINT32 flags;
  GWEN_DB_NODE *cn;

  assert(n);

  flags=n->h.nodeFlags;
  flags=((flags^newflags)&(mask))^flags;
  n->h.nodeFlags=flags;

  cn=n->h.child;
  while(cn) {
    GWEN_DB_ModifyBranchFlagsDown(cn, newflags, mask);
    cn=cn->h.next;
  } /* while cn */
}





int GWEN_DB_ReadFileAs(GWEN_DB_NODE *db,
                       const char *fname,
                       const char *type,
                       GWEN_DB_NODE *params,
                       GWEN_TYPE_UINT32 dbflags){
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;
  GWEN_DBIO *dbio;
  int fd;
  int rv;

  dbio=GWEN_DBIO_GetPlugin(dbio);
  if (!dbio) {
    DBG_ERROR(0, "Plugin \"%s\" is not supported", type);
    return -1;
  }
  fd=open(fname, O_RDONLY);
  if (fd==-1) {
    DBG_ERROR(0, "Error opening file \"%s\": %s",
              fname,
              strerror(errno));
    return -1;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);
  rv=GWEN_DBIO_Import(dbio, bio, dbflags, db, params);
  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    GWEN_BufferedIO_free(bio);
    return -1;
  }
  GWEN_BufferedIO_free(bio);
  return rv;

}



int GWEN_DB_WriteFileAs(GWEN_DB_NODE *db,
                        const char *fname,
                        const char *type,
                        GWEN_DB_NODE *params,
                        GWEN_TYPE_UINT32 dbflags){
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;
  GWEN_DBIO *dbio;
  int fd;
  int rv;

  dbio=GWEN_DBIO_GetPlugin(dbio);
  if (!dbio) {
    DBG_ERROR(0, "Plugin \"%s\" is not supported", type);
    return -1;
  }

  if (dbflags & GWEN_DB_FLAGS_APPEND_FILE)
    fd=open(fname, O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  else
    fd=open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd==-1) {
    DBG_ERROR(0, "Error opening file \"%s\": %s",
              fname,
              strerror(errno));
    return -1;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetWriteBuffer(bio, 0, 1024);
  rv=GWEN_DBIO_Export(dbio, bio, dbflags, db, params);
  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    GWEN_BufferedIO_free(bio);
    return -1;
  }
  GWEN_BufferedIO_free(bio);
  return rv;

}




