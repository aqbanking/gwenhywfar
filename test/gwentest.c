
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
#include <gwenhywfar/nettransportsock.h>
#include <gwenhywfar/nettransportssl.h>
#include <gwenhywfar/netconnection.h>
#include <gwenhywfar/netconnectionhttp.h>
#include <gwenhywfar/process.h>
#include <gwenhywfar/args.h>
#include <gwenhywfar/base64.h>
#ifdef OS_WIN32
# include <windows.h>
# define sleep(x) Sleep(x*1000)
# define strcasecmp(a, b) strcmp(a, b)
#endif


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
  if (GWEN_DB_ReadFile(db, argv[2], GWEN_DB_FLAGS_DEFAULT)) {
    fprintf(stderr,"Error reading file.\n");
    return 1;
  }
  //fprintf(stderr, "DB is:\n");
  //GWEN_DB_Dump(db, stderr, 2);

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
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);
  if (GWEN_XML_ReadFile(n, argv[2], GWEN_XML_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }
  fprintf(stderr, "XML file:\n");
  GWEN_XMLNode_Dump(n, stderr, 2);
  GWEN_XMLNode_free(n);
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
  for (j=3; j<argc; j++)
    GWEN_StringList_AppendString(sl, argv[j], 0, 1);

  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);
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

  key=GWEN_CryptKey_FromDb(db);
  if (!key) {
    fprintf(stderr, "Could not load key\n");
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



void connection_Up(GWEN_NETCONNECTION *conn){
  char addrBuffer[128];

  GWEN_InetAddr_GetAddress(GWEN_NetConnection_GetPeerAddr(conn),
			   addrBuffer, sizeof(addrBuffer));

  fprintf(stderr, "---- CALLBACK: Connection up (Peer is: %s, port %d) ----\n",
	  addrBuffer,
	  GWEN_InetAddr_GetPort(GWEN_NetConnection_GetPeerAddr(conn)));

}



void connection_Down(GWEN_NETCONNECTION *conn){
  char addrBuffer[128];

  GWEN_InetAddr_GetAddress(GWEN_NetConnection_GetPeerAddr(conn),
			   addrBuffer, sizeof(addrBuffer));

  fprintf(stderr, "---- CALLBACK: Connection down (Peer was: %s, port %d) ----\n",
	  addrBuffer,
	  GWEN_InetAddr_GetPort(GWEN_NetConnection_GetPeerAddr(conn)));
}



int testSocketAccept(int argc, char **argv) {
  GWEN_NETTRANSPORT *tr;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETCONNECTION *conn, *conn2;
  GWEN_NETTRANSPORT *incoming;
  char addrBuffer[128];
  const char *tstr;
  char buffer[1024];
  GWEN_TYPE_UINT32 bsize;
  int rv;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelVerbous);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  tr=GWEN_NetTransportSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  GWEN_InetAddr_SetAddress(addr, "192.168.115.2");
  GWEN_InetAddr_SetPort(addr, 55555);
  GWEN_NetTransport_SetLocalAddr(tr, addr);
  GWEN_InetAddr_free(addr);

  /* create connection layer */
  conn=GWEN_NetConnection_new(tr, 1, 1);
  GWEN_NetConnection_SetUpFn(conn, connection_Up);
  GWEN_NetConnection_SetDownFn(conn, connection_Down);

  fprintf(stderr, "Starting to listen\n");
  if (GWEN_NetConnection_StartListen(conn)) {
    fprintf(stderr, "Could not start to listen\n");
    return 2;
  }

  fprintf(stderr, "Wating for incoming connection...\n");
  incoming=GWEN_NetConnection_GetNextIncoming_Wait(conn,
                                                   60);
  if (!incoming) {
    fprintf(stderr, "No incoming connection, aborting.\n");
    return 2;
  }

  fprintf(stderr, "Got an incoming connection.\n");
  conn2=GWEN_NetConnection_new(incoming, 1, 1);
  GWEN_NetConnection_SetUpFn(conn2, connection_Up);
  GWEN_NetConnection_SetDownFn(conn2, connection_Down);
  GWEN_NetConnection_Up(conn2);
  GWEN_InetAddr_GetAddress(GWEN_NetTransport_GetPeerAddr(incoming),
                           addrBuffer, sizeof(addrBuffer));

  DBG_INFO(0, "Peer is: %s (port %d)",
           addrBuffer,
           GWEN_InetAddr_GetPort(GWEN_NetTransport_GetPeerAddr(incoming)));

  while(1) {
    fprintf(stderr, "Waiting for peer`s speach...\n");
    bsize=sizeof(buffer);
    rv=GWEN_NetConnection_Read_Wait(conn2, buffer, &bsize, 30);
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
    if (GWEN_NetConnection_Write_Wait(conn2, tstr, &bsize, 30)) {
      fprintf(stderr, "ERROR: Could not write\n");
      return 2;
    }
    if (bsize!=strlen(tstr)) {
      fprintf(stderr, "ERROR: Could not write all (only %d bytes)\n", bsize);
      return 2;
    }
  } /* while */

  fprintf(stderr, "Shutting down incoming connection...\n");
  GWEN_NetConnection_Disconnect_Wait(conn2, 30);
  GWEN_NetConnection_free(conn2);

  fprintf(stderr, "Shutting down listening connection...\n");
  GWEN_NetConnection_Disconnect_Wait(conn, 30);
  GWEN_NetConnection_free(conn);

  fprintf(stderr, "done.\n");
  return 0;
}



int testSocketConnect(int argc, char **argv) {
  GWEN_NETTRANSPORT *tr;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETCONNECTION *conn;
  const char *tstr;
  char buffer[1024];
  GWEN_TYPE_UINT32 bsize;
  int rv;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelVerbous);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  tr=GWEN_NetTransportSocket_new(sk, 1);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  GWEN_InetAddr_SetAddress(addr, "192.168.115.2");
  GWEN_InetAddr_SetPort(addr, 55555);
  GWEN_NetTransport_SetPeerAddr(tr, addr);
  GWEN_InetAddr_free(addr);

  /* create connection layer */
  conn=GWEN_NetConnection_new(tr, 1, 1);
  GWEN_NetConnection_SetUpFn(conn, connection_Up);
  GWEN_NetConnection_SetDownFn(conn, connection_Down);

  GWEN_NetConnection_Attach(conn);
  GWEN_NetConnection_free(conn);

  if (GWEN_NetConnection_Connect_Wait(conn, 30)) {
    fprintf(stderr, "ERROR: Could not connect\n");
    GWEN_NetConnection_free(conn);
    return 2;
  }
  fprintf(stderr, "Connected.\n");

  tstr="Hello server";
  bsize=strlen(tstr);
  fprintf(stderr, "Writing something to the peer...\n");
  if (GWEN_NetConnection_Write_Wait(conn, tstr, &bsize, 30)) {
    fprintf(stderr, "ERROR: Could not write\n");
    return 2;
  }
  if (bsize!=strlen(tstr)) {
    fprintf(stderr, "ERROR: Could not write all (only %d bytes)\n", bsize);
    return 2;
  }

  fprintf(stderr, "Waiting for response...\n");
  bsize=sizeof(buffer);
  rv=GWEN_NetConnection_Read_Wait(conn, buffer, &bsize, 30);
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
  GWEN_NetConnection_Disconnect_Wait(conn, 30);
  GWEN_NetConnection_free(conn);

  fprintf(stderr, "done.\n");
  return 0;
}



int testSocketSSL(int argc, char **argv) {
  GWEN_NETTRANSPORT *tr;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETCONNECTION *conn;
  const char *tstr;
  char buffer[8192];
  GWEN_TYPE_UINT32 bsize;
  int rv;
  GWEN_DB_NODE *ciphers;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  ciphers=GWEN_NetTransportSSL_GetCipherList();
  if (ciphers) {
    fprintf(stderr, "Available ciphers:\n");
    GWEN_DB_Dump(ciphers, stderr, 2);
    GWEN_DB_Group_free(ciphers);
  }

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  //tr=GWEN_NetTransportSSL_new(sk, "trusted", 0, "dh_1024.pem", 1, 1);
  //tr=GWEN_NetTransportSSL_new(sk, "trusted.pem", 0, 0, 1, 1);
  tr=GWEN_NetTransportSSL_new(sk, "trusted", 0, 0, 1, 1);
  if (!tr) {
    fprintf(stderr, "SSL not supported.\n");
    return 2;
  }
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  GWEN_InetAddr_SetAddress(addr, "192.168.115.2");
  GWEN_InetAddr_SetPort(addr, 443);
  GWEN_NetTransport_SetPeerAddr(tr, addr);
  GWEN_InetAddr_free(addr);

  /* create connection layer */
  conn=GWEN_NetConnection_new(tr, 1, 1);
  GWEN_NetConnection_SetUpFn(conn, connection_Up);
  GWEN_NetConnection_SetDownFn(conn, connection_Down);

  GWEN_NetConnection_Attach(conn);
  GWEN_NetConnection_free(conn);

  if (GWEN_NetConnection_Connect_Wait(conn, 30)) {
    fprintf(stderr, "ERROR: Could not connect\n");
    GWEN_NetConnection_free(conn);
    return 2;
  }
  fprintf(stderr, "\nConnected.\n");

  tstr="GET / HTTP/1.0\n\n";
  bsize=strlen(tstr);
  fprintf(stderr, "Writing something to the peer...\n");
  if (GWEN_NetConnection_Write_Wait(conn, tstr, &bsize, 30)) {
    fprintf(stderr, "ERROR: Could not write\n");
    return 2;
  }
  if (bsize!=strlen(tstr)) {
    fprintf(stderr, "ERROR: Could not write all (only %d bytes)\n", bsize);
    return 2;
  }

  fprintf(stderr, "Waiting for response...\n");
  bsize=sizeof(buffer);
  rv=GWEN_NetConnection_Read_Wait(conn, buffer, &bsize, 30);
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
  GWEN_NetConnection_Disconnect_Wait(conn, 30);
  GWEN_NetConnection_free(conn);

  fprintf(stderr, "done.\n");
  return 0;
}




int testProcess(int argc, char **argv) {
	GWEN_Logger_Open(0, "test", "gwentest.log", GWEN_LoggerTypeFile, 
		GWEN_LoggerFacilityUser);
	GWEN_Logger_SetLevel(0, GWEN_LoggerLevelVerbous);

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
    GWEN_ArgsTypeChar,            /* type */
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
    GWEN_ArgsTypeInt,             /* type */
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
    GWEN_ArgsTypeInt,             /* type */
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
  if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutTypeTXT)) {
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

  if (GWEN_Base64_Encode(GWEN_Buffer_GetStart(src),
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
  if (GWEN_Base64_Decode(GWEN_Buffer_GetStart(dst),
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



int testHTTPc(int argc, char **argv) {
  GWEN_NETTRANSPORT *tr;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETCONNECTION *conn;
  GWEN_DB_NODE *req;
  GWEN_NETMSG *msg;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelNotice);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  tr=GWEN_NetTransportSSL_new(sk, "trusted",
                              0, //"lancelot.crt",
                              "dh_1024.pem",
                              1, 1);
  if (!tr) {
    fprintf(stderr, "SSL not supported.\n");
    return 2;
  }
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  GWEN_InetAddr_SetAddress(addr, "192.168.115.2");
  GWEN_InetAddr_SetPort(addr, 55555);
  //GWEN_InetAddr_SetPort(addr, 443);
  //GWEN_InetAddr_SetPort(addr, 80);
  GWEN_NetTransport_SetPeerAddr(tr, addr);
  GWEN_InetAddr_free(addr);

  /* create connection layer */
  conn=GWEN_NetConnectionHTTP_new(tr,
                                  1,     /* take */
                                  0,     /* libId */
                                  1,0);  /* protocol version */
  GWEN_NetConnection_SetUpFn(conn, connection_Up);
  GWEN_NetConnection_SetDownFn(conn, connection_Down);
  GWEN_NetConnectionHTTP_SubMode(conn,
                                 GWEN_NETCONN_MODE_WAITBEFOREREAD |
                                 GWEN_NETCONN_MODE_WAITBEFOREBODYREAD);

  if (GWEN_NetConnection_Connect_Wait(conn, 30)) {
    fprintf(stderr, "ERROR: Could not connect\n");
    GWEN_NetConnection_free(conn);
    return 2;
  }
  fprintf(stderr, "Connected.\n");

  req=GWEN_DB_Group_new("request");
  GWEN_DB_SetCharValue(req, GWEN_DB_FLAGS_DEFAULT,
                       "command/cmd", "get");
  GWEN_DB_SetCharValue(req, GWEN_DB_FLAGS_DEFAULT,
                       "command/url", "/");
  /*
  GWEN_DB_SetCharValue(req, GWEN_DB_FLAGS_DEFAULT,
                       "command/vars/var1", "value1");
  GWEN_DB_SetCharValue(req, GWEN_DB_FLAGS_DEFAULT,
                       "command/vars/var2", "val%ue2");
  */
  if (GWEN_NetConnectionHTTP_AddRequest(conn,
                                        req,
                                        0,
                                        0)) {
    fprintf(stderr, "Could not add request.\n");
    return 1;
  }

  fprintf(stderr, "Waiting for response...\n");
  msg=GWEN_NetConnection_GetInMsg_Wait(conn, 30);
  if (!msg) {
    fprintf(stderr, "ERROR: Could not read\n");
    return 2;
  }
  fprintf(stderr, "Response was:\n");
  GWEN_Buffer_Dump(GWEN_NetMsg_GetBuffer(msg), stderr, 2);

  GWEN_NetMsg_free(msg);

  fprintf(stderr, "Shutting down connection...\n");
  GWEN_NetConnection_Disconnect_Wait(conn, 30);
  GWEN_NetConnection_free(conn);

  fprintf(stderr, "done.\n");
  return 0;
}



int testHTTPd(int argc, char **argv) {
  GWEN_NETTRANSPORT *tr;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_NETCONNECTION *conn, *conn2;
  GWEN_NETTRANSPORT *incoming;
  char addrBuffer[128];
  const char *tstr;
  char buffer[1024];
  GWEN_TYPE_UINT32 bsize;
  int rv;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelVerbous);

  /* create transport layer */
  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  tr=GWEN_NetTransportSSL_new(sk, "trusted", "lancelot.crt",
                              "dh_1024.pem",
                              0, /* secure */
                              1);
  if (!tr) {
    fprintf(stderr, "SSL not supported.\n");
    return 2;
  }
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  GWEN_InetAddr_SetAddress(addr, "192.168.115.2");
  GWEN_InetAddr_SetPort(addr, 55555);
  GWEN_NetTransport_SetLocalAddr(tr, addr);
  GWEN_InetAddr_free(addr);

  /* create connection layer */
  /* create connection layer */
  conn=GWEN_NetConnectionHTTP_new(tr,
                                  1,     /* take */
                                  0,     /* libId */
                                  1,0);  /* protocol version */
  GWEN_NetConnection_SetUpFn(conn, connection_Up);
  GWEN_NetConnection_SetDownFn(conn, connection_Down);

  fprintf(stderr, "Starting to listen\n");
  if (GWEN_NetConnection_StartListen(conn)) {
    fprintf(stderr, "Could not start to listen\n");
    return 2;
  }

  fprintf(stderr, "Wating for incoming connection...\n");
  incoming=GWEN_NetConnection_GetNextIncoming_Wait(conn,
                                                   60);
  if (!incoming) {
    fprintf(stderr, "No incoming connection, aborting.\n");
    return 2;
  }

  fprintf(stderr, "Got an incoming connection.\n");
  conn2=GWEN_NetConnection_new(incoming, 1, 1);
  GWEN_NetConnection_SetUpFn(conn2, connection_Up);
  GWEN_NetConnection_SetDownFn(conn2, connection_Down);
  GWEN_NetConnection_Up(conn2);
  GWEN_InetAddr_GetAddress(GWEN_NetTransport_GetPeerAddr(incoming),
                           addrBuffer, sizeof(addrBuffer));

  DBG_INFO(0, "Peer is: %s (port %d)",
           addrBuffer,
           GWEN_InetAddr_GetPort(GWEN_NetTransport_GetPeerAddr(incoming)));

  while(1) {
    fprintf(stderr, "Waiting for peer`s speach...\n");
    bsize=sizeof(buffer);
    rv=GWEN_NetConnection_Read_Wait(conn2, buffer, &bsize, 30);
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
    if (GWEN_NetConnection_Write_Wait(conn2, tstr, &bsize, 30)) {
      fprintf(stderr, "ERROR: Could not write\n");
      return 2;
    }
    if (bsize!=strlen(tstr)) {
      fprintf(stderr, "ERROR: Could not write all (only %d bytes)\n", bsize);
      return 2;
    }
  } /* while */

  fprintf(stderr, "Shutting down incoming connection...\n");
  GWEN_NetConnection_Disconnect_Wait(conn2, 30);
  GWEN_NetConnection_free(conn2);

  fprintf(stderr, "Shutting down listening connection...\n");
  GWEN_NetConnection_Disconnect_Wait(conn, 30);
  GWEN_NetConnection_free(conn);

  fprintf(stderr, "done.\n");
  return 0;
}



int main(int argc, char **argv) {
  int rv;

  if (argc<2) {
    fprintf(stderr, "Usage: %s <test>\n  where <test> is one of db, dbfile, dbfile2, list, key, mkkey, cpkey, xml, xml2, sn, ssl, accept, connect\n", argv[0]);
    return 1;
  }

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelNotice);

  if (strcasecmp(argv[1], "dbfile")==0)
    rv=testDBfile(argc, argv);
  else if (strcasecmp(argv[1], "db")==0)
    rv=testDB(argc, argv);
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
  else if (strcasecmp(argv[1], "sn")==0)
    rv=testSnprintf(argc, argv);
  else if (strcasecmp(argv[1], "accept")==0)
    rv=testSocketAccept(argc, argv);
  else if (strcasecmp(argv[1], "connect")==0)
    rv=testSocketConnect(argc, argv);
  else if (strcasecmp(argv[1], "process")==0)
    rv=testProcess(argc, argv);
  else if (strcasecmp(argv[1], "option")==0)
    rv=testOptions(argc, argv);
  else if (strcasecmp(argv[1], "ssl")==0)
    rv=testSocketSSL(argc, argv);
  else if (strcasecmp(argv[1], "base64")==0)
    rv=testBase64(argc, argv);
  else if (strcasecmp(argv[1], "httpc")==0)
    rv=testHTTPc(argc, argv);
  else if (strcasecmp(argv[1], "httpd")==0)
    rv=testHTTPd(argc, argv);
  else {
    fprintf(stderr, "Unknown command \"%s\"\n", argv[1]);
    return 1;
  }

  return rv;
}

