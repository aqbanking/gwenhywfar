
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifndef OS_WIN32
# include <unistd.h>
#endif
#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/md.h>
#include <gwenhywfar/crypt.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/msgengine.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/nl_ssl.h>
#include <gwenhywfar/process.h>
#include <gwenhywfar/args.h>
#include <gwenhywfar/ipc.h>
#include <gwenhywfar/base64.h>
#include <gwenhywfar/misc2.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/fslock.h>
#include <gwenhywfar/xsd.h>
#include <gwenhywfar/refptr.h>
#include <gwenhywfar/stringlist2.h>
#include <gwenhywfar/crypttoken.h>
#include "../src/parser/xsd_p.h"
#ifdef OS_WIN32
# include <windows.h>
# define sleep(x) Sleep(x*1000)
# define strcasecmp(a, b) strcmp(a, b)
#endif

#include <gwenhywfar/nl_socket.h>
#include <gwenhywfar/nl_file.h>
#include <gwenhywfar/nl_http.h>
#include <gwenhywfar/nl_log.h>
#include <gwenhywfar/net2.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


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

  GWEN_DB_Dump(cfg, stderr, 2);

  fprintf(stderr,"Releasing DB\n");
  GWEN_DB_Group_free(cfg);
  return 0;
}



int testDB2(int argc, char **argv) {
  GWEN_DB_NODE *cfg;
  int rv;
  const char *p;

  fprintf(stderr,"Creating DB\n");
  cfg=GWEN_DB_Group_new("Config");

  fprintf(stderr, "Setting char values\n");
  GWEN_DB_GetGroup(cfg, GWEN_PATH_FLAGS_CREATE_GROUP, "testgroup");
  GWEN_DB_GetGroup(cfg, GWEN_PATH_FLAGS_CREATE_GROUP, "testgroup");

  rv=GWEN_DB_SetCharValue(cfg,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "testgroup[1]/charvar",
                          "charvalue1");
  rv=GWEN_DB_SetCharValue(cfg,
                          0,
                          "testgroup[1]/charvar",
                          "charvalue2");

  fprintf(stderr, "Retrieving char values\n");
  p=GWEN_DB_GetCharValue(cfg,
                         "testgroup[1]/charvar", 0,
                         "defaultValue");
  fprintf(stderr," Retrieved value 1 is: %s\n", p);

  p=GWEN_DB_GetCharValue(cfg,
                         "testgroup[1]/charvar", 1,
                         "defaultValue");
  fprintf(stderr," Retrieved value 2 is: %s\n", p);

  fprintf(stderr, "Setting int values\n");
  rv=GWEN_DB_SetIntValue(cfg,
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "testgroup[1]/intvar",
                         11);

  rv=GWEN_DB_SetIntValue(cfg,
                         0,
                         "testgroup[1]/intvar",
                         22);
  fprintf(stderr, "Retrieving int values\n");
  rv=GWEN_DB_GetIntValue(cfg,
                         "testgroup[1]/intvar", 0,
                         -1);
  fprintf(stderr," Retrieved int value 1 is: %d\n", rv);

  rv=GWEN_DB_GetIntValue(cfg,
                         "testgroup[1]/intvar", 1,
                         -1);
  fprintf(stderr," Retrieved int value 2 is: %d\n", rv);

  GWEN_DB_Dump(cfg, stderr, 2);

  fprintf(stderr,"Releasing DB\n");
  GWEN_DB_Group_free(cfg);
  return 0;
}



int testDBfile(int argc, char **argv) {
  GWEN_DB_NODE *db;

  fprintf(stderr,"Creating DB\n");
  db=GWEN_DB_Group_new("Config");

  fprintf(stderr,"Reading file\n");
  if (GWEN_DB_ReadFile(db, "test.db", GWEN_DB_FLAGS_DEFAULT)) {
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
  if (GWEN_DB_ReadFile(db, argv[2], GWEN_DB_FLAGS_DEFAULT)) {
    fprintf(stderr,"Error reading file.\n");
    return 1;
  }

  if (GWEN_DB_WriteFile(db, argv[3],
                        GWEN_DB_FLAGS_DEFAULT
                        &~GWEN_DB_FLAGS_ESCAPE_CHARVALUES
                       )) {
    fprintf(stderr,"Error writing file.\n");
    return 1;
  }

  fprintf(stderr,"Releasing DB\n");
  GWEN_DB_Group_free(db);
  return 0;
}




int testXML(int argc, char **argv) {
  GWEN_XMLNODE *n;

  if (argc<3) {
    fprintf(stderr, "Name of testfile needed.\n");
    return 1;
  }
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Debug);
  if (GWEN_XML_ReadFile(n, argv[2], GWEN_XML_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }
  fprintf(stderr, "XML file:\n");
  GWEN_XMLNode_Dump(n, stderr, 2);
  GWEN_XMLNode_free(n);

  fprintf(stderr, "Memory before collection:\n");
  GWEN_Memory_Dump();
  GWEN_Memory_Collect();
  fprintf(stderr, "Memory after collection:\n");
  GWEN_Memory_Dump();

  return 0;
}



int testXML2(int argc, char **argv) {
  GWEN_XMLNODE *n;
  GWEN_STRINGLIST *sl;
  unsigned int j;

  if (argc<3) {
    fprintf(stderr, "Name of testfile needed.\n");
    return 1;
  }
  sl=GWEN_StringList_new();
  for (j=3; j<(unsigned int)argc; j++)
    GWEN_StringList_AppendString(sl, argv[j], 0, 1);

  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Debug);
  if (GWEN_XML_ReadFileSearch(n, argv[2],
                              GWEN_XML_FLAGS_DEFAULT |
                              GWEN_XML_FLAGS_SHARE_TOPLEVEL |
                              GWEN_XML_FLAGS_INCLUDE_TO_TOPLEVEL,
                              sl)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }
  fprintf(stderr, "XML file:\n");
  GWEN_XMLNode_Dump(n, stderr, 2);
  GWEN_XMLNode_free(n);
  return 0;
}



int testXML3(int argc, char **argv) {
  GWEN_XMLNODE *n;

  if (argc<3) {
    fprintf(stderr, "Name of testfile needed.\n");
    return 1;
  }
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Debug);
  if (GWEN_XML_ReadFile(n, argv[2], GWEN_XML_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }
  fprintf(stderr, "XML file:\n");
  GWEN_XMLNode_Dump(n, stderr, 2);
  if (GWEN_XMLNode_WriteFile(n, "xml.out",
                             GWEN_XML_FLAGS_DEFAULT)){
    fprintf(stderr, "Could not write file xml.out\n");
    return 2;
  }
  GWEN_XMLNode_free(n);
  return 0;
}



int testXML4(int argc, char **argv) {
  GWEN_XMLNODE *n;
  time_t startTime;
  time_t stopTime;

  if (argc<3) {
    fprintf(stderr, "Name of testfile needed.\n");
    return 1;
  }
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Debug);
  startTime=time(0);
  if (GWEN_XML_ReadFile(n, argv[2], GWEN_XML_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }
  stopTime=time(0);
  GWEN_XMLNode_free(n);

  fprintf(stderr, "Time for loading: %d secs\n",
          (int)(difftime(stopTime, startTime)));

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

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Info);

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
  err=GWEN_CryptKey_toDb(key,
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

  key=GWEN_CryptKey_fromDb(db);
  if (!key) {
    fprintf(stderr, "Could not load key\n");
    return 2;
  }
  GWEN_DB_Group_free(db);

  db2=GWEN_DB_Group_new("key");
  err=GWEN_CryptKey_toDb(key,
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


int testSnprintf(int argc, char **argv) {
  unsigned int i;
  char buffer[256];
  unsigned int p;

  p=0xdeadbeef;
  i=GWEN_Debug_Snprintf(buffer, sizeof(buffer),
                        "Test %010x %s [%s]\n", p, "Some crazy cow", 0);
  buffer[sizeof(buffer)-1]=0;
  fprintf(stderr, "Result: \"%s\" (%d)\n",
          buffer, i);
  return 0;
}



int testProcess(int argc, char **argv) {
  GWEN_Logger_Open(0, "test", "gwentest.log", GWEN_LoggerType_File,
                   GWEN_LoggerFacility_User);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Verbous);

  if (argc<3) {
    fprintf(stderr, "%s process client|server\n", argv[0]);
    return 1;
  }
  fprintf(stderr, "Ping...\n");
  fprintf(stderr, "argv2 is \"%s\"\n", argv[2]);
  if (strcasecmp(argv[2], "server")==0) {
    GWEN_PROCESS *pr;
    GWEN_PROCESS_STATE pst;
    GWEN_BUFFEREDIO *bio;
    GWEN_ERRORCODE err;
	
	fprintf(stderr, "Creating process\n");
    pr=GWEN_Process_new();
	fprintf(stderr, "Creating process: done\n");
	fprintf(stderr, "Setting flags\n");
    GWEN_Process_AddFlags(pr, GWEN_PROCESS_FLAGS_REDIR_STDOUT);
	fprintf(stderr, "Setting flags: done\n");
	
	fprintf(stderr, "Starting process\n");
    pst=GWEN_Process_Start(pr, argv[0], "process client");
	//pst=GWEN_ProcessStateNotStarted;
    if (pst!=GWEN_ProcessStateRunning) {
      fprintf(stderr, "Bad process state \"%d\"", pst);
      return 2;
    }
	fprintf(stderr, "Started process\n");
	//Sleep(15000);
	//return 0;

    bio=GWEN_Process_GetStdout(pr);
    assert(bio);
    while(!GWEN_BufferedIO_CheckEOF(bio)) {
      char buffer[256];

	  fprintf(stderr, "Will read from client\n");
      buffer[0]=0;
      err=GWEN_BufferedIO_ReadLine(bio, buffer, sizeof(buffer)-1);
      if (!GWEN_Error_IsOk(err)) {
        fprintf(stderr, "ERROR: Reading from clients output\n");
        return 2;
      }
      fprintf(stderr, "DATA: %s\n", buffer);
    } /* while */
    err=GWEN_BufferedIO_Close(bio);
    if (!GWEN_Error_IsOk(err)) {
      fprintf(stderr, "ERROR: Closing clients output stream\n");
      return 2;
    }

    fprintf(stderr, "INFO: Client disconnected.\n");
    return 0;
  } /* if server */
  else {
    fprintf(stderr, "Hello, I'm the client...\n");
    if (fprintf(stdout, "Hello, I'm the client...\n")<1) {
		fprintf(stderr, "ERROR: Client could not write.\n");
	}
    sleep(10);
    fprintf(stderr, "Client exiting\n");
    return 0;
  }
}



int testOptions(int argc, char **argv) {
  int rv;
  GWEN_DB_NODE *db;
  GWEN_BUFFER *ubuf;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,            /* type */
    "charOption",                 /* name */
    1,                            /* minnum */
    0,                            /* maxnum */
    "c",                          /* short option */
    "char",                       /* long option */
    "char option",                /* short description */
    "this is a char option"       /* long description */
  },
  {
    0,                            /* flags */
    GWEN_ArgsType_Int,             /* type */
    "boolOption",                 /* name */
    0,                            /* minnum */
    0,                            /* maxnum */
    "b",                          /* short option */
    "bool",                       /* long option */
    "bool option",                /* short description */
    "This is a bool option.\n"    /* long description */
    "It is used to show how the mere existence of an option is interpreted\n"
    "by the command line argument parser"
  },
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsType_Int,             /* type */
    "intOption",                  /* name */
    0,                            /* minnum */
    0,                            /* maxnum */
    "i",                          /* short option */
    "int",                        /* long option */
    "int option",                 /* short description */
    "this is an int option"       /* long description */
  }
  };

  db=GWEN_DB_Group_new("arguments");
  rv=GWEN_Args_Check(argc, argv, 1,
		     GWEN_ARGS_MODE_ALLOW_FREEPARAM,
		     args,
		     db);
  if (rv<1) {
    fprintf(stderr, "ERROR: Could not parse (%d)\n", rv);
  }
  else {
    GWEN_DB_Dump(db, stderr, 2);
  }

  GWEN_DB_Group_free(db);

  ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
  if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
    fprintf(stderr, "ERROR: Could not create help string\n");
    return 1;
  }
  fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));

  return 0;
}



int testBase64(int argc, char **argv) {
  GWEN_BUFFER *dst;
  GWEN_BUFFER *src;
  GWEN_BUFFER *tmp;
  FILE *f;
  char buffer[1024];
  int i;

  if (argc<2) {
    fprintf(stderr, "Name of a file needed\n");
    return 1;
  }

  dst=GWEN_Buffer_new(0, 600000, 0, 1);
  src=GWEN_Buffer_new(0, 600000, 0, 1);
  f=fopen(argv[2], "r");
  if (!f) {
    perror(argv[2]);
    return 1;
  }
  while(!feof(f)) {
    i=fread(buffer, 1, sizeof(buffer), f);
    if (i<1) {
      perror("fread");
      return 2;
    }
    GWEN_Buffer_AppendBytes(src, buffer, i);
  } /* while */
  fclose(f);

  if (GWEN_Base64_Encode(/* GCC4 pointer-signedness fix: */ (unsigned char*)GWEN_Buffer_GetStart(src),
                         GWEN_Buffer_GetUsedBytes(src),
                         dst, 76)) {
    fprintf(stderr, "Error encoding file.\n");
    return 3;
  }

  f=fopen("base64.out", "w+");
  if (!f) {
    perror(argv[2]);
    return 1;
  }
  if (fwrite(GWEN_Buffer_GetStart(dst),
             GWEN_Buffer_GetUsedBytes(dst), 1, f)!=1) {
    perror("fwrite");
    return 4;
  }
  if (fclose(f)) {
    perror("fclose");
    return 4;
  }

  tmp=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(src), 0, 1);
  if (GWEN_Base64_Decode(/* GCC4 pointer-signedness fix: */ (unsigned char*)GWEN_Buffer_GetStart(dst),
                         GWEN_Buffer_GetUsedBytes(src),
                         tmp)) {
    fprintf(stderr, "Error decoding file.\n");
    return 3;
  }

  f=fopen("base64.out.bin", "w+");
  if (!f) {
    perror(argv[2]);
    return 1;
  }
  if (fwrite(GWEN_Buffer_GetStart(tmp),
             GWEN_Buffer_GetUsedBytes(tmp), 1, f)!=1) {
    perror("fwrite");
    return 4;
  }
  if (fclose(f)) {
    perror("fclose");
    return 4;
  }

  return 0;
}


int testBase64_2(int argc, char **argv) {
  GWEN_BUFFER *dst;
  GWEN_BUFFER *src;
  FILE *f;
  char buffer[1024];
  int i;

  if (argc<2) {
    fprintf(stderr, "Name of a file needed\n");
    return 1;
  }

  dst=GWEN_Buffer_new(0, 600000, 0, 1);
  src=GWEN_Buffer_new(0, 600000, 0, 1);
  f=fopen(argv[2], "r");
  if (!f) {
    perror(argv[2]);
    return 1;
  }
  while(!feof(f)) {
    i=fread(buffer, 1, sizeof(buffer), f);
    if (i<1) {
      perror("fread");
      return 2;
    }
    GWEN_Buffer_AppendBytes(src, buffer, i);
  } /* while */
  fclose(f);

  if (GWEN_Base64_Decode(/* GCC4 pointer-signedness fix: */ (unsigned char*)GWEN_Buffer_GetStart(src),
                         0,
                         dst)) {
    fprintf(stderr, "Error decoding file.\n");
    return 3;
  }

  f=fopen("base64.out.bin", "w+");
  if (!f) {
    perror(argv[2]);
    return 1;
  }
  if (fwrite(GWEN_Buffer_GetStart(dst),
             GWEN_Buffer_GetUsedBytes(dst), 1, f)!=1) {
    perror("fwrite");
    return 4;
  }
  if (fclose(f)) {
    perror("fclose");
    return 4;
  }

  return 0;
}




GWEN_CONSTLIST2_FUNCTION_DEFS(GWEN_KEYSPEC, GWEN_KeySpec)
GWEN_CONSTLIST2_FUNCTIONS(GWEN_KEYSPEC, GWEN_KeySpec)

int testList2(int argc, char **argv) {
  GWEN_KEYSPEC_LIST2 *specList;
  GWEN_KEYSPEC_LIST2 *specList2;
  GWEN_KEYSPEC_LIST2_ITERATOR *kit;
  GWEN_KEYSPEC *spec;
  GWEN_REFPTR_INFO *rpi;

  specList=GWEN_KeySpec_List2_new();
  rpi=GWEN_RefPtrInfo_new();
  GWEN_RefPtrInfo_AddFlags(rpi, GWEN_REFPTR_FLAGS_AUTODELETE);
  GWEN_RefPtrInfo_SetFreeFn(rpi,
                            (GWEN_REFPTR_INFO_FREE_FN)GWEN_KeySpec_free);
  GWEN_List_SetRefPtrInfo((GWEN_LIST*)specList, rpi);
  spec=GWEN_KeySpec_new();
  GWEN_KeySpec_SetOwner(spec, "User1");
  GWEN_KeySpec_List2_PushBack(specList, spec);

  spec=GWEN_KeySpec_new();
  GWEN_KeySpec_SetOwner(spec, "User2");
  GWEN_KeySpec_List2_PushBack(specList, spec);

  spec=GWEN_KeySpec_new();
  GWEN_KeySpec_SetOwner(spec, "User3");
  GWEN_KeySpec_List2_PushBack(specList, spec);

  kit=GWEN_KeySpec_List2_First(specList);
  if (kit) {
    GWEN_KEYSPEC *ks;

    ks=GWEN_KeySpec_List2Iterator_Data(kit);
    while(ks) {
      GWEN_KeySpec_Dump(ks, stderr, 2);
      ks=GWEN_KeySpec_List2Iterator_Next(kit);
    }
    GWEN_KeySpec_List2Iterator_free(kit);
  }

  GWEN_KeySpec_List2_PopFront(specList);
  fprintf(stderr, "List1 after 1st POP:\n");
  kit=GWEN_KeySpec_List2_First(specList);
  if (kit) {
    GWEN_KEYSPEC *ks;

    ks=GWEN_KeySpec_List2Iterator_Data(kit);
    while(ks) {
      GWEN_KeySpec_Dump(ks, stderr, 2);
      ks=GWEN_KeySpec_List2Iterator_Next(kit);
    }
    GWEN_KeySpec_List2Iterator_free(kit);
  }

  specList2=GWEN_KeySpec_List2_dup(specList);

  fprintf(stderr, "List1 before POP:\n");
  kit=GWEN_KeySpec_List2_First(specList);
  if (kit) {
    GWEN_KEYSPEC *ks;

    ks=GWEN_KeySpec_List2Iterator_Data(kit);
    while(ks) {
      GWEN_KeySpec_Dump(ks, stderr, 2);
      ks=GWEN_KeySpec_List2Iterator_Next(kit);
    }
    GWEN_KeySpec_List2Iterator_free(kit);
  }
  fprintf(stderr, "List2 before POP:\n");
  kit=GWEN_KeySpec_List2_First(specList2);
  if (kit) {
    GWEN_KEYSPEC *ks;

    ks=GWEN_KeySpec_List2Iterator_Data(kit);
    while(ks) {
      GWEN_KeySpec_Dump(ks, stderr, 2);
      ks=GWEN_KeySpec_List2Iterator_Next(kit);
    }
    GWEN_KeySpec_List2Iterator_free(kit);
  }

  GWEN_KeySpec_List2_PopFront(specList2);

  fprintf(stderr, "List1 after POP:\n");
  kit=GWEN_KeySpec_List2_First(specList);
  if (kit) {
    GWEN_KEYSPEC *ks;

    ks=GWEN_KeySpec_List2Iterator_Data(kit);
    while(ks) {
      GWEN_KeySpec_Dump(ks, stderr, 2);
      ks=GWEN_KeySpec_List2Iterator_Next(kit);
    }
    GWEN_KeySpec_List2Iterator_free(kit);
  }

  fprintf(stderr, "List2 after POP:\n");
  kit=GWEN_KeySpec_List2_First(specList2);
  if (kit) {
    GWEN_KEYSPEC *ks;

    ks=GWEN_KeySpec_List2Iterator_Data(kit);
    while(ks) {
      GWEN_KeySpec_Dump(ks, stderr, 2);
      ks=GWEN_KeySpec_List2Iterator_Next(kit);
    }
    GWEN_KeySpec_List2Iterator_free(kit);
  }


  GWEN_KeySpec_List2_free(specList);
  return 0;
}



int testTime(int argc, char **argv) {
  GWEN_TIME *ti1;
  GWEN_TIME *ti2;
  GWEN_DB_NODE *db1;
  GWEN_DB_NODE *db2;

  ti1=GWEN_CurrentTime();
  assert(ti1);

  db1=GWEN_DB_Group_new("time");
  if (GWEN_Time_toDb(ti1, db1)) {
    fprintf(stderr, "Error saving time.\n");
    return 1;
  }
  DBG_NOTICE(0, "Time 1:");
  GWEN_DB_Dump(db1, stderr, 2);

  ti2=GWEN_Time_fromDb(db1);
  db2=GWEN_DB_Group_new("time");
  if (GWEN_Time_toDb(ti2, db2)) {
    fprintf(stderr, "Error saving time.\n");
    return 1;
  }
  DBG_NOTICE(0, "Time 2:");
  GWEN_DB_Dump(db2, stderr, 2);

  return 0;
}



int testDBIO(int argc, char **argv) {
  GWEN_BUFFEREDIO *bio;
  GWEN_DBIO *dbio;
  GWEN_ERRORCODE err;
  int rv;
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbParams;
  int fd;

  dbio=GWEN_DBIO_GetPlugin("swift");
  if (!dbio) {
    DBG_ERROR(0, "Plugin SWIFT is not supported");
    return -1;
  }

  fd=open("booked.mt", O_RDONLY);
  if (fd==-1) {
    DBG_ERROR(0, "Could not open file (%s)", strerror(errno));
    return 2;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);

  db=GWEN_DB_Group_new("transactions");
  dbParams=GWEN_DB_Group_new("params");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "type", "mt940");
  while(!GWEN_BufferedIO_CheckEOF(bio)) {
    rv=GWEN_DBIO_Import(dbio, bio, GWEN_PATH_FLAGS_CREATE_GROUP,
                        db, dbParams);
    if (rv) {
      DBG_ERROR(0, "Error parsing SWIFT mt940");
    }
  } /* while */
  GWEN_DB_Group_free(dbParams);
  err=GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    GWEN_DB_Group_free(db);
    return -1;
  }

  /* DEBUG */
  if (GWEN_DB_WriteFile(db,
                        "transactions.trans",
                        GWEN_DB_FLAGS_DEFAULT)) {
    DBG_ERROR(0, "Could not write transactions");
  }

  return 0;
}



int testTimeToString(int argc, char **argv) {
  GWEN_TIME *t;
  GWEN_BUFFER *tbuf;
  const char *tmpl;

  if (argc<3)
    tmpl="YYYY/MM/DD hh:mm:ss";
  else
    tmpl=argv[2];

  t=GWEN_CurrentTime();
  assert(t);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (GWEN_Time_toString(t, tmpl, tbuf)) {
    fprintf(stderr, "Could not convert time to string.\n");
    return 2;
  }
  fprintf(stdout, "Current date/time: %s\n",
          GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  if (GWEN_Time_toUtcString(t, tmpl, tbuf)) {
    fprintf(stderr, "Could not convert time to string.\n");
    return 2;
  }
  fprintf(stdout, "Current UTC date/time: %s\n",
          GWEN_Buffer_GetStart(tbuf));

  return 0;
}



int testTimeFromString(int argc, char **argv) {
  GWEN_TIME *t;
  const char *s;
  const char *tmpl;
  GWEN_BUFFER *tbuf;

  if (argc!=4) {
    fprintf(stderr, "Arguments needed: %s %s TEMPLATE DATA\n",
	    argv[0], argv[1]);
    return 1;
  }

  tmpl=argv[2];
  s=argv[3];

  t=GWEN_Time_fromUtcString(s, tmpl);
  if (!t) {
    fprintf(stderr, "Could not convert string to time.\n");
    return 2;
  }

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (GWEN_Time_toUtcString(t, "YYYY/MM/DD-hh:mm:ss", tbuf)) {
    fprintf(stderr, "Could not convert time to string.\n");
    return 2;
  }
  fprintf(stdout, "UTC date/time \"%s\": %s\n",
          s, GWEN_Buffer_GetStart(tbuf));

  return 0;
}



int testCsvExport(int argc, char **argv) {
  GWEN_BUFFEREDIO *bio;
  GWEN_DBIO *dbio;
  GWEN_ERRORCODE err;
  int rv;
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbParams;
  int fd;

  dbio=GWEN_DBIO_GetPlugin("csv");
  if (!dbio) {
    DBG_ERROR(0, "Plugin CSV is not supported");
    return -1;
  }

  fd=open("test.csv", O_RDONLY);
  if (fd==-1) {
    DBG_ERROR(0, "Could not open file (%s)", strerror(errno));
    return 2;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);

  db=GWEN_DB_Group_new("transactions");
  dbParams=GWEN_DB_Group_new("params");
  if (GWEN_DB_ReadFile(dbParams, "test.params",
                       GWEN_DB_FLAGS_DEFAULT |
                       GWEN_PATH_FLAGS_CREATE_GROUP)) {
    DBG_ERROR(0, "Could not read parameters");
    return 2;
  }

  while(!GWEN_BufferedIO_CheckEOF(bio)) {
    rv=GWEN_DBIO_Import(dbio, bio, GWEN_PATH_FLAGS_CREATE_GROUP,
                        db, dbParams);
    if (rv) {
      DBG_ERROR(0, "Error parsing CSV (%d)", rv);
    }
  } /* while */
  GWEN_DB_Group_free(dbParams);
  err=GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    GWEN_DB_Group_free(db);
    return -1;
  }

  /* DEBUG */
  if (GWEN_DB_WriteFile(db,
                        "transactions.trans",
                        GWEN_DB_FLAGS_DEFAULT)) {
    DBG_ERROR(0, "Could not write transactions");
  }

  return 0;
}



int testOldDbImport(int argc, char **argv) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbParams;

  db=GWEN_DB_Group_new("test");
  dbParams=GWEN_DB_Group_new("params");
  if (GWEN_DB_ReadFileAs(db,
                         "test.olddb",
                         "olddb",
                         dbParams,
                         GWEN_DB_FLAGS_DEFAULT |
                         GWEN_PATH_FLAGS_CREATE_GROUP)) {
    DBG_ERROR(0, "Could not read test file");
    return 2;
  }

  if (GWEN_DB_WriteFile(db,
                        "test.out",
                        GWEN_DB_FLAGS_DEFAULT)) {
    DBG_ERROR(0, "Could not write outfile");
  }

  return 0;
}



int testRfc822Import(int argc, char **argv) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbParams;

  db=GWEN_DB_Group_new("test");
  dbParams=GWEN_DB_Group_new("params");
  if (GWEN_DB_ReadFileAs(db,
                         "test.822",
                         "rfc822",
                         dbParams,
                         GWEN_PATH_FLAGS_CREATE_GROUP |
                         GWEN_DB_FLAGS_STOP_ON_EMPTY_LINE)) {
    DBG_ERROR(0, "Could not read test file");
    return 2;
  }

  if (GWEN_DB_WriteFile(db,
                        "test.out",
                        GWEN_DB_FLAGS_DEFAULT)) {
    DBG_ERROR(0, "Could not write outfile");
  }

  return 0;
}



int testFsLock(int argc, char **argv) {
  GWEN_FSLOCK *fl;
  GWEN_FSLOCK_RESULT res;

  if (argc<3) {
    fprintf(stderr, "Usage: %s fslock FILENAME\n", argv[0]);
    return 1;
  }

  fl=GWEN_FSLock_new(argv[2], GWEN_FSLock_TypeFile);
  fprintf(stderr, "Locking %s\n", argv[2]);
  res=GWEN_FSLock_Lock(fl, 30000);
  if (res!=GWEN_FSLock_ResultOk) {
    fprintf(stderr, "Error locking %s: %d\n", argv[2], res);
    return 2;
  }
  fprintf(stderr, "Holding lock on %s ...\n", argv[2]);
  sleep(10);
  fprintf(stderr, "Unlocking %s\n", argv[2]);
  res=GWEN_FSLock_Unlock(fl);
  if (res!=GWEN_FSLock_ResultOk) {
    fprintf(stderr, "Error unlocking %s: %d\n", argv[2], res);
    return 3;
  }
  fprintf(stderr, "Success.\n");
  return 0;
}



int testFsLock2(int argc, char **argv) {
  GWEN_FSLOCK *fl;
  GWEN_FSLOCK_RESULT res;

  if (argc<3) {
    fprintf(stderr, "Usage: %s fslock2 FOLDERNAME\n", argv[0]);
    return 1;
  }

  fl=GWEN_FSLock_new(argv[2], GWEN_FSLock_TypeDir);
  fprintf(stderr, "Locking %s\n", argv[2]);
  res=GWEN_FSLock_Lock(fl, 3000);
  if (res!=GWEN_FSLock_ResultOk) {
    fprintf(stderr, "Error locking %s: %d\n", argv[2], res);
    return 2;
  }
  fprintf(stderr, "Holding lock on %s ...\n", argv[2]);
  sleep(10);
  fprintf(stderr, "Unlocking %s\n", argv[2]);
  res=GWEN_FSLock_Unlock(fl);
  if (res!=GWEN_FSLock_ResultOk) {
    fprintf(stderr, "Error unlocking %s: %d\n", argv[2], res);
    return 3;
  }
  fprintf(stderr, "Success.\n");
  return 0;
}



int testXSD(int argc, char **argv) {
  GWEN_XSD_ENGINE *e;
  GWEN_XSD_NAMESPACE *ns;
  GWEN_XMLNODE *node;
  int rv;
  int i, j;
  GWEN_BUFFER *ibuf;

  if (argc<3) {
    fprintf(stderr, "Usage: %s xsd FILENAME\n", argv[0]);
    return 1;
  }

  e=GWEN_XSD_new();
  if (GWEN_XSD_ImportStart(e)) {
    fprintf(stderr, "Could not start importing.\n");
    return 2;
  }

  for (j=2; j<argc; j++) {
    node=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
    if (GWEN_XML_ReadFile(node, argv[j],
                          GWEN_XML_FLAGS_DEFAULT |
                          GWEN_XML_FLAGS_IGNORE_INCLUDE |
                          GWEN_XML_FLAGS_NO_CONDENSE |
                          GWEN_XML_FLAGS_KEEP_BLANKS)) {
      fprintf(stderr, "Could not read file \"%s\"\n", argv[j]);
      return 2;
    }

    rv=GWEN_XSD_ImportSchema(e, node, argv[j]);
    if (rv) {
      fprintf(stderr, "Could not import file \"%s\"\n", argv[j]);
      return 3;
    }
    fprintf(stderr, "File \"%s\" imported.\n", argv[j]);
  } /* for */

  if (GWEN_XSD_ImportEnd(e)) {
    fprintf(stderr, "Could not end importing.\n");
    return 3;
  }

  ns=GWEN_XSD_NameSpace_List_First(e->nameSpaces);
  i=1;
  while(ns) {
    fprintf(stderr, "Namespace %d: %s: %s (%s)\n",
            i, ns->id, ns->name, ns->url);
    ns=GWEN_XSD_NameSpace_List_Next(ns);
    i++;
  }
  if (GWEN_XMLNode_WriteFile(e->rootNode, "xsd.out",
                             GWEN_XML_FLAGS_DEFAULT)){
    fprintf(stderr, "Could not write file xsd.out\n");
    return 2;
  }

  ibuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (GWEN_XSD_ListTypes(e,
                         //"http://www.fints.org/spec/xmlschema/4.0/final/transactions",
                         //"AcctBal_1_Req",
                         "http://www.fints.org/spec/xmlschema/4.0/final/types",
                         "ReqMsgHeader",
                         ibuf)) {
    fprintf(stderr, "Type not found.\n");
  }
  else {
    fprintf(stderr, "Type found\n%s", GWEN_Buffer_GetStart(ibuf));
  }

  GWEN_XSD_free(e);
  return 0;
}



int testXSD2(int argc, char **argv) {
  GWEN_XSD_ENGINE *e;
  GWEN_XSD_NAMESPACE *ns;
  GWEN_XMLNODE *node;
  int rv;
  int i, j;
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbT;
  GWEN_XMLNODE *nStore;
  GWEN_XMLNODE *nProfile;

  if (argc<3) {
    fprintf(stderr, "Usage: %s %s FILENAME\n", argv[0], argv[1]);
    return 1;
  }

  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Info);

  e=GWEN_XSD_new();

  if (GWEN_XSD_ImportStart(e)) {
    fprintf(stderr, "Could not start importing.\n");
    return 2;
  }

  if (GWEN_XSD_SetNamespace(e,
                            "fintstrans",
                            "http://www.fints.org/spec/xmlschema/4.0/final/transactions",
                            0, 0)) {
    fprintf(stderr, "Could not set fintstrans namespace.\n");
    return 2;
  }
  if (GWEN_XSD_SetNamespace(e,
                            "fintstypes",
                            "http://www.fints.org/spec/xmlschema/4.0/final/types",
                            0, 0)) {
    fprintf(stderr, "Could not set fintstypes namespace.\n");
    return 2;
  }

  for (j=2; j<argc; j++) {
    node=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
    if (GWEN_XML_ReadFile(node, argv[j],
                          GWEN_XML_FLAGS_DEFAULT |
                          GWEN_XML_FLAGS_HANDLE_HEADERS /* |
                          GWEN_XML_FLAGS_IGNORE_INCLUDE |
                          GWEN_XML_FLAGS_NO_CONDENSE |
                          GWEN_XML_FLAGS_KEEP_BLANKS*/)) {
      fprintf(stderr, "Could not read file \"%s\"\n", argv[j]);
      return 2;
    }

    rv=GWEN_XSD_ImportSchema(e, node, argv[j]);
    if (rv) {
      fprintf(stderr, "Could not import file \"%s\"\n", argv[j]);
      return 3;
    }
    fprintf(stderr, "File \"%s\" imported.\n", argv[j]);
  } /* for */

  if (GWEN_XSD_ImportEnd(e)) {
    fprintf(stderr, "Could not end importing.\n");
    return 3;
  }


  ns=GWEN_XSD_NameSpace_List_First(e->nameSpaces);
  i=1;
  while(ns) {
    fprintf(stderr, "Namespace %d: %s: %s (%s)\n",
            i, ns->id, ns->name, ns->url);
    ns=GWEN_XSD_NameSpace_List_Next(ns);
    i++;
  }

  nProfile=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  node=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "?xml");
  GWEN_XMLNode_AddHeader(nProfile, node);
  GWEN_XMLNode_SetProperty(node, "version", "1.0");
  GWEN_XMLNode_SetProperty(node, "encoding", "utf-8");

  node=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "profile");
  GWEN_XMLNode_AddChild(nProfile, node);
  if (GWEN_XSD_ProfileToXml(e, node)) {
    fprintf(stderr, "Could not create profile\n");
  }

  if (GWEN_XMLNode_WriteFile(nProfile, "testprofile1.xml",
                             GWEN_XML_FLAGS_SIMPLE |
                             GWEN_XML_FLAGS_HANDLE_HEADERS)){
    fprintf(stderr, "Could not write file testprofile1.xml\n");
    return 2;
  }
  if (GWEN_XMLNode_WriteFile(nProfile, "testprofile2.xml",
                             GWEN_XML_FLAGS_DEFAULT |
                             GWEN_XML_FLAGS_SIMPLE|
                             GWEN_XML_FLAGS_HANDLE_HEADERS)){
    fprintf(stderr, "Could not write file testprofile2.xml\n");
    return 2;
  }

  dbData=GWEN_DB_Group_new("data");
  nStore=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");

  GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "All_Acct", "true");
  GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "MaxNo_Entries", "2");
  dbT=GWEN_DB_GetGroup(dbData, GWEN_DB_FLAGS_DEFAULT,
                       "OrderingCustAcct");
  assert(dbT);
  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "AcctNo", "123456789");
  dbT=GWEN_DB_GetGroup(dbT, GWEN_DB_FLAGS_DEFAULT,
                       "BankId");
  assert(dbT);
  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "CountryCode", "280");
  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "BankCode", "99999999");

  if (GWEN_XSD_SetCurrentTargetNameSpace(e,
                                         "http://www.fints.org/spec/xmlschema/4.0/final/types")) {
    fprintf(stderr, "Could not change current target namespace.\n");
  }
  rv=GWEN_XSD_WriteElement(e,
                           "http://www.fints.org/spec/xmlschema/4.0/final/transactions",
                           "AcctBal_1_Req",
                           //"http://www.fints.org/spec/xmlschema/4.0/final/types",
                           //"ReqMsgHeader",
                           dbData,
                           nStore,
                           1);
  if (rv) {
    fprintf(stderr, "Could not write type (%d).\n", rv);
    GWEN_XMLNode_Dump(nStore, stderr, 2);
  }
  else {
    fprintf(stderr, "Type written\n");
    GWEN_XMLNode_Dump(nStore, stderr, 2);
    if (GWEN_XMLNode_WriteFile(nStore, "xsd-result.xml",
                               GWEN_XML_FLAGS_DEFAULT |
                               GWEN_XML_FLAGS_SIMPLE)){
      fprintf(stderr, "Could not write file xsd-result.xml\n");
      return 2;
    }
    rv=GWEN_XMLNode_NormalizeNameSpaces(nStore);
    if (rv) {
      fprintf(stderr, "Could not normalize XML tree (%d)\n", rv);
      return 2;
    }
    else {
      fprintf(stderr, "XML tree normalized.\n");
    }
    if (GWEN_XMLNode_WriteFile(nStore, "xsd-result-norm.xml",
                               GWEN_XML_FLAGS_DEFAULT |
                               GWEN_XML_FLAGS_SIMPLE)){
      fprintf(stderr, "Could not write file xsd-result-norm.xml\n");
      return 2;
    }
  }

  GWEN_XSD_free(e);
  return 0;
}



int testXSD3(int argc, char **argv) {
  GWEN_XSD_ENGINE *e;
  GWEN_XSD_NAMESPACE *ns;
  int rv;
  int i; /* , j; */
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbT;
  GWEN_XMLNODE *nStore;
  GWEN_XMLNODE *nProfile;
  GWEN_XMLNODE *node;

  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Info);

  e=GWEN_XSD_new();

  nProfile=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  if (GWEN_XML_ReadFile(nProfile, "testprofile1.xml",
                        GWEN_XML_FLAGS_DEFAULT |
                        GWEN_XML_FLAGS_HANDLE_HEADERS /* |
                        GWEN_XML_FLAGS_IGNORE_INCLUDE |
                        GWEN_XML_FLAGS_NO_CONDENSE |
                        GWEN_XML_FLAGS_KEEP_BLANKS*/)) {
    fprintf(stderr, "Could not read file \"%s\"\n", "testprofile1.xml");
    return 2;
  }

  node=GWEN_XMLNode_FindFirstTag(nProfile, "profile", 0, 0);
  if (!node) {
    fprintf(stderr, "No profile inside the XML file.\n");
    return 2;
  }
  if (GWEN_XSD_ProfileFromXml(e, node)) {
    fprintf(stderr, "Could not setup profile.\n");
    return 2;
  }

  ns=GWEN_XSD_NameSpace_List_First(e->nameSpaces);
  i=1;
  while(ns) {
    fprintf(stderr, "Namespace %d: %s: %s (%s)\n",
            i, ns->id, ns->name, ns->url);
    ns=GWEN_XSD_NameSpace_List_Next(ns);
    i++;
  }

  dbData=GWEN_DB_Group_new("data");
  nStore=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");

  GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "All_Acct", "true");
  GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "MaxNo_Entries", "2");
  dbT=GWEN_DB_GetGroup(dbData, GWEN_DB_FLAGS_DEFAULT,
                       "OrderingCustAcct");
  assert(dbT);
  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "AcctNo", "123456789");
  dbT=GWEN_DB_GetGroup(dbT, GWEN_DB_FLAGS_DEFAULT,
                       "BankId");
  assert(dbT);
  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "CountryCode", "280");
  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "BankCode", "99999999");

  if (GWEN_XSD_SetCurrentTargetNameSpace(e,
                                         "http://www.fints.org/spec/xmlschema/4.0/final/types")) {
    fprintf(stderr, "Could not change current target namespace.\n");
  }
  rv=GWEN_XSD_WriteElement(e,
                           "http://www.fints.org/spec/xmlschema/4.0/final/transactions",
                           "AcctBal_1_Req",
                           //"http://www.fints.org/spec/xmlschema/4.0/final/types",
                           //"ReqMsgHeader",
                           dbData,
                           nStore,
                           1);
  if (rv) {
    fprintf(stderr, "Could not write type (%d).\n", rv);
    GWEN_XMLNode_Dump(nStore, stderr, 2);
  }
  else {
    node=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "?xml");
    GWEN_XMLNode_AddHeader(nStore, node);
    GWEN_XMLNode_SetProperty(node, "version", "1.0");
    GWEN_XMLNode_SetProperty(node, "encoding", "utf-8");
    fprintf(stderr, "Type written\n");
    GWEN_XMLNode_Dump(nStore, stderr, 2);
    if (GWEN_XMLNode_WriteFile(nStore, "xsd-result.xml",
                               GWEN_XML_FLAGS_DEFAULT |
                               GWEN_XML_FLAGS_SIMPLE |
                               GWEN_XML_FLAGS_HANDLE_HEADERS)){
      fprintf(stderr, "Could not write file xsd-result.xml\n");
      return 2;
    }
    rv=GWEN_XMLNode_NormalizeNameSpaces(nStore);
    if (rv) {
      fprintf(stderr, "Could not normalize XML tree (%d)\n", rv);
      return 2;
    }
    else {
      fprintf(stderr, "XML tree normalized.\n");
    }
    if (GWEN_XMLNode_WriteFile(nStore, "xsd-result-norm.xml",
                               GWEN_XML_FLAGS_DEFAULT |
                               GWEN_XML_FLAGS_SIMPLE|
                               GWEN_XML_FLAGS_HANDLE_HEADERS)){
      fprintf(stderr, "Could not write file xsd-result-norm.xml\n");
      return 2;
    }
  }

  GWEN_XSD_free(e);
  return 0;
}



int testXSD4(int argc, char **argv) {
  GWEN_XSD_ENGINE *e;
  GWEN_XSD_NAMESPACE *ns;
  int rv;
  int i; /* , j; */
  GWEN_XMLNODE *nStore;
  GWEN_XMLNODE *nProfile;
  GWEN_XMLNODE *node;

  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Debug);

  e=GWEN_XSD_new();

  nProfile=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  if (GWEN_XML_ReadFile(nProfile, "testprofile1.xml",
                        GWEN_XML_FLAGS_DEFAULT |
                        GWEN_XML_FLAGS_HANDLE_HEADERS /* |
                        GWEN_XML_FLAGS_IGNORE_INCLUDE |
                        GWEN_XML_FLAGS_NO_CONDENSE |
                        GWEN_XML_FLAGS_KEEP_BLANKS*/)) {
    fprintf(stderr, "Could not read file \"%s\"\n", "testprofile1.xml");
    return 2;
  }

  node=GWEN_XMLNode_FindFirstTag(nProfile, "profile", 0, 0);
  if (!node) {
    fprintf(stderr, "No profile inside the XML file.\n");
    return 2;
  }
  if (GWEN_XSD_ProfileFromXml(e, node)) {
    fprintf(stderr, "Could not setup profile.\n");
    return 2;
  }

  ns=GWEN_XSD_NameSpace_List_First(e->nameSpaces);
  i=1;
  while(ns) {
    fprintf(stderr, "Namespace %d: %s: %s (%s)\n",
            i, ns->id, ns->name, ns->url);
    ns=GWEN_XSD_NameSpace_List_Next(ns);
    i++;
  }

  nStore=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  if (GWEN_XML_ReadFile(nStore, "xsd-result-norm.xml",
                        GWEN_XML_FLAGS_DEFAULT |
                        GWEN_XML_FLAGS_HANDLE_HEADERS /* |
                        GWEN_XML_FLAGS_IGNORE_INCLUDE |
                        GWEN_XML_FLAGS_NO_CONDENSE |
                        GWEN_XML_FLAGS_KEEP_BLANKS*/)) {
    fprintf(stderr, "Could not read file \"%s\"\n", "xsd-result-norm.xml");
    return 2;
  }

  node=GWEN_XMLNode_GetFirstTag(nStore);
  assert(node);
  rv=GWEN_XSD_GlobalizeNode(e, node);
  if (rv) {
    fprintf(stderr, "Could not globalize node (%d).\n", rv);
  }
  else {
    node=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "?xml");
    GWEN_XMLNode_AddHeader(nStore, node);
    GWEN_XMLNode_SetProperty(node, "version", "1.0");
    GWEN_XMLNode_SetProperty(node, "encoding", "utf-8");
    fprintf(stderr, "Type written\n");
    GWEN_XMLNode_Dump(nStore, stderr, 2);
    if (GWEN_XMLNode_WriteFile(nStore, "xsd-result-globalized.xml",
                               GWEN_XML_FLAGS_DEFAULT |
                               GWEN_XML_FLAGS_SIMPLE |
                               GWEN_XML_FLAGS_HANDLE_HEADERS)){
      fprintf(stderr, "Could not write file xsd-result.xml\n");
      return 2;
    }
    rv=GWEN_XMLNode_NormalizeNameSpaces(nStore);
    if (rv) {
      fprintf(stderr, "Could not normalize XML tree (%d)\n", rv);
      return 2;
    }
    else {
      fprintf(stderr, "XML tree normalized.\n");
    }
    if (GWEN_XMLNode_WriteFile(nStore, "xsd-result-globalized-norm.xml",
                               GWEN_XML_FLAGS_DEFAULT |
                               GWEN_XML_FLAGS_SIMPLE|
                               GWEN_XML_FLAGS_HANDLE_HEADERS)){
      fprintf(stderr,
              "Could not write file xsd-result-globalized-norm.xml\n");
      return 2;
    }
  }

  GWEN_XSD_free(e);
  return 0;
}



int testPtr(int argc, char **argv) {
  GWEN_REFPTR *rp;
  GWEN_REFPTR *rp2;
  char *dp1;
  char *dp2;

  dp1=strdup("Hello, World");
  dp2=strdup("Goodbye ;-)");
  rp=GWEN_RefPtr_new(dp1, 0);

  fprintf(stderr, "Data is: %s\n",
          (char*)GWEN_RefPtr_GetData(rp));
  GWEN_RefPtr_SetData(rp, dp2, 0);
  fprintf(stderr, "Data is now: %s\n",
          (char*)GWEN_RefPtr_GetData(rp));

  rp2=GWEN_RefPtr_dup(rp);
  fprintf(stderr, "Data2 is: %s\n",
          (char*)GWEN_RefPtr_GetData(rp2));
  GWEN_RefPtr_SetData(rp2, dp2, 0);
  fprintf(stderr, "Data2 is now: %s\n",
          (char*)GWEN_RefPtr_GetData(rp2));

  GWEN_RefPtr_free(rp);
  return 0;
}



int testStringList2(int argc, char **argv) {
  GWEN_STRINGLIST2 *sl2;
  GWEN_STRINGLIST2 *sl2copy;
  GWEN_STRINGLIST2_ITERATOR *it;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Info);

  sl2=GWEN_StringList2_new();
  GWEN_StringList2_AppendString(sl2, "1:First string", 0,
                                GWEN_StringList2_IntertMode_NoDouble);
  GWEN_StringList2_AppendString(sl2, "2:Second string", 0,
                                GWEN_StringList2_IntertMode_NoDouble);
  GWEN_StringList2_AppendString(sl2, "3:Third string", 0,
                                GWEN_StringList2_IntertMode_NoDouble);
  GWEN_StringList2_AppendString(sl2, "4:Fourth string", 0,
                                GWEN_StringList2_IntertMode_NoDouble);

  fprintf(stderr, "List:\n");
  it=GWEN_StringList2_First(sl2);
  if (it) {
    const char *t;
    int i;

    t=GWEN_StringList2Iterator_Data(it);
    i=0;
    while(t) {
      fprintf(stderr, "String %d: %s [%d]\n", i, t,
              GWEN_StringList2Iterator_GetLinkCount(it));
      t=GWEN_StringList2Iterator_Next(it);
    }
    GWEN_StringList2Iterator_free(it);
  }

  GWEN_StringList2_AppendString(sl2, strdup("3:Third string"), 1,
                                GWEN_StringList2_IntertMode_NoDouble);

  fprintf(stderr, "List:\n");
  it=GWEN_StringList2_First(sl2);
  if (it) {
    const char *t;
    int i;

    t=GWEN_StringList2Iterator_Data(it);
    i=0;
    while(t) {
      fprintf(stderr, "String %d: %s [%d]\n", i, t,
              GWEN_StringList2Iterator_GetLinkCount(it));
      t=GWEN_StringList2Iterator_Next(it);
    }
    GWEN_StringList2Iterator_free(it);
  }

  GWEN_StringList2_AppendString(sl2, "3:Third string", 0,
                                GWEN_StringList2_IntertMode_Reuse);

  fprintf(stderr, "List:\n");
  it=GWEN_StringList2_First(sl2);
  if (it) {
    const char *t;
    int i;

    t=GWEN_StringList2Iterator_Data(it);
    i=0;
    while(t) {
      fprintf(stderr, "String %d: %s [%d]\n", i, t,
              GWEN_StringList2Iterator_GetLinkCount(it));
      t=GWEN_StringList2Iterator_Next(it);
    }
    GWEN_StringList2Iterator_free(it);
  }

  GWEN_StringList2_RemoveString(sl2, "3:Third string");
  fprintf(stderr, "List:\n");
  it=GWEN_StringList2_First(sl2);
  if (it) {
    const char *t;
    int i;

    t=GWEN_StringList2Iterator_Data(it);
    i=0;
    while(t) {
      fprintf(stderr, "String %d: %s [%d]\n", i, t,
              GWEN_StringList2Iterator_GetLinkCount(it));
      t=GWEN_StringList2Iterator_Next(it);
    }
    GWEN_StringList2Iterator_free(it);
  }

  sl2copy=GWEN_StringList2_dup(sl2);
  fprintf(stderr, "CopyList:\n");
  it=GWEN_StringList2_First(sl2copy);
  if (it) {
    const char *t;
    int i;

    t=GWEN_StringList2Iterator_Data(it);
    i=0;
    while(t) {
      fprintf(stderr, "String %d: %s [%d]\n", i, t,
              GWEN_StringList2Iterator_GetLinkCount(it));
      t=GWEN_StringList2Iterator_Next(it);
    }
    GWEN_StringList2Iterator_free(it);
  }

  GWEN_StringList2_RemoveString(sl2, "3:Third string");
  fprintf(stderr, "List:\n");
  it=GWEN_StringList2_First(sl2);
  if (it) {
    const char *t;
    int i;

    t=GWEN_StringList2Iterator_Data(it);
    i=0;
    while(t) {
      fprintf(stderr, "String %d: %s [%d]\n", i, t,
              GWEN_StringList2Iterator_GetLinkCount(it));
      t=GWEN_StringList2Iterator_Next(it);
    }
    GWEN_StringList2Iterator_free(it);
  }
  fprintf(stderr, "CopyList:\n");
  it=GWEN_StringList2_First(sl2copy);
  if (it) {
    const char *t;
    int i;

    t=GWEN_StringList2Iterator_Data(it);
    i=0;
    while(t) {
      fprintf(stderr, "String %d: %s [%d]\n", i, t,
              GWEN_StringList2Iterator_GetLinkCount(it));
      t=GWEN_StringList2Iterator_Next(it);
    }
    GWEN_StringList2Iterator_free(it);
  }

  GWEN_StringList2_RemoveString(sl2copy, "2:Second string");
  fprintf(stderr, "List:\n");
  it=GWEN_StringList2_First(sl2);
  if (it) {
    const char *t;
    int i;

    t=GWEN_StringList2Iterator_Data(it);
    i=0;
    while(t) {
      fprintf(stderr, "String %d: %s [%d]\n", i, t,
              GWEN_StringList2Iterator_GetLinkCount(it));
      t=GWEN_StringList2Iterator_Next(it);
    }
    GWEN_StringList2Iterator_free(it);
  }
  fprintf(stderr, "CopyList:\n");
  it=GWEN_StringList2_First(sl2copy);
  if (it) {
    const char *t;
    int i;

    t=GWEN_StringList2Iterator_Data(it);
    i=0;
    while(t) {
      fprintf(stderr, "String %d: %s [%d]\n", i, t,
              GWEN_StringList2Iterator_GetLinkCount(it));
      t=GWEN_StringList2Iterator_Next(it);
    }
    GWEN_StringList2Iterator_free(it);
  }


  GWEN_StringList2_free(sl2);

  return 0;
}



int testFuzzy(int argc, char **argv) {
  const char *w1, *w2;
  GWEN_TYPE_UINT32 score = 0;

  if (argc<4) {
    fprintf(stderr, "Two extra-arguments needed.\n");
    return 1;
  }
  w1=argv[2];
  w2=argv[3];

  //score=GWEN_Text_FuzzyCompare(w1, w2);
  fprintf(stderr, "Similarity: "GWEN_TYPE_TMPL_UINT32"\n", score);
  return 0;
}



int testSort(int argc, char **argv) {
  GWEN_STRINGLIST *sl;
  GWEN_STRINGLISTENTRY *se;
  unsigned int j;
  char *x;

  if (argc<3) {
    fprintf(stderr, "At least one argument for stringlist needed.\n");
    return 1;
  }
  sl=GWEN_StringList_new();
  GWEN_StringList_SetSenseCase(sl, 1);
  for (j=2; j<argc; j++) {
    GWEN_StringList_AppendString(sl, argv[j], 0, 1);
    fprintf(stderr, "Adding string \"%s\"\n", argv[j]);
  }
  GWEN_StringList_Sort(sl, 0, 0);

  se=GWEN_StringList_FirstEntry(sl);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    fprintf(stderr, "- %s\n", s);
    se=GWEN_StringListEntry_Next(se);
  }

  x=strdup("Simple test");
  free(x);

  return 0;
}



int testBIO(int argc, char **argv) {
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;
  int fd;
  FILE *f;
  char buf[128];
  char buf2[512];
  unsigned int bread;

  f=fopen("testfile.128", "w+");
  if (!f) {
    DBG_ERROR(0, "fopen: %s", strerror(errno));
    return 1;
  }

  memset(buf, 0xa5, 128);
  if (1!=fwrite(buf, 128, 1, f)) {
    DBG_ERROR(0, "fwrite: %s", strerror(errno));
    return 1;
  }

  if (fclose(f)) {
    DBG_ERROR(0, "fclose: %s", strerror(errno));
    return 1;
  }

  fd=open("testfile.128", O_RDONLY);
  if (fd==-1) {
    DBG_ERROR(0, "open: %s", strerror(errno));
    return 1;
  }

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);

  memset(buf2, 0, sizeof(buf2));
  bread=128;
  err=GWEN_BufferedIO_ReadRawForced(bio, buf2, &bread);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR(0, "Got an error (%d bytes read): %d",
              bread,
              GWEN_Error_GetSimpleCode(err));
    DBG_ERROR_ERR(0, err);
    return 2;
  }
  else {
    DBG_ERROR(0, "Got this return value (%d bytes read):", bread);
    DBG_ERROR_ERR(0, err);
  }

  bread=128;
  err=GWEN_BufferedIO_ReadRawForced(bio, buf2+128, &bread);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR(0, "Got an error (%d bytes read): %d",
              bread,
              GWEN_Error_GetSimpleCode(err));
    DBG_ERROR_ERR(0, err);
    return 2;
  }
  else {
    DBG_ERROR(0, "Got this return value (%d bytes read):", bread);
    DBG_ERROR_ERR(0, err);
  }

  f=fopen("testfile.out", "w+");
  if (!f) {
    DBG_ERROR(0, "fopen: %s", strerror(errno));
    return 1;
  }

  if (1!=fwrite(buf2, bread+128, 1, f)) {
    DBG_ERROR(0, "fwrite: %s", strerror(errno));
    return 1;
  }

  if (fclose(f)) {
    DBG_ERROR(0, "fclose: %s", strerror(errno));
    return 1;
  }

  return 0;
}



int testTransformPin(int argc, char **argv) {
  GWEN_CRYPTTOKEN_PINENCODING peSrc;
  GWEN_CRYPTTOKEN_PINENCODING peDst;
  unsigned char buffer[1024];
  unsigned int pinLength;
  int rv;

  if (argc<5) {
    fprintf(stderr, "Usage: %s %s SOURCE_ENCODING DEST_ENCODING PIN\n",
            argv[0], argv[1]);
    return 1;
  }

  memset(buffer, 0, sizeof(buffer));
  strcpy((char*)buffer, argv[4]);
  pinLength=strlen((const char*)buffer);

  peSrc=GWEN_CryptToken_PinEncoding_fromString(argv[2]);
  peDst=GWEN_CryptToken_PinEncoding_fromString(argv[3]);
  rv=GWEN_CryptToken_TransformPin(peSrc, peDst,
                                  buffer,
                                  sizeof(buffer),
                                  &pinLength);
  if (rv) {
    DBG_ERROR(0, "Could not transform pin: %d", rv);
    return 2;
  }

  GWEN_Text_DumpString((const char*)buffer, pinLength, stderr, 1);


  return 0;
}



int testDbKey(int argc, char **argv) {
  GWEN_DB_NODE *db;
  GWEN_CRYPTKEY *key;

  fprintf(stderr,"Creating DB\n");
  db=GWEN_DB_Group_new("Config");

  fprintf(stderr,"Reading file\n");
  if (GWEN_DB_ReadFile(db, "server.key", GWEN_DB_FLAGS_DEFAULT)) {
    fprintf(stderr,"Error reading file.\n");
    return 1;
  }

  key=GWEN_CryptKey_fromDb(db);
  if (key==0) {
    fprintf(stderr,"No key.\n");
    return 2;
  }

  fprintf(stderr, "Chunk size: %u\n",
          GWEN_CryptKey_GetChunkSize(key));

  fprintf(stderr,"Releasing DB\n");
  GWEN_DB_Group_free(db);
  return 0;
}



int testNlSocketConnect(int argc, char **argv) {
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETLAYER *nl;
  const char *tstr;
  char buffer[4096];
  int bsize;
  int rv;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Verbous);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  nl=GWEN_NetLayerSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  GWEN_InetAddr_SetAddress(addr, "192.168.115.1");
  GWEN_InetAddr_SetPort(addr, 80);
  GWEN_NetLayer_SetPeerAddr(nl, addr);
  GWEN_InetAddr_free(addr);

  GWEN_Net_AddConnectionToPool(nl);

  /* create connection layer */
  rv=GWEN_NetLayer_Connect_Wait(nl, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not connect (%d)\n", rv);
    return 2;
  }
  fprintf(stderr, "Connected.\n");

  tstr="GET / HTTP/1.0\r\n";
  bsize=strlen(tstr);
  fprintf(stderr, "Writing something to the peer...\n");
  rv=GWEN_NetLayer_Write(nl, tstr, &bsize);
  if (rv) {
    fprintf(stderr, "ERROR: Could not write (%d)\n", rv);
    return 2;
  }
  if (bsize!=strlen(tstr)) {
    fprintf(stderr, "ERROR: Could not write all (only %d bytes)\n", bsize);
    return 2;
  }

  fprintf(stderr, "Waiting for response...\n");
  bsize=sizeof(buffer);
  rv=GWEN_NetLayer_Read_Wait(nl, buffer, &bsize, 30);
  if (rv==-1) {
    fprintf(stderr, "ERROR: Could not read\n");
    return 2;
  }
  else if (rv==1) {
    fprintf(stderr, "ERROR: Could not read due to a timeout\n");
    return 2;
  }
  buffer[bsize]=0;
  fprintf(stderr, "Response was: \"%s\"\n", buffer);


  fprintf(stderr, "Shutting down connection...\n");
  GWEN_NetLayer_Disconnect(nl);
  GWEN_NetLayer_free(nl);

  fprintf(stderr, "done.\n");
  return 0;
}



int testNlSocketAccept(int argc, char **argv) {
  GWEN_NETLAYER *nl, *incoming;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  char addrBuffer[128];
  const char *tstr;
  char buffer[1024];
  int bsize;
  int rv;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Verbous);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  nl=GWEN_NetLayerSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  GWEN_InetAddr_SetAddress(addr, "192.168.115.2");
  GWEN_InetAddr_SetPort(addr, 55555);
  GWEN_NetLayer_SetLocalAddr(nl, addr);
  GWEN_InetAddr_free(addr);

  GWEN_Net_AddConnectionToPool(nl);

  fprintf(stderr, "Starting to listen\n");
  rv=GWEN_NetLayer_Listen(nl);
  if (rv) {
    fprintf(stderr, "Could not start to listen (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Waiting for incoming connection...\n");
  incoming=GWEN_NetLayer_GetIncomingLayer_Wait(nl, 60);
  if (!incoming) {
    fprintf(stderr, "No incoming connection, aborting.\n");
    return 2;
  }

  GWEN_Net_AddConnectionToPool(incoming);

  fprintf(stderr, "Got an incoming connection.\n");
  GWEN_InetAddr_GetAddress(GWEN_NetLayer_GetPeerAddr(incoming),
                           addrBuffer, sizeof(addrBuffer));

  DBG_INFO(0, "Peer is: %s (port %d)",
           addrBuffer,
           GWEN_InetAddr_GetPort(GWEN_NetLayer_GetPeerAddr(incoming)));

  while(1) {
    fprintf(stderr, "Waiting for peer`s speach...\n");
    bsize=sizeof(buffer);
    rv=GWEN_NetLayer_Read_Wait(incoming, buffer, &bsize, 30);
    if (rv==-1) {
      fprintf(stderr, "ERROR: Could not read\n");
      return 2;
    }
    else if (rv==1) {
      fprintf(stderr, "ERROR: Could not read due to a timeout\n");
      return 2;
    }
    if (bsize==0) {
      fprintf(stderr, "EOF met, leaving\n");
      break;
    }
    buffer[bsize]=0;
    fprintf(stderr, "Speach was: \"%s\"\n", buffer);

    tstr="Hello client";
    bsize=strlen(tstr);
    fprintf(stderr, "Writing answer to the peer...\n");
    rv=GWEN_NetLayer_Write_Wait(incoming, tstr, &bsize, 30);
    if (rv) {
      fprintf(stderr, "ERROR: Could not write (%d)\n", rv);
      return 2;
    }
    if (bsize!=strlen(tstr)) {
      fprintf(stderr, "ERROR: Could not write all (only %d bytes)\n", bsize);
      return 2;
    }
  } /* while */

  fprintf(stderr, "Shutting down incoming connection...\n");
  GWEN_NetLayer_Disconnect_Wait(incoming, 30);
  GWEN_NetLayer_free(incoming);

  fprintf(stderr, "Shutting down listening connection...\n");
  GWEN_NetLayer_Disconnect_Wait(nl, 30);
  GWEN_NetLayer_free(nl);

  fprintf(stderr, "done.\n");
  return 0;
}



int testNlHttpConnect1(int argc, char **argv) {
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETLAYER *baseLayer, *nl;
/*   char buffer[4096]; */
/*   int bsize; */
  int rv;
  GWEN_URL *url;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Verbous);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  baseLayer=GWEN_NetLayerSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  GWEN_InetAddr_SetAddress(addr, "192.168.115.1");
  GWEN_InetAddr_SetPort(addr, 80);
  GWEN_NetLayer_SetPeerAddr(baseLayer, addr);
  GWEN_InetAddr_free(addr);

  nl=GWEN_NetLayerHttp_new(baseLayer); /* attaches to baseLayer */
  GWEN_NetLayer_free(baseLayer);

  GWEN_Net_AddConnectionToPool(nl);

  /* create connection layer */
  rv=GWEN_NetLayer_Connect_Wait(nl, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not connect (%d)\n", rv);
    return 2;
  }
  fprintf(stderr, "Connected.\n");

  url=GWEN_Url_fromString("http://192.168.115.1/");
  assert(url);

  GWEN_NetLayerHttp_SetOutCommand(nl, "GET", url);
  rv=GWEN_NetLayer_BeginOutPacket(nl, 0); /* no body */
  if (rv) {
    fprintf(stderr, "ERROR: Could not begin packet (%d)\n", rv);
    return 2;
  }

  rv=GWEN_NetLayer_EndOutPacket_Wait(nl, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not end packet (%d)\n", rv);
    return 2;
  }

#if 0
  fprintf(stderr, "Waiting for response...\n");
  bsize=sizeof(buffer);
  rv=GWEN_NetLayer_Read_Wait(nl, buffer, &bsize, 30);
  if (rv==-1) {
    fprintf(stderr, "ERROR: Could not read\n");
    return 2;
  }
  else if (rv==1) {
    fprintf(stderr, "ERROR: Could not read due to a timeout\n");
    return 2;
  }
  buffer[bsize]=0;
  fprintf(stderr, "Response was: \"%s\"\n", buffer);
#endif

  fprintf(stderr, "Shutting down connection...\n");
  GWEN_NetLayer_Disconnect(nl);
  GWEN_NetLayer_free(nl);

  fprintf(stderr, "done.\n");
  return 0;
}



int testNlHttpConnect2(int argc, char **argv) {
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETLAYER *baseLayer, *nl;
  const char *tstr;
  char buffer[4096];
  int bsize;
  int rv;
  GWEN_URL *url;
  GWEN_BUFFER *rbuf;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Verbous);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  baseLayer=GWEN_NetLayerSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  GWEN_InetAddr_SetName(addr, "www.aquamaniac.de");
  GWEN_InetAddr_SetPort(addr, 80);
  GWEN_NetLayer_SetPeerAddr(baseLayer, addr);
  GWEN_InetAddr_free(addr);

  nl=GWEN_NetLayerHttp_new(baseLayer); /* attaches to baseLayer */
  GWEN_NetLayer_free(baseLayer);

  GWEN_Net_AddConnectionToPool(nl);

  /* create connection layer */
  rv=GWEN_NetLayer_Connect_Wait(nl, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not connect (%d)\n", rv);
    return 2;
  }
  fprintf(stderr, "Connected.\n");

  url=GWEN_Url_fromString("http://www.aquamaniac.de/aqbanking/");
  assert(url);

  tstr="Some data";
  bsize=strlen(tstr);
  GWEN_NetLayerHttp_SetOutCommand(nl, "GET", url);
  rv=GWEN_NetLayer_BeginOutPacket(nl, bsize);
  if (rv) {
    fprintf(stderr, "ERROR: Could not begin packet (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Writing something to the peer...\n");
  rv=GWEN_NetLayer_Write_Wait(nl, tstr, &bsize, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not write (%d)\n", rv);
    return 2;
  }
  if (bsize!=strlen(tstr)) {
    fprintf(stderr, "ERROR: Could not write all (only %d bytes)\n", bsize);
    return 2;
  }

  rv=GWEN_NetLayer_EndOutPacket_Wait(nl, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not end packet (%d)\n", rv);
    return 2;
  }

  rbuf=GWEN_Buffer_new(0, 512, 0, 1);
  rv=GWEN_NetLayer_BeginInPacket(nl);
  if (rv) {
    fprintf(stderr, "Could not start to read (%d)\n", rv);
    return 2;
  }

  for (;;) {
    rv=GWEN_NetLayer_CheckInPacket(nl);
    fprintf(stderr, "Check-Result: %d\n", rv);
    if (rv<0) {
      fprintf(stderr, "Error checking packet (%d)\n", rv);
      return 2;
    }
    else if (rv==1) {
      fprintf(stderr, "Reading...\n");
      bsize=sizeof(buffer);
      rv=GWEN_NetLayer_Read_Wait(nl, buffer, &bsize, 30);
      if (rv==-1) {
        fprintf(stderr, "ERROR: Could not read\n");
        return 2;
      }
      else if (rv==1) {
        fprintf(stderr, "ERROR: Could not read due to a timeout\n");
        return 2;
      }
      else {
        if (bsize!=0) {
          buffer[bsize]=0;
          GWEN_Buffer_AppendBytes(rbuf, buffer, bsize);
        }
      }
    }
    else
      break;
  }

  fprintf(stderr, "Response was: \"%s\"\n",
          GWEN_Buffer_GetStart(rbuf));

  fprintf(stderr, "Shutting down connection...\n");
  GWEN_NetLayer_Disconnect(nl);
  GWEN_NetLayer_free(nl);

  fprintf(stderr, "done.\n");
  return 0;
}



int testNlHttpConnect3(int argc, char **argv) {
  const char *urlString;
  const char *outFile=0;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETLAYER *baseLayer, *nl;
  char buffer[4096];
  int bsize;
  int rv;
  GWEN_URL *url;
  GWEN_BUFFER *rbuf;
  GWEN_DB_NODE *dbT;
  GWEN_ERRORCODE err;

  if (argc<3) {
    fprintf(stderr, "%s %s URL [FILE]\n", argv[0], argv[1]);
    return 1;
  }
  if (argc==4)
    outFile=argv[3];
  urlString=argv[2];

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Verbous);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Debug);

  url=GWEN_Url_fromString(urlString);
  assert(url);


  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  baseLayer=GWEN_NetLayerSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  err=GWEN_InetAddr_SetAddress(addr, GWEN_Url_GetServer(url));
  if (!GWEN_Error_IsOk(err))
    err=GWEN_InetAddr_SetName(addr, GWEN_Url_GetServer(url));
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  GWEN_InetAddr_SetPort(addr, GWEN_Url_GetPort(url));
  GWEN_NetLayer_SetPeerAddr(baseLayer, addr);
  GWEN_InetAddr_free(addr);

  nl=GWEN_NetLayerHttp_new(baseLayer); /* attaches to baseLayer */
  GWEN_NetLayer_free(baseLayer);

  GWEN_Net_AddConnectionToPool(nl);

  /* connect */
  rv=GWEN_NetLayer_Connect_Wait(nl, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not connect (%d)\n", rv);
    return 2;
  }
  fprintf(stderr, "Connected.\n");

  dbT=GWEN_NetLayerHttp_GetOutHeader(nl);
  assert(dbT);
  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "host",
                       GWEN_Url_GetServer(url));

  GWEN_NetLayerHttp_SetOutCommand(nl, "GET", url);
  rv=GWEN_NetLayer_BeginOutPacket(nl, 0);
  if (rv) {
    fprintf(stderr, "ERROR: Could not begin packet (%d)\n", rv);
    return 2;
  }

  rv=GWEN_NetLayer_EndOutPacket_Wait(nl, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not end packet (%d)\n", rv);
    return 2;
  }

  rbuf=GWEN_Buffer_new(0, 512, 0, 1);
  rv=GWEN_NetLayer_BeginInPacket(nl);
  if (rv) {
    fprintf(stderr, "Could not start to read (%d)\n", rv);
    return 2;
  }

  for (;;) {
    rv=GWEN_NetLayer_CheckInPacket(nl);
    fprintf(stderr, "Check-Result: %d\n", rv);
    if (rv<0) {
      fprintf(stderr, "Error checking packet (%d)\n", rv);
      return 2;
    }
    else if (rv==1) {
      fprintf(stderr, "Reading...\n");
      bsize=sizeof(buffer);
      rv=GWEN_NetLayer_Read_Wait(nl, buffer, &bsize, 30);
      if (rv<0) {
        fprintf(stderr, "ERROR: Could not read (%d)\n", rv);
        return 2;
      }
      else if (rv==1) {
        fprintf(stderr, "ERROR: Could not read due to a timeout\n");
        return 2;
      }
      else {
        if (bsize==0) {
          fprintf(stderr, "INFO: EOF met\n");
          break;
        }
        else {
          buffer[bsize]=0;
          GWEN_Buffer_AppendBytes(rbuf, buffer, bsize);
        }
      }
    }
    else
      break;
  }

  if (GWEN_Buffer_GetUsedBytes(rbuf) && outFile) {
    FILE *f;

    f=fopen(outFile, "w+");
    if (!f) {
      fprintf(stderr, "ERROR: Could not open outfile.\n");
      return 3;
    }
    if (1!=fwrite(GWEN_Buffer_GetStart(rbuf),
                  GWEN_Buffer_GetUsedBytes(rbuf),
                  1, f)) {
      fprintf(stderr, "ERROR: Could not write to outfile.\n");
      fclose(f);
      return 3;
    }
    if (fclose(f)) {
      fprintf(stderr, "ERROR: Could not close outfile.\n");
      return 3;
    }
  }

  fprintf(stderr, "Shutting down connection...\n");
  GWEN_NetLayer_Disconnect(nl);
  GWEN_NetLayer_free(nl);

  fprintf(stderr, "done.\n");
  return 0;
}



int testNlHttpConnect4(int argc, char **argv) {
  const char *urlString;
  const char *outFile=0;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETLAYER *baseLayer, *nl;
  int rv;
  GWEN_URL *url;
  GWEN_ERRORCODE err;
  GWEN_BUFFEREDIO *bio;
  int fd;

  if (argc<4) {
    fprintf(stderr, "%s %s URL FILE\n", argv[0], argv[1]);
    return 1;
  }
  outFile=argv[3];
  urlString=argv[2];

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Verbous);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Debug);

  url=GWEN_Url_fromString(urlString);
  assert(url);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  baseLayer=GWEN_NetLayerSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  err=GWEN_InetAddr_SetAddress(addr, GWEN_Url_GetServer(url));
  if (!GWEN_Error_IsOk(err))
    err=GWEN_InetAddr_SetName(addr, GWEN_Url_GetServer(url));
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  GWEN_InetAddr_SetPort(addr, GWEN_Url_GetPort(url));
  GWEN_NetLayer_SetPeerAddr(baseLayer, addr);
  GWEN_InetAddr_free(addr);

  nl=GWEN_NetLayerHttp_new(baseLayer); /* attaches to baseLayer */
  GWEN_NetLayer_free(baseLayer);

  GWEN_Net_AddConnectionToPool(nl);

  /* connect */
  rv=GWEN_NetLayer_Connect_Wait(nl, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not connect (%d)\n", rv);
    return 2;
  }
  fprintf(stderr, "Connected.\n");

  fd=open(outFile, O_CREAT | O_WRONLY);
  if (fd==-1) {
    fprintf(stderr, "Could not create outFile\n");
    return 2;
  }
  bio=GWEN_BufferedIO_File_new(fd);
  rv=GWEN_NetLayerHttp_Request(nl, "GET", url,
                               0, /* dbHeader */
                               0, 0, /* body */
                               bio);
  fprintf(stderr, "INFO: Result of request: %d\n", rv);
  if (rv<0) {
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return 3;
  }
  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 3;
  }
  GWEN_BufferedIO_free(bio);

  fprintf(stderr, "Shutting down connection...\n");
  GWEN_NetLayer_Disconnect(nl);
  GWEN_NetLayer_free(nl);

  fprintf(stderr, "done.\n");
  return 0;
}



int testNlHttpAccept1(int argc, char **argv) {
  GWEN_NETLAYER *baseLayer, *nl, *incoming;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  char addrBuffer[128];
  const char *tstr;
  char buffer[4096];
  int bsize;
  int rv;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Verbous);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  baseLayer=GWEN_NetLayerSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  GWEN_InetAddr_SetAddress(addr, "192.168.115.2");
  GWEN_InetAddr_SetPort(addr, 55555);
  GWEN_NetLayer_SetLocalAddr(baseLayer, addr);
  GWEN_InetAddr_free(addr);

  nl=GWEN_NetLayerHttp_new(baseLayer); /* attaches to baseLayer */
  GWEN_NetLayer_free(baseLayer);

  GWEN_Net_AddConnectionToPool(nl);

  fprintf(stderr, "Starting to listen\n");
  rv=GWEN_NetLayer_Listen(nl);
  if (rv) {
    fprintf(stderr, "Could not start to listen (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Waiting for incoming connection...\n");
  incoming=GWEN_NetLayer_GetIncomingLayer_Wait(nl, 60);
  if (!incoming) {
    fprintf(stderr, "No incoming connection, aborting.\n");
    return 2;
  }

  GWEN_Net_AddConnectionToPool(incoming);

  fprintf(stderr, "Got an incoming connection.\n");
  GWEN_InetAddr_GetAddress(GWEN_NetLayer_GetPeerAddr(incoming),
                           addrBuffer, sizeof(addrBuffer));

  DBG_INFO(0, "Peer is: %s (port %d)",
           addrBuffer,
           GWEN_InetAddr_GetPort(GWEN_NetLayer_GetPeerAddr(incoming)));

  fprintf(stderr, "Waiting for peer`s speach...\n");
  rv=GWEN_NetLayer_BeginInPacket(incoming);
  if (rv) {
    fprintf(stderr, "Could not start to read (%d)\n", rv);
    return 2;
  }

  for (;;) {
    rv=GWEN_NetLayer_CheckInPacket(incoming);
    fprintf(stderr, "Check-Result: %d\n", rv);
    if (rv<0) {
      fprintf(stderr, "Error checking packet (%d)\n", rv);
      return 2;
    }
    else if (rv==1) {
      fprintf(stderr, "Reading...\n");
      bsize=sizeof(buffer);
      rv=GWEN_NetLayer_Read_Wait(incoming, buffer, &bsize, 30);
      if (rv==-1) {
        fprintf(stderr, "ERROR: Could not read\n");
        return 2;
      }
      else if (rv==1) {
        fprintf(stderr, "ERROR: Could not read due to a timeout\n");
        return 2;
      }
      else {
        if (bsize!=0) {
          buffer[bsize]=0;
          fprintf(stderr, "Speach was: \"%s\"\n", buffer);
        }
      }
    }
    else
      break;
  }

  tstr="Hi, this is the result :-)";
  bsize=strlen(tstr);
  GWEN_NetLayerHttp_SetOutStatus(incoming, 200, "Ok, hello client");
  rv=GWEN_NetLayer_BeginOutPacket(incoming, bsize);
  if (rv) {
    fprintf(stderr, "ERROR: Could not begin packet (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Writing something to the peer...\n");
  rv=GWEN_NetLayer_Write_Wait(incoming, tstr, &bsize, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not write (%d)\n", rv);
    return 2;
  }
  if (bsize!=strlen(tstr)) {
    fprintf(stderr, "ERROR: Could not write all (only %d bytes)\n", bsize);
    return 2;
  }

  rv=GWEN_NetLayer_EndOutPacket_Wait(incoming, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not end packet (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Shutting down incoming connection...\n");
  GWEN_NetLayer_Disconnect_Wait(incoming, 30);
  GWEN_NetLayer_free(incoming);

  fprintf(stderr, "Shutting down listening connection...\n");
  GWEN_NetLayer_Disconnect_Wait(nl, 30);
  GWEN_NetLayer_free(nl);

  fprintf(stderr, "done.\n");
  return 0;

}



GWEN_NL_SSL_ASKADDCERT_RESULT _nlAskAddCert(GWEN_NETLAYER *nl,
                                            const GWEN_SSLCERTDESCR *cd,
                                            void *user_data) {
  GWEN_DB_NODE *dbCert;

  dbCert=GWEN_DB_Group_new("cert");
  GWEN_SslCertDescr_toDb(cd, dbCert);
  fprintf(stderr, "Got this certificate:");
  GWEN_DB_Dump(dbCert, stderr, 2);
  GWEN_DB_Group_free(dbCert);
  return GWEN_NetLayerSsl_AskAddCertResult_Tmp;
}



int testNlSslConnect1(int argc, char **argv) {
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETLAYER *baseLayer, *nl;
  int rv;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Verbous);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  baseLayer=GWEN_NetLayerSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  GWEN_InetAddr_SetAddress(addr, "192.168.115.1");
  GWEN_InetAddr_SetPort(addr, 443);
  GWEN_NetLayer_SetPeerAddr(baseLayer, addr);
  GWEN_InetAddr_free(addr);

  nl=GWEN_NetLayerSsl_new(baseLayer,
                          "trusted",
                          "newtrusted",
                          0, //"lancelot.crt",
                          "dh_1024.pem",
                          0);
  GWEN_NetLayer_free(baseLayer);

  GWEN_NetLayerSsl_SetAskAddCertFn(nl, _nlAskAddCert, 0);

  GWEN_Net_AddConnectionToPool(nl);

  /* create connection layer */
  rv=GWEN_NetLayer_Connect_Wait(nl, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not connect (%d)\n", rv);
    return 2;
  }
  fprintf(stderr, "Connected.\n");

  fprintf(stderr, "Shutting down connection...\n");
  GWEN_NetLayer_Disconnect(nl);
  GWEN_NetLayer_free(nl);

  fprintf(stderr, "done.\n");
  return 0;
}



int testNlSslConnect2(int argc, char **argv) {
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETLAYER *baseLayer, *nlssl, *nl;
  int rv;
  GWEN_URL *url;
  GWEN_ERRORCODE err;
  GWEN_BUFFEREDIO *bio;
  int fd;
  const char *urlString;
  const char *outFile=0;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Verbous);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);

  if (argc<4) {
    fprintf(stderr, "%s %s URL FILE\n", argv[0], argv[1]);
    return 1;
  }
  outFile=argv[3];
  urlString=argv[2];

  url=GWEN_Url_fromString(urlString);
  assert(url);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  baseLayer=GWEN_NetLayerSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  err=GWEN_InetAddr_SetAddress(addr, GWEN_Url_GetServer(url));
  if (!GWEN_Error_IsOk(err))
    err=GWEN_InetAddr_SetName(addr, GWEN_Url_GetServer(url));
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }
  GWEN_InetAddr_SetPort(addr, GWEN_Url_GetPort(url));
  GWEN_NetLayer_SetPeerAddr(baseLayer, addr);
  GWEN_InetAddr_free(addr);

  nlssl=GWEN_NetLayerSsl_new(baseLayer,
                             "trusted",
                             "newtrusted",
                             0, //"lancelot.crt",
                             "dh_1024.pem",
                             0);
  GWEN_NetLayer_free(baseLayer);
  GWEN_NetLayerSsl_SetAskAddCertFn(nlssl, _nlAskAddCert, 0);

  nl=GWEN_NetLayerHttp_new(nlssl);
  GWEN_NetLayer_free(nlssl);

  GWEN_Net_AddConnectionToPool(nl);

  /* connect */
  rv=GWEN_NetLayer_Connect_Wait(nl, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not connect (%d)\n", rv);
    return 2;
  }
  fprintf(stderr, "Connected.\n");

  fd=open(outFile, O_CREAT | O_WRONLY);
  if (fd==-1) {
    fprintf(stderr, "Could not create outFile\n");
    return 2;
  }
  bio=GWEN_BufferedIO_File_new(fd);
  rv=GWEN_NetLayerHttp_Request(nl, "GET", url,
                               0, /* dbHeader */
                               0, 0, /* body */
                               bio);
  fprintf(stderr, "INFO: Result of request: %d\n", rv);
  if (rv<0) {
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return 3;
  }
  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 3;
  }
  GWEN_BufferedIO_free(bio);

  fprintf(stderr, "Shutting down connection...\n");
  GWEN_NetLayer_Disconnect(nl);
  GWEN_NetLayer_free(nl);

  fprintf(stderr, "done.\n");
  return 0;
}



int testIpcServer1(int argc, char **argv) {
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETLAYER *baseLayer, *nl;
  int rv;
  GWEN_URL *url;
  GWEN_DB_NODE *dbUrl;
  GWEN_ERRORCODE err;
  GWEN_IPCMANAGER *ipcManager;
  GWEN_TYPE_UINT32 sid;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Verbous);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);

  url=GWEN_Url_fromString("/tmp/test.comm");
  assert(url);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeUnix);
  baseLayer=GWEN_NetLayerSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyUnix);
  dbUrl=GWEN_DB_Group_new("URL");
  GWEN_Url_toDb(url, dbUrl);
  GWEN_DB_Dump(dbUrl, stderr, 2);

  err=GWEN_InetAddr_SetAddress(addr, "/tmp/test.comm");
  if (!GWEN_Error_IsOk(err))
    err=GWEN_InetAddr_SetName(addr, GWEN_Url_GetServer(url));
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  GWEN_NetLayer_SetLocalAddr(baseLayer, addr);
  GWEN_InetAddr_free(addr);

  nl=GWEN_NetLayerHttp_new(baseLayer); /* attaches to baseLayer */
  GWEN_NetLayer_free(baseLayer);
  GWEN_Url_SetServer(url, "/tmp/test.comm");
  GWEN_Url_SetPath(url, "/ipc");
  GWEN_NetLayerHttp_SetOutCommand(nl, "POST", url);

  ipcManager=GWEN_IpcManager_new();
  sid=GWEN_IpcManager_AddServer(ipcManager, nl, 1);
  fprintf(stderr, "Server id: %08x\n", sid);

  for(;;) {
    GWEN_TYPE_UINT32 rid;

    rid=GWEN_IpcManager_GetNextInRequest(ipcManager, 1);
    if (rid==0) {
      GWEN_Net_HeartBeat(750);
      rv=GWEN_IpcManager_Work(ipcManager);
      if (rv<0) {
        fprintf(stderr, "ERROR: Could not work\n");
        break;
      }
    }
    else {
      GWEN_DB_NODE *dbRequest;
      GWEN_DB_NODE *dbResponse;

      fprintf(stderr, "Got an incoming request.\n");
      dbRequest=GWEN_IpcManager_GetInRequestData(ipcManager, rid);
      GWEN_DB_Dump(dbRequest, stderr, 2);
      dbResponse=GWEN_DB_Group_new("Test-Response");
      GWEN_DB_SetCharValue(dbResponse, GWEN_DB_FLAGS_DEFAULT,
                           "Response", "Hi :-)");
      rv=GWEN_IpcManager_SendResponse(ipcManager, rid, dbResponse);
      if (rv) {
        fprintf(stderr, "ERROR: Could not send response\n");
        break;
      }
    }
  }

  fprintf(stderr, "done.\n");
  return 0;
}



int testIpcClient1(int argc, char **argv) {
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETLAYER *baseLayer, *nl;
  int rv;
  GWEN_URL *url;
  GWEN_DB_NODE *dbUrl;
  GWEN_ERRORCODE err;
  GWEN_IPCMANAGER *ipcManager;
  GWEN_TYPE_UINT32 sid;
  GWEN_TYPE_UINT32 rid;
  GWEN_DB_NODE *dbRequest;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Verbous);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);

  url=GWEN_Url_fromString("/var/run/chipcard2.comm");
  assert(url);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeUnix);
  baseLayer=GWEN_NetLayerSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyUnix);
  dbUrl=GWEN_DB_Group_new("URL");
  GWEN_Url_toDb(url, dbUrl);
  GWEN_DB_Dump(dbUrl, stderr, 2);

  err=GWEN_InetAddr_SetAddress(addr, "/var/run/chipcard2.comm");
  if (!GWEN_Error_IsOk(err))
    err=GWEN_InetAddr_SetName(addr, GWEN_Url_GetServer(url));
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  GWEN_NetLayer_SetPeerAddr(baseLayer, addr);
  GWEN_InetAddr_free(addr);

  nl=GWEN_NetLayerHttp_new(baseLayer); /* attaches to baseLayer */
  GWEN_NetLayer_free(baseLayer);
  GWEN_Url_SetServer(url, "/var/run/chipcard2.comm");
  GWEN_Url_SetPath(url, "/ipc");
  GWEN_NetLayerHttp_SetOutCommand(nl, "POST", url);

  ipcManager=GWEN_IpcManager_new();
  sid=GWEN_IpcManager_AddClient(ipcManager, nl, 1);
  fprintf(stderr, "Server id: %08x\n", sid);

  dbRequest=GWEN_DB_Group_new("Test-Request");
  GWEN_DB_SetCharValue(dbRequest, GWEN_DB_FLAGS_DEFAULT,
                       "TestVar", "TestValue");
  rid=GWEN_IpcManager_SendRequest(ipcManager, sid, dbRequest);
  if (rid==0) {
    fprintf(stderr, "Could not send request.\n");
    return 2;
  }

  for(;;) {
    GWEN_DB_NODE *dbResponse;

    dbResponse=GWEN_IpcManager_GetResponseData(ipcManager, rid);
    if (dbResponse) {
      fprintf(stderr, "INFO: Response was:\n");
      GWEN_DB_Dump(dbResponse, stderr, 2);
      break;
    }
    else {
      if (GWEN_Net_HeartBeat(750)==GWEN_NetLayerResult_Error) {
        fprintf(stderr, "ERROR (heartbeat).\n");
        break;
      }
      rv=GWEN_IpcManager_Work(ipcManager);
      if (rv<0) {
        fprintf(stderr, "ERROR: Coukld not work\n");
        break;
      }
    }
  }

  fprintf(stderr, "done.\n");
  return 0;
}



int testNlFileConnect1(int argc, char **argv) {
  const char *urlString;
  const char *outFile=0;
  int fdRead;
  int fdWrite;
  GWEN_NETLAYER *baseLayer, *nl;
  int rv;
  GWEN_URL *url;
  GWEN_ERRORCODE err;
  GWEN_BUFFEREDIO *bio;
  int fd;

  if (argc<4) {
    fprintf(stderr, "%s %s URL FILE\n", argv[0], argv[1]);
    return 1;
  }
  outFile=argv[3];
  urlString=argv[2];

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Verbous);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Debug);

  url=GWEN_Url_fromString(urlString);
  assert(url);

  /* create transport layer */
  fdRead=open("test.read", O_RDONLY);
  if (fdRead==-1) {
    fprintf(stderr, "File \"test.read\" does not exist.\n");
    return 2;
  }
  fdWrite=open("test.write",
               O_RDWR | O_APPEND | O_CREAT,
               S_IRUSR | S_IWUSR);
  if (fdWrite==-1) {
    fprintf(stderr, "File \"test.write\" is not available.\n");
    return 2;
  }

  baseLayer=GWEN_NetLayerFile_new(fdRead, fdWrite, 1);

  nl=GWEN_NetLayerHttp_new(baseLayer); /* attaches to baseLayer */
  GWEN_NetLayer_free(baseLayer);

  GWEN_Net_AddConnectionToPool(nl);

  /* connect */
  rv=GWEN_NetLayer_Connect_Wait(nl, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not connect (%d)\n", rv);
    return 2;
  }
  fprintf(stderr, "Connected.\n");

  fd=open(outFile, O_CREAT | O_WRONLY);
  if (fd==-1) {
    fprintf(stderr, "Could not create outFile\n");
    return 2;
  }
  bio=GWEN_BufferedIO_File_new(fd);
  rv=GWEN_NetLayerHttp_Request(nl, "GET", url,
                               0, /* dbHeader */
                               0, 0, /* body */
                               bio);
  fprintf(stderr, "INFO: Result of request: %d\n", rv);
  if (rv<0) {
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return 3;
  }
  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 3;
  }
  GWEN_BufferedIO_free(bio);

  fprintf(stderr, "Shutting down connection...\n");
  GWEN_NetLayer_Disconnect(nl);
  GWEN_NetLayer_free(nl);

  fprintf(stderr, "done.\n");
  return 0;
}



int testNlLogConnect1(int argc, char **argv) {
  const char *urlString;
  const char *outFile=0;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETLAYER *baseLayer, *nl;
  int rv;
  GWEN_URL *url;
  GWEN_ERRORCODE err;
  GWEN_BUFFEREDIO *bio;
  int fd;

  if (argc<4) {
    fprintf(stderr, "%s %s URL FILE\n", argv[0], argv[1]);
    return 1;
  }
  outFile=argv[3];
  urlString=argv[2];

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Verbous);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Debug);

  url=GWEN_Url_fromString(urlString);
  assert(url);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  baseLayer=GWEN_NetLayerSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  err=GWEN_InetAddr_SetAddress(addr, GWEN_Url_GetServer(url));
  if (!GWEN_Error_IsOk(err))
    err=GWEN_InetAddr_SetName(addr, GWEN_Url_GetServer(url));
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  GWEN_InetAddr_SetPort(addr, GWEN_Url_GetPort(url));
  GWEN_NetLayer_SetPeerAddr(baseLayer, addr);
  GWEN_InetAddr_free(addr);

  nl=GWEN_NetLayerLog_new(baseLayer, "testlog");
  GWEN_NetLayer_free(baseLayer);
  baseLayer=nl;

  nl=GWEN_NetLayerHttp_new(baseLayer); /* attaches to baseLayer */
  GWEN_NetLayer_free(baseLayer);

  GWEN_Net_AddConnectionToPool(nl);

  /* connect */
  rv=GWEN_NetLayer_Connect_Wait(nl, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not connect (%d)\n", rv);
    return 2;
  }
  fprintf(stderr, "Connected.\n");

  fd=open(outFile, O_CREAT | O_WRONLY, S_IRUSR|S_IWUSR);
  if (fd==-1) {
    fprintf(stderr, "Could not create outFile\n");
    return 2;
  }
  bio=GWEN_BufferedIO_File_new(fd);
  rv=GWEN_NetLayerHttp_Request(nl, "GET", url,
                               0, /* dbHeader */
                               0, 0, /* body */
                               bio);
  fprintf(stderr, "INFO: Result of request: %d\n", rv);
  if (rv<0) {
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return 3;
  }
  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 3;
  }
  GWEN_BufferedIO_free(bio);

  fprintf(stderr, "Shutting down connection...\n");
  GWEN_NetLayer_Disconnect(nl);
  GWEN_NetLayer_free(nl);

  fprintf(stderr, "done.\n");
  return 0;
}



int testNlLogConnect2(int argc, char **argv) {
  const char *urlString;
  const char *outFile=0;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETLAYER *baseLayer, *nl;
  int rv;
  GWEN_URL *url;
  GWEN_ERRORCODE err;
  GWEN_BUFFEREDIO *bio;
  int fd;

  if (argc<4) {
    fprintf(stderr, "%s %s URL FILE\n", argv[0], argv[1]);
    return 1;
  }
  outFile=argv[3];
  urlString=argv[2];

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Verbous);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Debug);

  url=GWEN_Url_fromString(urlString);
  assert(url);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  baseLayer=GWEN_NetLayerSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  err=GWEN_InetAddr_SetAddress(addr, GWEN_Url_GetServer(url));
  if (!GWEN_Error_IsOk(err))
    err=GWEN_InetAddr_SetName(addr, GWEN_Url_GetServer(url));
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 2;
  }

  GWEN_InetAddr_SetPort(addr, GWEN_Url_GetPort(url));
  GWEN_NetLayer_SetPeerAddr(baseLayer, addr);
  GWEN_InetAddr_free(addr);

  nl=GWEN_NetLayerSsl_new(baseLayer,
                          "trusted",
                          "newtrusted",
                          0, //"lancelot.crt",
                          "dh_1024.pem",
                          0);
  GWEN_NetLayer_free(baseLayer);
  GWEN_NetLayerSsl_SetAskAddCertFn(nl, _nlAskAddCert, 0);
  baseLayer=nl;

  nl=GWEN_NetLayerLog_new(baseLayer, "testlog");
  GWEN_NetLayer_free(baseLayer);
  baseLayer=nl;

  nl=GWEN_NetLayerHttp_new(baseLayer); /* attaches to baseLayer */
  GWEN_NetLayer_free(baseLayer);

  GWEN_Net_AddConnectionToPool(nl);

  /* connect */
  rv=GWEN_NetLayer_Connect_Wait(nl, 30);
  if (rv) {
    fprintf(stderr, "ERROR: Could not connect (%d)\n", rv);
    return 2;
  }
  fprintf(stderr, "Connected.\n");

  fd=open(outFile, O_CREAT | O_WRONLY, S_IRUSR|S_IWUSR);
  if (fd==-1) {
    fprintf(stderr, "Could not create outFile\n");
    return 2;
  }
  bio=GWEN_BufferedIO_File_new(fd);
  rv=GWEN_NetLayerHttp_Request(nl, "GET", url,
                               0, /* dbHeader */
                               0, 0, /* body */
                               bio);
  fprintf(stderr, "INFO: Result of request: %d\n", rv);
  if (rv<0) {
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    return 3;
  }
  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 3;
  }
  GWEN_BufferedIO_free(bio);

  fprintf(stderr, "Shutting down connection...\n");
  GWEN_NetLayer_Disconnect(nl);
  GWEN_NetLayer_free(nl);

  fprintf(stderr, "done.\n");
  return 0;
}



int testMem(int argc, char **argv) {
  GWEN_XMLNODE *n;
  char *s;

  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  s=strdup("test");
  fprintf(stderr, "String: %s\n", s);
  free(s);
  GWEN_XMLNode_free(n);
  fprintf(stderr, "Memory before collection:\n");
  GWEN_Memory_Dump();
  GWEN_Memory_Collect();
  fprintf(stderr, "Memory after collection:\n");
  GWEN_Memory_Dump();
  return 0;
}



int main(int argc, char **argv) {
  int rv;

  GWEN_Init();
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Info);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Info);

  if (argc<2) {
    fprintf(stderr, "Usage: %s <test>\n  where <test> is one of db, dbfile, dbfile2, list, key, mkkey, cpkey, xml, xml2, sn, ssl, accept, connect\n", argv[0]);
    GWEN_Fini();
    return 1;
  }


  if (strcasecmp(argv[1], "dbfile")==0)
    rv=testDBfile(argc, argv);
  else if (strcasecmp(argv[1], "db")==0)
    rv=testDB(argc, argv);
  else if (strcasecmp(argv[1], "db2")==0)
    rv=testDB2(argc, argv);
  else if (strcasecmp(argv[1], "dbfile2")==0)
    rv=testDBfile2(argc, argv);
  else if (strcasecmp(argv[1], "list")==0)
    rv=testListMsg(argc, argv);
  else if (strcasecmp(argv[1], "key")==0)
    rv=testKey(argc, argv);
  else if (strcasecmp(argv[1], "mkkey")==0)
    rv=testMkKey(argc, argv);
  else if (strcasecmp(argv[1], "cpkey")==0)
    rv=testCopyKey(argc, argv);
  else if (strcasecmp(argv[1], "xml")==0)
    rv=testXML(argc, argv);
  else if (strcasecmp(argv[1], "xml2")==0)
    rv=testXML2(argc, argv);
  else if (strcasecmp(argv[1], "xml3")==0)
    rv=testXML3(argc, argv);
  else if (strcasecmp(argv[1], "xml4")==0)
    rv=testXML4(argc, argv);
  else if (strcasecmp(argv[1], "sn")==0)
    rv=testSnprintf(argc, argv);
  else if (strcasecmp(argv[1], "process")==0)
    rv=testProcess(argc, argv);
  else if (strcasecmp(argv[1], "option")==0)
    rv=testOptions(argc, argv);
  else if (strcasecmp(argv[1], "base64")==0)
    rv=testBase64(argc, argv);
  else if (strcasecmp(argv[1], "base64_2")==0)
    rv=testBase64_2(argc, argv);
  else if (strcasecmp(argv[1], "time")==0)
    rv=testTime(argc, argv);
  else if (strcasecmp(argv[1], "time2")==0)
    rv=testTimeFromString(argc, argv);
  else if (strcasecmp(argv[1], "list2")==0)
    rv=testList2(argc, argv);
  else if (strcasecmp(argv[1], "dbio")==0)
    rv=testDBIO(argc, argv);
  else if (strcasecmp(argv[1], "time1")==0)
    rv=testTimeToString(argc, argv);
  else if (strcasecmp(argv[1], "csvexport")==0)
    rv=testCsvExport(argc, argv);
  else if (strcasecmp(argv[1], "olddb")==0)
    rv=testOldDbImport(argc, argv);
  else if (strcasecmp(argv[1], "822")==0)
    rv=testRfc822Import(argc, argv);
  else if (strcasecmp(argv[1], "fslock")==0)
    rv=testFsLock(argc, argv);
  else if (strcasecmp(argv[1], "fslock2")==0)
    rv=testFsLock2(argc, argv);
  else if (strcasecmp(argv[1], "xsd")==0)
    rv=testXSD(argc, argv);
  else if (strcasecmp(argv[1], "xsd2")==0)
    rv=testXSD2(argc, argv);
  else if (strcasecmp(argv[1], "xsd3")==0)
    rv=testXSD3(argc, argv);
  else if (strcasecmp(argv[1], "xsd4")==0)
    rv=testXSD4(argc, argv);
  else if (strcasecmp(argv[1], "ptr")==0)
    rv=testPtr(argc, argv);
  else if (strcasecmp(argv[1], "sl2")==0)
    rv=testStringList2(argc, argv);
  else if (strcasecmp(argv[1], "fuzzy")==0)
    rv=testFuzzy(argc, argv);
  else if (strcasecmp(argv[1], "sort")==0)
    rv=testSort(argc, argv);
  else if (strcasecmp(argv[1], "bio")==0)
    rv=testBIO(argc, argv);
  else if (strcasecmp(argv[1], "transformpin")==0)
    rv=testTransformPin(argc, argv);
  else if (strcasecmp(argv[1], "dbkey")==0)
    rv=testDbKey(argc, argv);
  else if (strcasecmp(argv[1], "nlsocketconnect")==0)
    rv=testNlSocketConnect(argc, argv);
  else if (strcasecmp(argv[1], "nlsocketaccept")==0)
    rv=testNlSocketAccept(argc, argv);
  else if (strcasecmp(argv[1], "nlhttpconnect1")==0)
    rv=testNlHttpConnect1(argc, argv);
  else if (strcasecmp(argv[1], "nlhttpconnect2")==0)
    rv=testNlHttpConnect2(argc, argv);
  else if (strcasecmp(argv[1], "nlhttpconnect3")==0)
    rv=testNlHttpConnect3(argc, argv);
  else if (strcasecmp(argv[1], "nlhttpconnect4")==0)
    rv=testNlHttpConnect4(argc, argv);
  else if (strcasecmp(argv[1], "nlhttpaccept1")==0)
    rv=testNlHttpAccept1(argc, argv);
  else if (strcasecmp(argv[1], "nlsslconnect1")==0)
    rv=testNlSslConnect1(argc, argv);
  else if (strcasecmp(argv[1], "nlsslconnect2")==0)
    rv=testNlSslConnect2(argc, argv);
  else if (strcasecmp(argv[1], "ipcserver1")==0)
    rv=testIpcServer1(argc, argv);
  else if (strcasecmp(argv[1], "ipcclient1")==0)
    rv=testIpcClient1(argc, argv);
  else if (strcasecmp(argv[1], "nlfileconnect1")==0)
    rv=testNlFileConnect1(argc, argv);
  else if (strcasecmp(argv[1], "nllogconnect1")==0)
    rv=testNlLogConnect1(argc, argv);
  else if (strcasecmp(argv[1], "nllogconnect2")==0)
    rv=testNlLogConnect2(argc, argv);
  else if (strcasecmp(argv[1], "mem")==0)
    rv=testMem(argc, argv);
  else {
    fprintf(stderr, "Unknown command \"%s\"\n", argv[1]);
    GWEN_Fini();
    return 1;
  }

  GWEN_Fini();
  return rv;
}

