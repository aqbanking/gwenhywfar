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

//#define DISABLE_DEBUGLOG

#include "db_p.h"
#include <gwenhyfwar/misc.h>
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/path.h>
#include <gwenhyfwar/bufferedio.h>
#include <gwenhyfwar/text.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


GWEN_DB_NODE *GWEN_DB_ValueBin_new(const void *data,
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
      break;

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
        fprintf(f, "Value : %d bytes (bin)\n", n->val.b.dataSize);
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
                           unsigned int dbflags) {
  GWEN_ERRORCODE err;
  GWEN_DB_NODE *currGrp;
  GWEN_DB_NODE *currVar;
  char linebuf[GWEN_DB_LINE_MAXSIZE];
  char wbuf[256];
  char *p;
  const char *pos;
  unsigned int lineno;
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


    /* skip blanks */
    while(*pos && isspace(*pos))
      pos++;

    if (!*pos || *pos=='#') {
      DBG_DEBUG(0, "Line %d is empty", lineno);
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
        DBG_DEBUG(0, "Comment-only line");
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
                              "}{= #,",
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
            DBG_DEBUG(0, "Found group \"%s\"", wbuf);
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
          else if (*pos=='=') {
            /* found a variable */
            DBG_DEBUG(0, "Found a variable \"%s\", handle it later", wbuf);
            isVar=1;
          }
          else {
            /* found another word, so the previous word is a type specifier */
            isVar=1;
            DBG_DEBUG(0, "Found a type specifier \"%s\"", wbuf);
            if (strcasecmp(p, "int")==0)
              vt=GWEN_DB_VALUETYPE_INT;

            /* get the variable name */
            wbuf[0]=(char)0;
            p=GWEN_Text_GetWord(pos,
                                "}{= #",
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
            if (*pos!='=') {
              DBG_ERROR(0, "Expected \"=\" in line %d at %d",
                        lineno, pos-linebuf+1);
              return -1;
            }
            isVar=1;
          } /* if two words */
        } /* if !isVal */

        if (isVar) {
          DBG_DEBUG(0, "Creating variable \"%s\"", wbuf);
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
            DBG_DEBUG(0, "Reading value");
            /* get next value */
            p=GWEN_Text_GetWord(pos,
                                "}{= #,",
                                wbuf,
                                sizeof(wbuf)-1,
                                GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS |
                                GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS |
                                GWEN_TEXT_FLAGS_DEL_QUOTES |
                                GWEN_TEXT_FLAGS_CHECK_BACKSLASH,
                                &pos);
            if (!p || !*wbuf) {
              DBG_ERROR(0, "Error in line %d, pos %d (no word)",
                        lineno, pos-linebuf+1);
              return -1;
            }

            DBG_DEBUG(0, "Creating value \"%s\"", wbuf);

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
            default:
              DBG_ERROR(0, "Error in line %d, pos %d (bad type)",
                        lineno, pos-linebuf+1);
              return -1;
            } /* switch */
            GWEN_DB_Node_Append(currVar, tmpn);

            /* skip blanks */
            while(*pos && isspace(*pos))
              pos++;

            if (*pos!=',')
              break;
            pos++;
          }
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
                     unsigned int dbflags) {
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
    DBG_ERROR(0, "Sorce node is not a group");
    return -1;
  }

  nn=nn->h.child;
  while (nn) {
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
			       unsigned int dbflags,
			       int insert) {
  GWEN_DB_NODE *n;
  GWEN_DB_NODE *cn;
  int i;
  GWEN_ERRORCODE err;

  n=node->h.child;
  while(n) {
    DBG_DEBUG(0, "Writing node");
    switch(n->h.typ) {
    case GWEN_DB_NODETYPE_GROUP:
      if (dbflags & GWEN_DB_FLAGS_WRITE_SUBGROUPS) {
	err=GWEN_BufferedIO_WriteLine(bio, "");
	if (!GWEN_Error_IsOk(err)) {
	  DBG_INFO(0, "called from here");
	  return 1;
	}
	for (i=0; i<insert; i++) {
	  err=GWEN_BufferedIO_WriteChar(bio, ' ');
	  if (!GWEN_Error_IsOk(err)) {
	    DBG_INFO(0, "called from here");
	    return 1;
	  }
	}
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
	for (i=0; i<insert; i++) {
	  err=GWEN_BufferedIO_WriteChar(bio, ' ');
	  if (!GWEN_Error_IsOk(err)) {
	    DBG_INFO(0, "called from here");
	    return 1;
	  }
	}
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
      }
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
	  const char *pvalue;

	  pvalue=0;
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
	      typname="bin  ";
	      /* TODO: convert bin2hex */
	      DBG_WARN(0 , "Bin value (%d bytes) not supported\n",
		       cn->val.b.dataSize);
	      break;

	    default:
	      DBG_WARN(0, "Unknown value type (%d)\n", n->val.h.typ);
              break;
	    } /* switch value type */

	    if (!namewritten) {
	      /* write name */
	      for (i=0; i<insert; i++) {
		err=GWEN_BufferedIO_WriteChar(bio, ' ');
		if (!GWEN_Error_IsOk(err)) {
		  DBG_INFO(0, "called from here");
		  return 1;
		}
	      }
	      err=GWEN_BufferedIO_Write(bio, typname);
	      if (!GWEN_Error_IsOk(err)) {
		DBG_INFO(0, "called from here");
		return 1;
	      }
	      if (dbflags & GWEN_DB_FLAGS_QUOTE_VARNAMES) {
		err=GWEN_BufferedIO_Write(bio, "\"");
		if (!GWEN_Error_IsOk(err)) {
		  DBG_INFO(0, "called from here");
		  return 1;
		}
	      }
	      err=GWEN_BufferedIO_Write(bio, n->var.name);
	      if (!GWEN_Error_IsOk(err)) {
		DBG_INFO(0, "called from here");
		return 1;
	      }
	      if (dbflags & GWEN_DB_FLAGS_QUOTE_VARNAMES) {
		err=GWEN_BufferedIO_Write(bio, "\"");
		if (!GWEN_Error_IsOk(err)) {
		  DBG_INFO(0, "called from here");
		  return 1;
		}
	      }
	      err=GWEN_BufferedIO_Write(bio, "=");
	      if (!GWEN_Error_IsOk(err)) {
		DBG_INFO(0, "called from here");
		return 1;
	      }
	      namewritten=1;
	    } /* if !namewritten */

	    if (values) {
	      err=GWEN_BufferedIO_Write(bio, ", ");
	      if (!GWEN_Error_IsOk(err)) {
		DBG_INFO(0, "called from here");
		return 1;
	      }
	    }
	    values++;
	    if (dbflags & GWEN_DB_FLAGS_QUOTE_VALUES) {
	      err=GWEN_BufferedIO_Write(bio, "\"");
	      if (!GWEN_Error_IsOk(err)) {
		DBG_INFO(0, "called from here");
		return 1;
	      }
	    }
	    err=GWEN_BufferedIO_Write(bio, pvalue);
	    if (!GWEN_Error_IsOk(err)) {
	      DBG_INFO(0, "called from here");
	      return 1;
	    }
	    if (dbflags & GWEN_DB_FLAGS_QUOTE_VALUES) {
	      err=GWEN_BufferedIO_Write(bio, "\"");
	      if (!GWEN_Error_IsOk(err)) {
		DBG_INFO(0, "called from here");
		return 1;
	      }
	    }

	    break;
	  default:
	    break;
	  } /* switch */

	  cn=cn->h.next;
	} /* while cn */
	err=GWEN_BufferedIO_WriteLine(bio, "");
	if (!GWEN_Error_IsOk(err)) {
	  DBG_INFO(0, "called from here");
	  return 1;
	}
      } /* if children */
      break;

    default:
      DBG_WARN(0, "[unhandled node type %d]\n", n->h.typ);
    }

    n=n->h.next;
  } /* while */
  return 0;
}



int GWEN_DB_WriteToStream(GWEN_DB_NODE *node,
			  GWEN_BUFFEREDIO *bio,
			  unsigned int dbflags) {
  return GWEN_DB_WriteGroupToStream(node, bio, dbflags, 0);
}



int GWEN_DB_WriteFile(GWEN_DB_NODE *n,
                      const char *fname,
		      unsigned int dbflags){
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;
  int fd;
  int rv;

  fd=open(fname, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
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





