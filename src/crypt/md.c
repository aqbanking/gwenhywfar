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


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "md_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/crypt.h>
#include <stdlib.h>


#ifdef GWEN_MEMTRACE
static unsigned int GWEN_MD_Count=0;
#endif


static GWEN_MD_PROVIDER *gwen_md_providers=0;


GWEN_MD *GWEN_MD_new(unsigned int size){
  GWEN_MD *md;

  GWEN_NEW_OBJECT(GWEN_MD, md);
#ifdef GWEN_MEMTRACE
  GWEN_MD_Count++;
  DBG_INFO(GWEN_LOGDOMAIN, "New MD (now %d)", GWEN_MD_Count);
#endif
  if (size) {
    md->pDigest=(unsigned char*)malloc(size);
    md->lDigest=size;
  }
  return md;
}



void GWEN_MD_free(GWEN_MD *md){
  if (md) {
#ifdef GWEN_MEMTRACE
    assert(GWEN_MD_Count);
    GWEN_MD_Count--;
    DBG_INFO(GWEN_LOGDOMAIN, "Free MD (now %d)", GWEN_MD_Count);
#endif
    if (md->data && md->freeDataFn)
      md->freeDataFn(md);
    free(md->pDigest);
    free(md);
  }
}



unsigned char *GWEN_MD_GetDigestPtr(GWEN_MD *md){
  assert(md);
  return md->pDigest;
}



unsigned int GWEN_MD_GetDigestSize(GWEN_MD *md){
  assert(md);
  return md->lDigest;
}



void GWEN_MD_SetBeginFn(GWEN_MD *md, GWEN_MD_BEGIN_FN fn){
  assert(md);
  md->beginFn=fn;
}



void GWEN_MD_SetEndFn(GWEN_MD *md, GWEN_MD_END_FN fn){
  assert(md);
  md->endFn=fn;
}



void GWEN_MD_SetUpdateFn(GWEN_MD *md, GWEN_MD_UPDATE_FN fn){
  assert(md);
  md->updateFn=fn;
}



void GWEN_MD_SetFreeDataFn(GWEN_MD *md, GWEN_MD_FREEDATA_FN fn){
  assert(md);
  md->freeDataFn=fn;
}



int GWEN_MD_Begin(GWEN_MD *md){
  assert(md);
  assert(md->beginFn);
  return md->beginFn(md);
}



int GWEN_MD_End(GWEN_MD *md){
  assert(md);
  assert(md->endFn);
  return md->endFn(md);
}



int GWEN_MD_Update(GWEN_MD *md,
                   const char *buf,
                   unsigned int l){
  assert(md);
  assert(md->updateFn);
  return md->updateFn(md, buf, l);
}



void GWEN_MD_SetData(GWEN_MD *md, void *data){
  assert(md);
  if (md->data && md->freeDataFn)
    md->freeDataFn(md);
  md->data=data;
}



void *GWEN_MD_GetData(GWEN_MD *md){
  assert(md);
  return md->data;
}





GWEN_MD_PROVIDER *GWEN_MdProvider_new(){
  GWEN_MD_PROVIDER *pr;

  GWEN_NEW_OBJECT(GWEN_MD_PROVIDER, pr);
  return pr;
}



void GWEN_MdProvider_free(GWEN_MD_PROVIDER *pr){
  if (pr) {
    free(pr->name);
    free(pr);
  }
}



void GWEN_MdProvider_SetNewMdFn(GWEN_MD_PROVIDER *pr,
                                GWEN_MDPROVIDER_NEWMD_FN fn){
  assert(pr);
  pr->newMdFn=fn;
}



void GWEN_MdProvider_SetName(GWEN_MD_PROVIDER *pr,
                             const char *name){
  assert(pr);
  assert(name);
  free(pr->name);
  pr->name=strdup(name);
}



/**
 * On success this function takes over ownership of the given object.
 */
GWEN_ERRORCODE GWEN_MD_RegisterProvider(GWEN_MD_PROVIDER *pr){
  assert(pr);

  if (GWEN_MD_FindProvider(pr->name)){
    DBG_INFO(GWEN_LOGDOMAIN, "Service \"%s\" already registered", pr->name);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_ALREADY_REGISTERED);
  }

  GWEN_LIST_ADD(GWEN_MD_PROVIDER, pr, &gwen_md_providers);
  return 0;

}



GWEN_ERRORCODE GWEN_MD_UnregisterProvider(GWEN_MD_PROVIDER *pr){
  assert(pr);

  if (!GWEN_MD_FindProvider(pr->name)){
    DBG_INFO(GWEN_LOGDOMAIN, "Service \"%s\" not registered", pr->name);
    return GWEN_Error_new(0,
                          GWEN_ERROR_SEVERITY_ERR,
                          GWEN_Error_FindType(GWEN_CRYPT_ERROR_TYPE),
                          GWEN_CRYPT_ERROR_NOT_REGISTERED);
  }

  GWEN_LIST_DEL(GWEN_MD_PROVIDER, pr, &gwen_md_providers);
  return 0;
}



GWEN_MD_PROVIDER *GWEN_MD_FindProvider(const char *name){
  GWEN_MD_PROVIDER *pr;

  pr=gwen_md_providers;
  while(pr) {
    if (strcasecmp(pr->name, name)==0)
      return pr;
    pr=pr->next;
  } /* while */

  return 0;
}



GWEN_MD *GWEN_MD_Factory(const char *t){
  GWEN_MD_PROVIDER *pr;
  GWEN_MD *md;

  pr=GWEN_MD_FindProvider(t);
  if (!pr) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No MD provider for \"%s\" found", t);
    return 0;
  }

  md=pr->newMdFn(pr);
  return md;
}



void GWEN_MD_UnregisterAllProviders(){
  GWEN_MD_PROVIDER *pr;

  pr=gwen_md_providers;
  while(pr) {
    GWEN_MD_PROVIDER *npr;

    npr=pr->next;
    GWEN_MD_UnregisterProvider(pr);
    GWEN_MdProvider_free(pr);
    pr=npr;
  } /* while */
}



int GWEN_MD_Hash(const char *typ,
                 const char *data,
                 unsigned int dsize,
                 char *buffer,
                 unsigned int *bsize) {
  GWEN_MD *md;
  unsigned int i;

  md=GWEN_MD_Factory(typ);
  if (!md) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return -1;
  }

  if (GWEN_MD_Begin(md)) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    GWEN_MD_free(md);
    return -1;
  }

  if (GWEN_MD_Update(md, data, dsize)) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    GWEN_MD_free(md);
    return -1;
  }

  if (GWEN_MD_End(md)) {
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    GWEN_MD_free(md);
    return -1;
  }

  i=GWEN_MD_GetDigestSize(md);
  if (i>*bsize) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Buffer too small");
    GWEN_MD_free(md);
    return -1;
  }

  memmove(buffer, GWEN_MD_GetDigestPtr(md), i);
  *bsize=i;
  GWEN_MD_free(md);
  return 0;
}





