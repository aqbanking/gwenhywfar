
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
#include <gwenhyfwar/hbcidialog.h>
#include <gwenhyfwar/hbcimsg.h>
#include <gwenhyfwar/ipcxmlsecctx.h>
#include <gwenhyfwar/ipcxmlkeymanager.h>
#include <gwenhyfwar/ipcxml.h>


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



int testDBfile(int argc, char **argv) {
  GWEN_DB_NODE *db;

  fprintf(stderr,"Creating DB\n");
  db=GWEN_DB_Group_new("Config");

  fprintf(stderr,"Reading file\n");
  if (GWEN_DB_ReadFile(db, "test.db", 0)) {
    fprintf(stderr,"Error reading file.\n");
    return 1;
  }
  fprintf(stderr, "DB is:\n");
  GWEN_DB_Dump(db, stderr, 2);
  fprintf(stderr,"Releasing DB\n");
  GWEN_DB_Group_free(db);
  return 0;
}



int testDBfile2(int argc, char **argv) {
  GWEN_DB_NODE *db;

  if (argc<4) {
    fprintf(stderr, "%s dbfile2 src dest\n", argv[0]);
    return 1;
  }
  fprintf(stderr,"Creating DB\n");
  db=GWEN_DB_Group_new("Config");

  fprintf(stderr,"Reading file\n");
  if (GWEN_DB_ReadFile(db, argv[2], 0)) {
    fprintf(stderr,"Error reading file.\n");
    return 1;
  }
  fprintf(stderr, "DB is:\n");
  GWEN_DB_Dump(db, stderr, 2);

  if (GWEN_DB_WriteFile(db, argv[3], GWEN_DB_FLAGS_DEFAULT)) {
    fprintf(stderr,"Error writing file.\n");
    return 1;
  }

  fprintf(stderr,"Releasing DB\n");
  GWEN_DB_Group_free(db);
  return 0;
}




int testXML(int argc, char **argv) {
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  if (GWEN_XML_ReadFile(n, "test.xml", GWEN_XML_FLAGS_DEFAULT)) {
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

  if (GWEN_XML_ReadFile(n, "test.xml", GWEN_XML_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }

  GWEN_MsgEngine_SetDefinitions(e, n, 1);
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
                                  din,
                                  GWEN_MSGENGINE_READ_FLAGS_TRUSTINFO)) {
    fprintf(stderr, "Error parsing message.\n");
    return 3;
  }

  GWEN_Buffer_free(gbuf);
  GWEN_MsgEngine_free(e);
  GWEN_DB_Group_free(da);
  GWEN_DB_Group_free(din);
  return 0;
}



int testListMsg(int argc, char **argv) {
  GWEN_XMLNODE *n;
  GWEN_MSGENGINE *e;
  GWEN_XMLNODE *sn;
  const char *segname;

  if (argc<3) {
    fprintf(stderr, "Segment name needed.\n");
    return 1;
  }
  segname=argv[2];

  e=GWEN_MsgEngine_new();
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");

  if (GWEN_XML_ReadFile(n, "test.xml", GWEN_XML_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }

  GWEN_MsgEngine_SetDefinitions(e, n, 1);

  sn=GWEN_MsgEngine_ListMessage(e,
                                "SEG",
                                segname,
                                0,
                                /*GWEN_MSGENGINE_SHOW_FLAGS_NOSET*/ 0);
  if (!sn) {
    fprintf(stderr, "Error listing message.\n");
    return 3;
  }

  fprintf(stderr, "Node:\n");
  GWEN_XMLNode_Dump(sn, stderr, 2);

  GWEN_MsgEngine_free(e);
  return 0;
}




int testKey(int argc, char **argv) {
  GWEN_CRYPTKEY *key;
  GWEN_ERRORCODE err;
  GWEN_BUFFER *srcbuf;
  GWEN_BUFFER *dstbuf;
  GWEN_BUFFER *rawbuf;
  int i;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelInfo);

  if (argc<3) {
    fprintf(stderr, "Data needed\n");
    return 1;
  }
  key=GWEN_CryptKey_Factory("RSA");
  if (!key) {
    fprintf(stderr, "Could not create key\n");
    return 2;
  }

  fprintf(stderr, "Generating key...\n");
  err=GWEN_CryptKey_Generate(key, 768);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }
  fprintf(stderr, "Generating key done\n");

  srcbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  GWEN_Buffer_ReserveBytes(srcbuf, 128);
  dstbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  rawbuf=GWEN_Buffer_new(0, 1024, 0, 1);

  GWEN_Buffer_AppendBytes(srcbuf, argv[2], strlen(argv[2]));
  fprintf(stderr, "Buffer before padding:\n");
  GWEN_Text_DumpString(GWEN_Buffer_GetStart(srcbuf),
                       GWEN_Buffer_GetUsedBytes(srcbuf),
                       stderr, 2);

  GWEN_Buffer_SetPos(srcbuf, 0);
  i=GWEN_CryptKey_GetChunkSize(key)-GWEN_Buffer_GetUsedBytes(srcbuf);
  fprintf(stderr, "Inserting %d bytes\n", i);
  while(i-->0) {
    if (GWEN_Buffer_InsertByte(srcbuf, (char)0)) {
      fprintf(stderr, "Could not insert byte\n");
      return 2;
    }
  } /* while */

  fprintf(stderr, "Buffer after padding:\n");
  GWEN_Text_DumpString(GWEN_Buffer_GetStart(srcbuf),
                       GWEN_Buffer_GetUsedBytes(srcbuf),
                       stderr, 2);

  fprintf(stderr, "Encrypting\n");
  err=GWEN_CryptKey_Encrypt(key, srcbuf, dstbuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }
  fprintf(stderr, "Encrypting done.\n");

  fprintf(stderr, "Buffer after encryption:\n");
  GWEN_Text_DumpString(GWEN_Buffer_GetStart(dstbuf),
                       GWEN_Buffer_GetUsedBytes(dstbuf),
                       stderr, 2);

  fprintf(stderr, "Decrypting\n");
  GWEN_Buffer_SetPos(dstbuf, 0);
  err=GWEN_CryptKey_Decrypt(key, dstbuf, rawbuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }
  fprintf(stderr, "Decrypting done.\n");

  fprintf(stderr, "Buffer after decryption:\n");
  GWEN_Text_DumpString(GWEN_Buffer_GetStart(rawbuf),
                       GWEN_Buffer_GetUsedBytes(rawbuf),
                       stderr, 2);

  fprintf(stderr, "Signing.\n");
  GWEN_Buffer_Reset(dstbuf);
  GWEN_Buffer_Rewind(srcbuf);
  err=GWEN_CryptKey_Sign(key, srcbuf, dstbuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }
  fprintf(stderr, "Signing done.\n");
  fprintf(stderr, "Buffer after signing:\n");
  GWEN_Text_DumpString(GWEN_Buffer_GetStart(dstbuf),
                       GWEN_Buffer_GetUsedBytes(dstbuf),
                       stderr, 2);

  fprintf(stderr, "Verifying.\n");
  GWEN_Buffer_Rewind(srcbuf);
  GWEN_Buffer_Rewind(dstbuf);
  err=GWEN_CryptKey_Verify(key, srcbuf, dstbuf);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }
  fprintf(stderr, "Verifying done.\n");

  return 0;
}



int testDialog(int argc, char **argv) {
  GWEN_XMLNODE *n;
  GWEN_MSGENGINE *e;
  GWEN_XMLNODE *sn;
  GWEN_DB_NODE *da;
  GWEN_HBCIDIALOG *dlg;
  GWEN_CRYPTKEY *key;
  GWEN_ERRORCODE err;
  GWEN_HBCIMSG *hmsg;
  GWEN_SECCTX_MANAGER *scm;
  GWEN_SECCTX *sc;
  unsigned int requestId;

  if (argc<3) {
    fprintf(stderr, "Path of XML file needed.\n");
    return 1;
  }
  e=GWEN_MsgEngine_new();
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  da=GWEN_DB_Group_new("Data");

  if (GWEN_XML_ReadFile(n, argv[2], GWEN_XML_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }

  GWEN_MsgEngine_SetDefinitions(e, n, 1);
  GWEN_MsgEngine_SetProtocolVersion(e, 1);
  GWEN_MsgEngine_SetMode(e, "RDH");

  scm=GWEN_SecContextMgr_new("TestService-1");

  key=GWEN_CryptKey_Factory("RSA");
  if (!key) {
    fprintf(stderr, "Error creating key.\n");
    return 1;
  }

  GWEN_CryptKey_SetOwner(key, "martin");
  GWEN_CryptKey_SetKeyName(key, "B");

  fprintf(stderr, "Generating key.\n");
  err=GWEN_CryptKey_Generate(key, 768);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }
  fprintf(stderr, "Generating key done.\n");

  sc=GWEN_IPCXMLSecCtx_new("martin");
  GWEN_IPCXMLSecCtx_AddKey(sc, GWEN_CryptKey_dup(key));
  GWEN_IPCXMLSecCtx_SetSignSeq(sc, 4554);
  if (GWEN_SecContextMgr_AddContext(scm, sc)) {
    fprintf(stderr, "Could not add context.\n");
    return 2;
  }
  fprintf(stderr, "Context added.\n");

  dlg=GWEN_HBCIDialog_new(e, scm);
  GWEN_HBCIDialog_SetFlags(dlg, GWEN_HBCIDIALOG_FLAGS_INITIATOR);

  hmsg=GWEN_HBCIMsg_new(dlg);
  GWEN_HBCIMsg_SetMsgNumber(hmsg, 1);

  sn=GWEN_MsgEngine_FindNodeByProperty(e,
                                       "SEG",
                                       "code",
                                       1,
                                       "GBTEST");
  if (!sn) {
    fprintf(stderr, "Segment not found.\n");
    return 2;
  }

  GWEN_HBCIMsg_SetFlags(hmsg,
                        GWEN_HBCIMSG_FLAGS_SIGN /*|
                        GWEN_HBCIMSG_FLAGS_CRYPT*/);

  GWEN_HBCIMsg_AddSigner(hmsg,
                         GWEN_CryptKey_GetKeySpec(key));
  GWEN_HBCIMsg_SetCrypter(hmsg,
                          GWEN_CryptKey_GetKeySpec(key));

  requestId=GWEN_HBCIMsg_AddNode(hmsg, sn, da);
  if (requestId==0) {
    fprintf(stderr, "Could not add node.\n");
    return 2;
  }

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelInfo);

  fprintf(stderr, "Encoding message\n");
  if (GWEN_HBCIMsg_EncodeMsg(hmsg)) {
    fprintf(stderr, "Could not encode.\n");
    return 2;
  }
  fprintf(stderr, "Encoding message done\n");

  fprintf(stderr, "Buffer is: \n");
  GWEN_Buffer_Dump(GWEN_HBCIMsg_GetBuffer(hmsg), stderr, 2);

  GWEN_HBCIDialog_SetFlags(dlg, 0);
  fprintf(stderr, "Decoding message\n");
  if (GWEN_HBCIMsg_DecodeMsg(hmsg,
                             da, 0)) {
    fprintf(stderr, "Error decoding.\n");
    return 1;
  }
  fprintf(stderr, "Decoding message: done\n");
  GWEN_DB_Dump(da, stderr, 2);

  GWEN_MsgEngine_free(e);
  GWEN_DB_Group_free(da);
  GWEN_HBCIMsg_free(hmsg);
  GWEN_HBCIDialog_free(dlg);
  GWEN_CryptKey_free(key);

  return 0;
}



int testServer(int argc, char **argv) {
  GWEN_XMLNODE *n;
  GWEN_MSGENGINE *e;
  GWEN_XMLNODE *sn;
  GWEN_DB_NODE *da;
  GWEN_DB_NODE *keydb;
  GWEN_HBCIDIALOG *dlg;
  GWEN_CRYPTKEY *key;
  GWEN_ERRORCODE err;
  GWEN_HBCIMSG *hmsg;
  GWEN_SECCTX_MANAGER *scm;
  GWEN_SECCTX *sc;
  unsigned int requestId;
  GWEN_IPCXMLSERVICE *service;
  unsigned int serverId;

  if (argc<4) {
    fprintf(stderr, "Path of XML file and key file needed.\n");
    return 1;
  }
  e=GWEN_MsgEngine_new();
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  da=GWEN_DB_Group_new("Data");

  if (GWEN_XML_ReadFile(n, argv[2], GWEN_XML_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }

  GWEN_MsgEngine_SetDefinitions(e, n, 1);
  GWEN_MsgEngine_SetProtocolVersion(e, 1);
  GWEN_MsgEngine_SetMode(e, "RDH");

  scm=GWEN_SecContextMgr_new("TestService-1");

  key=GWEN_CryptKey_Factory("RSA");
  if (!key) {
    fprintf(stderr, "Error creating key.\n");
    return 1;
  }

  keydb=GWEN_DB_Group_new("key");
  if (GWEN_DB_ReadFile(keydb, argv[3], GWEN_DB_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error reading file \"%s\"", argv[3]);
    return 2;
  }

  err=GWEN_CryptKey_FromDb(key, keydb);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }
  GWEN_DB_Group_free(keydb);
  keydb=0;

  sc=GWEN_IPCXMLSecCtx_new(GWEN_CryptKey_GetOwner(key));
  GWEN_IPCXMLSecCtx_AddKey(sc, GWEN_CryptKey_dup(key));
  GWEN_IPCXMLSecCtx_SetSignSeq(sc, 4554);
  if (GWEN_SecContextMgr_AddContext(scm, sc)) {
    fprintf(stderr, "Could not add context.\n");
    return 2;
  }
  fprintf(stderr, "Context added.\n");

  fprintf(stderr, "Creating service.\n");
  service=GWEN_IPCXMLService_new(e, scm);
  fprintf(stderr, "Creating service: done.\n");

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelInfo);

  fprintf(stderr, "Creating server.\n");
  serverId=GWEN_IPCXMLService_AddServer(service,
                                        GWEN_IPCXMLServiceTypeTCP,
                                        "martin",
                                        1,
                                        "192.168.115.2",
                                        44444,
                                        0);
  if (!serverId) {
    fprintf(stderr, "Error.\n");
    return 2;
  }
  fprintf(stderr, "Creating server: done.\n");

  for (;;) {
    int chr;

    fprintf(stderr, "Hit ENTER to Work (or ESC and ENTER to abort)\n");
    chr=getchar();
    if (chr==27)
      break;
    err=GWEN_IPCXMLService_Work(service, 1000);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
    }
    err=GWEN_IPCXMLService_HandleMsgs(service, 0, 1);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
    }
  } /* for */

  fprintf(stderr, "Exit.\n");
  GWEN_IPCXMLService_free(service);
  return 0;
}



int testClient(int argc, char **argv) {
  GWEN_XMLNODE *n;
  GWEN_MSGENGINE *e;
  GWEN_XMLNODE *sn;
  GWEN_DB_NODE *da;
  GWEN_DB_NODE *keydb;
  GWEN_HBCIDIALOG *dlg;
  GWEN_CRYPTKEY *key;
  GWEN_ERRORCODE err;
  GWEN_HBCIMSG *hmsg;
  GWEN_SECCTX_MANAGER *scm;
  GWEN_SECCTX *sc;
  unsigned int requestId;
  GWEN_IPCXMLSERVICE *service;
  unsigned int serverId;

  if (argc<4) {
    fprintf(stderr, "Path of XML file and key file needed.\n");
    return 1;
  }
  e=GWEN_MsgEngine_new();
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  da=GWEN_DB_Group_new("Data");

  if (GWEN_XML_ReadFile(n, argv[2], GWEN_XML_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }

  GWEN_MsgEngine_SetDefinitions(e, n, 1);
  GWEN_MsgEngine_SetProtocolVersion(e, 1);
  GWEN_MsgEngine_SetMode(e, "RDH");

  scm=GWEN_SecContextMgr_new("TestService-1");

  key=GWEN_CryptKey_Factory("RSA");
  if (!key) {
    fprintf(stderr, "Error creating key.\n");
    return 1;
  }

  keydb=GWEN_DB_Group_new("key");
  if (GWEN_DB_ReadFile(keydb, argv[3], GWEN_DB_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error reading file \"%s\"", argv[3]);
    return 2;
  }

  err=GWEN_CryptKey_FromDb(key, keydb);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }
  GWEN_DB_Group_free(keydb);
  keydb=0;

  sc=GWEN_IPCXMLSecCtx_new(GWEN_CryptKey_GetOwner(key));
  GWEN_IPCXMLSecCtx_AddKey(sc, GWEN_CryptKey_dup(key));
  GWEN_IPCXMLSecCtx_SetSignSeq(sc, 4554);
  if (GWEN_SecContextMgr_AddContext(scm, sc)) {
    fprintf(stderr, "Could not add context.\n");
    return 2;
  }
  fprintf(stderr, "Context added.\n");

  fprintf(stderr, "Creating service.\n");
  service=GWEN_IPCXMLService_new(e, scm);
  fprintf(stderr, "Creating service: done.\n");

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelInfo);

  fprintf(stderr, "Creating client.\n");
  serverId=GWEN_IPCXMLService_AddClient(service,
                                        GWEN_IPCXMLServiceTypeTCP,
                                        "martin",
                                        1,
                                        "192.168.115.2",
                                        44444,
                                        0);
  if (!serverId) {
    fprintf(stderr, "Error.\n");
    return 2;
  }
  fprintf(stderr, "Creating client: done.\n");

  err=GWEN_IPCXMLService_SetSecurityEnv(service,
                                        serverId,
                                        GWEN_CryptKey_GetKeySpec(key),
                                        GWEN_CryptKey_GetKeySpec(key));
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  fprintf(stderr, "Adding request...\n");
  requestId=GWEN_IPCXMLService_AddRequest(service,
                                          1,
                                          "Test",
                                          0,
                                          da,
                                          1);
  if (!requestId) {
    DBG_ERROR(0, "No request created.");
    return 2;
  }

  for (;;) {
    int chr;

    fprintf(stderr, "Hit ENTER to Work (or ESC and ENTER to abort)\n");
    chr=getchar();
    if (chr==27)
      break;
    fprintf(stderr, "Working...\n");
    err=GWEN_IPCXMLService_Work(service, 1000);
    fprintf(stderr, "Working... done.\n");
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
    }
  } /* for */

  fprintf(stderr, "Exit.\n");
  GWEN_IPCXMLService_free(service);
  return 0;
}





int testMkKey(int argc, char **argv) {
  GWEN_CRYPTKEY *key;
  GWEN_ERRORCODE err;
  GWEN_DB_NODE *db;

  if (argc<3) {
    fprintf(stderr, "Path of key file needed.\n");
    return 1;
  }

  key=GWEN_CryptKey_Factory("RSA");
  if (!key) {
    fprintf(stderr, "Error creating key.\n");
    return 1;
  }

  GWEN_CryptKey_SetOwner(key, "martin");
  GWEN_CryptKey_SetKeyName(key, "B");

  fprintf(stderr, "Generating key.\n");
  err=GWEN_CryptKey_Generate(key, 768);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }
  fprintf(stderr, "Generating key done.\n");

  db=GWEN_DB_Group_new("key");
  err=GWEN_CryptKey_ToDb(key,
                         db, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  if (GWEN_DB_WriteFile(db, argv[2], GWEN_DB_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error writing file \"%s\"", argv[2]);
    return 2;
  }

  return 0;
}


int testCopyKey(int argc, char **argv) {
  GWEN_CRYPTKEY *key;
  GWEN_ERRORCODE err;
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *db2;

  if (argc<4) {
    fprintf(stderr, "Path of souce and destination key files needed.\n");
    return 1;
  }

  key=GWEN_CryptKey_Factory("RSA");
  if (!key) {
    fprintf(stderr, "Error creating key.\n");
    return 1;
  }

  db=GWEN_DB_Group_new("key");
  if (GWEN_DB_ReadFile(db, argv[2], GWEN_DB_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error reading file \"%s\"", argv[2]);
    return 2;
  }

  err=GWEN_CryptKey_FromDb(key, db);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }
  GWEN_DB_Group_free(db);

  db2=GWEN_DB_Group_new("key");
  err=GWEN_CryptKey_ToDb(key,
                         db2, 0);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }
  if (GWEN_DB_WriteFile(db2, argv[3], GWEN_DB_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error writing file \"%s\"", argv[3]);
    return 2;
  }

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
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelNotice);
  fprintf(stderr, "Gwenhywfar initialized\n");

  if (strcasecmp(argv[1], "dbfile")==0)
    rv=testDBfile(argc, argv);
  else if (strcasecmp(argv[1], "dbfile2")==0)
    rv=testDBfile2(argc, argv);
  else if (strcasecmp(argv[1], "list")==0)
    rv=testListMsg(argc, argv);
  else if (strcasecmp(argv[1], "key")==0)
    rv=testKey(argc, argv);
  else if (strcasecmp(argv[1], "dlg")==0)
    rv=testDialog(argc, argv);
  else if (strcasecmp(argv[1], "server")==0)
    rv=testServer(argc, argv);
  else if (strcasecmp(argv[1], "client")==0)
    rv=testClient(argc, argv);
  else if (strcasecmp(argv[1], "mkkey")==0)
    rv=testMkKey(argc, argv);
  else if (strcasecmp(argv[1], "cpkey")==0)
    rv=testCopyKey(argc, argv);
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

