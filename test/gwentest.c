
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
#include <gwenhywfar/nettransportsock.h>
#include <gwenhywfar/nettransportssl.h>
#include <gwenhywfar/netconnection.h>
#include <gwenhywfar/netconnectionhttp.h>
#include <gwenhywfar/process.h>
#include <gwenhywfar/args.h>
#include <gwenhywfar/base64.h>
#include <gwenhywfar/misc2.h>
#include <gwenhywfar/gwentime.h>
#ifdef OS_WIN32
# include <windows.h>
# define sleep(x) Sleep(x*1000)
# define strcasecmp(a, b) strcmp(a, b)
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef USE_NCURSES
# include <ncurses.h>
# include <gwenhywfar/ui/widget.h>
# include <gwenhywfar/ui/ui.h>
# include <gwenhywfar/ui/window.h>
# include <gwenhywfar/ui/textwidget.h>
# include <gwenhywfar/ui/tablewidget.h>
# include <gwenhywfar/ui/button.h>
# include <gwenhywfar/ui/scrollwidget.h>
# include <gwenhywfar/ui/messagebox.h>
# include <gwenhywfar/ui/editbox.h>
# include <gwenhywfar/ui/checkbox.h>
# include <gwenhywfar/ui/dropdownbox.h>
# include <gwenhywfar/ui/filedialog.h>
# include <gwenhywfar/ui/loader.h>
# include "../gwenui/loader_p.h"
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
  tr=GWEN_NetTransportSSL_new(sk, "trusted", "newtrusted", 0, 0, 1, 1);
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

  if (GWEN_Base64_Decode(GWEN_Buffer_GetStart(src),
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
  tr=GWEN_NetTransportSSL_new(sk, "trusted", "newtrusted",
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
  tr=GWEN_NetTransportSSL_new(sk, "trusted", "newtrusted", "lancelot.crt",
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






/* _________________________________________________________________________
 * AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
 *                                  UI Tests
 * YYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY
 */

#ifdef USE_NCURSES
int uitest1(int argc, char **argv) {
  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }
  sleep(2);

  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  return 0;
}



int uitest2(int argc, char **argv) {
  GWEN_WIDGET *w;
  GWEN_UI_RESULT res;

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }


  w=GWEN_Widget_new(0,
                    GWEN_WIDGET_FLAGS_DEFAULT |
                    GWEN_WIDGET_FLAGS_BORDER,
                    "Test-Widget",
                    "Test",
                    4, 4,
                    20, 6);
  assert(w);
  GWEN_Widget_SetColour(w, GWEN_WidgetColour_Default);
  GWEN_Widget_Redraw(w);

  GWEN_UI_Flush();

  res=GWEN_UI_Work();
  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}



int uitest3(int argc, char **argv) {
  GWEN_WIDGET *w;
  GWEN_UI_RESULT res;

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }


  w=GWEN_Window_new(0,
                    GWEN_WIDGET_FLAGS_DEFAULT |
                    GWEN_WIDGET_FLAGS_BORDER |
                    GWEN_WINDOW_FLAGS_TITLE,
                    "Test-Widget",
                    "Ueberschrift",
                    4, 4,
                    40, 10);
  assert(w);
  GWEN_Widget_SetColour(w, GWEN_WidgetColour_Default);
  GWEN_Widget_Redraw(w);

  GWEN_UI_Flush();

  res=GWEN_UI_Work();
  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}



int uitest4(int argc, char **argv) {
  GWEN_WIDGET *w;
  GWEN_UI_RESULT res;
  unsigned char buffer[]="%ff%02This%ff%00 is a %ff%01test";

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }


  w=GWEN_Window_new(0,
                    GWEN_WIDGET_FLAGS_DEFAULT |
                    GWEN_WIDGET_FLAGS_BORDER |
                    GWEN_WINDOW_FLAGS_TITLE,
                    "Test-Widget",
                    "Ueberschrift",
                    4, 4,
                    40, 10);
  assert(w);
  GWEN_Widget_SetColour(w, GWEN_WidgetColour_Message);
  GWEN_Widget_Redraw(w);

  GWEN_Widget_WriteAt(GWEN_Window_GetViewPort(w), 4, 4, buffer, 0);
  GWEN_Widget_Update(GWEN_Window_GetViewPort(w));
  GWEN_UI_Flush();

  res=GWEN_UI_Work();
  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}



int uitest5(int argc, char **argv) {
  GWEN_WIDGET *w;
  GWEN_UI_RESULT res;
  unsigned char buffer[]=
    "<gwen>"
    "<i>This</i> is a <b>test</b><br>"
    "And <strong>this here</strong>, tooAnd this line<br>"
    "is as good as a test<br>"
    "as the lines<br>"
    "above this one<br>"
    "<br>"
    "Well, I need to fill<br>"
    "this <b>space</b> here<br>"
    "but with what ?<br>"
    "Hmm, but I think...<br>"
    "this might be enough<br>"
    "At least now ;-)<br>"
    "</gwen>";

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }


  w=GWEN_TextWidget_new(0,
                        GWEN_WIDGET_FLAGS_DEFAULT |
                        GWEN_TEXTWIDGET_FLAGS_LINEMODE,
                        "Test-Widget",
                        buffer,
                        4, 4,
                        40, 10);
  GWEN_TextWidget_SetVirtualSize(w, 40, 20);
  assert(w);
  GWEN_Widget_SetColour(w, GWEN_WidgetColour_Default);
  GWEN_Widget_Redraw(w);
  GWEN_UI_Flush();

  res=GWEN_UI_Work();
  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}



int uitest6(int argc, char **argv) {
  GWEN_WIDGET *w;
  GWEN_WIDGET *tv;
  GWEN_UI_RESULT res;
  unsigned char buffer[]=
    "<gwen>"
    "<i>This</i> is a <b>test</b><br>"
    "And <strong>this here</strong>, tooAnd this line<br>"
    "is as good as a test<br>"
    "as the lines<br>"
    "above this one<br>"
    "<br>"
    "Well, I need to fill<br>"
    "this <b>space</b> here<br>"
    "but with what ?<br>"
    "Hmm, but I think...<br>"
    "this might be enough<br>"
    "At least now ;-)<br>"
    "</gwen>";

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }

  w=GWEN_Window_new(0,
                    GWEN_WIDGET_FLAGS_DEFAULT |
                    GWEN_WIDGET_FLAGS_BORDER |
                    GWEN_WINDOW_FLAGS_TITLE,
                    "Test-Widget",
                    "Ueberschrift",
                    4, 4,
                    50, 20);
  assert(w);
  GWEN_Widget_SetColour(w, GWEN_WidgetColour_Default);
  GWEN_Widget_Redraw(w);

  tv=GWEN_TextWidget_new(GWEN_Window_GetViewPort(w),
                         GWEN_WIDGET_FLAGS_DEFAULT |
                         GWEN_TEXTWIDGET_FLAGS_LINEMODE,
                         "Test-Liste",
                         buffer,
                         2, 2,
                         40, 10);
  GWEN_TextWidget_SetVirtualSize(tv, 40, 20);
  GWEN_Widget_SetColour(tv, GWEN_WidgetColour_Message);
  GWEN_Widget_Redraw(tv);
  GWEN_Widget_SetFocus(tv);
  GWEN_UI_Flush();

  GWEN_Widget_Dump(w, 1);

  res=GWEN_UI_Work();
  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}



int uitest7(int argc, char **argv) {
  GWEN_WIDGET *w;
  GWEN_WIDGET *tv;
  GWEN_UI_RESULT res;
  GWEN_TW_LINE *tl;
  unsigned char buffer[]=
    "<gwen>"
    "<i>This</i> is a <b>test</b><br>"
    "And <strong>this here</strong>, tooAnd this line<br>"
    "is as good as a test<br>"
    "as the lines<br>"
    "<i>above</i> this one<br>"
    "<br>"
    "Well, I need to fill<br>"
    "this <b>space</b> here<br>"
    "but with what ?<br>"
    "Hmm, but I think...<br>"
    "this might be enough<br>"
    "At least now ;-)<br>"
    "</gwen>";

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }

  w=GWEN_Window_new(0,
                    GWEN_WIDGET_FLAGS_DEFAULT |
                    GWEN_WIDGET_FLAGS_BORDER |
                    GWEN_WINDOW_FLAGS_TITLE,
                    "Test-Widget",
                    "Ueberschrift",
                    4, 4,
                    50, 20);
  assert(w);
  GWEN_Widget_SetColour(w, GWEN_WidgetColour_Default);
  GWEN_Widget_Redraw(w);

  tv=GWEN_TextWidget_new(GWEN_Window_GetViewPort(w),
                         GWEN_WIDGET_FLAGS_DEFAULT |
                         GWEN_TEXTWIDGET_FLAGS_LINEMODE |
                         GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT,
                         "Test-Liste",
                         buffer,
                         2, 2,
                         40, 10);
  GWEN_TextWidget_SetVirtualSize(tv, 40, 20);
  GWEN_Widget_SetColour(tv, GWEN_WidgetColour_Message);
  GWEN_Widget_Redraw(tv);
  GWEN_Widget_SetFocus(tv);
  GWEN_UI_Flush();

  GWEN_Widget_Dump(w, 1);

  tl=GWEN_TextWidget_LineOpen(tv, 18, 1);
  if (!tl) {
    DBG_ERROR(0, "Could not open");
    GWEN_UI_End();
    return 2;
  }

  if (GWEN_TextWidget_LineSetBorders(tv, tl, 3, 8)) {
    DBG_ERROR(0, "Could not set borders");
    GWEN_UI_End();
    return 2;
  }
  GWEN_UI_Flush();

  if (GWEN_TextWidget_LineClear(tv, tl)) {
    DBG_ERROR(0, "Could not clear line");
    GWEN_UI_End();
    return 2;
  }
  GWEN_UI_Flush();

  if (GWEN_TextWidget_LineSetPos(tv, tl, 3)) {
    DBG_ERROR(0, "Could not set pos");
    GWEN_UI_End();
    return 2;
  }
  GWEN_UI_Flush();

  if (GWEN_TextWidget_LineWriteText(tv, tl, "Pisskopp", 0)) {
    DBG_ERROR(0, "Could not write text");
    GWEN_UI_End();
    return 2;
  }
  GWEN_UI_Flush();

  if (GWEN_TextWidget_LineRedraw(tv, tl)) {
    DBG_ERROR(0, "Could not redraw line");
    GWEN_UI_End();
    return 2;
  }
  GWEN_UI_Flush();

  if (GWEN_TextWidget_LineClose(tv, tl, 0)) {
    DBG_ERROR(0, "Could not close line");
    GWEN_UI_End();
    return 2;
  }
  GWEN_UI_Flush();

  res=GWEN_UI_Work();
  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}



int uitest8(int argc, char **argv) {
  GWEN_WIDGET *w;
  GWEN_WIDGET *tv;
  GWEN_UI_RESULT res;
  int i, j;

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }

  w=GWEN_Window_new(0,
                    GWEN_WIDGET_FLAGS_DEFAULT |
                    GWEN_WIDGET_FLAGS_BORDER |
                    GWEN_WINDOW_FLAGS_TITLE |
                    GWEN_WINDOW_FLAGS_HSLIDER |
                    GWEN_WINDOW_FLAGS_VSLIDER |
                    0,
                    "Test-Widget",
                    "Ueberschrift",
                    4, 4,
                    50, 20);
  assert(w);
  GWEN_Widget_SetColour(w, GWEN_WidgetColour_Default);
  GWEN_Widget_Redraw(w);

  tv=GWEN_TableWidget_new(GWEN_Window_GetViewPort(w),
                          GWEN_WIDGET_FLAGS_DEFAULT |
                          GWEN_TABLEWIDGET_FLAGS_COLBORDER,
                          "Test-Tabelle",
                          0, 0,
                          0, 0);
  GWEN_TextWidget_SetVirtualSize(tv, 1200, 200);
  GWEN_Widget_SetColour(tv, GWEN_WidgetColour_Default);
  GWEN_Widget_Redraw(tv);
  GWEN_Widget_SetFocus(tv);
  GWEN_UI_Flush();

  GWEN_Widget_Dump(w, 1);

  for (i=0; i<10; i++)
    for (j=0; j<16; j++) {
      char numbuf[32];
      snprintf(numbuf, sizeof(numbuf), "%d/%d", i, j);
      GWEN_TableWidget_SetText(tv, i, j, numbuf);
    }
  GWEN_Widget_Redraw(tv);
  GWEN_UI_Flush();

  res=GWEN_UI_Work();
  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}



int uitest9(int argc, char **argv) {
  GWEN_WIDGET *mw;
  GWEN_WIDGET *w;
  GWEN_WIDGET *tv;
  GWEN_WIDGET *but;
  GWEN_UI_RESULT res;
  int i, j;

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }

  mw=GWEN_Window_new(0,
                     GWEN_WIDGET_FLAGS_DEFAULT |
                     GWEN_WIDGET_FLAGS_BORDER |
                     GWEN_WINDOW_FLAGS_TITLE |
                     0,
                     "Main-Widget",
                     "Ueberschrift",
                     1, 1,
                     60, 22);
  w=GWEN_Window_new(GWEN_Window_GetViewPort(mw),
                    GWEN_WIDGET_FLAGS_DEFAULT |
                    GWEN_WINDOW_FLAGS_HSLIDER |
                    GWEN_WINDOW_FLAGS_VSLIDER |
                    0,
                    "Test-View",
                    "Test",
                    0, 0,
                    0, 17);
  assert(w);
  GWEN_Widget_SetColour(GWEN_Window_GetViewPort(mw),
                        GWEN_WidgetColour_Chosen);

  DBG_NOTICE(0, "Creating table");
  tv=GWEN_TableWidget_new(GWEN_Window_GetViewPort(w),
                          GWEN_WIDGET_FLAGS_DEFAULT |
                          GWEN_TABLEWIDGET_FLAGS_COLBORDER,
                          "Test-Tabelle",
                          0, 0,
                          0, 0);
  GWEN_TextWidget_SetVirtualSize(tv, 200, 200);
  GWEN_Widget_SetColour(tv, GWEN_WidgetColour_Default);

  but=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                      GWEN_WIDGET_FLAGS_DEFAULT |
                      GWEN_WIDGET_FLAGS_BORDER |
                      GWEN_WIDGET_FLAGS_HCENTER |
                      GWEN_WIDGET_FLAGS_HIGHLIGHT,
                      "Test-Button",
                      "Button",
                      0xdeadbeef, /* commandId */
                      24, 18,
                      10, 1);
  GWEN_Widget_SetColour(but, GWEN_WidgetColour_Message);
  GWEN_Widget_Redraw(mw);
  GWEN_Widget_SetFocus(tv);
  GWEN_UI_Flush();

  GWEN_Widget_Dump(w, 1);

  for (i=0; i<10; i++)
    for (j=0; j<16; j++) {
      char numbuf[32];
      snprintf(numbuf, sizeof(numbuf), "%d/%d", i, j);
      GWEN_TableWidget_SetText(tv, i, j, numbuf);
    }
  GWEN_Widget_Redraw(tv);
  GWEN_UI_Flush();

  res=GWEN_UI_Work();
  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}



int uitest10(int argc, char **argv) {
  GWEN_WIDGET *mw;
  GWEN_WIDGET *w;
  GWEN_WIDGET *tv;
  GWEN_WIDGET *but;
  GWEN_UI_RESULT res;
  int i, j;

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }

  mw=GWEN_Window_new(0,
                     GWEN_WIDGET_FLAGS_DEFAULT |
                     GWEN_WIDGET_FLAGS_BORDER |
                     GWEN_WINDOW_FLAGS_TITLE |
                     0,
                     "Main-Widget",
                     "Ueberschrift",
                     1, 1,
                     60, 22);
  w=GWEN_ScrollWidget_new(GWEN_Window_GetViewPort(mw),
                          GWEN_WIDGET_FLAGS_DEFAULT |
                          GWEN_WINDOW_FLAGS_HSLIDER |
                          GWEN_WINDOW_FLAGS_VSLIDER |
                          0,
                          "Test-View",
                          0, 0,
                          0, 17);
  assert(w);
  GWEN_Widget_SetColour(GWEN_Window_GetViewPort(mw),
                        GWEN_WidgetColour_Chosen);

  DBG_NOTICE(0, "Creating table");
  tv=GWEN_TableWidget_new(GWEN_ScrollWidget_GetViewPort(w),
                          GWEN_WIDGET_FLAGS_DEFAULT |
                          GWEN_TABLEWIDGET_FLAGS_COLBORDER,
                          "Test-Tabelle",
                          0, 0,
                          0, 0);
  GWEN_TextWidget_SetVirtualSize(tv, 200, 200);
  GWEN_Widget_SetColour(tv, GWEN_WidgetColour_Default);

  but=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                      GWEN_WIDGET_FLAGS_DEFAULT |
                      GWEN_WIDGET_FLAGS_BORDER |
                      GWEN_WIDGET_FLAGS_HCENTER |
                      GWEN_WIDGET_FLAGS_HIGHLIGHT,
                      "Test-Button",
                      "Button",
                      0xdeadbeef, /* commandId */
                      24, 18,
                      10, 1);
  GWEN_Widget_SetColour(but, GWEN_WidgetColour_Message);
  GWEN_Widget_Redraw(mw);
  GWEN_Widget_SetFocus(tv);
  GWEN_UI_Flush();

  GWEN_Widget_Dump(w, 1);

  for (i=0; i<10; i++)
    for (j=0; j<16; j++) {
      char numbuf[32];
      snprintf(numbuf, sizeof(numbuf), "%d/%d", i, j);
      GWEN_TableWidget_SetText(tv, i, j, numbuf);
    }
  GWEN_Widget_Redraw(tv);
  GWEN_UI_Flush();

  res=GWEN_UI_Work();
  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}



int uitest11(int argc, char **argv) {
  GWEN_WIDGET *mw;
  GWEN_WIDGET *w;
  GWEN_WIDGET *tv;
  GWEN_WIDGET *but;
  GWEN_UI_RESULT res;
  int i, j;

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }

  mw=GWEN_Window_new(0,
                     GWEN_WIDGET_FLAGS_DEFAULT |
                     GWEN_WIDGET_FLAGS_BORDER |
                     GWEN_WINDOW_FLAGS_TITLE |
                     0,
                     "Main-Widget",
                     "Ueberschrift",
                     1, 1,
                     60, 22);
  w=GWEN_ScrollWidget_new(GWEN_Window_GetViewPort(mw),
                          GWEN_WIDGET_FLAGS_DEFAULT |
                          GWEN_WINDOW_FLAGS_HSLIDER |
                          GWEN_WINDOW_FLAGS_VSLIDER |
                          0,
                          "Test-View",
                          0, 0,
                          0, 17);
  assert(w);
  GWEN_Widget_SetColour(GWEN_Window_GetViewPort(mw),
                        GWEN_WidgetColour_Chosen);

  DBG_NOTICE(0, "Creating table");
  tv=GWEN_TableWidget_new(GWEN_ScrollWidget_GetViewPort(w),
                          GWEN_WIDGET_FLAGS_DEFAULT |
                          GWEN_TABLEWIDGET_FLAGS_COLBORDER,
                          "Test-Tabelle",
                          0, 0,
                          0, 0);
  GWEN_TextWidget_SetVirtualSize(tv, 200, 200);
  GWEN_Widget_SetColour(tv, GWEN_WidgetColour_Default);

  but=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                      GWEN_WIDGET_FLAGS_DEFAULT |
                      GWEN_WIDGET_FLAGS_BORDER |
                      GWEN_WIDGET_FLAGS_HCENTER |
                      GWEN_WIDGET_FLAGS_HIGHLIGHT,
                      "Test-Button",
                      "Button",
                      0xdeadbeef, /* commandId */
                      24, 18,
                      10, 1);
  GWEN_Widget_SetColour(but, GWEN_WidgetColour_Message);
  GWEN_Widget_SetFocus(tv);

  GWEN_Widget_Dump(w, 1);

  for (i=0; i<10; i++)
    for (j=0; j<16; j++) {
      char numbuf[32];
      snprintf(numbuf, sizeof(numbuf), "%d/%d", i, j);
      GWEN_TableWidget_SetText(tv, i, j, numbuf);
    }
  GWEN_Widget_Redraw(mw);

  res=GWEN_Widget_Run(mw);
  GWEN_Widget_Close(mw);
  GWEN_UI_Flush();

  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}



int uitest12(int argc, char **argv) {
  GWEN_WIDGET *mw;
  GWEN_WIDGET *w;
  GWEN_WIDGET *tv;
  GWEN_WIDGET *but;
  GWEN_UI_RESULT res;
  int i, j;

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }

  mw=GWEN_Window_new(0,
                     GWEN_WIDGET_FLAGS_DEFAULT |
                     GWEN_WIDGET_FLAGS_BORDER |
                     GWEN_WINDOW_FLAGS_TITLE |
                     0,
                     "Main-Widget",
                     "Ueberschrift",
                     1, 1,
                     60, 22);
  w=GWEN_ScrollWidget_new(GWEN_Window_GetViewPort(mw),
                          GWEN_WIDGET_FLAGS_DEFAULT |
                          GWEN_WINDOW_FLAGS_HSLIDER |
                          GWEN_WINDOW_FLAGS_VSLIDER |
                          0,
                          "Test-View",
                          0, 0,
                          0, 17);
  assert(w);
  GWEN_Widget_SetColour(GWEN_Window_GetViewPort(mw),
                        GWEN_WidgetColour_Chosen);

  DBG_NOTICE(0, "Creating table");
  tv=GWEN_TableWidget_new(GWEN_ScrollWidget_GetViewPort(w),
                          GWEN_WIDGET_FLAGS_DEFAULT |
                          GWEN_TABLEWIDGET_FLAGS_COLBORDER,
                          "Test-Tabelle",
                          0, 0,
                          0, 0);
  GWEN_TextWidget_SetVirtualSize(tv, 200, 200);
  GWEN_Widget_SetColour(tv, GWEN_WidgetColour_Default);

  but=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                      GWEN_WIDGET_FLAGS_DEFAULT |
                      GWEN_WIDGET_FLAGS_BORDER |
                      GWEN_WIDGET_FLAGS_HCENTER |
                      GWEN_WIDGET_FLAGS_HIGHLIGHT,
                      "Test-Button",
                      "Button",
                      0xdeadbeef, /* commandId */
                      24, 18,
                      10, 1);
  GWEN_Widget_SetColour(but, GWEN_WidgetColour_Message);
  GWEN_Widget_SetFocus(tv);

  GWEN_Widget_Dump(w, 1);

  for (i=0; i<10; i++)
    for (j=0; j<16; j++) {
      char numbuf[32];
      snprintf(numbuf, sizeof(numbuf), "%d/%d", i, j);
      GWEN_TableWidget_SetText(tv, i, j, numbuf);
    }
  GWEN_Widget_Redraw(mw);

  res=GWEN_UIResult_NotHandled;
  for (;;) {
    GWEN_EVENT *e;

    e=GWEN_UI_GetNextEvent();
    if (!e)
      break;
    DBG_NOTICE(0, "Got this event:");
    GWEN_Event_Dump(e);
    if (GWEN_Event_GetType(e)==GWEN_EventType_Command) {
      if (GWEN_EventCommand_GetCommandId(e)==0xdeadbeef) {
        int rv;

        DBG_NOTICE(0, "Starting message box");
        rv=GWEN_MessageBox(mw,
                           "TestMessage",
                           "Test-Message",
                           "<gwen>This is a test text</gwen>",
                           "Ok", 0, 0);
        DBG_NOTICE(0, "Result of message box: %d", rv);
      }
      else
        res=GWEN_UI_DispatchEvent(e);
    }
    else
      res=GWEN_UI_DispatchEvent(e);
    GWEN_Event_free(e);
    if (res==GWEN_UIResult_Finished ||
        res==GWEN_UIResult_Quit) {
      DBG_NOTICE(0, "Result: %d", res);
      break;
    }
  }

  GWEN_Widget_Close(mw);
  DBG_NOTICE(0, "Flushing event queue");
  GWEN_UI_Flush();

  GWEN_Widget_free(mw);

  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}


#include "../gwenui/textwidget_p.h"


int uitest13(int argc, char **argv) {
  GWEN_TW_LINE_LIST *ll;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelInfo);

  ll=GWEN_TextWidget_TextToLines("debugging is such a fun...", 0, 0);
  if (ll) {
    FILE *f;
    GWEN_TW_LINE *l;

    DBG_NOTICE(0, "%d Lines created", GWEN_TWLine_List_GetCount(ll));
    f=fopen("gwen-chars.dump", "w+");
    if (!f) {
      DBG_ERROR(0, "fopen: %s", strerror(errno));
      return 2;
    }

    l=GWEN_TWLine_List_First(ll);
    while(l) {
      if (GWEN_Buffer_GetUsedBytes(l->chars)) {
        if (1!=fwrite(GWEN_Buffer_GetStart(l->chars),
                      GWEN_Buffer_GetUsedBytes(l->chars),
                      1, f)) {
          DBG_ERROR(0, "fwrite: %s", strerror(errno));
          break;
        }
      }
      else {
        fprintf(f, "--- empty line ---");
      }
      fprintf(f, "\n");
      l=GWEN_TWLine_List_Next(l);
    }
    if (fclose(f)) {
      DBG_ERROR(0, "fclose: %s", strerror(errno));
      return GWEN_UIResult_Handled;
    }
  }
  else {
    DBG_ERROR(0, "Error creating lines");
  }

  return 0;
}




int uitest14(int argc, char **argv) {
  GWEN_WIDGET *mw;
  GWEN_WIDGET *w;
  GWEN_WIDGET *tv;
  GWEN_WIDGET *but;
  GWEN_UI_RESULT res;
  int i, j;
  unsigned char buffer[]=
    "<gwen>"
    "<i>This</i> is a <b>test</b><br>"
    "And <strong>this here</strong>, tooAnd this line<br>"
    "is as good as a test<br>"
    "as the lines<br>"
    "<i>above</i> this one<br>"
    "<br>"
    "Well, I need to fill<br>"
    "this <b>space</b> here<br>"
    "but with what ?<br>"
    "Hmm, but I think...<br>"
    "this might be enough<br>"
    "At least now ;-)<br>"
    "Ok, <i>one</i> more ;-)<br>"
    "Annnnd another one..."
    "</gwen>";

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }

  mw=GWEN_Window_new(0,
                     GWEN_WIDGET_FLAGS_DEFAULT |
                     GWEN_WIDGET_FLAGS_BORDER |
                     GWEN_WINDOW_FLAGS_TITLE |
                     0,
                     "Main-Widget",
                     "Ueberschrift",
                     1, 1,
                     60, 22);
  w=GWEN_ScrollWidget_new(GWEN_Window_GetViewPort(mw),
                          GWEN_WIDGET_FLAGS_DEFAULT |
                          GWEN_WINDOW_FLAGS_HSLIDER |
                          GWEN_WINDOW_FLAGS_VSLIDER |
                          0,
                          "Test-View",
                          0, 0,
                          0, 17);
  assert(w);
  GWEN_Widget_SetColour(GWEN_Window_GetViewPort(mw),
                        GWEN_WidgetColour_Chosen);

  DBG_NOTICE(0, "Creating table");
  tv=GWEN_TableWidget_new(GWEN_ScrollWidget_GetViewPort(w),
                          GWEN_WIDGET_FLAGS_DEFAULT |
                          GWEN_TABLEWIDGET_FLAGS_COLBORDER,
                          "Test-Tabelle",
                          0, 0,
                          0, 0);
  GWEN_TextWidget_SetVirtualSize(tv, 200, 200);
  GWEN_Widget_SetColour(tv, GWEN_WidgetColour_Default);

  but=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                      GWEN_WIDGET_FLAGS_DEFAULT |
                      GWEN_WIDGET_FLAGS_BORDER |
                      GWEN_WIDGET_FLAGS_HCENTER |
                      GWEN_WIDGET_FLAGS_HIGHLIGHT,
                      "Test-Button",
                      "Button",
                      0xdeadbeef, /* commandId */
                      24, 18,
                      10, 1);
  GWEN_Widget_SetColour(but, GWEN_WidgetColour_Message);
  GWEN_Widget_SetFocus(tv);

  GWEN_Widget_Dump(w, 1);

  for (i=0; i<10; i++)
    for (j=0; j<16; j++) {
      char numbuf[32];
      snprintf(numbuf, sizeof(numbuf), "%d/%d", i, j);
      GWEN_TableWidget_SetText(tv, i, j, numbuf);
    }
  GWEN_Widget_Redraw(mw);

  res=GWEN_UIResult_NotHandled;
  for (;;) {
    GWEN_EVENT *e;

    e=GWEN_UI_GetNextEvent();
    if (!e)
      break;
    DBG_NOTICE(0, "Got this event:");
    GWEN_Event_Dump(e);
    if (GWEN_Event_GetType(e)==GWEN_EventType_Command) {
      if (GWEN_EventCommand_GetCommandId(e)==0xdeadbeef) {
        int rv;

        DBG_NOTICE(0, "Starting message box");
        rv=GWEN_MessageBox(mw,
                           "TestMessage",
                           "Test-Message",
                           buffer,
                           "Ok", "Abort", "Do whatever you like");
        DBG_NOTICE(0, "Result of message box: %d", rv);
      }
      else
        res=GWEN_UI_DispatchEvent(e);
    }
    else
      res=GWEN_UI_DispatchEvent(e);
    GWEN_Event_free(e);
    if (res==GWEN_UIResult_Finished ||
        res==GWEN_UIResult_Quit) {
      DBG_NOTICE(0, "Result: %d", res);
      break;
    }
  }

  GWEN_Widget_Close(mw);
  DBG_NOTICE(0, "Flushing event queue");
  GWEN_UI_Flush();

  GWEN_Widget_free(mw);

  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}



int uitest15(int argc, char **argv) {
  GWEN_WIDGET *mw;
  GWEN_WIDGET *ev;
  GWEN_WIDGET *but;
  GWEN_UI_RESULT res;
  unsigned char buffer[]="FieldData";

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }

  mw=GWEN_Window_new(0,
                     GWEN_WIDGET_FLAGS_DEFAULT |
                     GWEN_WIDGET_FLAGS_BORDER |
                     GWEN_WINDOW_FLAGS_TITLE |
                     0,
                     "Main-Widget",
                     "Ueberschrift",
                     1, 1,
                     60, 22);
  ev=GWEN_EditBox_new(GWEN_Window_GetViewPort(mw),
                      GWEN_WIDGET_FLAGS_DEFAULT |
                      GWEN_EDITBOX_FLAGS_EDIT |
                      GWEN_WIDGET_FLAGS_BORDER,
                      "EditBox",
                      buffer,
                      0, 0, 18, 3, 16);
  GWEN_Widget_SetColour(ev, GWEN_WidgetColour_Default);

  but=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                      GWEN_WIDGET_FLAGS_DEFAULT |
                      GWEN_WIDGET_FLAGS_BORDER |
                      GWEN_WIDGET_FLAGS_HCENTER |
                      GWEN_WIDGET_FLAGS_HIGHLIGHT,
                      "Test-Button",
                      "Button",
                      0xdeadbeef, /* commandId */
                      24, 18,
                      10, 1);
  GWEN_Widget_SetColour(but, GWEN_WidgetColour_Message);
  GWEN_Widget_SetFocus(ev);

  GWEN_Widget_Dump(mw, 1);

  GWEN_Widget_Redraw(mw);

  res=GWEN_UIResult_NotHandled;
  for (;;) {
    GWEN_EVENT *e;

    e=GWEN_UI_GetNextEvent();
    if (!e)
      break;
    DBG_NOTICE(0, "Got this event:");
    GWEN_Event_Dump(e);
    if (GWEN_Event_GetType(e)==GWEN_EventType_Command) {
      if (GWEN_EventCommand_GetCommandId(e)==0xdeadbeef) {
        int rv;

        DBG_NOTICE(0, "Starting message box");
        rv=GWEN_MessageBox(mw,
                           "TestMessage",
                           "Test-Message",
                           "<gwen>This is a test message</gwen>",
                           "Ok", "Abort", "Do whatever you like");
        DBG_NOTICE(0, "Result of message box: %d", rv);
      }
      else
        res=GWEN_UI_DispatchEvent(e);
    }
    else
      res=GWEN_UI_DispatchEvent(e);
    GWEN_Event_free(e);
    if (res==GWEN_UIResult_Finished ||
        res==GWEN_UIResult_Quit) {
      DBG_NOTICE(0, "Result: %d", res);
      break;
    }
  }

  GWEN_Widget_Close(mw);
  DBG_NOTICE(0, "Flushing event queue");
  GWEN_UI_Flush();

  GWEN_Widget_free(mw);

  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}



int uitest16(int argc, char **argv) {
  GWEN_WIDGET *mw;
  GWEN_WIDGET *but1;
  GWEN_WIDGET *but2;
  GWEN_WIDGET *but3;
  GWEN_UI_RESULT res;

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }

  mw=GWEN_Window_new(0,
                     (GWEN_WIDGET_FLAGS_DEFAULT |
                      GWEN_WIDGET_FLAGS_BORDER |
                      GWEN_WINDOW_FLAGS_TITLE) &
                     ~GWEN_WIDGET_FLAGS_FOCUSABLE,
                      "Main-Widget",
                     "Ueberschrift",
                     1, 1,
                     60, 22);

  GWEN_Widget_SetHelpText(mw,
                          "<gwen>"
                          "This is a small <b>example</b> <i>of</i> a help "
                          "screen.<br>"
                          "You can assign a help text to any widget.<br>"
                          "<br>"
                          "If no help text for a widget is available<br>"
                          "all parents are consulted."
                          "</gwen>");

  but1=GWEN_CheckBox_new(GWEN_Window_GetViewPort(mw),
                        GWEN_WIDGET_FLAGS_DEFAULT |
                        //GWEN_WIDGET_FLAGS_BORDER |
                        GWEN_WIDGET_FLAGS_HIGHLIGHT,
                        "Test-Checkbox",
                        "<gwen>This is the checkbox text</gwen>",
                        0, 0,
                        40, 4);
  GWEN_Widget_SetColour(but1, GWEN_WidgetColour_Message);

  but2=GWEN_CheckBox_new(GWEN_Window_GetViewPort(mw),
                         GWEN_WIDGET_FLAGS_DEFAULT |
                         //GWEN_WIDGET_FLAGS_BORDER |
                         GWEN_WIDGET_FLAGS_HIGHLIGHT,
                         "Test-Checkbox",
                         "Second Box",
                         0, 5,
                         40, 4);
  GWEN_Widget_SetColour(but2, GWEN_WidgetColour_Message);

  but3=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                       GWEN_WIDGET_FLAGS_DEFAULT |
                       GWEN_WIDGET_FLAGS_BORDER |
                       GWEN_WIDGET_FLAGS_HCENTER |
                       GWEN_WIDGET_FLAGS_HIGHLIGHT,
                       "Test-Button",
                       "Quit",
                       0xdeadbeef, /* commandId */
                       24, 18,
                       10, 1);
  GWEN_Widget_SetColour(but3, GWEN_WidgetColour_Message);

  GWEN_Widget_SetFocus(but1);
  GWEN_Widget_Dump(mw, 1);

  GWEN_Widget_Redraw(mw);

  res=GWEN_UIResult_NotHandled;
  GWEN_Widget_Dump(mw, 4);
  for (;;) {
    GWEN_EVENT *e;

    e=GWEN_UI_GetNextEvent();
    if (!e)
      break;
    DBG_NOTICE(0, "Got this event:");
    GWEN_Event_Dump(e);
    if (GWEN_Event_GetType(e)==GWEN_EventType_Command) {
      if (GWEN_EventCommand_GetCommandId(e)==0xdeadbeef) {
        GWEN_Widget_Close(mw);
      }
      else
        res=GWEN_UI_DispatchEvent(e);
    }
    else
      res=GWEN_UI_DispatchEvent(e);
    GWEN_Event_free(e);
  }

  DBG_NOTICE(0, "Box 1 is %schecked",
             GWEN_CheckBox_IsChecked(but1)?"":"not ");
  DBG_NOTICE(0, "Box 2 is %schecked",
             GWEN_CheckBox_IsChecked(but2)?"":"not ");

  GWEN_Widget_free(mw);

  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}



int uitest17(int argc, char **argv) {
  GWEN_WIDGET *mw;
  GWEN_WIDGET *but1;
  GWEN_WIDGET *but2;
  GWEN_WIDGET *but3;
  GWEN_WIDGET *dd;
  GWEN_UI_RESULT res;
  GWEN_STRINGLIST *sl;

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelDebug);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }

  mw=GWEN_Window_new(0,
                     (GWEN_WIDGET_FLAGS_DEFAULT |
                      GWEN_WIDGET_FLAGS_BORDER |
                      GWEN_WINDOW_FLAGS_TITLE) &
                     ~GWEN_WIDGET_FLAGS_FOCUSABLE,
                      "Main-Widget",
                     "Ueberschrift",
                     1, 1,
                     60, 22);

  GWEN_Widget_SetHelpText(mw,
                          "<gwen>"
                          "This is a small <b>example</b> <i>of</i> a help "
                          "screen.<br>"
                          "You can assign a help text to any widget.<br>"
                          "<br>"
                          "If no help text for a widget is available<br>"
                          "all parents are consulted."
                          "</gwen>");

  but1=GWEN_CheckBox_new(GWEN_Window_GetViewPort(mw),
                        GWEN_WIDGET_FLAGS_DEFAULT |
                        //GWEN_WIDGET_FLAGS_BORDER |
                        GWEN_WIDGET_FLAGS_HIGHLIGHT,
                        "Test-Checkbox",
                        "<gwen>This is the checkbox text</gwen>",
                        0, 0,
                        40, 4);
  GWEN_Widget_SetColour(but1, GWEN_WidgetColour_Message);

  but2=GWEN_CheckBox_new(GWEN_Window_GetViewPort(mw),
                         GWEN_WIDGET_FLAGS_DEFAULT |
                         //GWEN_WIDGET_FLAGS_BORDER |
                         GWEN_WIDGET_FLAGS_HIGHLIGHT,
                         "Test-Checkbox",
                         "Second Box",
                         0, 5,
                         40, 4);
  GWEN_Widget_SetColour(but2, GWEN_WidgetColour_Message);

  sl=GWEN_StringList_new();
  GWEN_StringList_AppendString(sl, "Test1", 0, 1);
  GWEN_StringList_AppendString(sl, "Test2", 0, 1);
  GWEN_StringList_AppendString(sl, "Test3", 0, 1);
  GWEN_StringList_AppendString(sl, "Test4", 0, 1);
  GWEN_StringList_AppendString(sl, "Test5", 0, 1);
  GWEN_StringList_AppendString(sl, "Test6", 0, 1);
  dd=GWEN_DropDownBox_new(GWEN_Window_GetViewPort(mw),
                          GWEN_WIDGET_FLAGS_DEFAULT |
                          GWEN_WIDGET_FLAGS_BORDER |
                          GWEN_WIDGET_FLAGS_HCENTER |
                          GWEN_DROPDOWNBOX_FLAGS_EDIT |
                          GWEN_WIDGET_FLAGS_HIGHLIGHT,
                          "Test-Button",
                          0, 10,
                          20, 3,
                          sl);
  GWEN_Widget_SetColour(dd, GWEN_WidgetColour_Message);


  but3=GWEN_Button_new(GWEN_Window_GetViewPort(mw),
                       GWEN_WIDGET_FLAGS_DEFAULT |
                       GWEN_WIDGET_FLAGS_BORDER |
                       GWEN_WIDGET_FLAGS_HCENTER |
                       GWEN_WIDGET_FLAGS_HIGHLIGHT,
                       "Test-Button",
                       "Quit",
                       0xdeadbeef, /* commandId */
                       24, 18,
                       10, 1);
  GWEN_Widget_SetColour(but3, GWEN_WidgetColour_Message);

  GWEN_Widget_SetFocus(but1);
  GWEN_Widget_Dump(mw, 1);

  GWEN_Widget_Redraw(mw);

  res=GWEN_UIResult_NotHandled;
  GWEN_Widget_Dump(mw, 4);
  for (;;) {
    GWEN_EVENT *e;

    e=GWEN_UI_GetNextEvent();
    if (!e)
      break;
    DBG_NOTICE(0, "Got this event:");
    GWEN_Event_Dump(e);
    if (GWEN_Event_GetType(e)==GWEN_EventType_Command) {
      if (GWEN_EventCommand_GetCommandId(e)==0xdeadbeef) {
        GWEN_Widget_Close(mw);
      }
      else
        res=GWEN_UI_DispatchEvent(e);
    }
    else
      res=GWEN_UI_DispatchEvent(e);
    GWEN_Event_free(e);
  }

  DBG_NOTICE(0, "Box 1 is %schecked",
             GWEN_CheckBox_IsChecked(but1)?"":"not ");
  DBG_NOTICE(0, "Box 2 is %schecked",
             GWEN_CheckBox_IsChecked(but2)?"":"not ");

  GWEN_Widget_free(mw);

  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}



int uitest18(int argc, char **argv) {
  int res;
  GWEN_STRINGLIST *sl;
  GWEN_WIDGET *mw;

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelNotice);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }

  sl=GWEN_StringList_new();
  GWEN_StringList_AppendString(sl, "C source code files;*.c", 0, 1);
  GWEN_StringList_AppendString(sl, "C++ source code files;*.cpp", 0, 1);
  GWEN_StringList_AppendString(sl, "C header files;*.h", 0, 1);
  GWEN_StringList_AppendString(sl, "all files;*", 0, 1);

  mw=GWEN_FileDialog_new(GWEN_WIDGET_FLAGS_DEFAULT |
                         GWEN_FILEDIALOG_FLAGS_MULTI,
                         "FileDialog",
                         "Open File",
                         ".",
                         "",
                         sl);
  GWEN_Widget_Redraw(mw);
  GWEN_Widget_SetFocus(mw);
  GWEN_Widget_Dump(mw, 1);

  res=GWEN_Widget_Run(mw);
  GWEN_Widget_Close(mw);
  GWEN_UI_Flush();

  GWEN_Widget_free(mw);

  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  DBG_NOTICE(0, "Result was: %d", res);
  return 0;
}



int uitest19(int argc, char **argv) {
  GWEN_XMLNODE *n;
  GWEN_XMLNODE *nn;
  GWEN_DB_NODE *db;

  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelInfo);

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

  nn=GWEN_XMLNode_GetFirstTag(n);
  if (!nn) {
    DBG_ERROR(0, "No subtag");
    return 1;
  }
  db=GWEN_DB_Group_new("dialog");
  if (GWEN_UILoader_ParseWidget(nn,
                                db,
                                0, 0,
                                80, 25)) {
    DBG_ERROR(0, "Could not parse widget");
  }
  else {
    DBG_NOTICE(0, "DB is:");
    GWEN_DB_Dump(db, stdout, 2);
  }
  GWEN_XMLNode_free(n);
  return 0;


  return 0;
}



int uitest20(int argc, char **argv) {
  GWEN_XMLNODE *n;
  GWEN_XMLNODE *nn;
  GWEN_DB_NODE *dbData;
  int res;

  GWEN_Logger_Open(0, "test", "gwentest.log",
                   GWEN_LoggerTypeFile,
                   GWEN_LoggerFacilityUser);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelNotice);

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

  nn=GWEN_XMLNode_GetFirstTag(n);
  if (!nn) {
    DBG_ERROR(0, "No subtag");
    return 1;
  }

  dbData=GWEN_DB_Group_new("dialogData");

  GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_DEFAULT,
                       "type", "private");
  GWEN_DB_SetCharValue(dbData, GWEN_DB_FLAGS_DEFAULT,
                       "addr", "127.0.0.1");
  GWEN_DB_SetIntValue(dbData, GWEN_DB_FLAGS_DEFAULT,
                      "port", 32891);

  DBG_NOTICE(0, "Initializing UI");
  if (GWEN_UI_Begin()) {
    DBG_ERROR(0, "Could not init UI");
    return 2;
  }

  res=GWEN_UILoader_ExecDialog(0, nn, dbData);
  DBG_NOTICE(0, "Response was: %d", res);

  DBG_NOTICE(0, "Deinitializing UI");
  if (GWEN_UI_End()) {
    DBG_ERROR(0, "Could not deinit UI");
    return 2;
  }

  if (res==1) {
    GWEN_DB_Dump(dbData, stderr, 2);
  }

  return 0;
}


#endif /* USE_NCURSES */


GWEN_CONSTLIST2_FUNCTION_DEFS(GWEN_KEYSPEC, GWEN_KeySpec);
GWEN_CONSTLIST2_FUNCTIONS(GWEN_KEYSPEC, GWEN_KeySpec);

int testList2(int argc, char **argv) {
  GWEN_CRYPTKEY_LIST2 *keyList;
/*   GWEN_CRYPTKEY *key; */
/*   GWEN_KEYSPEC_LIST2 *specList; */
/*   GWEN_KEYSPEC *spec; */

  keyList=GWEN_CryptKey_List2_new();

  GWEN_CryptKey_List2_free(keyList);

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



int main(int argc, char **argv) {
  int rv;

  GWEN_Init();
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelNotice);

  if (argc<2) {
    fprintf(stderr, "Usage: %s <test>\n  where <test> is one of db, dbfile, dbfile2, list, key, mkkey, cpkey, xml, xml2, sn, ssl, accept, connect\n", argv[0]);
    GWEN_Fini();
    return 1;
  }


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
  else if (strcasecmp(argv[1], "base64_2")==0)
    rv=testBase64_2(argc, argv);
  else if (strcasecmp(argv[1], "httpc")==0)
    rv=testHTTPc(argc, argv);
  else if (strcasecmp(argv[1], "httpd")==0)
    rv=testHTTPd(argc, argv);
  else if (strcasecmp(argv[1], "time")==0)
    rv=testTime(argc, argv);
#ifdef USE_NCURSES
  else if (strcasecmp(argv[1], "u1")==0)
    rv=uitest1(argc, argv);
  else if (strcasecmp(argv[1], "u2")==0)
    rv=uitest2(argc, argv);
  else if (strcasecmp(argv[1], "u3")==0)
    rv=uitest3(argc, argv);
  else if (strcasecmp(argv[1], "u4")==0)
    rv=uitest4(argc, argv);
  else if (strcasecmp(argv[1], "u5")==0)
    rv=uitest5(argc, argv);
  else if (strcasecmp(argv[1], "u6")==0)
    rv=uitest6(argc, argv);
  else if (strcasecmp(argv[1], "u7")==0)
    rv=uitest7(argc, argv);
  else if (strcasecmp(argv[1], "u8")==0)
    rv=uitest8(argc, argv);
  else if (strcasecmp(argv[1], "u9")==0)
    rv=uitest9(argc, argv);
  else if (strcasecmp(argv[1], "u10")==0)
    rv=uitest10(argc, argv);
  else if (strcasecmp(argv[1], "u11")==0)
    rv=uitest11(argc, argv);
  else if (strcasecmp(argv[1], "u12")==0)
    rv=uitest12(argc, argv);
  else if (strcasecmp(argv[1], "u13")==0)
    rv=uitest13(argc, argv);
  else if (strcasecmp(argv[1], "u14")==0)
    rv=uitest14(argc, argv);
  else if (strcasecmp(argv[1], "u15")==0)
    rv=uitest15(argc, argv);
  else if (strcasecmp(argv[1], "u16")==0)
    rv=uitest16(argc, argv);
  else if (strcasecmp(argv[1], "u17")==0)
    rv=uitest17(argc, argv);
  else if (strcasecmp(argv[1], "u18")==0)
    rv=uitest18(argc, argv);
  else if (strcasecmp(argv[1], "u19")==0)
    rv=uitest19(argc, argv);
  else if (strcasecmp(argv[1], "u20")==0)
    rv=uitest20(argc, argv);
#endif /* USE_NCURSES */
  else if (strcasecmp(argv[1], "list2")==0)
    rv=testList2(argc, argv);
  else if (strcasecmp(argv[1], "dbio")==0)
    rv=testDBIO(argc, argv);
  else if (strcasecmp(argv[1], "time1")==0)
    rv=testTimeToString(argc, argv);
  else {
    fprintf(stderr, "Unknown command \"%s\"\n", argv[1]);
    GWEN_Fini();
    return 1;
  }

  GWEN_Fini();
  return rv;
}

