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


#include "c_clean.h"
#include "utils.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>

#include <unistd.h>




int GWB_Clean(const char *fname)
{
  GWB_FILE_LIST2 *fileList;

  fileList=GWB_Utils_ReadFileList2(fname);
  if (fileList) {
    GWB_FILE_LIST2_ITERATOR *it;

    it=GWB_File_List2_First(fileList);
    if (it) {
      GWB_FILE *file;
      GWEN_BUFFER *fnameBuf;

      fnameBuf=GWEN_Buffer_new(0, 256, 0, 1);
      file=GWB_File_List2Iterator_Data(it);
      while(file) {
	if (GWB_File_GetFlags(file) & GWB_FILE_FLAGS_GENERATED) {
	  const char *s;

	  s=GWB_File_GetFolder(file);
	  if (s && *s) {
	    GWEN_Buffer_AppendString(fnameBuf, s);
	    GWEN_Buffer_AppendString(fnameBuf, GWEN_DIR_SEPARATOR_S);
	  }
	  GWEN_Buffer_AppendString(fnameBuf, GWB_File_GetName(file));
	  fprintf(stdout, "Deleting '%s'\n", GWEN_Buffer_GetStart(fnameBuf));
	  unlink(GWEN_Buffer_GetStart(fnameBuf));
	  GWEN_Buffer_Reset(fnameBuf);
	}

        file=GWB_File_List2Iterator_Next(it);
      }

      GWB_File_List2Iterator_free(it);
    }
    GWB_File_List2_free(fileList);
  }

  return 0;
}




