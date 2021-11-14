/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define DISABLE_DEBUGLOG


#include "gwenbuild/buildctx/buildctx_p.h"
#include "gwenbuild/buildctx/buildctx_depfile.h"
#include "gwenbuild/buildctx/buildctx_run.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>

#include <unistd.h>
#include <ctype.h>



static GWEN_STRINGLIST *_readDepFile(const char *fileName);
static GWEN_STRINGLIST *_makeAbsolutePaths(GWEN_STRINGLIST *slInput, const char *folder);
static void _overwriteEscapedLineFeedsWithSpace(GWEN_BUFFER *buffer);



GWEN_STRINGLIST *GWB_BuildCtx_ReadAndTranslateDepfile(const char *folder, const char *fileName)
{
  GWEN_STRINGLIST *slInput;

  slInput=_readDepFile(fileName);
  if (slInput) {
    GWEN_STRINGLIST *slOutput;

    slOutput=_makeAbsolutePaths(slInput, folder);
    if (slOutput) {
      GWEN_StringList_free(slInput);
      return slOutput;
    }
    GWEN_StringList_free(slInput);
  }

  return NULL;
}



GWEN_STRINGLIST *_readDepFile(const char *fileName)
{
  GWEN_BUFFER *fileBuffer;
  int rv;
  char *s;

  fileBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_SyncIo_Helper_ReadFile(fileName, fileBuffer);
  if (rv<0) {
    DBG_ERROR(NULL, "here (%d)", rv);
    GWEN_Buffer_free(fileBuffer);
    return NULL;
  }
  _overwriteEscapedLineFeedsWithSpace(fileBuffer);
  GWEN_Text_CondenseBuffer(fileBuffer);

  s=strchr(GWEN_Buffer_GetStart(fileBuffer), ':');
  if (s) {
    GWEN_STRINGLIST *slDependencies;

    slDependencies=GWEN_StringList_fromString2(s+1, " ",
					       1,
					       GWEN_TEXT_FLAGS_DEL_QUOTES |
					       GWEN_TEXT_FLAGS_CHECK_BACKSLASH |
					       GWEN_TEXT_FLAGS_DEL_MULTIPLE_BLANKS|
					       GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS|
					       GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS);
    if (slDependencies) {
      GWEN_Buffer_free(fileBuffer);
      return slDependencies;
    }
  }
  GWEN_Buffer_free(fileBuffer);
  return NULL;
}



void _overwriteEscapedLineFeedsWithSpace(GWEN_BUFFER *buffer)
{
  char *ptr;

  ptr=GWEN_Buffer_GetStart(buffer);
  while(*ptr) {
    char *ptrNextEscape;

    ptrNextEscape=strchr(ptr, '\\');
    if (ptrNextEscape) {
      if (iscntrl(ptrNextEscape[1])) {
        /* overwrite backslash */
        *(ptrNextEscape++)=' ';
        /* overwrite every immediately following control character (such as LF, CR, TAB etc) */
        while(*ptrNextEscape && iscntrl(*ptrNextEscape))
          *(ptrNextEscape++)=' ';
      }
    }
    else
      break;
  }
}



GWEN_STRINGLIST *_makeAbsolutePaths(GWEN_STRINGLIST *slInput, const char *folder)
{
  GWEN_STRINGLISTENTRY *se;

  se=GWEN_StringList_FirstEntry(slInput);
  if (se) {
    GWEN_STRINGLIST *slOutput;

    slOutput=GWEN_StringList_new();
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s) {
	while(*s && *s<33)
	  s++;
	if (*s) {
	  if (*s=='/') {
	    GWEN_BUFFER *buf;

	    buf=GWEN_Buffer_new(0, 256, 0, 1);
	    while(*s && *s>31)
	      GWEN_Buffer_AppendByte(buf, *(s++));
	    GWEN_StringList_AppendString(slOutput, GWEN_Buffer_GetStart(buf), 0, 1);
	    GWEN_Buffer_free(buf);
	  }
	  else {
	    const char *ptrToSlash;

	    ptrToSlash=strrchr(s, '/');
	    if (ptrToSlash) {
	      GWEN_BUFFER *buf;
	      GWEN_BUFFER *absBuf;

              /* get absolute path for folder */
	      buf=GWEN_Buffer_new(0, 256, 0, 1);
	      if (folder) {
		GWEN_Buffer_AppendString(buf, folder);
		GWEN_Buffer_AppendString(buf, GWEN_DIR_SEPARATOR_S);
	      }
              GWEN_Buffer_AppendBytes(buf, s, ptrToSlash-s); /* deliberately not "+1": excluding '/' */
	      absBuf=GWEN_Buffer_new(0, 256, 0, 1);
	      GWEN_Directory_GetAbsoluteFolderPath(GWEN_Buffer_GetStart(buf), absBuf);
	      if (GWEN_Buffer_GetUsedBytes(absBuf))
		GWEN_Buffer_AppendString(absBuf, GWEN_DIR_SEPARATOR_S);

              /* add file name */
	      ptrToSlash++;   /* skip '/', ptrToSlash now points to file name */
              GWEN_Buffer_AppendString(absBuf, ptrToSlash);

              /* add complete absolute path to stringlist */
	      GWEN_StringList_AppendString(slOutput, GWEN_Buffer_GetStart(absBuf), 0, 1);
	      GWEN_Buffer_free(absBuf);
	      GWEN_Buffer_free(buf);
	    }
	  }
	}
      }

      se=GWEN_StringListEntry_Next(se);
    }
    if (GWEN_StringList_Count(slOutput)==0) {
      GWEN_StringList_free(slOutput);
      return NULL;
    }

    return slOutput;
  }

  return NULL;
}



