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


#include "gwenbuild/project_p.h"

#include <gwenhywfar/memory.h>




GWB_PROJECT *GWB_Project_new(GWENBUILD *gwbuild, GWB_CONTEXT *ctx)
{
  GWB_PROJECT *project;

  GWEN_NEW_OBJECT(GWB_PROJECT, project);

  project->gwbuild=gwbuild;
  project->contextTree=ctx;

  project->fileList=GWB_File_List2_new();
  project->targetList=GWB_Target_List2_new();
  project->builderList=GWB_Builder_List2_new();

  return project;
}



void GWB_Project_free(GWB_PROJECT *project)
{
  if (project) {
    GWB_File_List2_free(project->fileList);
    GWB_Context_free(project->contextTree);
    GWB_Target_List2_free(project->targetList);
    GWB_Builder_List2_free(project->builderList);

    GWEN_FREE_OBJECT(project);
  }
}



GWENBUILD *GWB_Project_GetGwbuild(const GWB_PROJECT *project)
{
  return project->gwbuild;
}



const char *GWB_Project_GetProjectName(const GWB_PROJECT *project)
{
  return project->projectName;
}



void GWB_Project_SetProjectName(GWB_PROJECT *project, const char *s)
{
  if (project->projectName)
    free(project->projectName);
  if (s)
    project->projectName=strdup(s);
  else
    project->projectName=NULL;
}



void GWB_Project_SetVersion(GWB_PROJECT *project, int vMajor, int vMinor, int vPatchlevel, int vBuild)
{
  project->versionMajor=vMajor;
  project->versionMinor=vMinor;
  project->versionPatchlevel=vPatchlevel;
  project->versionBuild=vBuild;
}



int GWB_Project_GetVersionMajor(const GWB_PROJECT *project)
{
  return project->versionMajor;
}



int GWB_Project_GetVersionMinor(const GWB_PROJECT *project)
{
  return project->versionMinor;
}



int GWB_Project_GetVersionPatchlevel(const GWB_PROJECT *project)
{
  return project->versionPatchlevel;
}



int GWB_Project_GetVersionBuild(const GWB_PROJECT *project)
{
  return project->versionBuild;
}



void GWB_Project_SetSoVersion(GWB_PROJECT *project, int vCurrent, int vAge, int vRevision)
{
  project->soVersionCurrent=vCurrent;
  project->soVersionAge=vAge;
  project->soVersionRevision=vRevision;
}



int GWB_Project_GetSoVersionCurrent(const GWB_PROJECT *project)
{
  return project->soVersionCurrent;
}



int GWB_Project_GetSoVersionAge(const GWB_PROJECT *project)
{
  return project->soVersionAge;
}



int GWB_Project_GetSoVersionRevision(const GWB_PROJECT *project)
{
  return project->soVersionRevision;
}





GWB_FILE *GWB_Project_GetFileByPathAndName(const GWB_PROJECT *project, const char *folder, const char *fname)
{
  GWB_FILE_LIST2_ITERATOR *it;

  it=GWB_File_List2_First(project->fileList);
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



void GWB_Project_AddFile(GWB_PROJECT *project, GWB_FILE *file)
{
  GWB_File_List2_PushBack(project->fileList, file);
}



GWB_CONTEXT *GWB_Project_GetRootContext(const GWB_PROJECT *project)
{
  return project->contextTree;
}




GWB_TARGET_LIST2 *GWB_Project_GetTargetList(const GWB_PROJECT *project)
{
  return project->targetList;
}



void GWB_Project_AddTarget(GWB_PROJECT *project, GWB_TARGET *target)
{
  GWB_Target_List2_PushBack(project->targetList, target);
}



GWB_BUILDER_LIST2 *GWB_Project_GetBuilderList(const GWB_PROJECT *project)
{
  return project->builderList;
}



void GWB_Project_AddBuilder(GWB_PROJECT *project, GWB_BUILDER *builder)
{
  GWB_Builder_List2_PushBack(project->builderList, builder);
}





