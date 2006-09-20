/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Dec 04 2004
    copyright   : (C) 2004 by Martin Preuss
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

#include "debug_p.h"

#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#include <gwenhywfar/misc.h>



static GWEN_MEMORY_DEBUG_OBJECT *gwen_debug__memobjects=0;



GWEN_TYPE_UINT32 GWEN_Debug_PrintDec(char *buffer,
                                     GWEN_TYPE_UINT32 size,
                                     GWEN_TYPE_UINT32 num,
                                     int leadingZero,
                                     GWEN_TYPE_UINT32 length) {
  GWEN_TYPE_UINT32 i;
  GWEN_TYPE_UINT32 j;
  GWEN_TYPE_UINT32 k;
  char numbuf[16];
  int numOr;

  /* first convert number */
  numOr=0;
  i=0;
  j=1000000000;

  while(j) {
    k=num/j;
    numOr|=k;
    if (numOr || leadingZero || j==1) {
      numbuf[i]=k+'0';
      i++;
    }
    num-=(k*j);
    j/=10;
  } /* while j */

  j=0;
  if (length) {
    if (i>length)
      i=length;

    /* fill left up to length-(sizeof number) */
    k=length-i;
    while(k) {
      if (j<size) {
        if (leadingZero)
          buffer[j]='0';
        else
          buffer[j]=' ';
      }
      j++;
      k--;
    } /* while k */
  } /* if length */

  /* copy number */
  for (k=0; k<i; k++) {
    if (j<size)
      buffer[j]=numbuf[k];
    j++;
  } /* while i */

  /* append trailing 0 */
  if (j<size)
    buffer[j]=0;
  j++;
  /* return length (or possible length) */
  return j;
}



GWEN_TYPE_UINT32 GWEN_Debug_PrintHex(char *buffer,
                                 GWEN_TYPE_UINT32 size,
                                 GWEN_TYPE_UINT32 num,
                                 int leadingZero,
                                 int up,
                                 GWEN_TYPE_UINT32 length) {
  GWEN_TYPE_UINT32 i;
  GWEN_TYPE_UINT32 j;
  GWEN_TYPE_UINT32 k;
  char numbuf[16];
  int numOr;

  /* first convert number */
  numOr=0;
  i=0;
  j=8;

  while(j) {
    k=(num>>((j-1)*4))&0xf;
    numOr|=k;
    if (numOr || leadingZero || j==1) {
      if (k>9) {
        if (up)
          numbuf[i]=k+'0'+7;
        else
          numbuf[i]=k+'0'+7+32;
      }
      else
        numbuf[i]=k+'0';
      i++;
    }
    j--;
  } /* while j */

  j=0;
  if (length) {
    if (i>length)
      i=length;

    /* fill left up to length-(sizeof number) */
    k=length-i;
    while(k) {
      if (j<size) {
        if (leadingZero)
          buffer[j]='0';
        else
          buffer[j]=' ';
      }
      j++;
      k--;
    } /* while k */
  } /* if length */

  /* copy number */
  for (k=0; k<i; k++) {
    if (j<size)
      buffer[j]=numbuf[k];
    j++;
  } /* while i */

  /* append trailing 0 */
  if (j<size)
    buffer[j]=0;
  j++;
  /* return length (or possible length) */
  return j;
}





GWEN_TYPE_UINT32 GWEN_Debug_Snprintf(char *buffer,
                                 GWEN_TYPE_UINT32 size,
                                 const char *fmt, ...) {
  va_list arguments;
  GWEN_TYPE_UINT32 i;

  i=0;
  va_start(arguments, fmt);
  while(*fmt) {
    if (*fmt=='%') {
      fmt++;
      if (*fmt=='%') {
        /* write character '%' */
        if (i<size)
          buffer[i]='%';
        i++;
      }
      else {
        GWEN_TYPE_UINT32 length;
        int leadingZero;

        leadingZero=0;
        length=0;

        /* read length */
        if ((*fmt)>='0' && (*fmt)<='9') {
          /* read number */
          if (*fmt=='0') {
            leadingZero=1;
          }
          while ((*fmt)>='0' && (*fmt)<='9') {
            length*=10;
            length+=*fmt-'0';
            fmt++;
          } /* while */
        }

        /* read type */
        switch(*fmt) {
        /* decimal integer */
        case 'c':
        case 'd': {
          int p;

          p=va_arg(arguments, int);
          if (p<0) {
            if (i<size)
              buffer[i]='-';
            i++;
            p=-p;
          }
          i+=GWEN_Debug_PrintDec(buffer+i,
                                 size-i,
                                 p,
                                 leadingZero,
                                 length)-1;
          break;
        }

        /* hexadecimal integer */
        case 'x': {
          unsigned int p;

          p=va_arg(arguments, unsigned int);
          i+=GWEN_Debug_PrintHex(buffer+i,
                                 size-i,
                                 p,
                                 leadingZero,
                                 0,
                                 length)-1;
          break;
        }

        /* hexadecimal integer */
        case 'X': {
          unsigned int p;

          p=va_arg(arguments, unsigned int);
          i+=GWEN_Debug_PrintHex(buffer+i,
                                 size-i,
                                 p,
                                 leadingZero,
                                 1,
                                 length)-1;
          break;
        }

        case 's': {
          const char *p;

          p=va_arg(arguments, const char*);
          if (!p)
            p="(null)";
          while(*p) {
            if (i<size)
              buffer[i]=*p;
            i++;
            p++;
          } /* while */
          break;
        }

        default:
          break;
        } /* switch */
      }
    }
    else {
      if (i<size)
        buffer[i]=*fmt;
      i++;
    }
    fmt++;
  } /* while */

  /* add trailing 0 */
  if (i<size)
    buffer[i]=0;
  i++;
  va_end(arguments);
  return i;
}




#ifdef NO_VARIADIC_MACROS
void DBG_ERROR(const char *dbg_logger, const char *format, ...) {
  va_list args;
  char dbg_buffer[256]; 
  va_start(args, format);
  vsnprintf(dbg_buffer, sizeof(dbg_buffer)-1, format, args);
  dbg_buffer[sizeof(dbg_buffer)-1] = 0; 
  GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelError, dbg_buffer);
  va_end(args);
}

void DBG_WARN(const char *dbg_logger, const char *format, ...) {
  va_list args;
  char dbg_buffer[256]; 
  va_start(args, format);
  vsnprintf(dbg_buffer, sizeof(dbg_buffer)-1, format, args);
  dbg_buffer[sizeof(dbg_buffer)-1] = 0; 
  GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelWarning, dbg_buffer);
  va_end(args);
}

void DBG_NOTICE(const char *dbg_logger, const char *format, ...) {
  if (GWEN_Logger_GetLevel(dbg_logger)>=GWEN_LoggerLevelNotice) {
    va_list args;
    char dbg_buffer[256]; 
    va_start(args, format);
    vsnprintf(dbg_buffer, sizeof(dbg_buffer)-1, format, args);
    dbg_buffer[sizeof(dbg_buffer)-1] = 0; 
    GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelNotice, dbg_buffer);
    va_end(args);
  }
}

void DBG_INFO(const char *dbg_logger, const char *format, ...) {
  if (GWEN_Logger_GetLevel(dbg_logger)>=GWEN_LoggerLevelInfo) {
    va_list args;
    char dbg_buffer[256]; 
    va_start(args, format);
    vsnprintf(dbg_buffer, sizeof(dbg_buffer)-1, format, args);
    dbg_buffer[sizeof(dbg_buffer)-1] = 0; 
    GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelInfo, dbg_buffer);
    va_end(args);
  }
}

void DBG_DEBUG(const char *dbg_logger, const char *format, ...) {
# ifndef DISABLE_DEBUGLOG
  if (GWEN_Logger_GetLevel(dbg_logger)>=GWEN_LoggerLevelDebug) {
    va_list args;
    char dbg_buffer[256]; 
    va_start(args, format);
    vsnprintf(dbg_buffer, sizeof(dbg_buffer)-1, format, args);
    dbg_buffer[sizeof(dbg_buffer)-1] = 0; 
    GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelDebug, dbg_buffer);
    va_end(args);
  }
# endif /* DISABLE_DEBUGLOG */
}

void DBG_VERBOUS(const char *dbg_logger, const char *format, ...) {
# ifndef DISABLE_DEBUGLOG
  if (GWEN_Logger_GetLevel(dbg_logger)>=GWEN_LoggerLevelVerbous) {
    va_list args;
    char dbg_buffer[256]; 
    va_start(args, format);
    vsnprintf(dbg_buffer, sizeof(dbg_buffer)-1, format, args);
    dbg_buffer[sizeof(dbg_buffer)-1] = 0; 
    GWEN_Logger_Log(dbg_logger, GWEN_LoggerLevelVerbous, dbg_buffer);
    va_end(args);
  }
# endif /* DISABLE_DEBUGLOG */
}

#endif /* NO_VARIADIC_MACROS */







GWEN_MEMORY_DEBUG_ENTRY*
GWEN_MemoryDebugEntry_new(GWEN_MEMORY_DEBUG_ENTRY_TYPE t,
                          const char *wFile,
                          int wLine){
  GWEN_MEMORY_DEBUG_ENTRY *e;

  assert(wFile);
  assert(wLine);
  GWEN_NEW_OBJECT(GWEN_MEMORY_DEBUG_ENTRY, e);
  e->file=strdup(wFile);
  e->line=wLine;
  e->type=t;
  return e;
}



void GWEN_MemoryDebugEntry_free(GWEN_MEMORY_DEBUG_ENTRY *e){
  if (e) {
    free(e->file);
    GWEN_FREE_OBJECT(e);
  }
}




GWEN_MEMORY_DEBUG_OBJECT *GWEN_MemoryDebugObject_new(const char *name){
  GWEN_MEMORY_DEBUG_OBJECT *o;

  assert(name);
  GWEN_NEW_OBJECT(GWEN_MEMORY_DEBUG_OBJECT, o);
  o->name=strdup(name);
  return o;
}



void GWEN_MemoryDebugObject_free(GWEN_MEMORY_DEBUG_OBJECT *o) {
  if (o) {
    GWEN_MEMORY_DEBUG_ENTRY *e;

    e=o->entries;
    while(e) {
      GWEN_MEMORY_DEBUG_ENTRY *next;

      next=e->next;
      GWEN_MemoryDebugEntry_free(e);
      e=next;
    }
    free(o->name);
    GWEN_FREE_OBJECT(o);
  }
}



GWEN_MEMORY_DEBUG_OBJECT *GWEN_MemoryDebug__FindObject(const char *name){
  GWEN_MEMORY_DEBUG_OBJECT *o;

  o=gwen_debug__memobjects;
  while(o) {
    assert(o->name);
    if (strcasecmp(o->name, name)==0)
      break;
    if (o->next==o) {
      DBG_ERROR(GWEN_LOGDOMAIN, "What ?? Pointing to myself ??");
      abort();
    }
    o=o->next;
  }

  return o;
}



void GWEN_MemoryDebug_Increment(const char *name,
                                const char *wFile,
                                int wLine,
                                int attach){
  GWEN_MEMORY_DEBUG_OBJECT *o;
  GWEN_MEMORY_DEBUG_ENTRY *e;

  assert(name);
  assert(wFile);
  assert(wLine);
  o=GWEN_MemoryDebug__FindObject(name);
  if (!o) {
    o=GWEN_MemoryDebugObject_new(name);
    GWEN_LIST_ADD(GWEN_MEMORY_DEBUG_OBJECT, o, &gwen_debug__memobjects);
    e=GWEN_MemoryDebugEntry_new(attach?GWEN_MemoryDebugEntryTypeAttach:
                                GWEN_MemoryDebugEntryTypeCreate,
                                wFile, wLine);
    GWEN_LIST_ADD(GWEN_MEMORY_DEBUG_ENTRY, e, &(o->entries));
    o->count++;
  }
  else {
    e=GWEN_MemoryDebugEntry_new(attach?GWEN_MemoryDebugEntryTypeAttach:
                                GWEN_MemoryDebugEntryTypeCreate,
                                wFile, wLine);
    GWEN_LIST_ADD(GWEN_MEMORY_DEBUG_ENTRY, e, &(o->entries));
    o->count++;
  }
}



void GWEN_MemoryDebug_Decrement(const char *name,
                                const char *wFile,
                                int wLine){
  GWEN_MEMORY_DEBUG_OBJECT *o;
  GWEN_MEMORY_DEBUG_ENTRY *e;

  assert(name);
  assert(wFile);
  assert(wLine);
  o=GWEN_MemoryDebug__FindObject(name);
  if (!o) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "Object to be freed not found (%s at %s:%d)",
              name, wFile, wLine);
    o=GWEN_MemoryDebugObject_new(name);
    GWEN_LIST_ADD(GWEN_MEMORY_DEBUG_OBJECT, o, &gwen_debug__memobjects);
    e=GWEN_MemoryDebugEntry_new(GWEN_MemoryDebugEntryTypeFree,
                                wFile, wLine);
    GWEN_LIST_ADD(GWEN_MEMORY_DEBUG_ENTRY, e, &(o->entries));
    o->count--;
  }
  else {
    e=GWEN_MemoryDebugEntry_new(GWEN_MemoryDebugEntryTypeFree,
                                wFile, wLine);
    GWEN_LIST_ADD(GWEN_MEMORY_DEBUG_ENTRY, e, &(o->entries));
    o->count--;
  }
}



void GWEN_MemoryDebug__DumpObject(GWEN_MEMORY_DEBUG_OBJECT *o,
                                  GWEN_TYPE_UINT32 mode){

  DBG_ERROR(0, "Object \"%s\" (count=%ld)",
            o->name, o->count);
  if (o->count!=0 || mode==GWEN_MEMORY_DEBUG_MODE_DETAILED) {
    GWEN_MEMORY_DEBUG_ENTRY *e;

    if (mode!=GWEN_MEMORY_DEBUG_MODE_SHORT) {
      e=o->entries;
      while(e) {
        const char *s;

        fprintf(stderr, " ");
        switch(e->type) {
        case GWEN_MemoryDebugEntryTypeCreate:
          s="created";
          break;
        case GWEN_MemoryDebugEntryTypeAttach:
          s="attached";
          break;
        case GWEN_MemoryDebugEntryTypeFree:
          s="freed";
          break;
        default:
          s="<unknown action>";
          break;
        }
        DBG_ERROR(0, " %s at %s:%d", s, e->file, e->line);
        e=e->next;
      } /* while e */
    }
  }
}



void GWEN_MemoryDebug_DumpObject(const char *name,
                                 GWEN_TYPE_UINT32 mode){
  GWEN_MEMORY_DEBUG_OBJECT *o;

  assert(name);
  o=GWEN_MemoryDebug__FindObject(name);
  if (!o) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Object \"%s\" not found", name);
  }
  else
    GWEN_MemoryDebug__DumpObject(o, mode);
}



long int GWEN_MemoryDebug_GetObjectCount(const char *name){
  GWEN_MEMORY_DEBUG_OBJECT *o;

  assert(name);
  o=GWEN_MemoryDebug__FindObject(name);
  if (!o) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Object \"%s\" not found", name);
    return 0;
  }
  else
    return o->count;
}



void GWEN_MemoryDebug_Dump(GWEN_TYPE_UINT32 mode){
  GWEN_MEMORY_DEBUG_OBJECT *o;

  DBG_ERROR(0, "Gwenhywfar Memory Debugger Statistics:");
  DBG_ERROR(0, "====================================== begin\n");
  o=gwen_debug__memobjects;
  while(o) {
    GWEN_MemoryDebug__DumpObject(o, mode);
    o=o->next;
  }
  DBG_ERROR(0, "====================================== end\n");
}



void GWEN_MemoryDebug_CleanUp(){
  GWEN_MEMORY_DEBUG_OBJECT *o;

  o=gwen_debug__memobjects;
  while(o) {
    GWEN_MEMORY_DEBUG_OBJECT *next;

    next=o->next;
    GWEN_MemoryDebugObject_free(o);
    o=next;
  }
  gwen_debug__memobjects=0;
}




