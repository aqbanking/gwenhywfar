/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Fri Feb 07 2003
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

/* #define DISABLE_DEBUGLOG */


#include "filter_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>


GWEN_LIST_FUNCTIONS(GWEN_FILTER, GWEN_Filter)
GWEN_INHERIT_FUNCTIONS(GWEN_FILTER)


GWEN_FILTER *GWEN_Filter_new(const char *name){
  GWEN_FILTER *f;

  assert(name);
  GWEN_NEW_OBJECT(GWEN_FILTER, f);
  GWEN_INHERIT_INIT(GWEN_FILTER, f);
  GWEN_LIST_INIT(GWEN_FILTER, f);
  f->filterName=strdup(name);
  f->inBuffer=GWEN_RingBuffer_new(GWEN_FILTER_BUFFERSIZE);
  f->outBuffer=GWEN_RingBuffer_new(GWEN_FILTER_BUFFERSIZE);

  f->nextElements=GWEN_Filter_List_new();

  return f;
}



void GWEN_Filter_free(GWEN_FILTER *f){
  if (f) {
    GWEN_INHERIT_FINI(GWEN_FILTER, f);
    GWEN_Filter_List_free(f->nextElements);
    GWEN_RingBuffer_free(f->inBuffer);
    GWEN_RingBuffer_free(f->outBuffer);
    free(f->filterName);
    GWEN_LIST_FINI(GWEN_FILTER, f);
    GWEN_FREE_OBJECT(f);
  }
}



GWEN_RINGBUFFER *GWEN_Filter_GetInBuffer(const GWEN_FILTER *f){
  assert(f);
  return f->inBuffer;
}



GWEN_RINGBUFFER *GWEN_Filter_GetOutBuffer(const GWEN_FILTER *f){
  assert(f);
  return f->outBuffer;
}



void GWEN_Filter_SetWorkFn(GWEN_FILTER *f, GWEN_FILTER_WORKFN fn){
  assert(f);
  f->workFn=fn;
}



GWEN_FILTER_RESULT GWEN_Filter__Work(GWEN_FILTER *f){
  assert(f);
  assert(f->workFn);
  return f->workFn(f);
}



void GWEN_Filter_AppendNext(GWEN_FILTER *fPredecessor, GWEN_FILTER *fNew){
  assert(fPredecessor);
  assert(fNew);
  GWEN_Filter_List_Add(fNew, fPredecessor->nextElements);
}



GWEN_FILTER_RESULT GWEN_Filter__WriteToAllNext(GWEN_FILTER *filter) {
  GWEN_FILTER *f;
  uint32_t maxFree;
  const char *p;

  /* get maximum of bytes for the next level (least number of writeable
   * bytes) */
  maxFree=GWEN_RingBuffer_GetMaxUnsegmentedRead(filter->outBuffer);
  if (maxFree) {
    f=GWEN_Filter_List_First(filter->nextElements);
    while(f) {
      uint32_t currFree;

      currFree=GWEN_RingBuffer_GetMaxUnsegmentedWrite(f->inBuffer);
      if (currFree<maxFree)
        maxFree=currFree;
      f=GWEN_Filter_List_Next(f);
    } /* while */

    if (!maxFree) {
      DBG_INFO(GWEN_LOGDOMAIN, "Buffers are full");
      return GWEN_Filter_ResultFull;
    }

    /* write to every next element */
    p=GWEN_RingBuffer_GetReadPointer(filter->outBuffer);
    DBG_INFO(GWEN_LOGDOMAIN,
             "Writing %u bytes",
             maxFree);
    assert(p);
    f=GWEN_Filter_List_First(filter->nextElements);
    while(f) {
      uint32_t written;

      written=maxFree;
      if (GWEN_RingBuffer_WriteBytes(f->inBuffer, p, &written)) {
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Error writing bytes to inbuffer of filter \"%s\"",
                  f->filterName);
        return GWEN_Filter_ResultError;
      }
      /* here the number of bytes written must also equal the number of
       * bytes to write */
      assert(written==maxFree);

      f=GWEN_Filter_List_Next(f);
    } /* while */
    GWEN_RingBuffer_SkipBytesRead(filter->outBuffer, maxFree);
  } /* if there is something to write */

  return GWEN_Filter_ResultOk;
}



GWEN_FILTER_RESULT GWEN_Filter_Work(GWEN_FILTER *filter, int oneLoop) {
  int wasFull=0;

  for (;;) {
    GWEN_FILTER *f;
    GWEN_FILTER_RESULT res;
    int allNeedData=0;

    /* let only this element work */
    res=GWEN_Filter__Work(filter);
    if (res==GWEN_Filter_ResultError)
      return res;

    /* write to all next elements */
    res=GWEN_Filter__WriteToAllNext(filter);
    if (res==GWEN_Filter_ResultFull) {
      if (wasFull)
        /* was already full the last time we tried, return */
        return res;
      wasFull=1;
    }
    else if (res!=GWEN_Filter_ResultOk)
      return res;

    /* let all next elements flush */
    allNeedData=1;
    f=GWEN_Filter_List_First(filter->nextElements);
    while(f) {
      res=GWEN_Filter_Work(f, oneLoop);
      if (res==GWEN_Filter_ResultError)
        return res;
      else if (res!=GWEN_Filter_ResultNeedMore)
        allNeedData=0;
      f=GWEN_Filter_List_Next(f);
    } /* while */

    if (allNeedData && wasFull) {
      DBG_INFO(GWEN_LOGDOMAIN,
               "All elements need data, finished");
      return GWEN_Filter_ResultNeedMore;
    }

    if (oneLoop)
      return GWEN_Filter_ResultOk;
  } /* for */
}






















