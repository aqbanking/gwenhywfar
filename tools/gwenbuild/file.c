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
#include <gwenhywfar/buffer.h>

#include <string.h>



GWEN_LIST2_FUNCTIONS(GWB_FILE, GWB_File)


static void _writeFileFlagsToXml(uint32_t flags, GWEN_XMLNODE *xmlNode, const char *varName);
static uint32_t _readFlagsFromChar(const char *flagsAsText);




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



GWB_FILE *GWB_File_dup(const GWB_FILE *oldFile)
{
  if (oldFile) {
    GWB_FILE *fileOut;

    fileOut=GWB_File_new(oldFile->folder, oldFile->name, 0);
    GWB_File_SetFileType(fileOut, oldFile->fileType);
    GWB_File_SetInstallPath(fileOut, oldFile->installPath);
    GWB_File_SetFlags(fileOut, oldFile->flags);
    return fileOut;
  }

  return NULL;
}



void GWB_File_free(GWB_FILE *f)
{
  if (f) {
    GWB_BuildCmd_List2_free(f->waitingBuildCmdList2);
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



const char *GWB_File_GetExt(const GWB_FILE *f)
{
  if (f->name)
    return (const char*) strrchr(f->name, '.');
  return NULL;
}



const char *GWB_File_GetInstallPath(const GWB_FILE *f)
{
  return f->installPath;
}



void GWB_File_SetInstallPath(GWB_FILE *f, const char *s)
{
  if (f->installPath)
    free(f->installPath);
  if (s && *s)
    f->installPath=strdup(s);
  else
    f->installPath=NULL;
}



const char *GWB_File_GetFileType(const GWB_FILE *f)
{
  return f->fileType;
}



void GWB_File_SetFileType(GWB_FILE *f, const char *s)
{
  if (f->fileType)
    free(f->fileType);
  if (s && *s)
    f->fileType=strdup(s);
  else
    f->fileType=NULL;
}



GWB_BUILD_CMD_LIST2 *GWB_File_GetWaitingBuildCmdList2(const GWB_FILE *f)
{
  return f->waitingBuildCmdList2;
}



void GWB_File_AddWaitingBuildCmd(GWB_FILE *f, GWB_BUILD_CMD *bcmd)
{
  if (f->waitingBuildCmdList2==NULL)
    f->waitingBuildCmdList2=GWB_BuildCmd_List2_new();
  GWB_BuildCmd_List2_PushBack(f->waitingBuildCmdList2, bcmd);
}



void GWB_File_ClearWaitingBuildCmds(GWB_FILE *f)
{
  if (f->waitingBuildCmdList2)
    GWB_BuildCmd_List2_Clear(f->waitingBuildCmdList2);
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



void GWB_File_ReplaceExtension(GWB_FILE *file, const char *newExt)
{
  const char *s;

  s=file->name;
  if (s && *s) {
    const char *ext;
    GWEN_BUFFER *buf;

    buf=GWEN_Buffer_new(0, 64, 0, 1);
    ext=strrchr(s, '.');
    if (ext) {
      int len;

      len=(ext-s); /* exclude "." */
      if (len) {
        GWEN_Buffer_AppendBytes(buf, s, len);
      }
    }
    GWEN_Buffer_AppendString(buf, newExt);
    GWB_File_SetName(file, GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
  }
}



GWB_FILE *GWB_File_CopyObjectAndChangeExtension(const GWB_FILE *file, const char *newExt)
{
  GWB_FILE *fileOut;
  const char *s1;
  const char *s2;

  fileOut=GWB_File_dup(file);
  GWB_File_ReplaceExtension(fileOut, newExt);
  s1=GWB_File_GetName(file);
  s2=GWB_File_GetName(fileOut);
  if (strcasecmp(s1, s2)==0) {
    DBG_ERROR(NULL, "Output file has the same name as input file (%s)!", s1);
    GWB_File_free(fileOut);
    return NULL;
  }

  return fileOut;
}



GWB_FILE *GWB_File_List2_GetFileByPathAndName(const GWB_FILE_LIST2 *fileList, const char *folder, const char *fname)
{
  GWB_FILE_LIST2_ITERATOR *it;

  it=GWB_File_List2_First(fileList);
  if (it) {
    GWB_FILE *file;

    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      const char *currentName;

      currentName=GWB_File_GetName(file);
      if (currentName && *currentName && strcasecmp(currentName, fname)==0) {
        const char *currentFolder;

        currentFolder=GWB_File_GetFolder(file);
        if (currentFolder && *currentFolder && strcasecmp(currentFolder, folder)==0) {
          GWB_File_List2Iterator_free(it);
          return file;
        }
      }
      file=GWB_File_List2Iterator_Next(it);
    }
    GWB_File_List2Iterator_free(it);
  }

  return NULL;
}



GWB_FILE *GWB_File_List2_GetFileById(const GWB_FILE_LIST2 *fileList, uint32_t id)
{
  GWB_FILE_LIST2_ITERATOR *it;

  it=GWB_File_List2_First(fileList);
  if (it) {
    GWB_FILE *file;

    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      if (GWB_File_GetId(file)==id) {
        GWB_File_List2Iterator_free(it);
        return file;
      }
      file=GWB_File_List2Iterator_Next(it);
    }
    GWB_File_List2Iterator_free(it);
  }

  return NULL;
}



void GWB_File_AddFileList2ToFileList2(GWB_FILE_LIST2 *sourceList, GWB_FILE_LIST2 *destList, const char *ext)
{
  GWB_FILE_LIST2_ITERATOR *it;

  it=GWB_File_List2_First(sourceList);
  if (it) {
    GWB_FILE *file;

    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      if (ext && *ext) {
	const char *s;

	s=GWB_File_GetExt(file);
        if (s && strcasecmp(s, ext)==0) {
          GWB_File_List2_PushBack(destList, file);
        }
      }
      else
	GWB_File_List2_PushBack(destList, file);
      file=GWB_File_List2Iterator_Next(it);
    }
    GWB_File_List2Iterator_free(it);
  }
}



void GWB_File_toXml(const GWB_FILE *file, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNode_SetIntProperty(xmlNode, "id", (int) (file->id));
  if (file->folder)
    GWEN_XMLNode_SetCharValue(xmlNode, "folder", file->folder);
  if (file->name)
    GWEN_XMLNode_SetCharValue(xmlNode, "name", file->name);
  if (file->fileType)
    GWEN_XMLNode_SetCharValue(xmlNode, "type", file->fileType);
  if (file->installPath)
    GWEN_XMLNode_SetCharValue(xmlNode, "installPath", file->installPath);
  _writeFileFlagsToXml(GWB_File_GetFlags(file), xmlNode, "flags");
}



GWB_FILE *GWB_File_fromXml(GWEN_XMLNODE *xmlNode)
{
  uint32_t id;
  GWB_FILE *file;
  const char *folder;
  const char *name;
  const char *s;

  id=(uint32_t) GWEN_XMLNode_GetIntProperty(xmlNode, "id", 0);

  folder=GWEN_XMLNode_GetCharValue(xmlNode, "folder", NULL);
  name=GWEN_XMLNode_GetCharValue(xmlNode, "name", NULL);

  file=GWB_File_new(folder, name, id);
  s=GWEN_XMLNode_GetCharValue(xmlNode, "flags", NULL);
  if (s)
    file->flags=_readFlagsFromChar(s);
  GWB_File_SetFileType(file, GWEN_XMLNode_GetCharValue(xmlNode, "type", NULL));
  GWB_File_SetInstallPath(file, GWEN_XMLNode_GetCharValue(xmlNode, "installPath", NULL));

  return file;
}



void _writeFileFlagsToXml(uint32_t flags, GWEN_XMLNODE *xmlNode, const char *varName)
{
  if (flags) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);

    if (flags & GWB_FILE_FLAGS_DIST) {
      if (GWEN_Buffer_GetUsedBytes(dbuf))
        GWEN_Buffer_AppendString(dbuf, " ");
      GWEN_Buffer_AppendString(dbuf, "DIST");
    }

    if (flags & GWB_FILE_FLAGS_INSTALL) {
      if (GWEN_Buffer_GetUsedBytes(dbuf))
        GWEN_Buffer_AppendString(dbuf, " ");
      GWEN_Buffer_AppendString(dbuf, "INSTALL");
    }

    if (flags & GWB_FILE_FLAGS_GENERATED) {
      if (GWEN_Buffer_GetUsedBytes(dbuf))
        GWEN_Buffer_AppendString(dbuf, " ");
      GWEN_Buffer_AppendString(dbuf, "GENERATED");
    }

    GWEN_XMLNode_SetCharValue(xmlNode, varName, GWEN_Buffer_GetStart(dbuf));
    GWEN_Buffer_free(dbuf);
  }
}



uint32_t _readFlagsFromChar(const char *flagsAsText)
{
  GWEN_STRINGLIST *sl;
  uint32_t flags=0;

  sl=GWEN_StringList_fromString(flagsAsText, " ", 1);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      if (s && *s) {
        if (strcasecmp(s, "DIST")==0)
          flags|=GWB_FILE_FLAGS_DIST;
        else if (strcasecmp(s, "INSTALL")==0)
          flags|=GWB_FILE_FLAGS_INSTALL;
        else if (strcasecmp(s, "GENERATED")==0)
          flags|=GWB_FILE_FLAGS_GENERATED;
        else {
          DBG_ERROR(NULL, "Unexpected FILE flag \"%s\"", s);
        }
      }
      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_StringList_free(sl);
  }

  return flags;
}



void GWB_File_List2_WriteXml(const GWB_FILE_LIST2 *fileList, GWEN_XMLNODE *xmlNode, const char *groupName)
{
  GWB_FILE_LIST2_ITERATOR *it;

  it=GWB_File_List2_First(fileList);
  if (it) {
    GWB_FILE *file;

    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      GWEN_XMLNODE *entryNode;

      entryNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, groupName);
      GWB_File_toXml(file, entryNode);
      GWEN_XMLNode_AddChild(xmlNode, entryNode);
      file=GWB_File_List2Iterator_Next(it);
    }
    GWB_File_List2Iterator_free(it);
  }
}



void GWB_File_List2_ReadXml(GWEN_XMLNODE *xmlNode, const char *groupName, GWB_FILE_LIST2 *destFileList)
{
  GWEN_XMLNODE *xmlEntry;

  xmlEntry=GWEN_XMLNode_FindFirstTag(xmlNode, groupName, NULL, NULL);
  while(xmlEntry) {
    GWB_FILE *file;

    file=GWB_File_fromXml(xmlEntry);
    if (file)
      GWB_File_List2_PushBack(destFileList, file);
    xmlEntry=GWEN_XMLNode_FindNextTag(xmlEntry, groupName, NULL, NULL);
  }
}





