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


#include "gwenbuild/file_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/memory.h>



GWEN_LIST2_FUNCTIONS(GWB_FILE, GWB_File)



GWB_FILE *GWB_File_new(const char *folder, const char *fName, uint32_t id)
{
  GWB_FILE *f;

  GWEN_NEW_OBJECT(GWB_FILE, f);
  if (folder && *folder)
    GWB_File_SetFolder(f, folder);
  if (fName && *fName)
    GWB_File_SetName(f, fName);
  f->id=id;

  return f;
}



void GWB_File_free(GWB_FILE *f)
{
  if (f) {
    GWB_Generator_List2_free(f->waitingGeneratorList2);
    free(f->folder);
    free(f->name);

    GWEN_FREE_OBJECT(f);
  }
}



uint32_t GWB_File_GetId(const GWB_FILE *f)
{
  return f->id;
}



void GWB_File_SetId(GWB_FILE *f, uint32_t i)
{
  f->id=i;
}



uint32_t GWB_File_GetFlags(const GWB_FILE *f)
{
  return f->flags;
}



void GWB_File_SetFlags(GWB_FILE *f, uint32_t i)
{
  f->flags=i;
}



void GWB_File_AddFlags(GWB_FILE *f, uint32_t i)
{
  f->flags|=i;
}



void GWB_File_DelFlags(GWB_FILE *f, uint32_t i)
{
  f->flags&=~i;
}



const char *GWB_File_GetFolder(const GWB_FILE *f)
{
  return f->folder;
}



void GWB_File_SetFolder(GWB_FILE *f, const char *s)
{
  if (f->folder)
    free(f->folder);
  if (s && *s)
    f->folder=strdup(s);
  else
    f->folder=NULL;
}



const char *GWB_File_GetName(const GWB_FILE *f)
{
  return f->name;
}



void GWB_File_SetName(GWB_FILE *f, const char *s)
{
  if (f->name)
    free(f->name);
  if (s && *s)
    f->name=strdup(s);
  else
    f->name=NULL;
}



GWB_GENERATOR_LIST2 *GWB_File_GetWaitingGeneratorList2(const GWB_FILE *f)
{
  return f->waitingGeneratorList2;
}



void GWB_File_AddWaitingGenerator(GWB_FILE *f, GWB_GENERATOR *gen)
{
  if (f->waitingGeneratorList2==NULL)
    f->waitingGeneratorList2=GWB_Generator_List2_new();
  GWB_Generator_List2_PushBack(f->waitingGeneratorList2, gen);
}



void GWB_File_List2_FreeAll(GWB_FILE_LIST2 *fileList2)
{
  if (fileList2) {
    GWB_FILE_LIST2_ITERATOR *it;

    it=GWB_File_List2_First(fileList2);
    if (it) {
      GWB_FILE *f;

      f=GWB_File_List2Iterator_Data(it);
      while(f) {
	GWB_File_free(f);
	f=GWB_File_List2Iterator_Next(it);
      }
    }
    GWB_File_List2_free(fileList2);
  }
}




