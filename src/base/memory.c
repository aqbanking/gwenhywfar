/***************************************************************************
 begin       : Sat Jun 28 2003
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

/*#define ENABLE_MY_SMALL_BLOCK_ALLOC*/



#include "memory_p.h"
#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/stringlist.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#include <assert.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif


static GWEN_MEMORY_TABLE *gwen_memory__first_table=0;
static int gwen_memory__debug=0;
static int gwen_memory__nofree=0;
static int gwen_memory__verbous=0;
static size_t gwen_memory__allocated_bytes=0;
static size_t gwen_memory__allocated_calls=0;
static size_t gwen_memory__allocated_reused=0;

#ifdef ENABLE_MY_SMALL_BLOCK_ALLOC
static size_t gwen_memory__released_since_collect=0;
#endif



int GWEN_Memory_ModuleInit(void){
  const char *s;

  s=getenv(GWEN_MEMORY_ENV_DEBUG);
  if (s) {
    fprintf(stderr, "Memory debugging is enabled\n");
    gwen_memory__debug=1;
    gwen_memory__nofree=(getenv(GWEN_MEMORY_ENV_NO_FREE)!=0);
    gwen_memory__verbous=(getenv(GWEN_MEMORY_ENV_VERBOUS)!=0);
  }
  return 0;
}



int GWEN_Memory_ModuleFini(void){
  GWEN_MEMORY_TABLE *mt;

  mt=gwen_memory__first_table;
  while(mt) {
    GWEN_MEMORY_TABLE *next;

    next=mt->next;
    GWEN_Memory_Table_free(mt);
    mt=next;
  }

  if (gwen_memory__verbous) {
    size_t avg=0;
    size_t bytes;
    const char *suffix;

    if (gwen_memory__allocated_calls)
      avg=gwen_memory__allocated_bytes/gwen_memory__allocated_calls;

    if (gwen_memory__allocated_bytes>(1024*1024)) {
      bytes=gwen_memory__allocated_bytes/(1024*1024);
      suffix="mb";
    }
    else if (gwen_memory__allocated_bytes>1024) {
      bytes=gwen_memory__allocated_bytes/1024;
      suffix="kb";
    }
    else {
      bytes=gwen_memory__allocated_bytes;
      suffix="bytes";
    }

    fprintf(stderr,
            "GWEN info: %zu %s allocated in %zu calls "
            "(%zu times reused, average %zu bytes)\n",
            bytes, suffix,
            gwen_memory__allocated_calls,
            gwen_memory__allocated_reused,
            avg);
  }

  return 0;
}



void GWEN_Memory_Report(void){
  return;
}





GWEN_MEMORY_TABLE *GWEN_Memory_Table_new(void) {
  GWEN_MEMORY_TABLE *mt;
  unsigned char *p;
  unsigned short dsize;

  if (gwen_memory__verbous)
    fprintf(stderr, "GWEN info: allocating memory table\n");
  mt=(GWEN_MEMORY_TABLE*)malloc(sizeof(GWEN_MEMORY_TABLE));
  assert(mt);
  memset(mt, 0, sizeof(GWEN_MEMORY_TABLE));
  dsize=GWEN_MEMORY_MAXBLOCK;
  p=mt->data;
  GWEN_MEMORY_WRITESIZE(p, dsize);

  return mt;
}



void GWEN_Memory_Table_free(GWEN_MEMORY_TABLE *mt) {
  if (mt) {
    if (gwen_memory__debug) {
      unsigned char *p;
      unsigned char *end;
  
      p=mt->data;
      end=p+GWEN_MEMORY_TABLE_LEN;
      while(p<end) {
        unsigned short bsize;
        unsigned short rsize;
  
        bsize=GWEN_MEMORY_READSIZE(p);
        rsize=bsize & GWEN_MEMORY_MASK_LEN;
        if (bsize & GWEN_MEMORY_MASK_MALLOCED) {
          fprintf(stderr,
                  "GWEN warning: Block %p still allocated (%d bytes)\n",
                  GWEN_MEMORY_GETDATA(p),
                  rsize);
        }
        p+=rsize+GWEN_MEMORY_SIZELEN;
      }
    }
    free(mt);
  }
}



void GWEN_Memory_Table_Append(GWEN_MEMORY_TABLE *head, GWEN_MEMORY_TABLE *mt){
  GWEN_MEMORY_TABLE *last;

  assert(head);
  assert(mt);

  last=head;
  while(last->next)
    last=last->next;
  last->next=mt;
}



void GWEN_Memory_Table_Insert(GWEN_MEMORY_TABLE *mt){
  mt->next=gwen_memory__first_table;
  gwen_memory__first_table=mt;
}



unsigned char *GWEN_Memory_Table__FindFreeBlock(GWEN_MEMORY_TABLE *mt,
                                                unsigned short dsize) {
  unsigned char *end;
  unsigned char *p;

  end=mt->data+GWEN_MEMORY_TABLE_LEN;
  p=mt->data;
  while(p<end) {
    unsigned short bsize;
    unsigned short rsize;

    bsize=GWEN_MEMORY_READSIZE(p);
    rsize=bsize & GWEN_MEMORY_MASK_LEN;
    /*fprintf(stderr, "GWEN debug: at %u: found block with %u bytes (%s)\n",
            p-mt->data,
            rsize,
            (bsize & GWEN_MEMORY_MASK_INUSE)?"used":"free");*/
    if (rsize && !(bsize & GWEN_MEMORY_MASK_INUSE)) {
      /* unused block */
      if (rsize==dsize ||
          rsize>=(dsize+GWEN_MEMORY_SIZELEN+GWEN_MEMORY_MINREMAIN)) {
        return p;
      }
    }
    p+=rsize+GWEN_MEMORY_SIZELEN;
  }

  return 0;
}



void GWEN_Memory_Table__CollectAt(GWEN_MEMORY_TABLE *mt,
                                  unsigned char *p) {
  unsigned char *end;
  unsigned short nsize=0;
  unsigned char *np;
  int cnt=0;

  np=p;
  end=mt->data+GWEN_MEMORY_TABLE_LEN;

  while(np<end) {
    unsigned short bsize;
    unsigned short rsize;

    bsize=GWEN_MEMORY_READSIZE(np);
    rsize=bsize & GWEN_MEMORY_MASK_LEN;
    if (rsize && !(bsize & GWEN_MEMORY_MASK_INUSE)) {
      nsize+=rsize;
      if (cnt)
        nsize+=GWEN_MEMORY_SIZELEN;
      cnt++;
    }
    else
      break;

    np+=rsize+GWEN_MEMORY_SIZELEN;
  }

  if (cnt>1) {
    fprintf(stderr, "GWEN info: collected %u bytes\n", nsize);
    GWEN_MEMORY_WRITESIZE(p, nsize);
  }


}



void GWEN_Memory_Table__Collect(GWEN_MEMORY_TABLE *mt) {
  unsigned char *p;
  unsigned char *end;

  end=mt->data+GWEN_MEMORY_TABLE_LEN;
  p=mt->data;
  while(p<end) {
    unsigned short bsize;
    unsigned short rsize;

    GWEN_Memory_Table__CollectAt(mt, p);
    bsize=GWEN_MEMORY_READSIZE(p);
    rsize=bsize & GWEN_MEMORY_MASK_LEN;
    p+=rsize+GWEN_MEMORY_SIZELEN;
  }
}



void GWEN_Memory_Table__Dump(GWEN_MEMORY_TABLE *mt) {
  unsigned char *p;
  unsigned char *end;

  p=mt->data;
  end=p+GWEN_MEMORY_TABLE_LEN;
  while(p<end) {
    unsigned short bsize;
    unsigned short rsize;

    bsize=GWEN_MEMORY_READSIZE(p);
    rsize=bsize & GWEN_MEMORY_MASK_LEN;
    fprintf(stderr,
            "GWEN debug: at %5zu: found block with %5u bytes [%p] (%s)\n",
            p-mt->data,
            rsize,
            p,
            (bsize & GWEN_MEMORY_MASK_INUSE)?"used":"free");
    p+=rsize+GWEN_MEMORY_SIZELEN;
  }
}



unsigned char *GWEN_Memory__FindFreeBlock(unsigned short dsize) {
  GWEN_MEMORY_TABLE *mt;
  unsigned char *p=0;

  if (dsize>GWEN_MEMORY_MAXBLOCK) {
    fprintf(stderr, "GWEN error: Memory block too big (%d>%d)\n",
            dsize, GWEN_MEMORY_MAXBLOCK);
    abort();
  }
  if (gwen_memory__first_table==0)
    gwen_memory__first_table=GWEN_Memory_Table_new();

  mt=gwen_memory__first_table;
  assert(mt);

  while(mt) {
    p=GWEN_Memory_Table__FindFreeBlock(mt, dsize);
    if (p)
      return p;
    mt=mt->next;
  }

  mt=GWEN_Memory_Table_new();
  //GWEN_Memory_Table_Append(gwen_memory__first_table, mt);
  GWEN_Memory_Table_Insert(mt);
  p=GWEN_Memory_Table__FindFreeBlock(mt, dsize);
  assert(p);

  return p;
}



void *GWEN_Memory__Malloc(unsigned short dsize) {
  unsigned char *p;
  unsigned short bsize;
  unsigned short rsize;

  p=GWEN_Memory__FindFreeBlock(dsize);
  assert(p);

  bsize=GWEN_MEMORY_READSIZE(p);
  rsize=bsize & GWEN_MEMORY_MASK_LEN;

  if (rsize>dsize) {
    unsigned char *np;
    unsigned short nsize;

    /* write header for next block */
    nsize=rsize-dsize-GWEN_MEMORY_SIZELEN;
    np=p+GWEN_MEMORY_SIZELEN+dsize;
    /*fprintf(stderr,
            "Splitting block from %u to %u/%u (relpos %u)\n",
            rsize, dsize, nsize, np-p); */
    GWEN_MEMORY_WRITESIZE(np, (nsize & GWEN_MEMORY_MASK_LEN));
  }
  else
    gwen_memory__allocated_reused++;

  GWEN_MEMORY_WRITESIZE(p, (dsize |
                            GWEN_MEMORY_MASK_INUSE |
                            GWEN_MEMORY_MASK_MALLOCED));
  /* fprintf(stderr, "GWEN debug: allocated block internally (%p).\n", p); */

  return (void*)GWEN_MEMORY_GETDATA(p);
}



void *GWEN_Memory_malloc(size_t wsize) {
#ifdef ENABLE_MY_SMALL_BLOCK_ALLOC
  void *p;
  size_t dsize;
#endif

  if (GWEN_UNLIKELY(wsize==0)) {
    fprintf(stderr,
            "GWEN error: allocating 0 bytes, maybe a program error\n");
    abort();
  }

#ifdef ENABLE_MY_SMALL_BLOCK_ALLOC
  dsize=(wsize+GWEN_MEMORY_GRANULARITY-1) & ~(GWEN_MEMORY_GRANULARITY-1);

  if (dsize<GWEN_MEMORY_MAXBLOCK) {
    /* allocate a small block */
    /*if (gwen_memory__verbous)
      fprintf(stderr, "GWEN info: Allocating %u bytes internally\n",
              dsize);*/
    p=GWEN_Memory__Malloc(dsize & GWEN_MEMORY_MASK_LEN);
  }
  else {
    unsigned char *pc;

    /* allocate big block via system */
    if (gwen_memory__verbous)
      fprintf(stderr, "GWEN info: Allocating %u bytes externally\n",
              dsize);
    pc=(unsigned char*)malloc(dsize+GWEN_MEMORY_SIZELEN);
    assert(pc);
    GWEN_MEMORY_WRITESIZE(pc, GWEN_MEMORY_EXTERNAL);
    p=GWEN_MEMORY_GETDATA(pc);
  }

  gwen_memory__allocated_bytes+=dsize;
  gwen_memory__allocated_calls++;
  /*fprintf(stderr, "GWEN debug: allocated block (%p).\n", p);*/
  return p;
#else
  return malloc(wsize);
#endif
}



void *GWEN_Memory_realloc(void *oldp, size_t nsize) {
#ifdef ENABLE_MY_SMALL_BLOCK_ALLOC
  void *p;
  unsigned char *pc;
  unsigned short dsize;
  unsigned short rsize;
#endif

  assert(oldp);
  assert(nsize);

#ifdef ENABLE_MY_SMALL_BLOCK_ALLOC
  pc=GWEN_MEMORY_GETSTART(oldp);
  dsize=GWEN_MEMORY_READSIZE(pc);
  rsize=dsize & GWEN_MEMORY_MASK_LEN;

  if (!(dsize & GWEN_MEMORY_MASK_MALLOCED)) {
    fprintf(stderr, "GWEN error: Block %p already free'd\n", oldp);
    abort();
  }

  if (!(dsize & GWEN_MEMORY_MASK_INUSE)) {
    fprintf(stderr, "GWEN error: Block %p not in use\n", oldp);
    abort();
  }

  p=GWEN_Memory_malloc(nsize);
  memmove(p, oldp, rsize);
  GWEN_Memory_dealloc(oldp);
  return p;
#else
  return realloc(oldp, nsize);
#endif
}



void GWEN_Memory_dealloc(void *p) {
#ifdef ENABLE_MY_SMALL_BLOCK_ALLOC
  if (p) {
    unsigned char *pc;
    unsigned short dsize;

    pc=GWEN_MEMORY_GETSTART(p);
    dsize=GWEN_MEMORY_READSIZE(pc);

    if (!(dsize & GWEN_MEMORY_MASK_MALLOCED)) {
      fprintf(stderr, "GWEN error: Block %p already free'd\n", p);
      abort();
    }

    if (!(dsize & GWEN_MEMORY_MASK_INUSE)) {
      fprintf(stderr, "GWEN error: Block %p not in use\n", p);
      abort();
    }

    if (gwen_memory__nofree==0) {
      GWEN_MEMORY_WRITESIZE(pc,
                            (dsize &
                            ~GWEN_MEMORY_MASK_MALLOCED &
                            ~GWEN_MEMORY_MASK_INUSE));
    }
    else {
      GWEN_MEMORY_WRITESIZE(pc,
                            (dsize &
                            ~GWEN_MEMORY_MASK_MALLOCED));
    }

    if (dsize==GWEN_MEMORY_EXTERNAL) {
      /*fprintf(stderr,
              "GWEN debug: deallocating block at %p externally\n", p); */

      if (gwen_memory__nofree==0)
        free((void*)pc);
    }
    else {
      /*fprintf(stderr,
              "GWEN debug: deallocating %u bytes at %p internally\n",
              (dsize & GWEN_MEMORY_MASK_LEN), p); */
      //gwen_memory__released_since_collect+=dsize;
      if (gwen_memory__released_since_collect>GWEN_MEMORY_COLLECT_AFTER){
        fprintf(stderr, "GWEN info: collecting free blocks\n");
        GWEN_Memory_Collect();
        gwen_memory__released_since_collect=0;
      }
    }
  }
#else
  if (GWEN_LIKELY(gwen_memory__nofree==0))
    free(p);
#endif
}



char *GWEN_Memory_strdup(const char *s) {
#ifdef ENABLE_MY_SMALL_BLOCK_ALLOC
  unsigned int dsize;
  char *p;
#endif

  assert(s);
#ifdef ENABLE_MY_SMALL_BLOCK_ALLOC
  dsize=strlen(s);

  p=(char*)GWEN_Memory_malloc(dsize+1);
  assert(p);
  memmove(p, s, dsize+1);
  return p;
#else
  return strdup(s);
#endif
}



void GWEN_Memory_Dump(void) {
  GWEN_MEMORY_TABLE *mt;

  mt=gwen_memory__first_table;
  while(mt) {
    GWEN_Memory_Table__Dump(mt);
    mt=mt->next;
  }
}



void GWEN_Memory_Collect(void) {
  GWEN_MEMORY_TABLE *mt;

  mt=gwen_memory__first_table;
  while(mt) {
    GWEN_Memory_Table__Collect(mt);
    mt=mt->next;
  }
}



