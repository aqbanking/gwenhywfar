/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Apr 24 2004
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

#include "args_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <string.h>

#define DISABLE_DEBUGLOG




int GWEN_Args_Check(int argc, char **argv,
                    int startAt,
                    GWEN_TYPE_UINT32 mode,
                    GWEN_ARGS *args,
                    GWEN_DB_NODE *db){
  int i;
  const char *p;
  GWEN_ARGS *tmpArgs;

  i=startAt;

  /* reset count fields of all options */
  for(tmpArgs=args;;tmpArgs++) {
    tmpArgs->count=0;

    if (tmpArgs->flags & GWEN_ARGS_FLAGS_LAST)
      break;
  } /* for */

  while(i<argc) {
    GWEN_ARGS_ELEMENT_TYPE t;
    char *tmpBuf;
    const char *v;
    int value;

    DBG_INFO(0, "Argument[%d] is \"%s\"", i, argv[i]);
    p=argv[i];
    if (*p=='-') {
      p++;
      if (*p=='-') {
        p++;
        t=GWEN_ArgsElementTypeLong;
      }
      else
        t=GWEN_ArgsElementTypeShort;
    }
    else
      t=GWEN_ArgsElementTypeFreeParam;

    switch(t) {
    case GWEN_ArgsElementTypeFreeParam:
      if (mode & GWEN_ARGS_MODE_ALLOW_FREEPARAM) {
        GWEN_DB_SetCharValue(db,
                             GWEN_DB_FLAGS_DEFAULT |
                             GWEN_PATH_FLAGS_CREATE_VAR,
                             "params/param", p);
        i++;
      }
      else {
        DBG_ERROR(0, "Only options are allowed");
        return GWEN_ARGS_RESULT_ERROR;
      }
      if (mode & GWEN_ARGS_MODE_STOP_AT_FREEPARAM) {
        DBG_DEBUG(0, "Free parameter found, stopping as requested");
        return i;
      }
      break;

    case GWEN_ArgsElementTypeShort:
      for(tmpArgs=args;;tmpArgs++) {
        if (strcasecmp(tmpArgs->shortOption, p)==0) {
          /* found option */
          tmpArgs->count++;
          break;
        }

        if (tmpArgs->flags & GWEN_ARGS_FLAGS_LAST) {
          DBG_ERROR(0, "Unknown short option \"%s\"", p);
          return GWEN_ARGS_RESULT_ERROR;
        }
      } /* for */
      i++;

      if (tmpArgs->flags & GWEN_ARGS_FLAGS_HAS_ARGUMENT) {
        /* argument needed */
        if (i>=argc) {
          DBG_ERROR(0, "Argument needed for option \"%s\"", tmpArgs->name);
          return GWEN_ARGS_RESULT_ERROR;
        }
        switch(tmpArgs->type) {
        case GWEN_ArgsTypeChar:
          GWEN_DB_SetCharValue(db,
                               GWEN_DB_FLAGS_DEFAULT |
                               GWEN_PATH_FLAGS_CREATE_VAR,
                               tmpArgs->name, argv[i]);
          break;

        case GWEN_ArgsTypeInt:
          if (sscanf(argv[i], "%i", &value)!=1) {
            DBG_ERROR(0, "Non-integer argument for short option \"%s\"", p);
            return GWEN_ARGS_RESULT_ERROR;
          }
          GWEN_DB_SetIntValue(db,
                              GWEN_DB_FLAGS_DEFAULT |
                              GWEN_PATH_FLAGS_CREATE_VAR,
                              tmpArgs->name, value);
          break;

        default:
          DBG_ERROR(0, "Unknown option type \"%d\"", tmpArgs->type);
          return GWEN_ARGS_RESULT_ERROR;
        } /* switch */
        i++;
      }
      else {
        GWEN_DB_SetIntValue(db,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            tmpArgs->name, tmpArgs->count);
      }
      break;

    case GWEN_ArgsElementTypeLong:
      /* copy option name up to (but excluding) the "=" if any,
       * determine the start of possible argument */
      v=p;
      while(*v && *v!='=') v++;
      tmpBuf=(char*)malloc(v-p+1);
      assert(tmpBuf);
      memmove(tmpBuf, p, v-p);
      tmpBuf[v-p]=0;

      for(tmpArgs=args;;tmpArgs++) {
        if (strcasecmp(tmpArgs->longOption, tmpBuf)==0) {
          /* found option */
          tmpArgs->count++;
          break;
        }

        if (tmpArgs->flags & GWEN_ARGS_FLAGS_LAST) {
          DBG_ERROR(0, "Unknown long option \"%s\"", tmpBuf);
          free(tmpBuf);
          return GWEN_ARGS_RESULT_ERROR;
        }
      } /* for */
      i++;

      if (*v=='=') {
        if (!(tmpArgs->flags & GWEN_ARGS_FLAGS_HAS_ARGUMENT)) {
          DBG_ERROR(0, "No argument allowed for option \"%s\"",
                    tmpArgs->name);
          free(tmpBuf);
          return GWEN_ARGS_RESULT_ERROR;
        }
        v++;
      }

      if (tmpArgs->flags & GWEN_ARGS_FLAGS_HAS_ARGUMENT) {
        /* argument needed */
        if (*v==0) {
          DBG_ERROR(0, "Argument needed for option \"%s\"", tmpArgs->name);
          free(tmpBuf);
          return GWEN_ARGS_RESULT_ERROR;
        }
        switch(tmpArgs->type) {
        case GWEN_ArgsTypeChar:
          GWEN_DB_SetCharValue(db,
                               GWEN_DB_FLAGS_DEFAULT |
                               GWEN_PATH_FLAGS_CREATE_VAR,
                               tmpArgs->name, v);
          break;

        case GWEN_ArgsTypeInt:
          if (sscanf(v, "%i", &value)!=1) {
            DBG_ERROR(0, "Non-integer argument for long option \"%s\"",
                      tmpBuf);
            free(tmpBuf);
            return GWEN_ARGS_RESULT_ERROR;
          }
          GWEN_DB_SetIntValue(db,
                              GWEN_DB_FLAGS_DEFAULT |
                              GWEN_PATH_FLAGS_CREATE_VAR,
                              tmpArgs->name, value);
          break;

        default:
          DBG_ERROR(0, "Unknown option type \"%d\"", tmpArgs->type);
          return GWEN_ARGS_RESULT_ERROR;
        } /* switch */
      }
      else {
        GWEN_DB_SetIntValue(db,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            tmpArgs->name, tmpArgs->count);
      }
      free(tmpBuf);

      break;

    default:
      DBG_ERROR(0, "Internal error (unknown argv type \"%d\")", t);
      return GWEN_ARGS_RESULT_ERROR;
      break;
    } /* switch */
  } /* while */

  /* check argument counts */
  for(tmpArgs=args;;tmpArgs++) {
    const char *s;

    if (tmpArgs->longOption)
      s=tmpArgs->longOption;
    else
      s=tmpArgs->shortOption;

    /* check minnum */
    if (tmpArgs->minNum && (tmpArgs->count<tmpArgs->minNum)) {
      if (tmpArgs->minNum>1) {
        DBG_ERROR(0, "Option \"%s\" needed %d times (have %d)",
                  s, tmpArgs->minNum, tmpArgs->count);
      }
      else {
        DBG_ERROR(0, "Option \"%s\" needed", s);
      }
      return GWEN_ARGS_RESULT_ERROR;
    }

    /* check maxnum */
    if (tmpArgs->maxNum && (tmpArgs->count>tmpArgs->maxNum)) {
      DBG_ERROR(0, "Option \"%s\" needed at most %d times (have %d)",
                s, tmpArgs->maxNum, tmpArgs->count);
      return GWEN_ARGS_RESULT_ERROR;
    }

    if (tmpArgs->flags & GWEN_ARGS_FLAGS_LAST)
      break;
  } /* for */

  return i;
}


int GWEN_Args__AppendTXT(GWEN_BUFFER *ubuf, const char *s, unsigned int ins){
  unsigned int i;

  while(*s) {
    for (i=0; i<ins; i++) GWEN_Buffer_AppendByte(ubuf, ' ');
    while(*s) {
      char c;

      c=*s;
      s++;
      GWEN_Buffer_AppendByte(ubuf, c);
      if (c=='\n')
        break;
    } /* while */
  } /* while */

  return 0;
}



int GWEN_Args_UsageTXT(GWEN_ARGS *args, GWEN_BUFFER *ubuf){
  GWEN_ARGS *tmpArgs;

  for(tmpArgs=args;;tmpArgs++) {
    const char *s;

    GWEN_Buffer_AppendString(ubuf, "\n");
    if (tmpArgs->shortOption || tmpArgs->longOption) {
      if (tmpArgs->shortOption) {
        GWEN_Buffer_AppendString(ubuf, " ");
        if (tmpArgs->minNum==0)
          GWEN_Buffer_AppendString(ubuf, "[");
        else
          GWEN_Buffer_AppendString(ubuf, " ");
        GWEN_Buffer_AppendString(ubuf, "-");
        GWEN_Buffer_AppendString(ubuf, tmpArgs->shortOption);
        if (tmpArgs->flags & GWEN_ARGS_FLAGS_HAS_ARGUMENT)
          GWEN_Buffer_AppendString(ubuf, " PARAM");
        if (tmpArgs->minNum==0)
          GWEN_Buffer_AppendString(ubuf, "]");
        GWEN_Buffer_AppendString(ubuf, "\n");
      } /* if short option */

      if (tmpArgs->longOption) {
        GWEN_Buffer_AppendString(ubuf, " ");
        if (tmpArgs->minNum==0)
          GWEN_Buffer_AppendString(ubuf, "[");
        else
          GWEN_Buffer_AppendString(ubuf, " ");
        GWEN_Buffer_AppendString(ubuf, "--");
        GWEN_Buffer_AppendString(ubuf, tmpArgs->longOption);
        if (tmpArgs->flags & GWEN_ARGS_FLAGS_HAS_ARGUMENT)
          GWEN_Buffer_AppendString(ubuf, "=PARAM");
        if (tmpArgs->minNum==0)
          GWEN_Buffer_AppendString(ubuf, "]");
        GWEN_Buffer_AppendString(ubuf, "\n");
      } /* if short option */

      s=tmpArgs->longDescription;
      if (!s)
        s=tmpArgs->shortDescription;

      if (s) {
        GWEN_Args__AppendTXT(ubuf, s, 3);
        GWEN_Buffer_AppendString(ubuf, "\n");
      }
    } /* if any option */
    else {
      DBG_ERROR(0, "Option \"%s\" has neither a long nor a short name",
                tmpArgs->name);
      return -1;
    }

    if (tmpArgs->flags & GWEN_ARGS_FLAGS_LAST)
      break;
  } /* for */

  return 0;
}



int GWEN_Args_UsageHTML(GWEN_ARGS *args, GWEN_BUFFER *ubuf){
  return 0;
}



int GWEN_Args_Usage(GWEN_ARGS *args, GWEN_BUFFER *ubuf, GWEN_ARGS_OUTTYPE ot){
  int rv;

  switch(ot) {
  case GWEN_ArgsOutTypeTXT:
    rv=GWEN_Args_UsageTXT(args, ubuf);
    break;
  case GWEN_ArgsOutTypeHTML:
    rv=GWEN_Args_UsageHTML(args, ubuf);
    break;
  default:
    DBG_ERROR(0, "Unknown output type %d", ot);
    rv=-1;
  } /* switch */

  return rv;
}



int GWEN_Args_ShortUsage(GWEN_ARGS *args, GWEN_BUFFER *ubuf,
                         GWEN_ARGS_OUTTYPE ot){
  return 0;
}







