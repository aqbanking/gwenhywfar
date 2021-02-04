/***************************************************************************
 begin       : Fri Feb 07 2003
 copyright   : (C) 2021 by Martin Preuss
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

/* included from gui.c */


int GWEN_Gui_ConvertString(const char *text, size_t len, GWEN_BUFFER *tbuf,
                           const char *fromCs, const char *toCs)
{
#ifdef HAVE_ICONV
  int rv=0;
  iconv_t ic;

  assert(tbuf);

  ic=iconv_open(toCs, fromCs);
  if (ic==(iconv_t)-1) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Cannot convert from \"%s\" to \"%s\", %s",
              fromCs, toCs, strerror(errno));
    rv=GWEN_ERROR_GENERIC;
  }
  else {
    /* Some systems have iconv in libc, some have it in libiconv
       (OSF/1 and those with the standalone portable GNU libiconv
       installed). Check which one is available. The define
       ICONV_CONST will be "" or "const" accordingly. */
    ICONV_CONST char *pInbuf;
    char *pOutbuf;
    size_t inLeft;
    size_t outLeft;
    size_t done;
    size_t space;

    /* convert */
    pInbuf=(char *)text;
    inLeft=len;
    outLeft=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf);
    space=outLeft;
  retry:
    pOutbuf=GWEN_Buffer_GetPosPointer(tbuf);
    done=iconv(ic, &pInbuf, &inLeft, &pOutbuf, &outLeft);
    GWEN_Buffer_SetPos(tbuf, space-outLeft);
    GWEN_Buffer_AdjustUsedBytes(tbuf);
    if (done==(size_t)-1) {
      if (errno==E2BIG) {
        uint32_t room;

        room=2*inLeft;
        if (room<=outLeft)
          room+=outLeft;
        GWEN_Buffer_AllocRoom(tbuf, room);
        /* How much additional room has actually been allocated? */
        room=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf)-outLeft;
        outLeft+=room;
        space+=room;
        goto retry;
      }

      DBG_ERROR(GWEN_LOGDOMAIN, "Error in conversion: %s (%d)",
                strerror(errno), errno);
      rv=GWEN_ERROR_GENERIC;
    }
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Conversion done.");
    }
    iconv_close(ic);
  }

  return rv;
#else /* !HAVE_ICONV */
  return GWEN_Buffer_AppendBytes(tbuf, text, len);
#endif
}



int GWEN_Gui_ConvertFromUtf8(const GWEN_GUI *gui, const char *text, int len, GWEN_BUFFER *tbuf)
{
  int rv;

  rv=GWEN_Gui_ConvertString(text, len, tbuf, "UTF-8", gui->charSet);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int GWEN_Gui_StdPrintf(const GWEN_GUI *gui, FILE *stream,
                       const char *fmt, ...)
{
  va_list args;
  int rv;

  assert(gui);

  va_start(args, fmt);
#ifndef HAVE_ICONV
  rv=vfprintf(stream, fmt, args);
#else
  if (!gui->charSet)
    rv=vfprintf(stream, fmt, args);
  else {
    GWEN_BUFFER *tbuf;
    GWEN_BUFFER *outbuf;
    int bufLen;
    size_t spaceNeeded;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    bufLen=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf)-1;
    spaceNeeded=vsnprintf(GWEN_Buffer_GetStart(tbuf), bufLen, fmt, args);
    if (spaceNeeded==(size_t) -1) {
      fprintf(stderr, "GWEN INTERNAL ERROR: vsnprintf returned -1 on fmt=\"%s\"?\n", fmt);
      va_end(args);
      return EOF;
    }
    if (spaceNeeded>=bufLen) {
      GWEN_Buffer_AllocRoom(tbuf, spaceNeeded+1);
      bufLen=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf)-1;
      va_end(args);
      va_start(args, fmt);
      //vsprintf(GWEN_Buffer_GetStart(tbuf), fmt, args);
      spaceNeeded=vsnprintf(GWEN_Buffer_GetStart(tbuf), bufLen, fmt, args);
      if (spaceNeeded>=bufLen) {
        fprintf(stderr, "GWEN INTERNAL ERROR: Still not enough space (%lu >=%lu)? SNH!\n",
                (long unsigned int) spaceNeeded, (long unsigned int) bufLen);
        assert(spaceNeeded<bufLen);
      }
    }
    GWEN_Buffer_IncrementPos(tbuf, spaceNeeded);
    GWEN_Buffer_AdjustUsedBytes(tbuf);

    outbuf=GWEN_Buffer_new(0, 2*spaceNeeded, 0, 1);
    rv=GWEN_Gui_ConvertString(GWEN_Buffer_GetStart(tbuf), spaceNeeded,
                              outbuf, "UTF-8", gui->charSet);
    if (rv) {
      GWEN_Buffer_free(outbuf);
      outbuf=tbuf;
    }
    else
      GWEN_Buffer_free(tbuf);

    /* let's try to return the same value as fprintf() would */
    if (fputs(GWEN_Buffer_GetStart(outbuf), stderr)!=EOF)
      rv=spaceNeeded;
    else
      rv=EOF;

    GWEN_Buffer_free(outbuf);
  }
#endif

  va_end(args);
  return rv;
}



int GWEN_Gui_ReadString(const char *text, GWEN_BUFFER *tbuf)
{
#ifdef HAVE_ICONV
  if (gwenhywfar_gui) {
    const char *fromCs;

    if (gwenhywfar_gui->charSet)
      fromCs=gwenhywfar_gui->charSet;
    else
      /* UTF-8 to UTF-8 conversion does not seem to make much sense, but
       * it is a convenient way to check whether the input text actually
       * is properly UTF-8 encoded.
       */
      fromCs="UTF-8";

    return GWEN_Gui_ConvertString(text, strlen(text), tbuf, fromCs, "UTF-8");
  }
  else
#endif /* HAVE_ICONV */
    return GWEN_Buffer_AppendString(tbuf, text);
}



void GWEN_Gui_GetRawText(GWEN_UNUSED const GWEN_GUI *gui, const char *text, GWEN_BUFFER *tbuf)
{
  size_t len;
  const char *p;

  assert(text);
  p=text;
  while ((p=strchr(p, '<'))) {
    const char *t;

    t=p;
    t++;
    if (toupper(*t)=='H') {
      t++;
      if (toupper(*t)=='T') {
        t++;
        if (toupper(*t)=='M') {
          t++;
          if (toupper(*t)=='L') {
            break;
          }
        }
      }
    }
    p++;
  } /* while */

  if (p)
    len=p-text;
  else
    len=strlen(text);
  GWEN_Buffer_AppendBytes(tbuf, text, len);
}

