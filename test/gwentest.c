
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gwenhyfwar/gwenhyfwar.h>
#include <gwenhyfwar/debug.h>
#include <gwenhyfwar/db.h>
#include <gwenhyfwar/xml.h>
#include <gwenhyfwar/msgengine.h>
#include <gwenhyfwar/text.h>
#include <gwenhyfwar/cmdlayer.h>


int testDB(int argc, char **argv) {
  GWEN_DB_NODE *cfg;
  int rv;
  const char *p;

  fprintf(stderr,"Creating DB\n");
  cfg=GWEN_DB_Group_new("Config");

  fprintf(stderr, "Setting char values\n");
  rv=GWEN_DB_SetCharValue(cfg,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "testgroup/charvar",
                          "charvalue1");
  rv=GWEN_DB_SetCharValue(cfg,
                          0,
                          "testgroup/charvar",
                          "charvalue2");

  fprintf(stderr, "Retrieving char values\n");
  p=GWEN_DB_GetCharValue(cfg,
                         "testgroup/charvar", 0,
                         "defaultValue");
  fprintf(stderr," Retrieved value 1 is: %s\n", p);

  p=GWEN_DB_GetCharValue(cfg,
                         "testgroup/charvar", 1,
                         "defaultValue");
  fprintf(stderr," Retrieved value 2 is: %s\n", p);

  fprintf(stderr, "Setting int values\n");
  rv=GWEN_DB_SetIntValue(cfg,
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "testgroup/intvar",
                         11);

  rv=GWEN_DB_SetIntValue(cfg,
                         0,
                         "testgroup/intvar",
                         22);
  fprintf(stderr, "Retrieving int values\n");
  rv=GWEN_DB_GetIntValue(cfg,
                         "testgroup/intvar", 0,
                         -1);
  fprintf(stderr," Retrieved int value 1 is: %d\n", rv);

  rv=GWEN_DB_GetIntValue(cfg,
                         "testgroup/intvar", 1,
                         -1);
  fprintf(stderr," Retrieved int value 2 is: %d\n", rv);


  fprintf(stderr,"Releasing DB\n");
  GWEN_DB_Group_free(cfg);
  return 0;
}



int testXML(int argc, char **argv) {
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  if (GWEN_XML_ReadFile(n, "test.xml")) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }
  fprintf(stderr, "XML file:\n");
  GWEN_XMLNode_Dump(n, stderr, 2);
  GWEN_XMLNode_free(n);
  return 0;
}



int testMsg(int argc, char **argv) {
  GWEN_XMLNODE *n;
  GWEN_MSGENGINE *e;
  GWEN_XMLNODE *sn;
  GWEN_DB_NODE *da;
  GWEN_DB_NODE *din;
  GWEN_BUFFER *gbuf;

  e=GWEN_MsgEngine_new();
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  da=GWEN_DB_Group_new("Data");
  din=GWEN_DB_Group_new("ParsedData");
  gbuf=GWEN_Buffer_new(0, 1024,0,1);

  if (GWEN_XML_ReadFile(n, "test.xml")) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }

  GWEN_MsgEngine_SetDefinitions(e, n);
  sn=GWEN_MsgEngine_FindNodeByProperty(e,
                                       "SEG",
                                       "code",
                                       1,
                                       "PING");
  if (!sn) {
    fprintf(stderr, "Segment not found.\n");
    return 2;
  }

  fprintf(stderr, "Node:\n");
  GWEN_XMLNode_Dump(sn, stderr, 2);

  GWEN_DB_SetCharValue(da,
                       GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "name",
                       "Gwenhyfwar-Test");
  GWEN_DB_SetCharValue(da,
                       GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "version",
                       "0.0.1");

  if (GWEN_MsgEngine_CreateMessageFromNode(e,
                                           sn,
                                           gbuf,
                                           da)) {
    fprintf(stderr, "Error creating message.\n");
    return 3;
  }

  GWEN_Text_DumpString(GWEN_Buffer_GetStart(gbuf),
                       GWEN_Buffer_GetUsedBytes(gbuf),
                       stderr, 1);

  GWEN_Buffer_SetPos(gbuf, 0);
  if (GWEN_MsgEngine_ParseMessage(e,
                                  sn,
                                  gbuf,
                                  din)) {
    fprintf(stderr, "Error parsing message.\n");
    return 3;
  }

  GWEN_Buffer_free(gbuf);
  GWEN_MsgEngine_free(e);
  GWEN_DB_Group_free(da);
  GWEN_DB_Group_free(din);
  return 0;
}




int testService(int argc, char **argv) {
  GWEN_IPCSERVICECMD *s;
  GWEN_ERRORCODE err;
  unsigned int connId;
  int i;
  GWEN_IPCMSG *msg;

  fprintf(stderr, "Initializing service\n");
  s=GWEN_IPCServiceCmd_new();
  err=GWEN_IPCServiceCmd_Init(s, "test.xml");
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 1;
  }
  fprintf(stderr, "Initializing service: done\n");


  fprintf(stderr, "Adding listener\n");
  connId=GWEN_IPCServiceCmd_AddListener(s, "127.0.0.1", 44444, 1);
  if (connId==0) {
    fprintf(stderr, "Could not add listener\n");
    return 2;
  }
  fprintf(stderr, "New listener added (id=%d)\n", connId);

  for (i=0; 1; i++) {
    fprintf(stderr, "\n\nWorking (loop %d)...\n\n", i);
    err=GWEN_IPCServiceCmd_Work(s, 60*1000);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      return 1;
    }
    msg=GWEN_IPCServiceCmd_GetRequest(s);
    if (msg) {
      fprintf(stderr, "Got a request.\n");
      GWEN_Buffer_Dump(GWEN_Msg_GetBuffer(msg), stderr, 2);
      GWEN_Msg_free(msg);
    }
    else {
      fprintf(stderr, "No request.\n");
    }
    fprintf(stderr, "Working done\n");
  }

  fprintf(stderr, "Deinitializing service\n");
  err=GWEN_IPCServiceCmd_Fini(s);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 1;
  }
  GWEN_IPCServiceCmd_free(s);
  fprintf(stderr, "Deinitializing service: done\n");

  return 0;
}



int testClient(int argc, char **argv) {
  GWEN_IPCSERVICECMD *s;
  GWEN_ERRORCODE err;
  unsigned int connId;
  int i;
  GWEN_IPCCONNLAYER *cl;

  fprintf(stderr, "Initializing service\n");
  s=GWEN_IPCServiceCmd_new();
  err=GWEN_IPCServiceCmd_Init(s, "test.xml");
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 1;
  }
  fprintf(stderr, "Initializing service: done\n");


  fprintf(stderr, "Adding peer\n");
  connId=GWEN_IPCServiceCmd_AddPeer(s, "127.0.0.1", 44444, 1);
  if (connId==0) {
    fprintf(stderr, "Could not add peer\n");
    return 2;
  }
  fprintf(stderr, "New peer added (id=%d)\n", connId);

  cl=GWEN_IPCServiceCmd_FindConnection(s, connId, 0);
  if (!cl) {
    fprintf(stderr, "Haeh ? Connection %d not found ?!\n", connId);
    return 3;
  }

  err=GWEN_ConnectionLayer_Open(cl);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 3;
  }

  for (i=0; i< 12; i++) {
    fprintf(stderr, "\n\nWorking (loop %d)...\n\n", i);
    err=GWEN_IPCServiceCmd_Work(s, 10*1000);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      return 1;
    }
    fprintf(stderr, "Working done\n");
  }

  fprintf(stderr, "Deinitializing service\n");
  err=GWEN_IPCServiceCmd_Fini(s);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 1;
  }
  GWEN_IPCServiceCmd_free(s);
  fprintf(stderr, "Deinitializing service: done\n");

  return 0;
}



int testPing(int argc, char **argv) {
  GWEN_IPCSERVICECMD *s;
  GWEN_ERRORCODE err;
  unsigned int connId;
  int i;
  GWEN_IPCMSG *pingMsg;
  GWEN_DB_NODE *pingDb;

  fprintf(stderr, "Initializing service\n");
  s=GWEN_IPCServiceCmd_new();
  err=GWEN_IPCServiceCmd_Init(s, "test.xml");
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 1;
  }
  fprintf(stderr, "Initializing service: done\n");


  fprintf(stderr, "Adding peer\n");
  connId=GWEN_IPCServiceCmd_AddPeer(s, "127.0.0.1", 44444, 1);
  if (connId==0) {
    fprintf(stderr, "Could not add peer\n");
    return 2;
  }
  fprintf(stderr, "New peer added (id=%d)\n", connId);

  pingDb=GWEN_DB_Group_new("pingdb");
  pingMsg=GWEN_IPCServiceCmd_CreateMsg(s,
                                       connId,
                                       0,
                                       "Ping",
                                       0,
                                       pingDb);
  if (!pingMsg) {
    fprintf(stderr, "Could not create ping message\n");
    return 2;
  }

  fprintf(stderr, "Sending PING message\n");
  err=GWEN_IPCServiceCmd_SendMessage(s, pingMsg);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 1;
  }
  fprintf(stderr, "Sending PING message: done.\n");


  for (i=0; i< 12; i++) {
    fprintf(stderr, "\n\nWorking (loop %d)...\n\n", i);
    err=GWEN_IPCServiceCmd_Work(s, 10*1000);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      return 1;
    }
    fprintf(stderr, "Working done\n");
  }

  fprintf(stderr, "Deinitializing service\n");
  err=GWEN_IPCServiceCmd_Fini(s);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 1;
  }
  GWEN_IPCServiceCmd_free(s);
  fprintf(stderr, "Deinitializing service: done\n");

  return 0;
}



int main(int argc, char **argv) {
  GWEN_ERRORCODE err;
  int rv;

  if (argc<2) {
    fprintf(stderr, "Usage: %s client|server\n", argv[0]);
    return 1;
  }

  fprintf(stderr, "Initializing Gwenhywfar\n");
  err=GWEN_Init();
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 1;
  }
  fprintf(stderr, "Gwenhywfar initialized\n");

  //rv=testDB(argc, argv);
  //rv=testXML(argc, argv);
  //rv=testMsg(argc, argv);
  if (strcasecmp(argv[1], "server")==0)
    rv=testService(argc, argv);
  else if (strcasecmp(argv[1], "client")==0)
    rv=testClient(argc, argv);
  else if (strcasecmp(argv[1], "ping")==0)
    rv=testPing(argc, argv);
  else {
    fprintf(stderr, "Unknown command \"%s\"", argv[1]);
    return 1;
  }

  fprintf(stderr, "Deinitializing Gwenhywfar\n");
  err=GWEN_Fini();
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 1;
  }
  fprintf(stderr, "Gwenhywfar deinitialized\n");
  return rv;
}

