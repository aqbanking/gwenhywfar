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

#include "gwenbuild/buildctx/buildsubcmd_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>


GWEN_LIST_FUNCTIONS(GWB_BUILD_SUBCMD, GWB_BuildSubCmd);




GWB_BUILD_SUBCMD *GWB_BuildSubCmd_new(void)
{
  GWB_BUILD_SUBCMD *cmd;

  GWEN_NEW_OBJECT(GWB_BUILD_SUBCMD, cmd);
  GWEN_LIST_INIT(GWB_BUILD_SUBCMD, cmd);

  return cmd;
}



GWB_BUILD_SUBCMD *GWB_BuildSubCmd_dup(const GWB_BUILD_SUBCMD *origCmd)
{
  if (origCmd==NULL)
    return NULL;
  else {
    GWB_BUILD_SUBCMD *cmd;

    cmd=GWB_BuildSubCmd_new();
    cmd->flags=origCmd->flags;
    cmd->command=(origCmd->command)?strdup(origCmd->command):NULL;
    cmd->arguments=(origCmd->arguments)?strdup(origCmd->arguments):NULL;
    cmd->buildMessage=(origCmd->buildMessage)?strdup(origCmd->buildMessage):NULL;
    cmd->depFilePath=(origCmd->depFilePath)?strdup(origCmd->depFilePath):NULL;

    return cmd;
  }

}



void GWB_BuildSubCmd_free(GWB_BUILD_SUBCMD *cmd)
{
  if (cmd) {
    GWEN_LIST_FINI(GWB_BUILD_SUBCMD, cmd);
    free(cmd->command);
    free(cmd->arguments);
    free(cmd->buildMessage);

    GWEN_FREE_OBJECT(cmd);
  }
}



uint32_t GWB_BuildSubCmd_GetFlags(const GWB_BUILD_SUBCMD *cmd)
{
  return cmd->flags;
}



void GWB_BuildSubCmd_SetFlags(GWB_BUILD_SUBCMD *cmd, uint32_t f)
{
  cmd->flags=f;
}



void GWB_BuildSubCmd_AddFlags(GWB_BUILD_SUBCMD *cmd, uint32_t f)
{
  cmd->flags|=f;
}



void GWB_BuildSubCmd_DelFlags(GWB_BUILD_SUBCMD *cmd, uint32_t f)
{
  cmd->flags&=~f;
}



const char *GWB_BuildSubCmd_GetCommand(const GWB_BUILD_SUBCMD *cmd)
{
  return cmd->command;
}



void GWB_BuildSubCmd_SetCommand(GWB_BUILD_SUBCMD *cmd, const char *s)
{
  free(cmd->command);
  cmd->command=s?strdup(s):NULL;
}



const char *GWB_BuildSubCmd_GetArguments(const GWB_BUILD_SUBCMD *cmd)
{
  return cmd->arguments;
}



void GWB_BuildSubCmd_SetArguments(GWB_BUILD_SUBCMD *cmd, const char *s)
{
  free(cmd->arguments);
  cmd->arguments=s?strdup(s):NULL;
}



const char *GWB_BuildSubCmd_GetBuildMessage(const GWB_BUILD_SUBCMD *cmd)
{
  return cmd->buildMessage;
}



void GWB_BuildSubCmd_SetBuildMessage(GWB_BUILD_SUBCMD *cmd, const char *s)
{
  free(cmd->buildMessage);
  cmd->buildMessage=s?strdup(s):NULL;
}



const char *GWB_BuildSubCmd_GetDepFilePath(const GWB_BUILD_SUBCMD *cmd)
{
  return cmd->depFilePath;
}



void GWB_BuildSubCmd_SetDepFilePath(GWB_BUILD_SUBCMD *cmd, const char *s)
{
  free(cmd->depFilePath);
  cmd->depFilePath=s?strdup(s):NULL;
}




void GWB_BuildSubCmd_toXml(const GWB_BUILD_SUBCMD *cmd, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNode_SetIntValue(xmlNode, "flags", cmd->flags);

  if (cmd->command)
    GWEN_XMLNode_SetCharValue(xmlNode, "command", cmd->command);

  if (cmd->arguments)
    GWEN_XMLNode_SetCharValue(xmlNode, "arguments", cmd->arguments);

  if (cmd->buildMessage)
    GWEN_XMLNode_SetCharValue(xmlNode, "buildMessage", cmd->buildMessage);

  if (cmd->depFilePath)
    GWEN_XMLNode_SetCharValue(xmlNode, "depFilePath", cmd->depFilePath);
}



GWB_BUILD_SUBCMD *GWB_BuildSubCmd_fromXml(GWEN_XMLNODE *xmlNode)
{
  GWB_BUILD_SUBCMD *cmd;
  const char *s;

  cmd=GWB_BuildSubCmd_new();

  cmd->flags=GWEN_XMLNode_GetIntValue(xmlNode, "flags", 0);

  s=GWEN_XMLNode_GetCharValue(xmlNode, "command", NULL);
  GWB_BuildSubCmd_SetCommand(cmd, s);

  s=GWEN_XMLNode_GetCharValue(xmlNode, "arguments", NULL);
  GWB_BuildSubCmd_SetArguments(cmd, s);

  s=GWEN_XMLNode_GetCharValue(xmlNode, "buildMessage", NULL);
  GWB_BuildSubCmd_SetBuildMessage(cmd, s);

  s=GWEN_XMLNode_GetCharValue(xmlNode, "depFilePath", NULL);
  GWB_BuildSubCmd_SetDepFilePath(cmd, s);

  return cmd;
}



GWB_BUILD_SUBCMD_LIST *GWB_BuildSubCmd_List_dup(const GWB_BUILD_SUBCMD_LIST *cmdList)
{
  GWB_BUILD_SUBCMD_LIST *newList;
  GWB_BUILD_SUBCMD *cmd;

  newList=GWB_BuildSubCmd_List_new();
  cmd=GWB_BuildSubCmd_List_First(cmdList);
  while(cmd) {
    GWB_BuildSubCmd_List_Add(GWB_BuildSubCmd_dup(cmd), newList);
    cmd=GWB_BuildSubCmd_List_Next(cmd);
  }

  return newList;
}



void GWB_BuildSubCmd_Dump(const GWB_BUILD_SUBCMD *cmd, int indent)
{
  if (cmd) {
    int i;

    for(i=0; i<indent; i++)
      fprintf(stderr, " ");
    fprintf(stderr, "Command    : %s\n", (cmd->command)?(cmd->command):"<empty>");

    for(i=0; i<indent; i++)
      fprintf(stderr, " ");
    fprintf(stderr, "Arguments  : %s\n", (cmd->arguments)?(cmd->arguments):"<empty>");

    for(i=0; i<indent; i++)
      fprintf(stderr, " ");
    fprintf(stderr, "Message    : %s\n", (cmd->buildMessage)?(cmd->buildMessage):"<empty>");

    for(i=0; i<indent; i++)
      fprintf(stderr, " ");
    fprintf(stderr, "DepFilePath: %s\n", (cmd->depFilePath)?(cmd->depFilePath):"<empty>");
  }
}



void GWB_BuildSubCmd_List_Dump(const GWB_BUILD_SUBCMD_LIST *cmdList, int indent, const char *text)
{
  int i;

  if (text) {
    for(i=0; i<indent; i++)
      fprintf(stderr, " ");
    fprintf(stderr, "%s\n", text);
  }
  if (cmdList) {
    const GWB_BUILD_SUBCMD *cmd;

    cmd=GWB_BuildSubCmd_List_First(cmdList);
    while(cmd) {
      for(i=0; i<indent+2; i++)
        fprintf(stderr, " ");
      fprintf(stderr, "Sub Command:\n");
      GWB_BuildSubCmd_Dump(cmd, indent+4);
      cmd=GWB_BuildSubCmd_List_Next(cmd);
    }
  }
  else {
    for(i=0; i<indent+2; i++)
      fprintf(stderr, " ");
    fprintf(stderr, "<empty>\n");
  }
}



