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

#include "db_p.h"
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/path.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_DB_NODE *GWEN_DB_ValueBin_new(void *data,
                                   unsigned int datasize){
  GWEN_DB_VALUE_BIN *v;

  GWEN_NEW_OBJECT(GWEN_DB_VALUE_BIN, v);
  v->h.h.typ=GWEN_DB_NODETYPE_VALUE;
  v->h.typ=GWEN_DB_VALUETYPE_BIN;
  if (datasize) {
    assert(data);
    v->dataSize=datasize;
    v->data=malloc(datasize);
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



void GWEN_DB_Node_Append(GWEN_DB_NODE *parent,
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



void GWEN_DB_Node_Insert(GWEN_DB_NODE *parent,
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



void GWEN_DB_Node_Unlink(GWEN_DB_NODE *n) {
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



void GWEN_DB_Node_free(GWEN_DB_NODE *n){
  if (n) {
    GWEN_DB_NODE *cn;

    /* free children */
    cn=n->h.child;
    while(cn) {
      GWEN_DB_NODE *ncn;

      ncn=cn->h.next;
      DBG_DEBUG(0, "Freeing child node");
      GWEN_DB_Node_free(cn);
      cn=ncn;
    }

    /* free dynamic (allocated) data */
    switch(n->h.typ) {
    case GWEN_DB_NODETYPE_GROUP:
      DBG_DEBUG(0, "Freeing dynamic data of group \"%s\"", n->group.name);
      free(n->group.name);
      break;

    case GWEN_DB_NODETYPE_VAR:
      DBG_DEBUG(0, "Freeing dynamic data of var \"%s\"", n->var.name);
      free(n->var.name);
      break;

    case GWEN_DB_NODETYPE_VALUE:
      switch(n->val.h.typ) {
      case GWEN_DB_VALUETYPE_CHAR:
        DBG_DEBUG(0, "Freeing dynamic data of char value");
        free(n->val.c.data);
        break;

      case GWEN_DB_VALUETYPE_INT:
        /* no dynamic data, nothing to do */
        DBG_DEBUG(0, "Freeing dynamic data of int value");
        break;

      case GWEN_DB_VALUETYPE_BIN:
        DBG_DEBUG(0, "Freeing dynamic data of bin value");
        free(n->val.b.data);
        break;

      default:
        DBG_WARN(0, "Unknown value type (%d)", n->val.h.typ);
      }

    default:
      DBG_WARN(0, "Unknown node type (%d)", n->h.typ);
    }
    DBG_DEBUG(0, "Freeing node itself");
    free(n);
  }

}


GWEN_DB_NODE *GWEN_DB_Node_dup(GWEN_DB_NODE *n){
  GWEN_DB_NODE *nn;

  switch(n->h.typ) {
  case GWEN_DB_NODETYPE_GROUP:
    DBG_DEBUG(0, "Duplicating group \"%s\"", n->group.name);
    nn=GWEN_DB_Group_new(n->group.name);
    break;

  case GWEN_DB_NODETYPE_VAR:
    DBG_DEBUG(0, "Duplicating variable \"%s\"", n->var.name);
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

  default:
    DBG_WARN(0, "Unknown node type (%d)", n->h.typ);
    nn=0;
  }

  /* duplicate all children and add them to the new node */
  if (nn) {
    GWEN_DB_NODE *cn;

    cn=n->h.child;
    while(cn) {
      GWEN_DB_NODE *ncn;

      /* duplicate child and add it */
      ncn=GWEN_DB_Node_dup(cn);
      GWEN_DB_Node_Append(nn, ncn);
      cn=cn->h.next;
    } /* while cn */
  }

  return nn;
}



void GWEN_DB_Group_free(GWEN_DB_NODE *n){
  GWEN_DB_Node_free(n);
}



GWEN_DB_NODE *GWEN_DB_Group_dup(GWEN_DB_NODE *n){
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
  assert(n);
  if (n->h.typ!=GWEN_DB_NODETYPE_GROUP) {
    DBG_ERROR(0, "Node is not a group");
    return 0;
  }
  while(n) {
    if (n->h.typ==GWEN_DB_NODETYPE_GROUP)
      break;
    n=n->h.next;
  } /* while node */
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



void *GWEN_DB_GetBinValueFromNode(GWEN_DB_NODE *n,
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
                         unsigned int flags) {
  GWEN_DB_NODE *n;
  GWEN_DB_NODE *nn;

  n=(GWEN_DB_NODE*)data;

  /* check whether we are allowed to simply create the node */
  if (
      ((flags & GWEN_PATH_FLAGS_LAST) &
       (flags & GWEN_PATH_FLAGS_NAMECREATE)) ||
      (!(flags & GWEN_PATH_FLAGS_LAST) &
       (flags & GWEN_PATH_FLAGS_PATHCREATE))
     ) {
    /* simply create the new variable/group */
    if (flags & GWEN_PATH_FLAGS_VARIABLE) {
      DBG_VERBOUS(0, "Creating variable \"%s\"", entry);
      nn=GWEN_DB_Var_new(entry);
      GWEN_DB_Node_Append(n, nn);
      return nn;
    }
    else {
      DBG_VERBOUS(0, "Creating group \"%s\"", entry);
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
        (!(flags & GWEN_PATH_FLAGS_LAST) &
         (flags & GWEN_PATH_FLAGS_PATHMUSTEXIST)) ||
        (flags & GWEN_PATH_FLAGS_NAMEMUSTEXIST)
       ) {
      DBG_VERBOUS(0, "Entry \"%s\" does not exist", entry);
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
      DBG_DEBUG(0, "Entry \"%s\" already exists", entry);
      return 0;
    }
  }

  return nn;
}



GWEN_DB_NODE *GWEN_DB_GetNode(GWEN_DB_NODE *n,
                              const char *path,
                              unsigned int flags){
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
    DBG_DEBUG(0, "Path \"%s\" not found",
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
  DBG_DEBUG(0, "No value[%d] for path \"%s\"",
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
    DBG_DEBUG(0, "Path \"%s\" not found",
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
    DBG_DEBUG(0, "Path \"%s\" not found",
	      path);
    return 1;
  }
  GWEN_DB_Node_Unlink(nn);
  GWEN_DB_Node_free(nn);
  return 0;
}



int GWEN_DB_ClearGroup(GWEN_DB_NODE *n,
		       const char *path){
  GWEN_DB_NODE *nn;

  /* find corresponding node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     GWEN_PATH_FLAGS_PATHMUSTEXIST |
		     GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  if (!nn) {
    DBG_DEBUG(0, "Path \"%s\" not found",
	      path);
    return 1;
  }
  GWEN_DB_ClearNode(nn);
  return 0;
}



const char *GWEN_DB_GetCharValue(GWEN_DB_NODE *n,
                                 const char *path,
                                 int idx,
				 const char *defVal){
  GWEN_DB_NODE *nn;

  nn=GWEN_DB_GetValue(n, path, idx);
  if (!nn){
    DBG_DEBUG(0, "Value for \"%s\" not found, returning default value",
	      path);
    return defVal;
  }
  if (nn->val.h.typ!=GWEN_DB_VALUETYPE_CHAR) {
    /* bad type */
    DBG_DEBUG(0, "Bad type for path \"%s\", returning default value",
	      path);
    return defVal;
  }
  return nn->val.c.data;
}



int GWEN_DB_SetCharValue(GWEN_DB_NODE *n,
			 unsigned int flags,
			 const char *path,
			 const char *val){
  GWEN_DB_NODE *nn;
  GWEN_DB_NODE *nv;

  /* select/create node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     flags | GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_DEBUG(0, "Path \"%s\" not available",
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
  DBG_VERBOUS(0, "Added char value \%s\" to variable \"%s\"", val, path);

  return 0;
}



int GWEN_DB_GetIntValue(GWEN_DB_NODE *n,
			const char *path,
			int idx,
			int defVal){
  GWEN_DB_NODE *nn;

  nn=GWEN_DB_GetValue(n, path, idx);
  if (!nn){
    DBG_DEBUG(0, "Value[%d] for \"%s\" not found, returning default value",
	      idx, path);
    return defVal;
  }
  if (nn->val.h.typ==GWEN_DB_VALUETYPE_CHAR) {
    /* try to convert the char value into an integer */
    const char *p;
    int res;

    DBG_DEBUG(0, "Converting char value to int");
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
    DBG_DEBUG(0, "Value[%d] of \"%s\" is not an int or char value",
	      idx, path);
    return defVal;
  }
  DBG_DEBUG(0, "Returning value from node");
  return nn->val.i.data;
}



int GWEN_DB_SetIntValue(GWEN_DB_NODE *n,
			unsigned int flags,
			const char *path,
			int val){
  GWEN_DB_NODE *nn;
  GWEN_DB_NODE *nv;

  /* select/create node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     flags | GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_DEBUG(0, "Path \"%s\" not available",
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



void *GWEN_DB_GetBinValue(GWEN_DB_NODE *n,
			  const char *path,
			  int idx,
			  void *defVal,
			  unsigned int defValSize,
			  unsigned int *returnValueSize){
  GWEN_DB_NODE *nn;

  assert(returnValueSize);
  nn=GWEN_DB_GetValue(n, path, idx);
  if (!nn){
    DBG_DEBUG(0, "Value for \"%s\" not found, returning default value",
	      path);
    *returnValueSize=defValSize;
    return defVal;
  }
  if (nn->val.h.typ!=GWEN_DB_VALUETYPE_BIN) {
    /* bad type */
    DBG_DEBUG(0, "Bad type for path \"%s\", returning default value",
	      path);
    *returnValueSize=defValSize;
    return defVal;
  }
  *returnValueSize=nn->val.b.dataSize;
  return nn->val.b.data;
}



int GWEN_DB_SetBinValue(GWEN_DB_NODE *n,
			unsigned int flags,
			const char *path,
			void *val,
			unsigned int valSize){
  GWEN_DB_NODE *nn;
  GWEN_DB_NODE *nv;

  assert(val);
  /* select/create node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     flags | GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_DEBUG(0, "Path \"%s\" not available",
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
			       unsigned int flags,
			       const char *path) {
  GWEN_DB_NODE *nn;

  /* select/create node */
  nn=GWEN_DB_GetNode(n,
		     path,
		     flags & ~GWEN_PATH_FLAGS_VARIABLE);
  if (!nn) {
    DBG_DEBUG(0, "Path \"%s\" not available",
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







