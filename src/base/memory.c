/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
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



#include "memory_p.h"
#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/stringlist.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>


static GWEN_MEMORY__OBJECT_LIST *GWEN_Memory__Objects=0;
static int GWEN_Memory__DoubleCheck=0;


GWEN_LIST_FUNCTIONS(GWEN_MEMORY__OBJECT, GWEN_Memory__Object);
GWEN_LIST_FUNCTIONS(GWEN_MEMORY__OBJECT_STRING, GWEN_Memory__Object_String);



GWEN_ERRORCODE GWEN_Memory_ModuleInit(){
  const char *s;

  s=getenv(GWEN_MEMORY_ENV_DEBUG);
  if (s) {
    GWEN_MEMORY__OBJECT_LIST *l;

    DBG_INFO(0, "Memory debugging is enabled");

    l=GWEN_Memory__Object_List_new();

    GWEN_Memory__DoubleCheck=(getenv(GWEN_MEMORY_ENV_DOUBLE_CHECK)!=0);

    GWEN_Memory__Objects=l;

  }
  return 0;
}



GWEN_ERRORCODE GWEN_Memory_ModuleFini(){
  GWEN_Memory_Report();
  GWEN_Memory__Object_List_free(GWEN_Memory__Objects);
  GWEN_Memory__Objects=0;

  return 0;
}



void GWEN_Memory_Report(){
  if (GWEN_Memory__Objects) {
    int dumpAll;
    GWEN_MEMORY__OBJECT_LIST *l;
    GWEN_MEMORY__OBJECT *o;
    GWEN_STRINGLIST *sl;
    GWEN_STRINGLISTENTRY *se;

    l=GWEN_Memory__Objects;
    GWEN_Memory__Objects=0;

    fprintf(stderr, "Memory Debugging is enabled, list follows\n");
    fprintf(stderr, "=================================================\n");

    dumpAll=(getenv(GWEN_MEMORY_ENV_DUMP_ALL)!=0);
    o=GWEN_Memory__Object_List_First(l);
    if (!o) {
      fprintf(stderr, "No objects.\n");
    }

    sl=GWEN_StringList_new();
    /* sample type names */
    while(o) {
      if (o->typeName)
        GWEN_StringList_AppendString(sl, o->typeName, 0, 1);
      else
        GWEN_StringList_AppendString(sl, "", 0, 1);
      o=GWEN_Memory__Object_List_Next(o);
    } /* while */

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *s;
      int headerShown;
      GWEN_TYPE_UINT32 constr;
      GWEN_TYPE_UINT32 destr;

      headerShown=0;
      constr=0;
      destr=0;
      s=GWEN_StringListEntry_Data(se);
      o=GWEN_Memory__Object_List_First(l);
      while(o) {
        int gotcha;

        gotcha=0;
        if (o->typeName==0) {
          if (*s==0)
            gotcha=1;
        }
        else if (strcmp(s, o->typeName)==0)
          gotcha=1;

        if (gotcha) {
          constr++;
          if (o->usage<1)
            destr++;

          /* now really print */
          if (o->usage!=0 || dumpAll) {
            if (!headerShown) {
              if (*s)
                fprintf(stderr, "Objects of type \"%s\":\n", s);
              else
                fprintf(stderr, "Objects of unspecified Type :\n");
              fprintf(stderr,
                      "-------------------------------------------------\n");
              headerShown=1;
            }
            fprintf(stderr, "%08x ",
                    (unsigned int)o->object);
            if (o->usage>0)
              fprintf(stderr,
                      "(not freed, usage left is %d, should be 0) ",
                      o->usage);
            else if (o->usage==0)
              fprintf(stderr, "(freed normally) ");
            else
              fprintf(stderr, "(usage counter is %d, should be 0) ",
                      o->usage);
            fprintf(stderr, "\n");
            if (o->locationNew)
              fprintf(stderr, " Created at   : %s\n", o->locationNew);
            if (GWEN_Memory__Object_String_List_GetCount(o->locationsAttach)){
              GWEN_MEMORY__OBJECT_STRING *so;
              int first;

              first=1;
              so=GWEN_Memory__Object_String_List_First(o->locationsAttach);
              while(so) {
                if (first) {
                  fprintf(stderr, " Attached at  : %s\n", so->text);
                  first=0;
                }
                else
                  fprintf(stderr, "                %s\n", so->text);
                so=GWEN_Memory__Object_String_List_Next(so);
              }
            } /* if locationsFree list not empty */
            if (GWEN_Memory__Object_String_List_GetCount(o->locationsFree)){
              GWEN_MEMORY__OBJECT_STRING *so;
              int first;

              first=1;
              so=GWEN_Memory__Object_String_List_First(o->locationsFree);
              while(so) {
                if (first) {
                  fprintf(stderr, " Destroyed at : %s\n", so->text);
                  first=0;
                }
                else
                  fprintf(stderr, "                %s\n", so->text);
                so=GWEN_Memory__Object_String_List_Next(so);
              }
            } /* if locationsFree list not empty */
          } /* if object should be dumped */
        } /* if object type matches currently selected one */

        o=GWEN_Memory__Object_List_Next(o);
      } /* while */

      if (headerShown) {
        fprintf(stderr,
                "-------------------------------------------------\n");
        fprintf(stderr,
                "Type summary: %d created, %d destroyed\n",
                constr, destr);
        fprintf(stderr, "\n");
      }
      else {
        fprintf(stderr,
                "Type \"%s\": %d created, %d destroyed\n",
                (s?s:"unspecified"), constr, destr);
      }
      se=GWEN_StringListEntry_Next(se);
    } /* while there still are types to dump */

    fprintf(stderr, "=================================================\n");
    fprintf(stderr, "Summary: %d objects\n",
            GWEN_Memory__Object_List_GetCount(l));
    GWEN_Memory__Objects=l;
  }
}



GWEN_MEMORY__OBJECT_STRING *GWEN_Memory__Object_String_new(const char *s) {
  GWEN_MEMORY__OBJECT_STRING *p;
  GWEN_MEMORY__OBJECT_LIST *l;

  p=(GWEN_MEMORY__OBJECT_STRING*)malloc(sizeof(GWEN_MEMORY__OBJECT_STRING));
  assert(p);
  memset(p, 0, sizeof(GWEN_MEMORY__OBJECT_STRING));
  l=GWEN_Memory__Objects;
  GWEN_Memory__Objects=0;
  GWEN_LIST_INIT(GWEN_MEMORY__OBJECT_STRING, p);
  GWEN_Memory__Objects=l;
  if (s)
    p->text=strdup(s);
  return p;
}



void GWEN_Memory__Object_String_free(GWEN_MEMORY__OBJECT_STRING *s) {
  if (s) {
    free(s->text);
    GWEN_LIST_FINI(GWEN_MEMORY__OBJECT_STRING, s);
    free(s);
  }
}




GWEN_MEMORY__OBJECT *GWEN_Memory__Object_new(void *ptr,
                                             const char *typeName,
                                             const char *location) {
  GWEN_MEMORY__OBJECT *o;
  GWEN_MEMORY__OBJECT_LIST *l;

  o=(GWEN_MEMORY__OBJECT*)malloc(sizeof(GWEN_MEMORY__OBJECT));
  assert(o);
  memset(o, 0, sizeof(GWEN_MEMORY__OBJECT));
  l=GWEN_Memory__Objects;
  GWEN_Memory__Objects=0;
  GWEN_LIST_INIT(GWEN_MEMORY__OBJECT, o);

  o->object=ptr;
  if (typeName)
    o->typeName=strdup(typeName);
  if (location)
    o->locationNew=strdup(location);
  o->locationsFree=GWEN_Memory__Object_String_List_new();
  o->locationsAttach=GWEN_Memory__Object_String_List_new();

  o->usage=1;
  GWEN_Memory__Objects=l;

  return o;
}



void GWEN_Memory__Object_free(GWEN_MEMORY__OBJECT *o) {
  if (o) {
    GWEN_MEMORY__OBJECT_LIST *l;

    if (GWEN_Memory__Objects==0) {
      fprintf(stderr, "No memory object list.\n");
      abort();
    }
    l=GWEN_Memory__Objects;
    GWEN_Memory__Objects=0;
    GWEN_Memory__Object_String_List_free(o->locationsFree);
    GWEN_Memory__Object_String_List_free(o->locationsAttach);
    GWEN_LIST_FINI(GWEN_MEMORY__OBJECT, o);
    GWEN_Memory__Objects=l;
    free(o);
  }
}



void *GWEN_Memory_NewObject(void *p,
                            const char *typeName,
                            const char *function,
                            const char *file,
                            int line){
  if (GWEN_Memory__Objects!=0) {
    GWEN_MEMORY__OBJECT *o;
    GWEN_MEMORY__OBJECT_LIST *l;
    char buffer[256];

    if (strncmp(typeName, "GWEN_MEMORY", 11)==0) {
      abort();
    }

    l=GWEN_Memory__Objects;
    GWEN_Memory__Objects=0;

    snprintf(buffer, sizeof(buffer)-1,
             "%s:%-5d (%s)", file, line, function);
    buffer[sizeof(buffer)-1]=0;

    o=GWEN_Memory__Object_new(p,
                              typeName,
                              buffer);
    GWEN_Memory__Object_List_Insert(o, l);
    /*fprintf(stderr,
            "INFO at %s: Object \"%s\" created (%d)\n",
            buffer, o->varName, GWEN_Memory__Object_List_GetCount(l));
            */
    GWEN_Memory__Objects=l;
  }
  return p;
}



void GWEN_Memory_FreeObject(void *object,
                            const char *function,
                            const char *file,
                            int line){
  if (GWEN_Memory__Objects!=0) {
    GWEN_MEMORY__OBJECT *o;
    GWEN_MEMORY__OBJECT_LIST *l;
    char buffer[256];

    l=GWEN_Memory__Objects;
    GWEN_Memory__Objects=0;

    snprintf(buffer, sizeof(buffer)-1,
             "%s:%-5d (%s)", file, line, function);
    buffer[sizeof(buffer)-1]=0;
    o=GWEN_Memory__Object_List_First(l);
    while (o) {
      if (o->object==object) {
        if (GWEN_Memory__DoubleCheck || o->usage==1) {
          if (function || file || line) {
            GWEN_MEMORY__OBJECT_STRING *s;

            s=GWEN_Memory__Object_String_new(buffer);
            GWEN_Memory__Object_String_List_Add(s, o->locationsFree);
          }
          o->usage--;
          if (o->usage<0) {
            fprintf(stderr,
                    "WARNING at %s: Object from \"%s\" already freed (%d)\n",
                    buffer, o->locationNew, o->usage);
          }
        }
        break;
      }
      o=GWEN_Memory__Object_List_Next(o);
    }
    if (!o)
      fprintf(stderr, "WARNING at %s: Object does not exist\n",
              buffer);
    GWEN_Memory__Objects=l;
  }
}



void GWEN_Memory_AttachObject(void *object,
                              const char *function,
                              const char *file,
                              int line){
  if (GWEN_Memory__Objects!=0) {
    GWEN_MEMORY__OBJECT *o;
    GWEN_MEMORY__OBJECT_LIST *l;
    char buffer[256];

    l=GWEN_Memory__Objects;
    GWEN_Memory__Objects=0;

    snprintf(buffer, sizeof(buffer)-1,
             "%s:%-5d (%s)", file, line, function);
    buffer[sizeof(buffer)-1]=0;
    o=GWEN_Memory__Object_List_First(l);
    while (o) {
      if (o->object==object) {
        if (GWEN_Memory__DoubleCheck  && o->usage>0) {
          if (function || file || line) {
            if (GWEN_Memory__DoubleCheck) {
              GWEN_MEMORY__OBJECT_STRING *s;

              s=GWEN_Memory__Object_String_new(buffer);
              GWEN_Memory__Object_String_List_Add(s, o->locationsAttach);
            }
          }
          o->usage++;
        }
        break;
      }
      o=GWEN_Memory__Object_List_Next(o);
    }
    if (!o)
      fprintf(stderr, "WARNING at %s: Object does not exist\n",
              buffer);
    GWEN_Memory__Objects=l;
  }
}





