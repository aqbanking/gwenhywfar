
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
#include <gwenhywfar/xml.h>
#include <gwenhywfar/msgengine.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/process.h>
#include <gwenhywfar/args.h>
#include <gwenhywfar/base64.h>
#include <gwenhywfar/misc2.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/gwendate.h>
#include <gwenhywfar/fslock.h>
#include <gwenhywfar/refptr.h>
#include <gwenhywfar/stringlist2.h>
#include <gwenhywfar/url.h>
#include <gwenhywfar/tree.h>
#include <gwenhywfar/syncio_file.h>
#include <gwenhywfar/syncio_buffered.h>
#include <gwenhywfar/syncio_http.h>
#include <gwenhywfar/syncio_tls.h>
#include <gwenhywfar/smalltresor.h>
#include <gwenhywfar/sar.h>
#ifdef OS_WIN32
# include <windows.h>
# include <winsock.h>
# define sleep(x) Sleep(x*1000)
# define strcasecmp(a, b) strcmp(a, b)
#else
# include <sys/socket.h>
# include <netinet/in.h>
#endif

#include <gwenhywfar/idmap.h>
#include <gwenhywfar/idlist.h>
#include <gwenhywfar/gwensignal.h>
#include <gwenhywfar/cgui.h>

#include <gwenhywfar/cryptdefs.h>
#include <gwenhywfar/cryptkeysym.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/padd.h>

#include <gwenhywfar/httpsession.h>
#include <gwenhywfar/dialog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gnutls/gnutls.h>
#include <gcrypt.h>

#ifdef WITH_ZIP_TEST
# include <zlib.h>
#endif


#ifdef USE_LIBXML2
# include <libxml/tree.h>
# include <libxml/parser.h>
#endif


#include <gwenhywfar/cryptkeyrsa.h>
#include <gwenhywfar/cryptmgrkeys.h>
#include <gwenhywfar/passwdstore.h>



#include <sys/types.h>
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif



static int readFile(const char *fname, GWEN_BUFFER *dbuf) {
  FILE *f;

  f=fopen(fname, "rb");
  if (f) {
    while(!feof(f)) {
      uint32_t l;
      ssize_t s;
      char *p;

      GWEN_Buffer_AllocRoom(dbuf, 1024);
      l=GWEN_Buffer_GetMaxUnsegmentedWrite(dbuf);
      p=GWEN_Buffer_GetPosPointer(dbuf);
      s=fread(p, 1, l, f);
      if (s==0)
	break;
      if (s==(ssize_t)-1) {
	DBG_ERROR(0,
		  "fread(%s): %s",
		  fname, strerror(errno));
	fclose(f);
	return GWEN_ERROR_IO;
      }

      GWEN_Buffer_IncrementPos(dbuf, s);
      GWEN_Buffer_AdjustUsedBytes(dbuf);
    }

    fclose(f);
    return 0;
  }
  else {
    DBG_ERROR(0,
	     "fopen(%s): %s",
	     fname, strerror(errno));
    return GWEN_ERROR_IO;
  }
}



static int removeCTRL(GWEN_BUFFER *dbuf) {
  char *p1;
  char *p2;

  p1=GWEN_Buffer_GetStart(dbuf);
  p2=p1;
  while(*p1) {
    if ((*p1)>31)
      *(p2++)=*p1;
    p1++;
  }
  GWEN_Buffer_Crop(dbuf, 0,
		   p2-GWEN_Buffer_GetStart(dbuf));
  return 0;
}



static int writeToFile(FILE *f, const char *p, int len) {
  while(len>0) {
    ssize_t l;
    ssize_t s;

    l=1024;
    if (l>len)
      l=len;
    s=fwrite(p, 1, l, f);
    if (s==(ssize_t)-1 || s==0) {
      DBG_ERROR(0,
	       "fwrite: %s",
	       strerror(errno));
      return GWEN_ERROR_IO;
    }
    p+=s;
    len-=s;
  }

  return 0;
}



static int writeFile(const char *fname, const char *p, int len) {
  FILE *f;

  f=fopen(fname, "wb");
  if (f) {
    int rv;

    rv=writeToFile(f, p, len);
    if (rv<0) {
      DBG_ERROR(0, "here (%d)", rv);
      fclose(f);
      return rv;
    }
    if (fclose(f)) {
      DBG_ERROR(0, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_ERROR(0, "fopen(%s): %s",
	      fname, strerror(errno));
    return GWEN_ERROR_IO;
  }

  return 0;
}



static int zip_inflate(const char *ptr, unsigned int size, GWEN_BUFFER *buf) {
#ifdef WITH_ZIP_TEST
  z_stream z;
  char outbuf[512];
  int rv;
  int mode;

  z.next_in=(unsigned char*)ptr;
  z.avail_in=size;
  z.next_out=(unsigned char*)outbuf;
  z.avail_out=sizeof(outbuf);
  z.zalloc=Z_NULL;
  z.zfree=Z_NULL;

  rv=inflateInit(&z);
  if (rv!=Z_OK) {
    DBG_ERROR(0, "Error on deflateInit (%d)", rv);
    return -1;
  }

  mode=Z_NO_FLUSH;
  for(;;) {
    rv=inflate(&z, mode);
    if (z.avail_out!=sizeof(outbuf)) {
      GWEN_Buffer_AppendBytes(buf, outbuf, sizeof(outbuf)-z.avail_out);
      z.next_out=(unsigned char*)outbuf;
      z.avail_out=sizeof(outbuf);
    }

    if (rv==Z_STREAM_END)
      break;
    if (rv!=Z_OK) {
      DBG_ERROR(0, "Error on inflate (%d) [%d, %d]",
		rv, z.avail_out, z.avail_in);
      deflateEnd(&z);
      return -1;
    }
    if (z.avail_in==0)
      mode=Z_FINISH;
    if (z.avail_out==0) {
      GWEN_Buffer_AppendBytes(buf, outbuf, sizeof(outbuf));
      z.next_out=(unsigned char*)outbuf;
      z.avail_out=sizeof(outbuf);
    }
  }

  if (z.avail_out!=sizeof(outbuf)) {
    GWEN_Buffer_AppendBytes(buf, outbuf, sizeof(outbuf)-z.avail_out);
    z.next_out=(unsigned char*)outbuf;
    z.avail_out=sizeof(outbuf);
  }

  inflateEnd(&z);
#endif
  return 0;
}



int testDB(int argc, char **argv) {
  GWEN_DB_NODE *cfg;
  int rv;
  const char *p;
  GWEN_DB_NODE *dbT;

  fprintf(stderr,"Creating DB\n");
  cfg=GWEN_DB_Group_new("Config");

  /* test var 1 */
  fprintf(stderr, "Setting char values\n");
  rv=GWEN_DB_SetCharValue(cfg,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "testgroup/charvar",
			  "charvalue1");
  if (rv) {
    fprintf(stderr, "ERROR: Could not set value1.\n");
    return 2;
  }

  dbT=GWEN_DB_GetGroup(cfg, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "testgroup");
  if (!dbT) {
    fprintf(stderr, "ERROR: Could not get group \"testgroup\".\n");
    return 2;
  }
  p=GWEN_DB_GetCharValue(dbT, "charvar", 0, NULL);
  if (p==NULL) {
    fprintf(stderr, "ERROR: Could not get charvar.\n");
    return 2;
  }
  if (strcmp(p, "charvalue1")!=0) {
    fprintf(stderr, "ERROR: Value for charvar does not equal.\n");
    return 2;
  }

  p=GWEN_DB_GetCharValue(cfg, "testgroup/charvar", 0, NULL);
  if (p==NULL) {
    fprintf(stderr, "ERROR: Could not get charvar with path.\n");
    return 2;
  }
  if (strcmp(p, "charvalue1")!=0) {
    fprintf(stderr, "ERROR: Value for charvar from path does not equal.\n");
    return 2;
  }

  /* test var 2 */
  fprintf(stderr, "Setting char values 2\n");
  rv=GWEN_DB_SetCharValue(cfg,
			  0,
			  "testgroup/charvar",
			  "charvalue2");
  if (rv) {
    fprintf(stderr, "ERROR: Could not set value2.\n");
    return 2;
  }

  /* test whether charvar1 has changed (it should not) */
  dbT=GWEN_DB_GetGroup(cfg, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "testgroup");
  if (!dbT) {
    fprintf(stderr, "ERROR: Could not get group \"testgroup\".\n");
    return 2;
  }
  p=GWEN_DB_GetCharValue(dbT, "charvar", 0, NULL);
  if (p==NULL) {
    fprintf(stderr, "ERROR: Could not get charvar.\n");
    return 2;
  }
  if (strcmp(p, "charvalue1")!=0) {
    fprintf(stderr,
	    "ERROR: Value 1 for charvar does not equal [%s]!=[%s].\n",
	    p, "charvalue1");
    return 2;
  }

  p=GWEN_DB_GetCharValue(cfg, "testgroup/charvar", 0, NULL);
  if (p==NULL) {
    fprintf(stderr, "ERROR: Could not get charvar with path.\n");
    return 2;
  }
  if (strcmp(p, "charvalue1")!=0) {
    fprintf(stderr, "ERROR: Value for charvar from path does not equal.\n");
    return 2;
  }

  /* test whether charvar value 2 exists (it should) */
  p=GWEN_DB_GetCharValue(dbT, "charvar", 1, NULL);
  if (p==NULL) {
    fprintf(stderr, "ERROR: Could not get charvar 2.\n");
    return 2;
  }
  if (strcmp(p, "charvalue2")!=0) {
    fprintf(stderr, "ERROR: Value 2 for charvar does not equal.\n");
    return 2;
  }

  p=GWEN_DB_GetCharValue(cfg, "testgroup/charvar", 1, NULL);
  if (p==NULL) {
    fprintf(stderr, "ERROR: Could not get charvar with path.\n");
    return 2;
  }
  if (strcmp(p, "charvalue2")!=0) {
    fprintf(stderr, "ERROR: Value for charvar 2 from path does not equal.\n");
    return 2;
  }


  /* test var 2 again, this time overwrite*/
  fprintf(stderr, "Setting char values 2 (overwrite)\n");
  rv=GWEN_DB_SetCharValue(cfg,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "testgroup/charvar",
			  "charvalue2");
  if (rv) {
    fprintf(stderr, "ERROR: Could not overwrite value2.\n");
    return 2;
  }

  /* test whether charvar1 has changed (it should have) */
  dbT=GWEN_DB_GetGroup(cfg, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "testgroup");
  if (!dbT) {
    fprintf(stderr, "ERROR: Could not get group \"testgroup\".\n");
    return 2;
  }
  p=GWEN_DB_GetCharValue(dbT, "charvar", 0, NULL);
  if (p==NULL) {
    fprintf(stderr, "ERROR: Could not get charvar.\n");
    return 2;
  }
  if (strcmp(p, "charvalue2")!=0) {
    fprintf(stderr,
	    "ERROR: Value 2 (overwrite) for charvar does not "
	    "equal [%s]!=[%s].\n",
	    p, "charvalue2");
    return 2;
  }

  p=GWEN_DB_GetCharValue(cfg, "testgroup/charvar", 0, NULL);
  if (p==NULL) {
    fprintf(stderr, "ERROR: Could not get charvar with path.\n");
    return 2;
  }
  if (strcmp(p, "charvalue2")!=0) {
    fprintf(stderr,
	    "ERROR: Value for overwritten charvar from "
	    "path does not equal.\n");
    return 2;
  }

  /* test whether charvar value 2 exists (it should not) */
  p=GWEN_DB_GetCharValue(dbT, "charvar", 1, NULL);
  if (p!=NULL) {
    fprintf(stderr, "ERROR: charvar 2 should have been overwritten.\n");
    return 2;
  }

  p=GWEN_DB_GetCharValue(cfg, "testgroup/charvar", 1, NULL);
  if (p!=NULL) {
    fprintf(stderr, "ERROR: Could not get charvar with path.\n");
    return 2;
  }

  /* test int values */
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
  if (rv!=11) {
    fprintf(stderr,"ERROR: int value 1 is not 11: %d\n", rv);
    return 2;
  }

  rv=GWEN_DB_GetIntValue(cfg,
                         "testgroup/intvar", 1,
                         -1);
  if (rv!=22) {
    fprintf(stderr,"ERROR: int value 1 is not 22: %d\n", rv);
    return 2;
  }

  GWEN_DB_Dump(cfg, 2);

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

  GWEN_DB_Dump(cfg, 2);

  fprintf(stderr,"Releasing DB\n");
  GWEN_DB_Group_free(cfg);
  return 0;
}



int testDBfile(int argc, char **argv) {
  GWEN_DB_NODE *db;

  fprintf(stderr,"Creating DB\n");
  db=GWEN_DB_Group_new("Config");

  fprintf(stderr,"Reading file\n");
  if (GWEN_DB_ReadFile(db, "test.db",
		       GWEN_DB_FLAGS_DEFAULT |
		       GWEN_PATH_FLAGS_CREATE_GROUP)) {
    fprintf(stderr,"Error reading file.\n");
    return 1;
  }
  fprintf(stderr, "DB is:\n");
  GWEN_DB_Dump(db, 2);
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
  if (GWEN_DB_ReadFile(db, argv[2],
		       GWEN_DB_FLAGS_DEFAULT |
		       GWEN_PATH_FLAGS_CREATE_GROUP)) {
    fprintf(stderr,"Error reading file.\n");
    return 1;
  }

  if (GWEN_DB_WriteFile(db, argv[3],
			GWEN_DB_FLAGS_DEFAULT
			&~GWEN_DB_FLAGS_ESCAPE_CHARVALUES)) {
    fprintf(stderr,"Error writing file.\n");
    return 1;
  }

  fprintf(stderr,"Releasing DB\n");
  GWEN_DB_Group_free(db);
  return 0;
}



int testDBfile3(int argc, char **argv) {
  GWEN_DB_NODE *db;

  if (argc<4) {
    fprintf(stderr, "%s dbfile3 src dest\n", argv[0]);
    return 1;
  }
  fprintf(stderr,"Creating DB\n");
  db=GWEN_DB_Group_new("Config");

  fprintf(stderr,"Reading file\n");
  if (GWEN_DB_ReadFile(db, argv[2],
		       GWEN_DB_FLAGS_DEFAULT |
		       GWEN_PATH_FLAGS_CREATE_GROUP)) {
    fprintf(stderr,"Error reading file.\n");
    return 1;
  }

  if (GWEN_DB_WriteFile(db, argv[3],
			GWEN_DB_FLAGS_DEFAULT
			&~GWEN_DB_FLAGS_ESCAPE_CHARVALUES)) {
    fprintf(stderr,"Error writing file.\n");
    return 1;
  }

  fprintf(stderr,"Releasing DB\n");
  GWEN_DB_Group_free(db);
  return 0;
}



int testDBfile4(int argc, char **argv) {
  GWEN_DB_NODE *db;

  fprintf(stderr,"Creating DB\n");
  db=GWEN_DB_Group_new("Config");

  fprintf(stderr,"Reading file\n");
  if (GWEN_DB_ReadFile(db, "test.db",
		       GWEN_DB_FLAGS_DEFAULT |
		       GWEN_PATH_FLAGS_CREATE_GROUP)) {
    fprintf(stderr,"Error reading file.\n");
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
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);
  if (GWEN_XML_ReadFile(n, argv[2],
			GWEN_XML_FLAGS_DEFAULT |
			GWEN_XML_FLAGS_HANDLE_HEADERS |
			GWEN_XML_FLAGS_TOLERANT_ENDTAGS |
			GWEN_XML_FLAGS_HANDLE_OPEN_HTMLTAGS)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }
  fprintf(stderr, "XML file:\n");
  GWEN_XMLNode_Dump(n, 2);
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

  if (argc<3) {
    fprintf(stderr, "Name of testfile needed.\n");
    return 1;
  }
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Debug);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);
  if (GWEN_XML_ReadFile(n, argv[2],
			GWEN_XML_FLAGS_TOLERANT_ENDTAGS |
			GWEN_XML_FLAGS_HANDLE_OPEN_HTMLTAGS |
			GWEN_XML_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }
  fprintf(stderr, "XML file:\n");
  GWEN_XMLNode_Dump(n, 2);
  if (GWEN_XMLNode_WriteFile(n, "xml.out",
                             GWEN_XML_FLAGS_DEFAULT)){
    fprintf(stderr, "Could not write file xml.out\n");
    return 2;
  }
  GWEN_XMLNode_free(n);
  return 0;
}



int testXML3(int argc, char **argv) {
  GWEN_XMLNODE *n;
  GWEN_XMLNODE *nXml;
  int rv;
  GWEN_XMLNODE_NAMESPACE_LIST *l;

  if (argc<3) {
    fprintf(stderr, "Name of testfile needed.\n");
    return 1;
  }
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Debug);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);
  if (GWEN_XML_ReadFile(n, argv[2],
			GWEN_XML_FLAGS_TOLERANT_ENDTAGS |
			GWEN_XML_FLAGS_HANDLE_OPEN_HTMLTAGS |
			GWEN_XML_FLAGS_HANDLE_NAMESPACES |
			GWEN_XML_FLAGS_DEFAULT)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }

  nXml=GWEN_XMLNode_GetFirstTag(n);
  assert(nXml);

  l=GWEN_XMLNode_GetNameSpaces(nXml);
  if (l) {
    GWEN_XMLNODE_NAMESPACE *ns;

    ns=GWEN_XMLNode_NameSpace_List_First(l);
    while(ns) {
      fprintf(stderr, "- [%s] = [%s]\n",
	      GWEN_XMLNode_NameSpace_GetName(ns),
	      GWEN_XMLNode_NameSpace_GetUrl(ns));
      ns=GWEN_XMLNode_NameSpace_List_Next(ns);
    }
  }

  rv=GWEN_XMLNode_Globalize(nXml);
  if (rv) {
    fprintf(stderr, "Could not globalize (%d)\n", rv);
    return 2;
  }

  if (GWEN_XMLNode_WriteFile(n, "xml.out",
			     GWEN_XML_FLAGS_HANDLE_NAMESPACES |
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



int testXML5(int argc, char **argv) {
#ifdef USE_LIBXML2
  xmlDocPtr doc;
  time_t startTime;
  time_t stopTime;

  if (argc<3) {
    fprintf(stderr, "Name of testfile needed.\n");
    return 1;
  }
  startTime=time(0);
  doc=xmlParseFile(argv[2]);
  stopTime=time(0);
  if (doc==0) {
    fprintf(stderr, "Error loading file \"%s\"", argv[2]);
    return -1;
  }

  fprintf(stderr, "Time for loading: %d secs\n",
	  (int)(difftime(stopTime, startTime)));
#else
  fprintf(stderr, "Compiled without support for LibXML\n");
#endif
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
                                       3,
                                       "HKTAN");
  if (!sn) {
    fprintf(stderr, "Segment not found.\n");
    return 2;
  }

  fprintf(stderr, "Node:\n");
  GWEN_XMLNode_Dump(sn, 2);

  GWEN_DB_SetIntValue(da,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "head/seq",
                      77);
  GWEN_DB_SetIntValue(da,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "process",
                      4);
  GWEN_DB_SetCharValue(da,
                       GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "tanMediumId",
                       "TESTMEDIUM");

  if (GWEN_MsgEngine_CreateMessageFromNode(e,
                                           sn,
                                           gbuf,
                                           da)) {
    fprintf(stderr, "Error creating message.\n");
    return 3;
  }

  GWEN_Text_DumpString(GWEN_Buffer_GetStart(gbuf),
                       GWEN_Buffer_GetUsedBytes(gbuf),
                       1);

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
  GWEN_XMLNode_Dump(sn, 2);

  GWEN_MsgEngine_free(e);
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
    GWEN_SYNCIO *sio;
    GWEN_BUFFER *tbuf;
    int err;

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

    sio=GWEN_Process_GetStdout(pr);
    assert(sio);
    sio=GWEN_SyncIo_Buffered_new(sio);
    assert(sio);
    tbuf=GWEN_Buffer_new(0, 512, 0, 1);
    do {
      fprintf(stderr, "Will read from client\n");
      err=GWEN_SyncIo_Buffered_ReadLineToBuffer(sio, tbuf);
      if (err<0) {
	fprintf(stderr, "ERROR: Reading from clients output (%d)\n", err);
	return 2;
      }
      fprintf(stderr, "DATA: %s\n", GWEN_Buffer_GetStart(tbuf));
    } while(err>=0);
    err=GWEN_SyncIo_Disconnect(sio);
    if (err<0) {
      fprintf(stderr, "ERROR: Closing clients output stream (%d)\n", err);
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
    GWEN_DB_Dump(db, 2);
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

  DBG_ERROR(0, "Size of src data: %d bytes",
	    GWEN_Buffer_GetUsedBytes(src));

  removeCTRL(src);
  DBG_ERROR(0, "Size of raw data: %d bytes",
	    GWEN_Buffer_GetUsedBytes(src));

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
  GWEN_DB_Dump(db1, 2);

  ti2=GWEN_Time_fromDb(db1);
  db2=GWEN_DB_Group_new("time");
  if (GWEN_Time_toDb(ti2, db2)) {
    fprintf(stderr, "Error saving time.\n");
    return 1;
  }
  DBG_NOTICE(0, "Time 2:");
  GWEN_DB_Dump(db2, 2);

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
			 GWEN_DB_FLAGS_UNTIL_EMPTY_LINE)) {
    GWEN_DB_Dump(db, 2);
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



int testRfc822Export(int argc, char **argv) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbParams;
  GWEN_GUI *gui;

  fprintf(stderr, "Creating gui.\n");
  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  db=GWEN_DB_Group_new("test");
  dbParams=GWEN_DB_Group_new("params");
  if (GWEN_DB_ReadFileAs(db,
                         "test.822",
                         "rfc822",
                         dbParams,
                         GWEN_PATH_FLAGS_CREATE_GROUP |
			 GWEN_DB_FLAGS_UNTIL_EMPTY_LINE)) {
    DBG_ERROR(0, "Could not read test file");
    return 2;
  }

  //GWEN_DB_Dump(db, stderr, 2);

  if (GWEN_DB_WriteFileAs(db,
                          "test.822.out",
                          "rfc822",
                          dbParams,
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
  res=GWEN_FSLock_Lock(fl, 30000, 0);
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
  res=GWEN_FSLock_Lock(fl, 3000, 0);
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
  uint32_t score = 0;

  if (argc<4) {
    fprintf(stderr, "Two extra-arguments needed.\n");
    return 1;
  }
  w1=argv[2];
  w2=argv[3];

  //score=GWEN_Text_FuzzyCompare(w1, w2);
  fprintf(stderr, "Similarity: %u\n", score);
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



int testBuffer2(int argc, char **argv) {
  GWEN_BUFFER *buf;
  unsigned int bsize;
  char *ptr;

  buf=GWEN_Buffer_new(0, 256, 0, 1);

  GWEN_Buffer_AppendString(buf, "Test");
  GWEN_Buffer_AllocRoom(buf, 512);
  bsize=GWEN_Buffer_GetMaxUnsegmentedWrite(buf);
  ptr=GWEN_Buffer_GetPosPointer(buf);
  memset(ptr, 'X', bsize);
  GWEN_Buffer_IncrementPos(buf, bsize);
  GWEN_Buffer_AdjustUsedBytes(buf);
  GWEN_Buffer_AppendString(buf, "Behind");

  GWEN_Buffer_Dump(buf, 2);
  GWEN_Buffer_free(buf);

  return 0;
}


int testFloatDouble(int argc, char **argv) {
  fprintf(stderr, "Sizeof double: %d float: %d\n",
          (int)sizeof(double), (int)sizeof(float));
  return 0;
}



int testMap(int argc, char **argv) {
  GWEN_IDMAP *map;
  const char *s1="Test-String1";
  const char *s2="Test-String2";
  const char *s3="Test-String3";
  const char *s;
  uint32_t id;
  GWEN_IDMAP_RESULT res;

  map=GWEN_IdMap_new(GWEN_IdMapAlgo_Hex4);

  res=GWEN_IdMap_Insert(map, 1, (void*)s1);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Could not set pointer1 (%d).\n", res);
    return 2;
  }

  res=GWEN_IdMap_Insert(map, 2, (void*)s2);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Could not set pointer2 (%d).\n", res);
    return 2;
  }

  res=GWEN_IdMap_Insert(map, 0x1234, (void*)s3);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Could not set pointer3 (%d).\n", res);
    return 2;
  }

  s=(const char*)GWEN_IdMap_Find(map, 1);
  if (s && strcasecmp(s, s1)==0) {
    fprintf(stderr, "  Pointer1: Ok.\n");
  }
  else {
    fprintf(stderr, "FAILED: Bad pointer1 (%d).\n", res);
    return 2;
  }

  s=(const char*)GWEN_IdMap_Find(map, 2);
  if (s && strcasecmp(s, s2)==0) {
    fprintf(stderr, "  Pointer2: Ok.\n");
  }
  else {
    fprintf(stderr, "FAILED: Bad pointer2 (%d).\n", res);
    return 2;
  }

  s=(const char*)GWEN_IdMap_Find(map, 0x1234);
  if (s && strcasecmp(s, s3)==0) {
    fprintf(stderr, "  Pointer3: Ok.\n");
  }
  else {
    fprintf(stderr, "FAILED: Bad pointer3 (%d).\n", res);
    return 2;
  }

  res=GWEN_IdMap_GetFirst(map, &id);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Not found first pointer (%d).\n", res);
    return 2;
  }
  if (id!=1) {
    fprintf(stderr, "FAILED: First pointer is not 1 (%x).\n", id);
    return 2;
  }

  res=GWEN_IdMap_GetNext(map, &id);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Not found 2nd pointer (%d).\n", res);
    return 2;
  }
  if (id!=2) {
    fprintf(stderr, "FAILED: 2nd pointer is not 2 (%x).\n", id);
    return 2;
  }

  res=GWEN_IdMap_GetNext(map, &id);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Not found 3rd pointer (%d).\n", res);
    return 2;
  }
  if (id!=0x1234) {
    fprintf(stderr, "FAILED: 3rd pointer is not 0x1234 (%x).\n", id);
    return 2;
  }

  res=GWEN_IdMap_GetNext(map, &id);
  if (res!=GWEN_IdMapResult_NotFound) {
    fprintf(stderr,
            "FAILED: Found more pointers than there should be (%d) [%x].\n",
            res, id);
    return 2;
  }


  res=GWEN_IdMap_Remove(map, 1);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Could not reset pointer1 (%d).\n", res);
    return 2;
  }

  res=GWEN_IdMap_Remove(map, 2);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Could not reset pointer1 (%d).\n", res);
    return 2;
  }

  res=GWEN_IdMap_Remove(map, 0x1234);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Could not reset pointer3 (%d).\n", res);
    return 2;
  }

  return 0;
}



#define TEST_MAP2_MANY 1000000
int testMap2(int argc, char **argv) {
  GWEN_IDMAP *map;
  const char *s1="Test-String1";
  const char *s2="Test-String2";
  int i;
  uint32_t id;
  GWEN_IDMAP_RESULT res;

  map=GWEN_IdMap_new(GWEN_IdMapAlgo_Hex4);

  fprintf(stderr, "  Storing many pointers...");
  for (i=0; i<TEST_MAP2_MANY; i++) {
    const char *s;

    s=(i & 1)?s2:s1;
    res=GWEN_IdMap_Insert(map, i, (void*)s);
    if (res!=GWEN_IdMapResult_Ok) {
      fprintf(stderr, "FAILED: Could not set pointer %x (%d).\n",
              id, res);
      return 2;
    }
  }
  fprintf(stderr, "  done\n");

  fprintf(stderr, "  Reading many pointers...");
  i=0;
  for (res=GWEN_IdMap_GetFirst(map, &id);
       res==GWEN_IdMapResult_Ok;
       res=GWEN_IdMap_GetNext(map, &id)) {
    const char *s;
    const char *sc;

    if (id!=i) {
      fprintf(stderr,
              "FAILED: Unexpected id %x.\n", id);
      return 2;
    }
    sc=(id & 1)?s2:s1;
    s=(const char*)GWEN_IdMap_Find(map, id);
    if (s!=sc) {
      fprintf(stderr,
              "FAILED: Unexpected pointer for id %x [%s].\n",
              id, s);
      return 2;
    }
    i++;
  }
  if (res!=GWEN_IdMapResult_NotFound) {
    fprintf(stderr,
            "FAILED: Unexpected result (%d).\n", res);
    return 2;
  }
  if (i!=TEST_MAP2_MANY) {
    fprintf(stderr,
            "FAILED: Bad number of ids returned (%d).\n", i);
    return 2;
  }
  fprintf(stderr, "  done\n");

  fprintf(stderr, "  Resetting many pointers...");
  for (i=0; i<TEST_MAP2_MANY; i++) {
    const char *s;

    s=(i & 1)?s2:s1;
    res=GWEN_IdMap_Remove(map, i);
    if (res!=GWEN_IdMapResult_Ok) {
      fprintf(stderr, "FAILED: Could not reset pointer (%d).\n", res);
      return 2;
    }
  }
  fprintf(stderr, "  done\n");

  GWEN_IdMap_free(map);

  return 0;
}



GWEN_IDMAP_FUNCTION_DEFS(GWEN_BUFFER, GWEN_Buffer)
GWEN_IDMAP_FUNCTIONS(GWEN_BUFFER, GWEN_Buffer)


int testMap3(int argc, char **argv) {
  GWEN_BUFFER_IDMAP *map;
  uint32_t id;
  GWEN_IDMAP_RESULT res;
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  GWEN_BUFFER *buf;

  map=GWEN_Buffer_IdMap_new(GWEN_IdMapAlgo_Hex4);
  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(buf1, "TestBuffer1");
  buf2=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(buf2, "TestBuffer2");

  res=GWEN_Buffer_IdMap_Insert(map, 10, buf1);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Could not set pointer1 (%d).\n", res);
    return 2;
  }

  res=GWEN_Buffer_IdMap_Insert(map, 2, buf2);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Could not set pointer2 (%d).\n", res);
    return 2;
  }

  buf=GWEN_Buffer_IdMap_Find(map, 10);
  if (buf==buf1) {
    fprintf(stderr, "  Pointer1: Ok.\n");
  }
  else {
    fprintf(stderr, "FAILED: Bad pointer1 (%d).\n", res);
    return 2;
  }

  buf=GWEN_Buffer_IdMap_Find(map, 2);
  if (buf==buf2) {
    fprintf(stderr, "  Pointer2: Ok.\n");
  }
  else {
    fprintf(stderr, "FAILED: Bad pointer2 (%d).\n", res);
    return 2;
  }

  res=GWEN_Buffer_IdMap_GetFirst(map, &id);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Not found first pointer (%d).\n", res);
    return 2;
  }
  if (id!=2) {
    fprintf(stderr, "FAILED: First pointer is not 2 (%x).\n", id);
    GWEN_IdMap_Dump(map, stderr, 2);
    return 2;
  }

  res=GWEN_Buffer_IdMap_GetNext(map, &id);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Not found 2nd pointer (%d).\n", res);
    return 2;
  }
  if (id!=10) {
    fprintf(stderr, "FAILED: 2nd pointer is not 10 (%x).\n", id);
    GWEN_IdMap_Dump(map, stderr, 2);
    return 2;
  }

  res=GWEN_Buffer_IdMap_GetNext(map, &id);
  if (res!=GWEN_IdMapResult_NotFound) {
    fprintf(stderr,
            "FAILED: Found more pointers than there should be (%d) [%x].\n",
            res, id);
    return 2;
  }

  fprintf(stderr, "IdMap:\n");
  GWEN_IdMap_Dump(map, stderr, 2);


  res=GWEN_Buffer_IdMap_Remove(map, 10);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Could not reset pointer1 (%d).\n", res);
    return 2;
  }

  res=GWEN_Buffer_IdMap_Remove(map, 2);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Could not reset pointer1 (%d).\n", res);
    return 2;
  }

  return 0;
}



int testMap4(int argc, char **argv) {
  GWEN_BUFFER_IDMAP *map;
  uint32_t id;
  GWEN_IDMAP_RESULT res;
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  GWEN_BUFFER *buf;

  map=GWEN_Buffer_IdMap_new(GWEN_IdMapAlgo_Hex4);
  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(buf1, "TestBuffer1");
  buf2=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(buf2, "TestBuffer2");

  res=GWEN_Buffer_IdMap_Insert(map, 10, buf1);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Could not set pointer1 (%d).\n", res);
    return 2;
  }

  buf=GWEN_Buffer_IdMap_Find(map, 10);
  if (buf==buf1) {
    fprintf(stderr, "  Pointer1: Ok.\n");
  }
  else {
    fprintf(stderr, "FAILED: Bad pointer1 (%d).\n", res);
    return 2;
  }

  res=GWEN_Buffer_IdMap_GetFirst(map, &id);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Not found first pointer (%d).\n", res);
    return 2;
  }
  if (id!=2) {
    fprintf(stderr, "FAILED: First pointer is not 2 (%x).\n", id);
    return 2;
  }

  res=GWEN_Buffer_IdMap_GetNext(map, &id);
  if (res!=GWEN_IdMapResult_NotFound) {
    fprintf(stderr,
            "FAILED: Found more pointers than there should be (%d) [%x].\n",
            res, id);
    return 2;
  }


  res=GWEN_Buffer_IdMap_Remove(map, 10);
  if (res!=GWEN_IdMapResult_Ok) {
    fprintf(stderr, "FAILED: Could not reset pointer1 (%d).\n", res);
    return 2;
  }

  return 0;
}



int testIdList(int argc, char **argv) {
  GWEN_IDLIST *idl;
  uint32_t id;
  uint32_t hdl;
  int rv;

  idl=GWEN_IdList_new();

  rv=GWEN_IdList_AddId(idl, 1);
  if (rv) {
    fprintf(stderr, "FAILED: Could not add id (%d).\n", rv);
    return 2;
  }

  rv=GWEN_IdList_AddId(idl, 2);
  if (rv) {
    fprintf(stderr, "FAILED: Could not add id (%d).\n", rv);
    return 2;
  }

  rv=GWEN_IdList_AddId(idl, 0x1234);
  if (rv) {
    fprintf(stderr, "FAILED: Could not add id (%d).\n", rv);
    return 2;
  }

  rv=GWEN_IdList_HasId(idl, 1);
  if (rv==0) {
    fprintf(stderr, "FAILED: Could not find id 1 (%d).\n", rv);
    return 2;
  }

  rv=GWEN_IdList_HasId(idl, 2);
  if (rv==0) {
    fprintf(stderr, "FAILED: Could not find id 2 (%d).\n", rv);
    return 2;
  }

  rv=GWEN_IdList_HasId(idl, 0x1234);
  if (rv==0) {
    fprintf(stderr, "FAILED: Could not find id 0x1234 (%d).\n", rv);
    return 2;
  }

  id=GWEN_IdList_GetFirstId2(idl, &hdl);
  if (id==0) {
    fprintf(stderr, "FAILED: Not found first id.\n");
    return 2;
  }
  if (id!=1) {
    fprintf(stderr, "FAILED: First id is not 1 (%x).\n", id);
    return 2;
  }

  id=GWEN_IdList_GetNextId2(idl, &hdl);
  if (id==0) {
    fprintf(stderr, "FAILED: Not found 2nd id.\n");
    return 2;
  }
  if (id!=2) {
    fprintf(stderr, "FAILED: 2nd id is not 2 (%x).\n", id);
    return 2;
  }

  id=GWEN_IdList_GetNextId2(idl, &hdl);
  if (id==0) {
    fprintf(stderr, "FAILED: Not found 3rd id.\n");
    return 2;
  }
  if (id!=0x1234) {
    fprintf(stderr, "FAILED: 3rd id is not 0x1234 (%x).\n", id);
    return 2;
  }

  id=GWEN_IdList_GetNextId2(idl, &hdl);
  if (id!=0) {
    fprintf(stderr,
            "FAILED: Found more ids than there should be [%x].\n",
            id);
    return 2;
  }

  GWEN_IdList_free(idl);

  fprintf(stderr, "PASSED.\n");
  return 0;
}



int testXmlDbExport(int argc, char **argv) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbT;
  GWEN_DB_NODE *dbParams;
  const char *x="BIN_0123456789ABCDEF";

  db=GWEN_DB_Group_new("RootGroup");
  dbParams=GWEN_DB_Group_new("params");

  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "rootElement", "RootElement");

  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
                       "TextVar0_1", "CharValue0_1");
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
                      "IntVar0_2", 12345);
  GWEN_DB_SetBinValue(db, GWEN_DB_FLAGS_DEFAULT,
                      "BinVar0_3", x, strlen(x));

  dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "SubGroup1");
  assert(dbT);

  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_DEFAULT,
                       "TextVar1_1", "CharValue1_1");
  GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_DEFAULT,
                      "IntVar1_2", 12345);
  GWEN_DB_SetBinValue(dbT, GWEN_DB_FLAGS_DEFAULT,
                      "BinVar1_3", x, strlen(x));

  dbT=GWEN_DB_GetGroup(dbT, GWEN_DB_FLAGS_DEFAULT, "SubGroup2");
  assert(dbT);

  GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_DEFAULT,
                       "TextVar2_1", "CharValue2_1");
  GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_DEFAULT,
                      "IntVar2_2", 12345);
  GWEN_DB_SetBinValue(dbT, GWEN_DB_FLAGS_DEFAULT,
                      "BinVar2_3", x, strlen(x));

  if (GWEN_DB_WriteFileAs(db,
                          "test.xmldb.out",
                          "xmldb",
                          dbParams,
			  GWEN_DB_FLAGS_DEFAULT)) {
    DBG_ERROR(0, "Could not write outfile");
  }

  return 0;
}



int testXmlDbImport(int argc, char **argv) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbParams;

  db=GWEN_DB_Group_new("TestGroup");
  dbParams=GWEN_DB_Group_new("params");
  GWEN_DB_SetCharValue(dbParams, GWEN_DB_FLAGS_DEFAULT,
                       "rootElement", "RootElement");

  if (GWEN_DB_ReadFileAs(db,
                         "test.xmldb.out",
                         "xmldb",
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



int testSignalsFunc(GWEN_SLOT *slot,
                    void *userData,
                    void *pArg1,
                    void *pArg2,
                    int iArg3,
                    int iArg4) {
  const char *text1;
  const char *text2;

  text1=(const char*)pArg1;
  text2=(const char*)pArg2;
  DBG_ERROR(0, "Slot %p: \"%s\", \"%s\", %d, %d",
            slot, text1, text2, iArg3, iArg4);
  return 0;
}



int testSignals1(int argc, char **argv) {
  GWEN_SIGNALOBJECT *so1;
  GWEN_SIGNALOBJECT *so2;
  GWEN_SIGNAL *sigTextChanged;
  GWEN_SLOT *slotTextChanged1;
  GWEN_SLOT *slotTextChanged2;

  so1=GWEN_SignalObject_new();
  so2=GWEN_SignalObject_new();
  sigTextChanged=GWEN_Signal_new(so1, 0, "sigTextChanged",
                                 "const char", "const char");
  slotTextChanged1=GWEN_Slot_new(so1, 0,
                                 "slotTextChanged",
                                 "const char", "const char",
                                 testSignalsFunc,
                                 0);

  slotTextChanged2=GWEN_Slot_new(so2, 0,
                                 "slotTextChanged",
                                 "const char", "const char",
                                 testSignalsFunc,
                                 0);

  GWEN_Signal_Connect(sigTextChanged, slotTextChanged1);
  GWEN_Signal_Connect(sigTextChanged, slotTextChanged2);
  GWEN_Signal_Emit(sigTextChanged,
                   "New Text-A", "Text-B", 1, 2);

  GWEN_SignalObject_free(so2);
  GWEN_SignalObject_free(so1);

  return 0;
}



int testSignals2(int argc, char **argv) {
  GWEN_SIGNALOBJECT *so1;
  GWEN_SIGNALOBJECT *so2;
  GWEN_SIGNAL *sigTextChanged;
  GWEN_SLOT *slotTextChanged1;
  GWEN_SLOT *slotTextChanged2;

  so1=GWEN_SignalObject_new();
  so2=GWEN_SignalObject_new();
  sigTextChanged=GWEN_Signal_new(so1, 0, "sigTextChanged",
                                 "const char", "const char");
  slotTextChanged1=GWEN_Slot_new(so1, 0,
                                 "slotTextChanged",
                                 "const char", "const char",
                                 testSignalsFunc,
                                 0);

  slotTextChanged2=GWEN_Slot_new(so2, 0,
                                 "slotTextChanged",
                                 "const char", "const char",
                                 testSignalsFunc,
                                 0);

  GWEN_Signal_Connect(sigTextChanged, slotTextChanged1);
  GWEN_Signal_Connect(sigTextChanged, slotTextChanged2);
  GWEN_Signal_Emit(sigTextChanged,
                   "New Text-A", "Text-B", 1, 2);

  GWEN_Signal_Disconnect(sigTextChanged, slotTextChanged2);
  GWEN_Signal_Emit(sigTextChanged,
                   "New Text2-A", "Text2-B", 1, 2);

  GWEN_SignalObject_free(so2);
  GWEN_SignalObject_free(so1);

  return 0;
}



int testSignals3(int argc, char **argv) {
  GWEN_SIGNALOBJECT *so1;
  GWEN_SIGNALOBJECT *so2;
  GWEN_SIGNAL *sigTextChanged;
  GWEN_SLOT *slotTextChanged1;
  GWEN_SLOT *slotTextChanged2;

  so1=GWEN_SignalObject_new();
  so2=GWEN_SignalObject_new();
  sigTextChanged=GWEN_Signal_new(so1, 0, "sigTextChanged",
                                 "const char", "const char");
  slotTextChanged1=GWEN_Slot_new(so1, 0,
                                 "slotTextChanged",
                                 "const char", "const char",
                                 testSignalsFunc,
                                 0);

  slotTextChanged2=GWEN_Slot_new(so2, 0,
                                 "slotTextChanged",
                                 "const char", 0,
                                 testSignalsFunc,
                                 0);

  if (GWEN_Signal_Connect(sigTextChanged, slotTextChanged1)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not connect signal and slot");
  }
  if (GWEN_Signal_Connect(sigTextChanged, slotTextChanged2)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not connect signal and slot");
  }
  GWEN_Signal_Emit(sigTextChanged,
                   "New Text-A", "Text-B", 1, 2);

  GWEN_SignalObject_free(so2);
  GWEN_SignalObject_free(so1);

  return 0;
}



int testUrl(int argc, char **argv) {
  const char *urlString;
  GWEN_URL *url;
  GWEN_DB_NODE *db;

  if (argc<3) {
    fprintf(stderr, "%s %s URL\n", argv[0], argv[1]);
    return 1;
  }
  urlString=argv[2];

  url=GWEN_Url_fromString(urlString);
  assert(url);

  db=GWEN_DB_Group_new("url");
  GWEN_Url_toDb(url, db);
  GWEN_DB_Dump(db, 2);

  return 0;
}



int testNewXML(int argc, char **argv) {
  GWEN_XMLNODE *n;
  time_t startTime;
  time_t stopTime;
  GWEN_XML_CONTEXT *ctx;
  GWEN_SYNCIO *sio;
  int rv;

  if (argc<3) {
    fprintf(stderr, "Name of testfile needed.\n");
    return 1;
  }

  sio=GWEN_SyncIo_File_new(argv[2], GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  //GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Debug);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Error);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);
  //GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Error);

  ctx=GWEN_XmlCtxStore_new(n,
			   GWEN_XML_FLAGS_DEFAULT |
			   GWEN_XML_FLAGS_TOLERANT_ENDTAGS |
			   GWEN_XML_FLAGS_HANDLE_HEADERS);

  startTime=time(0);

  if (GWEN_XMLContext_ReadFromIo(ctx, sio)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }

  stopTime=time(0);

  if (GWEN_XMLNode_WriteFile(n, "xml.out",
			     GWEN_XML_FLAGS_DEFAULT|GWEN_XML_FLAGS_SIMPLE|GWEN_XML_FLAGS_HANDLE_HEADERS)){
    fprintf(stderr, "Could not write file xml.out\n");
    return 2;
  }


  GWEN_XmlCtx_free(ctx);
  GWEN_XMLNode_free(n);

  fprintf(stderr, "Time for loading: %d secs\n",
	  (int)(difftime(stopTime, startTime)));

  return 0;
}



int testCrypt3Rsa(int argc, char **argv) {
  int rv;
  GWEN_CRYPT_KEY *pubKey;
  GWEN_CRYPT_KEY *secretKey;
  uint8_t testData[]=
    "This is the test data           "
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
    "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy";
  uint8_t buffer[256];
  uint32_t len;
  uint8_t buffer2[256];
  uint32_t len2;

  fprintf(stderr, "Generating key pair...\n");
  rv=GWEN_Crypt_KeyRsa_GeneratePair(768, 1, &pubKey, &secretKey);
  if (rv) {
    fprintf(stderr, "ERROR: Could not generate key pair (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "Generating key pair... done.\n");

  fprintf(stderr, "Signing message...\n");
  len=sizeof(buffer);
  rv=GWEN_Crypt_Key_Sign(secretKey, testData, sizeof(testData)-1, buffer, &len);
  if (rv) {
    fprintf(stderr, "ERROR: Could not sign (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "Signing message... done\n");

  rv=GWEN_Crypt_Key_Verify(pubKey, testData, sizeof(testData)-1, buffer, len);
  if (rv) {
    fprintf(stderr, "ERROR: Could not verify (%d).\n", rv);
    GWEN_Text_DumpString((const char*)buffer, len, 1);
    return 2;
  }

  fprintf(stderr, "Signature is valid.\n");


  fprintf(stderr, "Encrypting message...\n");
  len=sizeof(buffer);
  rv=GWEN_Crypt_Key_Encipher(pubKey, testData, sizeof(testData)-1, buffer, &len);
  if (rv) {
    fprintf(stderr, "ERROR: Could not encipher (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "Encrypting message... done (%d)\n", len);

  fprintf(stderr, "Decrypting message...\n");
  len2=sizeof(buffer2);
  rv=GWEN_Crypt_Key_Decipher(secretKey, buffer, len, buffer2, &len2);
  if (rv) {
    fprintf(stderr, "ERROR: Could not decipher (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "Encrypting message... done\n");

  if ((sizeof(testData)-1)!=len2) {
    fprintf(stderr, "Length of deciphered message does not match that of src (%d!=%d)\n",
	    len, len2);
    GWEN_Text_DumpString((const char*)buffer2, len2, 1);
    return 2;
  }
  if (memcmp(testData, buffer2, len2)) {
    fprintf(stderr, "Deciphered message does not match src\n");
    GWEN_Text_DumpString((const char*)buffer2, len2, 1);
    return 2;
  }

  return 0;
}



int testCrypt3Rsa2(int argc, char **argv) {
  int rv;
  GWEN_CRYPT_KEY *pubKey;
  GWEN_CRYPT_KEY *secretKey;
  GWEN_DB_NODE *dbPubKey;
  GWEN_DB_NODE *dbSecretKey;
  uint8_t testData[]=
    "This is the test data           "
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
    "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy";
  uint8_t buffer[512];
  uint32_t len;
  uint8_t buffer2[512];
  uint32_t len2;

  fprintf(stderr, "Generating key pair...\n");
  rv=GWEN_Crypt_KeyRsa_GeneratePair(128, 1, &pubKey, &secretKey);
  if (rv) {
    fprintf(stderr, "ERROR: Could not generate key pair (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "Generating key pair... done.\n");

  dbPubKey=GWEN_DB_Group_new("PublicKey");
  rv=GWEN_Crypt_KeyRsa_toDb(pubKey, dbPubKey, 1);
  if (rv) {
    fprintf(stderr, "ERROR: Could not store public key (%d).\n", rv);
    return 2;
  }
  GWEN_Crypt_Key_free(pubKey);

  dbSecretKey=GWEN_DB_Group_new("SecretKey");
  rv=GWEN_Crypt_KeyRsa_toDb(secretKey, dbSecretKey, 0);
  if (rv) {
    fprintf(stderr, "ERROR: Could not store secret key (%d).\n", rv);
    return 2;
  }
  GWEN_Crypt_Key_free(secretKey);

  pubKey=GWEN_Crypt_KeyRsa_fromDb(dbPubKey);
  if (pubKey==NULL) {
    fprintf(stderr, "ERROR: Could not read public key from db\n");
    GWEN_DB_Dump(dbPubKey, 2);
    return 2;
  }

  secretKey=GWEN_Crypt_KeyRsa_fromDb(dbSecretKey);
  if (secretKey==NULL) {
    fprintf(stderr, "ERROR: Could not read secret key from db\n");
    GWEN_DB_Dump(dbSecretKey, 2);
    return 2;
  }

  fprintf(stderr, "Signing message...\n");
  len=sizeof(buffer);
  rv=GWEN_Crypt_Key_Sign(secretKey, testData, sizeof(testData)-1, buffer, &len);
  if (rv) {
    fprintf(stderr, "ERROR: Could not sign (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "Signing message... done\n");

  rv=GWEN_Crypt_Key_Verify(pubKey, testData, sizeof(testData)-1, buffer, len);
  if (rv) {
    fprintf(stderr, "ERROR: Could not verify (%d).\n", rv);
    GWEN_Text_DumpString((const char*)buffer, len, 1);
    return 2;
  }

  fprintf(stderr, "Signature is valid.\n");


  fprintf(stderr, "Encrypting message...\n");
  len=sizeof(buffer);
  rv=GWEN_Crypt_Key_Encipher(pubKey, testData, sizeof(testData)-1, buffer, &len);
  if (rv) {
    fprintf(stderr, "ERROR: Could not encipher (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "Encrypting message... done (%d)\n", len);

  fprintf(stderr, "Decrypting message...\n");
  len2=sizeof(buffer2);
  rv=GWEN_Crypt_Key_Decipher(secretKey, buffer, len, buffer2, &len2);
  if (rv) {
    fprintf(stderr, "ERROR: Could not decipher (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "Decrypting message... done\n");

  if ((sizeof(testData)-1)!=len2) {
    fprintf(stderr, "Length of deciphered message does not match that of src (%d!=%d)\n",
	    len, len2);
    GWEN_Text_DumpString((const char*)buffer2, len2, 1);
    return 2;
  }
  if (memcmp(testData, buffer2, len2)) {
    fprintf(stderr, "Deciphered message does not match src\n");
    GWEN_Text_DumpString((const char*)buffer2, len2, 1);
    return 2;
  }

  return 0;
}



int testCrypt3Rsa3(int argc, char **argv) {
  int rv;
  GWEN_CRYPT_KEY *pubKey;
  GWEN_CRYPT_KEY *secretKey;
  GWEN_DB_NODE *dbPubKey;
  GWEN_DB_NODE *dbSecretKey;
  uint8_t testData[]=
    "This is the test data           "
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
    "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy";
  uint8_t buffer[512];
  uint32_t len;
  uint8_t buffer2[512];
  uint32_t len2;

  fprintf(stderr, "Generating key pair (using method 2)...\n");
  rv=GWEN_Crypt_KeyRsa_GeneratePair(128, 1, &pubKey, &secretKey);
  if (rv) {
    fprintf(stderr, "ERROR: Could not generate key pair (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "Generating key pair... done.\n");

  dbPubKey=GWEN_DB_Group_new("PublicKey");
  rv=GWEN_Crypt_KeyRsa_toDb(pubKey, dbPubKey, 1);
  if (rv) {
    fprintf(stderr, "ERROR: Could not store public key (%d).\n", rv);
    return 2;
  }
  GWEN_Crypt_Key_free(pubKey);

  dbSecretKey=GWEN_DB_Group_new("SecretKey");
  rv=GWEN_Crypt_KeyRsa_toDb(secretKey, dbSecretKey, 0);
  if (rv) {
    fprintf(stderr, "ERROR: Could not store secret key (%d).\n", rv);
    return 2;
  }
  GWEN_Crypt_Key_free(secretKey);

  pubKey=GWEN_Crypt_KeyRsa_fromDb(dbPubKey);
  if (pubKey==NULL) {
    fprintf(stderr, "ERROR: Could not read public key from db\n");
    GWEN_DB_Dump(dbPubKey, 2);
    return 2;
  }

  secretKey=GWEN_Crypt_KeyRsa_fromDb(dbSecretKey);
  if (secretKey==NULL) {
    fprintf(stderr, "ERROR: Could not read secret key from db\n");
    GWEN_DB_Dump(dbSecretKey, 2);
    return 2;
  }

  fprintf(stderr, "Signing message...\n");
  len=sizeof(buffer);
  rv=GWEN_Crypt_Key_Sign(secretKey, testData, sizeof(testData)-1, buffer, &len);
  if (rv) {
    fprintf(stderr, "ERROR: Could not sign (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "Signing message... done\n");

  rv=GWEN_Crypt_Key_Verify(pubKey, testData, sizeof(testData)-1, buffer, len);
  if (rv) {
    fprintf(stderr, "ERROR: Could not verify (%d).\n", rv);
    GWEN_Text_DumpString((const char*)buffer, len, 1);
    return 2;
  }

  fprintf(stderr, "Signature is valid.\n");


  fprintf(stderr, "Encrypting message...\n");
  len=sizeof(buffer);
  rv=GWEN_Crypt_Key_Encipher(pubKey, testData, sizeof(testData)-1, buffer, &len);
  if (rv) {
    fprintf(stderr, "ERROR: Could not encipher (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "Encrypting message... done (%d)\n", len);

  fprintf(stderr, "Decrypting message...\n");
  len2=sizeof(buffer2);
  rv=GWEN_Crypt_Key_Decipher(secretKey, buffer, len, buffer2, &len2);
  if (rv) {
    fprintf(stderr, "ERROR: Could not decipher (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "Decrypting message... done\n");

  if ((sizeof(testData)-1)!=len2) {
    fprintf(stderr, "Length of deciphered message does not match that of src (%d!=%d)\n",
	    len, len2);
    GWEN_Text_DumpString((const char*)buffer2, len2, 1);
    return 2;
  }
  if (memcmp(testData, buffer2, len2)) {
    fprintf(stderr, "Deciphered message does not match src\n");
    GWEN_Text_DumpString((const char*)buffer2, len2, 1);
    return 2;
  }

  return 0;
}



int testCrypt3Rsa4(int argc, char **argv) {
  int rv;
  GWEN_CRYPT_KEY *pubKey;
  GWEN_CRYPT_KEY *secretKey;
  GWEN_DB_NODE *dbPubKey;
  uint8_t testData[]=
    "This is the test data           "
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
    "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy";
  uint8_t buffer[512];
  uint32_t len;
  uint8_t buffer2[512];
  uint32_t len2;
  uint8_t modBuffer[512];
  uint32_t modLen;
  uint8_t pubExpBuffer[512];
  uint32_t pubExpLen;
  uint8_t privExpBuffer[512];
  uint32_t privExpLen;
  int nbytes;

  nbytes=128;

  fprintf(stderr, "testCrypt3Rsa4\n");
  fprintf(stderr, "  Generating key pair ...\n");
  rv=GWEN_Crypt_KeyRsa_GeneratePair(nbytes, 1, &pubKey, &secretKey);
  if (rv) {
    fprintf(stderr, "ERROR: Could not generate key pair (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "  Generating key pair... done.\n");

  /* get data for secret key */
  modLen=sizeof(modBuffer);
  rv=GWEN_Crypt_KeyRsa_GetModulus(secretKey, modBuffer, &modLen);
  if (rv) {
    fprintf(stderr, "ERROR: Could not store modulus (%d).\n", rv);
    return 2;
  }

  pubExpLen=sizeof(pubExpBuffer);
  rv=GWEN_Crypt_KeyRsa_GetExponent(secretKey, pubExpBuffer, &pubExpLen);
  if (rv) {
    fprintf(stderr, "ERROR: Could not store public exponent (%d).\n", rv);
    return 2;
  }

  privExpLen=sizeof(privExpBuffer);
  rv=GWEN_Crypt_KeyRsa_GetSecretExponent(secretKey, privExpBuffer, &privExpLen);
  if (rv) {
    fprintf(stderr, "ERROR: Could not store private exponent (%d).\n", rv);
    return 2;
  }


  dbPubKey=GWEN_DB_Group_new("PublicKey");
  rv=GWEN_Crypt_KeyRsa_toDb(pubKey, dbPubKey, 1);
  if (rv) {
    fprintf(stderr, "ERROR: Could not store public key (%d).\n", rv);
    return 2;
  }
  GWEN_Crypt_Key_free(pubKey);

  pubKey=GWEN_Crypt_KeyRsa_fromDb(dbPubKey);
  if (pubKey==NULL) {
    fprintf(stderr, "ERROR: Could not read public key from db\n");
    GWEN_DB_Dump(dbPubKey, 2);
    return 2;
  }

  secretKey=GWEN_Crypt_KeyRsa_fromModPrivExp(nbytes,
                                             modBuffer, modLen,
                                             pubExpBuffer, pubExpLen,
                                             privExpBuffer, privExpLen);
  if (secretKey==NULL) {
    fprintf(stderr, "ERROR: Could not read secret key from buffers\n");
    return 2;
  }

  fprintf(stderr, "  Signing message...\n");
  len=sizeof(buffer);
  rv=GWEN_Crypt_Key_Sign(secretKey, testData, sizeof(testData)-1, buffer, &len);
  if (rv) {
    fprintf(stderr, "ERROR: Could not sign (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "  Signing message... done\n");

  rv=GWEN_Crypt_Key_Verify(pubKey, testData, sizeof(testData)-1, buffer, len);
  if (rv) {
    fprintf(stderr, "ERROR: Could not verify (%d).\n", rv);
    GWEN_Text_DumpString((const char*)buffer, len, 1);
    return 2;
  }

  fprintf(stderr, "  Signature is valid.\n");


  fprintf(stderr, "  Encrypting message...\n");
  len=sizeof(buffer);
  rv=GWEN_Crypt_Key_Encipher(pubKey, testData, sizeof(testData)-1, buffer, &len);
  if (rv) {
    fprintf(stderr, "ERROR: Could not encipher (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "  Encrypting message... done (%d)\n", len);

  fprintf(stderr, "  Decrypting message...\n");
  len2=sizeof(buffer2);
  rv=GWEN_Crypt_Key_Decipher(secretKey, buffer, len, buffer2, &len2);
  if (rv) {
    fprintf(stderr, "ERROR: Could not decipher (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "  Decrypting message... done\n");

  if ((sizeof(testData)-1)!=len2) {
    fprintf(stderr, "Length of deciphered message does not match that of src (%d!=%d)\n",
	    len, len2);
    GWEN_Text_DumpString((const char*)buffer2, len2, 1);
    return 2;
  }
  if (memcmp(testData, buffer2, len2)) {
    fprintf(stderr, "Deciphered message does not match src\n");
    GWEN_Text_DumpString((const char*)buffer2, len2, 1);
    return 2;
  }

  return 0;
}



int testGnutls(int argc, char **argv) {
  gnutls_session_t session;
  gnutls_certificate_credentials_t credentials;
  int rv;
  /* Use long int here because on 64bit platforms the cast at
     gnutls_transport_set_ptr() would give an error otherwise. See
     http://www.mail-archive.com/help-gnutls@gnu.org/msg00286.html */
  long int sd;
  struct sockaddr *address;
  struct sockaddr_in *aptr;
  int size;

  address=(struct sockaddr*)malloc(sizeof(struct sockaddr_in));
  assert(address);
  aptr=(struct sockaddr_in*)(address);
  size=sizeof(struct sockaddr_in);
  memset(address,0,size);
#ifdef PF_INET
  aptr->sin_family=PF_INET;
#else
  aptr->sin_family=AF_INET;
#endif
  DBG_ERROR(0, "ping");
  aptr->sin_addr.s_addr=inet_addr("85.10.225.70");
  aptr->sin_port=htons(443);

  DBG_ERROR(0, "ping");
  sd=socket(PF_INET, SOCK_STREAM,0);
  if (sd==-1) {
    DBG_ERROR(0, "socket: %s", strerror(errno));
    return 2;
  }
  DBG_ERROR(0, "ping");
  rv=connect(sd, address, size);
  DBG_ERROR(0, "ping");
  if (rv) {
    DBG_ERROR(0, "connect: %s", strerror(errno));
    return 2;
  }

  DBG_ERROR(0, "ping");
  rv=gnutls_init(&session, GNUTLS_CLIENT);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_init: %d (%s)", rv, gnutls_strerror(rv));
    return GWEN_ERROR_GENERIC;
  }

  rv=gnutls_set_default_priority(session);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_set_default_priority: %d (%s)", rv, gnutls_strerror(rv));
    return GWEN_ERROR_GENERIC;
  }

  rv=gnutls_certificate_allocate_credentials(&credentials);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_certificate_allocate_credentials: %d (%s)", rv, gnutls_strerror(rv));
    return GWEN_ERROR_GENERIC;
  }

  rv=gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, credentials);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_credentials_set: %d (%s)", rv, gnutls_strerror(rv));
    gnutls_certificate_free_credentials(credentials);
    gnutls_deinit(session);
    return GWEN_ERROR_GENERIC;
  }

  gnutls_transport_set_ptr(session, (gnutls_transport_ptr_t)sd);

  DBG_ERROR(0, "ping");
  rv=gnutls_handshake(session);
  if (rv<0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "gnutls_handshake: %d (%s)", rv, gnutls_strerror(rv));
    return GWEN_ERROR_GENERIC;
  }

  DBG_ERROR(0, "Ok.");
  return 0;
}



static void
print_mpi (const char *text, gcry_mpi_t a)
{
  char *buf;
  void *bufaddr = &buf;
  gcry_error_t rc;

  rc = gcry_mpi_aprint (GCRYMPI_FMT_HEX, bufaddr, NULL, a);
  if (rc)
    fprintf (stderr, "%s=[error printing number: %s]\n",
             text, gpg_strerror (rc));
  else
    {
      fprintf (stderr, "%s=0x%s\n", text, buf);
      gcry_free (buf);
    }
}



static void
check_generated_rsa_key (gcry_sexp_t key, unsigned long expected_e){
  gcry_sexp_t skey, pkey, list;

  pkey = gcry_sexp_find_token (key, "public-key", 0);
  if (!pkey) {
    DBG_ERROR(0, "public part missing in return value");
    exit(2);
  }
  else {
    gcry_mpi_t e = NULL;

    list = gcry_sexp_find_token (pkey, "e", 0);
    if (!list || !(e=gcry_sexp_nth_mpi (list, 1, 0)) ) {
      DBG_ERROR(0, "public exponent not found");
      exit(2);
    }
    else if (!expected_e) {
      print_mpi ("e", e);
    }
    else {
      if ( gcry_mpi_cmp_ui (e, expected_e)){
	print_mpi ("e", e);
	fprintf(stderr, "public exponent is not %lu\n", expected_e);
	exit(2);
      }
      else {
	fprintf(stderr, "public exponent is as expected\n");
	print_mpi ("e", e);
      }
    }
    gcry_sexp_release (list);
    gcry_mpi_release (e);
    gcry_sexp_release (pkey);
  }

  skey = gcry_sexp_find_token (key, "private-key", 0);
  if (!skey) {
    DBG_ERROR(0, "private part missing in return value");
    exit(2);
  }
  else {
    int rc = gcry_pk_testkey (skey);
    if (rc) {
      DBG_ERROR(0, "gcry_pk_testkey failed: %s", gpg_strerror (rc));
      exit(2);
    }
    gcry_sexp_release (skey);
  }
}



static void
check_rsa_keys (void){
  gcry_sexp_t keyparm, key;
  int rc;

  fprintf (stderr, "creating 1024 bit RSA key using old interface\n");
  rc = gcry_sexp_new (&keyparm,
		      "(genkey\n"
		      " (rsa\n"
		      "  (nbits 4:1024)\n"
		      " ))", 0, 1);
  if (rc) {
    DBG_ERROR(0, "error creating S-expression: %s", gpg_strerror (rc));
    exit(2);
  }
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc) {
    DBG_ERROR(0, "error generating RSA key: %s", gpg_strerror (rc));
    exit(2);
  }

  check_generated_rsa_key (key, 65537);
  gcry_sexp_release (key);

  fprintf (stderr, "creating 512 bit RSA key with e=257\n");
  rc = gcry_sexp_new (&keyparm,
		      "(genkey\n"
		      " (rsa\n"
		      "  (nbits 3:512)\n"
		      "  (rsa-use-e 3:257)\n"
		      " ))", 0, 1);
  if (rc) {
    DBG_ERROR(0, "error creating S-expression: %s", gpg_strerror (rc));
    exit(2);
  }
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc) {
    DBG_ERROR(0, "error generating RSA key: %s", gpg_strerror (rc));
    exit(2);
  }

  check_generated_rsa_key (key, 257);
  gcry_sexp_release (key);

  fprintf (stderr, "creating 512 bit RSA key with default e\n");
  rc = gcry_sexp_new (&keyparm,
		      "(genkey\n"
		      " (rsa\n"
		      "  (nbits 3:512)\n"
		      "  (rsa-use-e 1:0)\n"
		      " ))", 0, 1);
  if (rc) {
    DBG_ERROR(0, "error creating S-expression: %s", gpg_strerror (rc));
    exit(2);
  }
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc) {
    DBG_ERROR(0, "error generating RSA key: %s", gpg_strerror (rc));
    exit(2);
  }

  check_generated_rsa_key (key, 0); /* We don't expect a constant exponent. */
  gcry_sexp_release (key);


  fprintf (stderr, "creating 768 bit RSA key with e=65537\n");
  rc = gcry_sexp_new (&keyparm,
		      "(genkey\n"
		      " (rsa\n"
		      "  (nbits 3:768)\n"
		      "  (rsa-use-e 5:65537)\n"
		      " ))", 0, 1);
  if (rc) {
    DBG_ERROR(0, "error creating S-expression: %s", gpg_strerror (rc));
    exit(2);
  }
  rc = gcry_pk_genkey (&key, keyparm);
  gcry_sexp_release (keyparm);
  if (rc) {
    DBG_ERROR(0, "error generating RSA key: %s", gpg_strerror (rc));
    exit(2);
  }

  check_generated_rsa_key (key, 65537);
  gcry_sexp_release (key);

}



int testHttpSession(int argc, char **argv) {
  GWEN_GUI *gui;
  const char *urlString;
  GWEN_HTTP_SESSION *sess;
  int rv;
  GWEN_BUFFER *buf;

  if (argc<3) {
    fprintf(stderr, "%s %s URL\n", argv[0], argv[1]);
    return 1;
  }
  urlString=argv[2];

  fprintf(stderr, "Creating gui.\n");
  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  sess=GWEN_HttpSession_new(urlString, "http", 80);
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    fprintf(stderr, "ERROR: Could not init http session.\n");
    return 3;
  }

  rv=GWEN_HttpSession_SendPacket(sess, "GET", NULL, 0);
  if (rv<0) {
    fprintf(stderr, "ERROR: Could not send request.\n");
    return 3;
  }

  buf=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=GWEN_HttpSession_RecvPacket(sess, buf);
  if (rv<0) {
    fprintf(stderr, "ERROR: Could not receive response.\n");
    return 3;
  }

  GWEN_Buffer_Dump(buf, 2);

  return 0;
}



int testDES(int argc, char **argv) {
  GWEN_CRYPT_KEY *skey;
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  GWEN_BUFFER *buf3;
  uint32_t l;
  int rv;
  const char testString[]={
    0x90, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x18, 0x27, 0x36, 0x45, 0x54, 0x63, 0x72,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
  };

  skey=GWEN_Crypt_KeyDes3K_Generate(GWEN_Crypt_CryptMode_Cbc, 24, 2);
  if (skey==NULL) {
    DBG_ERROR(0, "Unable to generate DES key");
    return 2;
  }

  buf1=GWEN_Buffer_new(0, sizeof(testString), 0, 1);
  buf2=GWEN_Buffer_new(0, sizeof(testString), 0, 1);
  buf3=GWEN_Buffer_new(0, sizeof(testString), 0, 1);

  GWEN_Buffer_AppendBytes(buf1, testString, sizeof(testString));

  /* encrypt buf1 */
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(buf2);
  rv=GWEN_Crypt_Key_Encipher(skey,
			     (uint8_t*)GWEN_Buffer_GetStart(buf1),
			     GWEN_Buffer_GetUsedBytes(buf1),
			     (uint8_t*)GWEN_Buffer_GetPosPointer(buf2),
			     &l);
  if (rv<0) {
    DBG_ERROR(0, "Unable to encipher");
    return 2;
  }
  GWEN_Buffer_IncrementPos(buf2, l);
  GWEN_Buffer_AdjustUsedBytes(buf2);

  /* decrypt buf2 */
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(buf3);
  rv=GWEN_Crypt_Key_Decipher(skey,
			     (uint8_t*)GWEN_Buffer_GetStart(buf2),
			     GWEN_Buffer_GetUsedBytes(buf2),
			     (uint8_t*)GWEN_Buffer_GetPosPointer(buf3),
			     &l);
  if (rv<0) {
    DBG_ERROR(0, "Unable to decipher");
    return 2;
  }
  GWEN_Buffer_IncrementPos(buf3, l);
  GWEN_Buffer_AdjustUsedBytes(buf3);

  if (GWEN_Buffer_GetUsedBytes(buf1)!=
      GWEN_Buffer_GetUsedBytes(buf3)) {
    DBG_ERROR(0, "Buffer size does not match");
    return 2;
  }

  if (memcmp(GWEN_Buffer_GetStart(buf1),
	     GWEN_Buffer_GetStart(buf3),
	     GWEN_Buffer_GetUsedBytes(buf1))!=0) {
    DBG_ERROR(0, "Buffers do not match");
    DBG_ERROR(0, "Expected:");
    GWEN_Buffer_Dump(buf1, 2);
    DBG_ERROR(0, "Found:");
    GWEN_Buffer_Dump(buf3, 2);
    DBG_ERROR(0, "Encrypted:");
    GWEN_Buffer_Dump(buf2, 2);
    return 2;
  }

  GWEN_Buffer_free(buf3);
  GWEN_Buffer_free(buf2);
  GWEN_Buffer_free(buf1);
  GWEN_Crypt_Key_free(skey);

  return 0;
}



int testDES2(int argc, char **argv) {
  GWEN_CRYPT_KEY *skey;
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  GWEN_BUFFER *buf3;
  uint32_t l;
  int rv;
  uint8_t kd[24];
  const char testString[]={
    0x90, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x18, 0x27, 0x36, 0x45, 0x54, 0x63, 0x72,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
  };

  GWEN_Crypt_Random(2, kd, 16);
  memmove(kd+16, kd, 8);

  skey=GWEN_Crypt_KeyDes3K_fromData(GWEN_Crypt_CryptMode_Cbc,
				    24,
				    kd,
				    24);
  if (skey==NULL) {
    DBG_ERROR(0, "Unable to generate DES key");
    return 2;
  }

  buf1=GWEN_Buffer_new(0, sizeof(testString), 0, 1);
  buf2=GWEN_Buffer_new(0, sizeof(testString), 0, 1);
  buf3=GWEN_Buffer_new(0, sizeof(testString), 0, 1);

  GWEN_Buffer_AppendBytes(buf1, testString, sizeof(testString));

  /* encrypt buf1 */
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(buf2);
  rv=GWEN_Crypt_Key_Encipher(skey,
			     (uint8_t*)GWEN_Buffer_GetStart(buf1),
			     GWEN_Buffer_GetUsedBytes(buf1),
			     (uint8_t*)GWEN_Buffer_GetPosPointer(buf2),
			     &l);
  if (rv<0) {
    DBG_ERROR(0, "Unable to encipher");
    return 2;
  }
  GWEN_Buffer_IncrementPos(buf2, l);
  GWEN_Buffer_AdjustUsedBytes(buf2);

  /* decrypt buf2 */
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(buf3);
  rv=GWEN_Crypt_Key_Decipher(skey,
			     (uint8_t*)GWEN_Buffer_GetStart(buf2),
			     GWEN_Buffer_GetUsedBytes(buf2),
			     (uint8_t*)GWEN_Buffer_GetPosPointer(buf3),
			     &l);
  if (rv<0) {
    DBG_ERROR(0, "Unable to decipher");
    return 2;
  }
  GWEN_Buffer_IncrementPos(buf3, l);
  GWEN_Buffer_AdjustUsedBytes(buf3);

  if (GWEN_Buffer_GetUsedBytes(buf1)!=
      GWEN_Buffer_GetUsedBytes(buf3)) {
    DBG_ERROR(0, "Buffer size does not match");
    return 2;
  }

  if (memcmp(GWEN_Buffer_GetStart(buf1),
	     GWEN_Buffer_GetStart(buf3),
	     GWEN_Buffer_GetUsedBytes(buf1))!=0) {
    DBG_ERROR(0, "Buffers do not match");
    DBG_ERROR(0, "Expected:");
    GWEN_Buffer_Dump(buf1, 2);
    DBG_ERROR(0, "Found:");
    GWEN_Buffer_Dump(buf3, 2);
    DBG_ERROR(0, "Encrypted:");
    GWEN_Buffer_Dump(buf2, 2);
    return 2;
  }

  GWEN_Buffer_free(buf3);
  GWEN_Buffer_free(buf2);
  GWEN_Buffer_free(buf1);
  GWEN_Crypt_Key_free(skey);

  return 0;
}



int testDES3(int argc, char **argv) {
  GWEN_CRYPT_KEY *skey;
  uint32_t l2;
  uint32_t l3;
  int rv;
  const uint8_t iv[8]={
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  const uint8_t testString[]={
    0x90, 0x80, 0x70, 0x60, 0x50, 0x40, 0x30, 0x20,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x18, 0x27, 0x36, 0x45, 0x54, 0x63, 0x72,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
    0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22,
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
  };
  uint8_t buf2[sizeof(testString)];
  uint8_t buf3[sizeof(testString)];

  skey=GWEN_Crypt_KeyDes3K_Generate(GWEN_Crypt_CryptMode_Cbc, 24, 2);
  if (skey==NULL) {
    DBG_ERROR(0, "Unable to generate DES key");
    return 2;
  }

  /* encrypt buf1 */
  l2=sizeof(buf2);
  rv=GWEN_Crypt_Key_Encipher(skey,
			     testString,
			     sizeof(testString),
			     buf2,
			     &l2);
  if (rv<0) {
    DBG_ERROR(0, "Unable to encipher");
    return 2;
  }

  GWEN_Crypt_KeyDes3K_SetIV(skey, iv, sizeof(iv));

  /* decrypt buf2 */
  l3=sizeof(buf3);
  rv=GWEN_Crypt_Key_Decipher(skey,
			     buf2,
                             l2,
			     buf3,
			     &l3);
  if (rv<0) {
    DBG_ERROR(0, "Unable to decipher");
    return 2;
  }

  if (l2!=l3) {
    DBG_ERROR(0, "Buffer size does not match");
    return 2;
  }

  if (l2!=sizeof(testString)) {
    DBG_ERROR(0, "Buffer size does not match size of testString");
    return 2;
  }

  if (memcmp(testString, buf3, l3)!=0) {
    DBG_ERROR(0, "Buffers do not match");
    DBG_ERROR(0, "Expected:");
    GWEN_Text_DumpString((const char*)testString, sizeof(testString),
			 2);
    DBG_ERROR(0, "Found:");
    GWEN_Text_DumpString((const char*)buf3, l3, 2);
    DBG_ERROR(0, "Encrypted:");
    GWEN_Text_DumpString((const char*)buf2, l2, 2);
    return 2;
  }

  GWEN_Crypt_Key_free(skey);

  return 0;
}



int testDES4(int argc, char **argv) {
  GWEN_CRYPT_KEY *skey;
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  GWEN_BUFFER *buf3;
  uint32_t l;
  int rv;
  const char *fname;
#ifdef NEU1
  const uint8_t keyData[]={
    0x2a, 0x79, 0xc4, 0x45, 0xbc, 0x13, 0x5d, 0x13,
    0xe0, 0xec, 0xe6, 0x43, 0x6d, 0x73, 0x91, 0x0b
  };
#else
  const uint8_t keyData[]={
    0x01, 0x3e, 0x1f, 0xf1, 0xab, 0xbf, 0x13, 0x79,
    0x5b, 0x68, 0x9d, 0x26, 0x31, 0xdc, 0xe0, 0x52
  };
#endif

  if (argc<3) {
    DBG_ERROR(0, "Missing filename");
    return 1;
  }
  fname=argv[2];

  skey=GWEN_Crypt_KeyDes3K_fromData(GWEN_Crypt_CryptMode_Cbc, 16,
                                    keyData, 16);
  if (skey==NULL) {
    DBG_ERROR(0, "Unable to create DES key");
    return 2;
  }

  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  rv=readFile(fname, buf1);
  if (rv<0) {
    DBG_ERROR(0, "Unable to read file [%s]: %d",
	      fname, rv);
    return 2;
  }
#ifdef NEU1
  GWEN_Buffer_FillWithBytes(buf1, 0, 7);
#endif

  buf2=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(buf1)+16, 0, 1);

  /* decrypt buf1 */
  fprintf(stderr, "Decrypting %d bytes", GWEN_Buffer_GetUsedBytes(buf1));
  l=GWEN_Buffer_GetMaxUnsegmentedWrite(buf2);

  GWEN_Crypt_KeyDes3K_SetIV(skey, NULL, 0);

  rv=GWEN_Crypt_Key_Decipher(skey,
			     (uint8_t*)GWEN_Buffer_GetStart(buf1),
			     GWEN_Buffer_GetUsedBytes(buf1),
			     (uint8_t*)GWEN_Buffer_GetPosPointer(buf2),
			     &l);
  if (rv<0) {
    DBG_ERROR(0, "Unable to decipher");
    return 2;
  }
  fprintf(stderr, "Decrypted %d bytes", l);
  GWEN_Buffer_IncrementPos(buf2, l);
  GWEN_Buffer_AdjustUsedBytes(buf2);

  writeFile("des4.out1",
	    GWEN_Buffer_GetStart(buf2),
	    GWEN_Buffer_GetUsedBytes(buf2));

  buf3=GWEN_Buffer_new(0, 256, 0, 1);
  rv=zip_inflate(GWEN_Buffer_GetStart(buf2),
		 GWEN_Buffer_GetUsedBytes(buf2),
		 buf3);

  writeFile("des4.out2",
	    GWEN_Buffer_GetStart(buf3),
	    GWEN_Buffer_GetUsedBytes(buf3));

  if (rv<0) {
    DBG_ERROR(0, "Unable to unzip");
    return 2;
  }

  GWEN_Buffer_free(buf2);
  GWEN_Buffer_free(buf1);
  GWEN_Crypt_Key_free(skey);

  return 0;
}



int testCryptMgr1(int argc, char **argv) {
  int rv;
  GWEN_CRYPT_KEY *pubKey;
  GWEN_CRYPT_KEY *secretKey;
  GWEN_CRYPTMGR *cm;
  uint8_t testData[]=
    "This is the test data           "
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
    "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy";
  GWEN_BUFFER *tbuf1;
  GWEN_BUFFER *tbuf2;

  fprintf(stderr, "Generating key pair...\n");
  rv=GWEN_Crypt_KeyRsa_GeneratePair(2048/8, 1, &pubKey, &secretKey);
  if (rv) {
    fprintf(stderr, "ERROR: Could not generate key pair (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "Generating key pair... done.\n");

  GWEN_Crypt_KeyRsa_AddFlags(pubKey, GWEN_CRYPT_KEYRSA_FLAGS_DIRECTSIGN);
  GWEN_Crypt_KeyRsa_AddFlags(secretKey, GWEN_CRYPT_KEYRSA_FLAGS_DIRECTSIGN);

  cm=GWEN_CryptMgrKeys_new("local", secretKey, "local", pubKey, 1);
  assert(cm);

  fprintf(stderr, "Signing message...\n");
  tbuf1=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_CryptMgr_Sign(cm, testData, sizeof(testData)-1, tbuf1);
  if (rv) {
    fprintf(stderr, "ERROR: Could not sign (%d).\n", rv);
    GWEN_Buffer_Dump(tbuf1, 2);
    return 2;
  }
  fprintf(stderr, "Signing message... done\n");

  fprintf(stderr, "Verifying message...\n");
  tbuf2=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_CryptMgr_Verify(cm,
			  (const uint8_t*)GWEN_Buffer_GetStart(tbuf1),
			  GWEN_Buffer_GetUsedBytes(tbuf1),
			  tbuf2);
  if (rv) {
    fprintf(stderr, "ERROR: Could not verify (%d).\n", rv);
    GWEN_Buffer_Dump(tbuf2, 2);
    return 2;
  }
  fprintf(stderr, "Verifying message... done\n");

  fprintf(stderr, "Signature is valid.\n");

  return 0;
}



int testCryptMgr2(int argc, char **argv) {
  int rv;
  GWEN_CRYPT_KEY *pubKey;
  GWEN_CRYPT_KEY *secretKey;
  GWEN_CRYPTMGR *cm;
  uint8_t testData[]=
    "This is the test data           "
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
    "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy";
  GWEN_BUFFER *tbuf1;
  GWEN_BUFFER *tbuf2;

  fprintf(stderr, "Generating key pair...\n");
  rv=GWEN_Crypt_KeyRsa_GeneratePair(2048/8, 1, &pubKey, &secretKey);
  if (rv) {
    fprintf(stderr, "ERROR: Could not generate key pair (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "Generating key pair... done.\n");

  GWEN_Crypt_KeyRsa_AddFlags(pubKey, GWEN_CRYPT_KEYRSA_FLAGS_DIRECTSIGN);
  GWEN_Crypt_KeyRsa_AddFlags(secretKey, GWEN_CRYPT_KEYRSA_FLAGS_DIRECTSIGN);

  cm=GWEN_CryptMgrKeys_new("local", secretKey, "local", pubKey, 1);
  assert(cm);

  fprintf(stderr, "Encrypting message...\n");
  tbuf1=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_CryptMgr_Encrypt(cm, testData, sizeof(testData)-1, tbuf1);
  if (rv) {
    fprintf(stderr, "ERROR: Could not encrypt (%d).\n", rv);
    GWEN_Buffer_Dump(tbuf1, 2);
    return 2;
  }
  fprintf(stderr, "Encrypting message... done\n");

  fprintf(stderr, "Decrypting message...\n");
  tbuf2=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_CryptMgr_Decrypt(cm,
			   (const uint8_t*)GWEN_Buffer_GetStart(tbuf1),
			   GWEN_Buffer_GetUsedBytes(tbuf1),
			   tbuf2);
  if (rv) {
    fprintf(stderr, "ERROR: Could not verify (%d).\n", rv);
    GWEN_Buffer_Dump(tbuf2, 2);
    return 2;
  }
  fprintf(stderr, "Decrypting message... done\n");

  if (sizeof(testData)-1!=GWEN_Buffer_GetUsedBytes(tbuf2)) {
    fprintf(stderr, "Crypto-Error (size):\n");
    GWEN_Buffer_Dump(tbuf1, 2);
    GWEN_Buffer_Dump(tbuf2, 2);
    return 2;
  }

  if (memcmp(testData, GWEN_Buffer_GetStart(tbuf2), GWEN_Buffer_GetUsedBytes(tbuf2))!=0) {
    fprintf(stderr, "Crypto-Error (content):\n");
    GWEN_Buffer_Dump(tbuf1, 2);
    GWEN_Buffer_Dump(tbuf2, 2);
    return 2;
  }

  fprintf(stderr, "Crypto-stuff ok.\n");

  return 0;
}



int testCryptMgr3(int argc, char **argv) {
  int rv;
  GWEN_CRYPT_KEY *pubKey;
  GWEN_CRYPT_KEY *secretKey;
  GWEN_CRYPTMGR *cm;
  uint8_t testData[]=
    "This is the test data           "
    "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
    "yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy";
  GWEN_BUFFER *tbuf1;
  GWEN_BUFFER *tbuf2;

  fprintf(stderr, "Generating key pair...\n");
  rv=GWEN_Crypt_KeyRsa_GeneratePair(2048/8, 1, &pubKey, &secretKey);
  if (rv) {
    fprintf(stderr, "ERROR: Could not generate key pair (%d).\n", rv);
    return 2;
  }
  fprintf(stderr, "Generating key pair... done.\n");

  GWEN_Crypt_KeyRsa_AddFlags(pubKey, GWEN_CRYPT_KEYRSA_FLAGS_DIRECTSIGN);
  GWEN_Crypt_KeyRsa_AddFlags(secretKey, GWEN_CRYPT_KEYRSA_FLAGS_DIRECTSIGN);

  cm=GWEN_CryptMgrKeys_new("local", secretKey, "local", pubKey, 1);
  assert(cm);

  fprintf(stderr, "Encoding message...\n");
  tbuf1=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_CryptMgr_Encode(cm, testData, sizeof(testData)-1, tbuf1);
  if (rv) {
    fprintf(stderr, "ERROR: Could not encrypt (%d).\n", rv);
    GWEN_Buffer_Dump(tbuf1, 2);
    return 2;
  }
  fprintf(stderr, "Encoding message... done\n");

  fprintf(stderr, "Decoding message...\n");
  tbuf2=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_CryptMgr_Decode(cm,
			  (const uint8_t*)GWEN_Buffer_GetStart(tbuf1),
			  GWEN_Buffer_GetUsedBytes(tbuf1),
			  tbuf2);
  if (rv) {
    fprintf(stderr, "ERROR: Could not verify (%d).\n", rv);
    GWEN_Buffer_Dump(tbuf2, 2);
    return 2;
  }
  fprintf(stderr, "Decoding message... done\n");

  if (sizeof(testData)-1!=GWEN_Buffer_GetUsedBytes(tbuf2)) {
    fprintf(stderr, "Crypto-Error (size):\n");
    GWEN_Buffer_Dump(tbuf1, 2);
    GWEN_Buffer_Dump(tbuf2, 2);
    return 2;
  }

  if (memcmp(testData, GWEN_Buffer_GetStart(tbuf2), GWEN_Buffer_GetUsedBytes(tbuf2))!=0) {
    fprintf(stderr, "Crypto-Error (content):\n");
    GWEN_Buffer_Dump(tbuf1, 2);
    GWEN_Buffer_Dump(tbuf2, 2);
    return 2;
  }

  fprintf(stderr, "Crypto-stuff ok.\n");

  return 0;
}



int testPss1(int argc, char **argv) {
  uint8_t em[]={
    0x57, 0xa4, 0x11, 0xc3, 0x48, 0x99, 0xf6, 0x8e, 0xb8, 0x93, 0x83, 0x6a, 0xe5, 0xc6, 0xe1, 0xc3,
    0x11, 0x45, 0xea, 0x6e, 0x8c, 0xf7, 0xd3, 0xb1, 0xc6, 0x17, 0xb6, 0x55, 0xf9, 0xc6, 0x64, 0x7a,
    0xa8, 0xe1, 0x07, 0x1b, 0xa0, 0xc6, 0x08, 0xfa, 0x70, 0xfd, 0xbb, 0xa4, 0x52, 0xb6, 0xa9, 0xcb,
    0x42, 0x31, 0xb0, 0x5a, 0x70, 0xe4, 0xa7, 0x09, 0xb0, 0x37, 0x15, 0xe4, 0xc5, 0x89, 0xd1, 0xc1,
    0x24, 0x0f, 0x65, 0xa9, 0x7a, 0x1a, 0x20, 0xe0, 0x2a, 0xd8, 0x91, 0x00, 0xb2, 0x8a, 0xf7, 0x93,
    0x5b, 0x86, 0x4e, 0x40, 0xb8, 0x20, 0x15, 0xb5, 0x34, 0x0a, 0xe5, 0x4c, 0xf2, 0x29, 0x02, 0x1d,
    0x8c, 0x15, 0xcf, 0x1e, 0x81, 0x6a, 0x7d, 0x33, 0x12, 0xc6, 0x39, 0xe6, 0xa4, 0x1e, 0x2d, 0x95,
    0x07, 0xe4, 0x2d, 0x23, 0xb5, 0x8a, 0xd6, 0xb2, 0xef, 0x1c, 0xed, 0x92, 0x5a, 0x7c, 0xa9, 0x1f,
    0xb0, 0xce, 0xb9, 0x2b, 0x1c, 0xa8, 0x73, 0x25, 0xa2, 0x82, 0xed, 0x13, 0x50, 0x47, 0x58, 0xd5,
    0xdf, 0xd9, 0x94, 0x5f, 0x3a, 0x7e, 0x73, 0xbe, 0xd0, 0x09, 0xa8, 0xcf, 0xdf, 0xd8, 0x71, 0x77,
    0x0c, 0x70, 0xe6, 0xb3, 0xc0, 0x10, 0x96, 0x70, 0x27, 0x31, 0x4d, 0x8b, 0x17, 0x5b, 0x30, 0x92,
    0xdb, 0xac, 0x62, 0x41, 0x90, 0x8e, 0x6b, 0xac, 0xcf, 0x7b, 0x42, 0x63, 0x66, 0xe7, 0xb3, 0xee,
    0xd4, 0x32, 0xab, 0x2d, 0xa4, 0xf0, 0x5d, 0x71, 0x25, 0x4a, 0x6f, 0x8c, 0x0d, 0xda, 0x43, 0xf0,
    0x53, 0xb5, 0x80, 0xc3, 0xd6, 0x77, 0xed, 0x5f, 0xe6, 0x1b, 0x3c, 0x5a, 0xa7, 0x23, 0xd7, 0x0b,
    0xa6, 0x0c, 0xdc, 0xbd, 0xd6, 0x59, 0x12, 0xfa, 0x37, 0x9b, 0xcd, 0xa1, 0x78, 0x06, 0x07, 0x2e,
    0x35, 0xa7, 0x7d, 0x11, 0xf5, 0xa7, 0xac, 0xbc
  };
  uint8_t hash[]={
    0x00, 0x6d, 0x9d, 0x67, 0x58, 0xa9, 0x23, 0xf5, 0x46, 0xdc, 0x6d, 0x2b, 0xd7, 0xea, 0x78, 0xab,
    0x1d, 0xcf, 0x6d, 0x47, 0x37, 0x35, 0xae, 0x93, 0xa9, 0x3b, 0x69, 0x17, 0xcc, 0xe1, 0xf5, 0x05
  };
  GWEN_MDIGEST *md;
  int rv;

  md=GWEN_MDigest_Sha256_new();
  rv=GWEN_Padd_VerifyPkcs1Pss(em, sizeof(em),
			      1984,
			      hash, sizeof(hash),
			      32,
			      md);
  if (rv<0) {
    fprintf(stderr, "Error decoding em\n");
    return 2;
  }

  fprintf(stderr, "Success\n");
  return 0;
}



int testPss2(int argc, char **argv) {
  uint8_t m[]={"This is the test message"};
  uint8_t em[1024];
  uint8_t hash[32];
  GWEN_MDIGEST *md;
  int rv;

  md=GWEN_MDigest_Sha256_new();

  /* hash message */
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=GWEN_MDigest_Update(md, m, sizeof(m));
  if (rv<0) {
    fprintf(stderr, "Error hashing m (update: %d)\n", rv);
    return 2;
  }

  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    fprintf(stderr, "Error hashing m (end: %d)\n", rv);
    return 2;
  }

  memmove(hash,
	  GWEN_MDigest_GetDigestPtr(md),
	  GWEN_MDigest_GetDigestSize(md));

  fprintf(stderr, "Message encoded.\n");
  GWEN_MDigest_free(md);

  md=GWEN_MDigest_Sha256_new();
  rv=GWEN_Padd_AddPkcs1Pss(em, sizeof(em),
			   1984,
			   hash, sizeof(hash),
			   32,
			   md);
  if (rv<0) {
    fprintf(stderr, "Error decoding em\n");
    return 2;
  }

  rv=GWEN_Padd_VerifyPkcs1Pss(em, rv,
			      1984,
			      hash, sizeof(hash),
			      32,
			      md);
  if (rv<0) {
    fprintf(stderr, "Error decoding em\n");
    return 2;
  }

  fprintf(stderr, "Success\n");
  return 0;
}



int testDialog(int argc, char **argv) {
  GWEN_XMLNODE *n;
  GWEN_XMLNODE *nDialog;
  GWEN_DIALOG *dlg;
  int rv;

  if (argc<3) {
    fprintf(stderr, "Name of testfile needed.\n");
    return 1;
  }
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"root");
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Debug);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);
  if (GWEN_XML_ReadFile(n, argv[2],
			GWEN_XML_FLAGS_DEFAULT |
			GWEN_XML_FLAGS_HANDLE_HEADERS)) {
    fprintf(stderr, "Error reading XML file.\n");
    return 1;
  }
  fprintf(stderr, "XML file:\n");
  GWEN_XMLNode_Dump(n, 2);

  nDialog=GWEN_XMLNode_FindFirstTag(n, "dialog", NULL, NULL);
  if (nDialog==NULL) {
    fprintf(stderr, "Dialog not found in XML file\n");
    return 2;
  }


  dlg=GWEN_Dialog_new("testdialog");
  rv=GWEN_Dialog_ReadXml(dlg, nDialog);
  if (rv) {
    fprintf(stderr, "Error reading widgets from XML node: %d\n", rv);
    return 2;
  }
  GWEN_XMLNode_free(n);

  return 0;
}



int testSyncIo1(int argc, char **argv) {
  int rv;
  const char *fname;
  GWEN_SYNCIO *sio;
  GWEN_BUFFER *tbuf;

  if (argc<3) {
    fprintf(stderr, "Name of testfile needed.\n");
    return 1;
  }

  fname=argv[2];
  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  fprintf(stderr, "Opening file\n");
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    fprintf(stderr, "Error opening file: %d\n", rv);
    return 2;
  }

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  do {
    uint8_t *p;

    GWEN_Buffer_AllocRoom(tbuf, 1024);
    p=(uint8_t*) GWEN_Buffer_GetPosPointer(tbuf);
    rv=GWEN_SyncIo_Read(sio, p, 1024);
    if (rv<0) {
      fprintf(stderr, "Error reading file: %d\n", rv);
      GWEN_Buffer_Dump(tbuf, 2);
      return 2;
    }
    else if (rv>0) {
      fprintf(stderr, "Received %d bytes\n", rv);
      GWEN_Buffer_IncrementPos(tbuf, rv);
      GWEN_Buffer_AdjustUsedBytes(tbuf);
    }
    else if (rv==0) {
      fprintf(stderr, "EOF met.\n");
    }
  } while(rv>0);

  fprintf(stderr, "File received.\n");
  GWEN_Buffer_Dump(tbuf, 2);

  rv=GWEN_SyncIo_Disconnect(sio);
  if (rv<0) {
    fprintf(stderr, "Error closing file: %d\n", rv);
    return 2;
  }

  fprintf(stderr, "Finished.\n");

  return 0;
}



int testSyncIo2(int argc, char **argv) {
  int rv;
  const char *fname;
  GWEN_SYNCIO *baseIo;
  GWEN_SYNCIO *sio;
  GWEN_BUFFER *tbuf;

  if (argc<3) {
    fprintf(stderr, "Name of testfile needed.\n");
    return 1;
  }

  fname=argv[2];
  baseIo=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  sio=GWEN_SyncIo_Buffered_new(baseIo);

  fprintf(stderr, "Opening file\n");
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    fprintf(stderr, "Error opening file: %d\n", rv);
    return 2;
  }

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  do {
    uint8_t *p;

    GWEN_Buffer_AllocRoom(tbuf, 1024);
    p=(uint8_t*) GWEN_Buffer_GetPosPointer(tbuf);
    rv=GWEN_SyncIo_Read(sio, p, 1024);
    if (rv<0) {
      fprintf(stderr, "Error reading file: %d\n", rv);
      GWEN_Buffer_Dump(tbuf, 2);
      return 2;
    }
    else if (rv>0) {
      fprintf(stderr, "Received %d bytes\n", rv);
      GWEN_Buffer_IncrementPos(tbuf, rv);
      GWEN_Buffer_AdjustUsedBytes(tbuf);
      if (p[rv-1]==10) {
	fprintf(stderr, "Received line:\n");
	GWEN_Buffer_Dump(tbuf, 2);
	GWEN_Buffer_Reset(tbuf);
        getchar();
      }
    }
    else if (rv==0) {
      fprintf(stderr, "EOF met.\n");
    }
  } while(rv>0);

  rv=GWEN_SyncIo_Disconnect(sio);
  if (rv<0) {
    fprintf(stderr, "Error closing file: %d\n", rv);
    return 2;
  }

  fprintf(stderr, "Finished.\n");

  return 0;
}



int testSyncIo3(int argc, char **argv) {
  int rv;
  const char *fname;
  GWEN_SYNCIO *baseIo;
  GWEN_SYNCIO *sio;
  GWEN_BUFFER *tbuf;

  if (argc<3) {
    fprintf(stderr, "Name of testfile needed.\n");
    return 1;
  }

  fname=argv[2];
  baseIo=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  sio=GWEN_SyncIo_Buffered_new(baseIo);

  fprintf(stderr, "Opening file\n");
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    fprintf(stderr, "Error opening file: %d\n", rv);
    return 2;
  }

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  do {
    uint8_t *p;

    GWEN_Buffer_AllocRoom(tbuf, 1024);
    p=(uint8_t*) GWEN_Buffer_GetPosPointer(tbuf);
    rv=GWEN_SyncIo_Read(sio, p, 1024);
    if (rv<0) {
      fprintf(stderr, "Error reading file: %d\n", rv);
      GWEN_Buffer_Dump(tbuf, 2);
      return 2;
    }
    else if (rv>0) {
      fprintf(stderr, "Received %d bytes\n", rv);
      GWEN_Buffer_IncrementPos(tbuf, rv);
      GWEN_Buffer_AdjustUsedBytes(tbuf);
      if (p[rv-1]==10) {
	fprintf(stderr, "Received line:\n");
	GWEN_Buffer_Dump(tbuf, 2);
	getchar();
      }
    }
    else if (rv==0) {
      fprintf(stderr, "EOF met.\n");
    }
    GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FLAGS_TRANSPARENT);
  } while(rv>0);


  fprintf(stderr, "File received.\n");
  GWEN_Buffer_Dump(tbuf, 2);

  rv=GWEN_SyncIo_Disconnect(sio);
  if (rv<0) {
    fprintf(stderr, "Error closing file: %d\n", rv);
    return 2;
  }

  fprintf(stderr, "Finished.\n");

  return 0;
}



int testHttp1(int argc, char **argv) {
  int rv;
  const char *fname;
  GWEN_SYNCIO *sio=NULL;
  GWEN_SYNCIO *sioTls;
  int firstRead=1;
  int bodySize=-1;
  int bytesRead=0;
  GWEN_BUFFER *tbuf;
  GWEN_GUI *gui;
  GWEN_DB_NODE *db;

  if (argc<3) {
    fprintf(stderr, "Name of testfile needed.\n");
    return 1;
  }

  fprintf(stderr, "Creating gui.\n");
  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  fname=argv[2];



  rv=GWEN_Gui_GetSyncIo(fname, "http", 80, &sio);
  if (rv<0) {
    fprintf(stderr,
	    "ERROR: Could not get SyncIO (%d)\n", rv);
    return 2;
  }

  sioTls=GWEN_SyncIo_GetBaseIoByTypeName(sio, GWEN_SYNCIO_TLS_TYPE);
  if (sioTls) {
    GWEN_SyncIo_SubFlags(sioTls, GWEN_SYNCIO_TLS_FLAGS_FORCE_SSL_V3);
    GWEN_SyncIo_AddFlags(sioTls, GWEN_SYNCIO_TLS_FLAGS_ALLOW_V1_CA_CRT);
    GWEN_SyncIo_AddFlags(sioTls, GWEN_SYNCIO_TLS_FLAGS_ADD_TRUSTED_CAS);
    fprintf(stderr, "Remote host: %s\n", GWEN_SyncIo_Tls_GetRemoteHostName(sioTls));
  }

  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    fprintf(stderr,
	    "ERROR: Could not connect (%d)\n", rv);
    return 2;
  }

  /* send request */
  db=GWEN_SyncIo_Http_GetDbHeaderOut(sio);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Content-length", 0);
  rv=GWEN_SyncIo_Write(sio, (uint8_t*)"", 0);
  if (rv<0) {
    fprintf(stderr,
	    "ERROR: Could not write (%d)\n", rv);
    return 2;
  }

  /* get response */
  tbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  for (;;) {
    uint8_t *p;
    uint32_t l;

    rv=GWEN_Buffer_AllocRoom(tbuf, 1024);
    if (rv<0) {
      fprintf(stderr,
	      "ERROR in check_syncio_http1: Could not allocRoom (%d)\n", rv);
      return 2;
    }

    p=(uint8_t*) GWEN_Buffer_GetPosPointer(tbuf);
    l=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf);
    do {
      rv=GWEN_SyncIo_Read(sio, p, l-1);
    } while(rv==GWEN_ERROR_INTERRUPTED);
    if (rv==0)
      break;
    else if (rv<0) {
      if (rv==GWEN_ERROR_EOF) {
	if (bodySize!=-1 && bytesRead<bodySize) {
	  fprintf(stderr,
		  "ERROR: Received too few bytes (%d<%d)\n",
		  bytesRead, bodySize);
	  return 2;
	}
      }
      fprintf(stderr,
	      "ERROR: Could not read (%d) [%d / %d]\n",
	      rv, bytesRead, bodySize);
      return 2;
    }
    else {
      GWEN_Buffer_IncrementPos(tbuf, rv);
      GWEN_Buffer_AdjustUsedBytes(tbuf);
      if (firstRead) {
	GWEN_DB_NODE *db;

	db=GWEN_SyncIo_Http_GetDbHeaderIn(sio);
	bodySize=GWEN_DB_GetIntValue(db, "Content-length", 0, -1);
      }
      bytesRead+=rv;
    }

    if (bodySize!=-1 && bytesRead>=bodySize) {
      break;
    }
    firstRead=0;
  }

#if 1
  fprintf(stderr, "Received:\n");
  GWEN_Buffer_Dump(tbuf, 2);
#endif
  GWEN_Buffer_free(tbuf);

  rv=GWEN_SyncIo_Disconnect(sio);
  if (rv<0) {
    fprintf(stderr,
	    "ERROR in check_syncio_http1: Could not disconnect (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Finished.\n");

  return 0;
}



int testHttp2(int argc, char **argv) {
  int rv;
  const char *foutName;
  const char *finName;
  const char *url;
  GWEN_SYNCIO *sio=NULL;
  GWEN_SYNCIO *sioTls;
  int firstRead=1;
  int bodySize=-1;
  int bytesRead=0;
  GWEN_BUFFER *ibuf;
  GWEN_BUFFER *tbuf;
  GWEN_GUI *gui;
  GWEN_DB_NODE *db;

  if (argc<5) {
    fprintf(stderr, "Usage: %s url sendfile recvfile\n", argv[0]);
    return 1;
  }

  fprintf(stderr, "Creating gui.\n");
  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  url=argv[2];
  foutName=argv[3];
  finName=argv[4];

  ibuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=readFile(foutName, ibuf);
  if (rv<0) {
    fprintf(stderr,
	    "ERROR: Could not read file (%d)\n", rv);
    return 2;
  }

  rv=GWEN_Gui_GetSyncIo(url, "http", 80, &sio);
  if (rv<0) {
    fprintf(stderr,
	    "ERROR: Could not get SyncIO (%d)\n", rv);
    return 2;
  }

  sioTls=GWEN_SyncIo_GetBaseIoByTypeName(sio, GWEN_SYNCIO_TLS_TYPE);
  if (sioTls) {
    GWEN_SyncIo_SubFlags(sioTls, GWEN_SYNCIO_TLS_FLAGS_FORCE_SSL_V3);
    GWEN_SyncIo_AddFlags(sioTls, GWEN_SYNCIO_TLS_FLAGS_ALLOW_V1_CA_CRT);
    GWEN_SyncIo_AddFlags(sioTls, GWEN_SYNCIO_TLS_FLAGS_ADD_TRUSTED_CAS);
    fprintf(stderr, "Remote host: %s\n", GWEN_SyncIo_Tls_GetRemoteHostName(sioTls));
  }

  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    fprintf(stderr,
	    "ERROR: Could not connect (%d)\n", rv);
    return 2;
  }

  /* send request */
  db=GWEN_SyncIo_Http_GetDbCommandOut(sio);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Command", "POST");

  db=GWEN_SyncIo_Http_GetDbHeaderOut(sio);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Content-Type", "application/x-www-form-urlencoded");
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "Content-length", GWEN_Buffer_GetUsedBytes(ibuf));
  rv=GWEN_SyncIo_WriteForced(sio,
			     (uint8_t*) GWEN_Buffer_GetStart(ibuf),
			     GWEN_Buffer_GetUsedBytes(ibuf));
  if (rv<0) {
    fprintf(stderr,
	    "ERROR: Could not write (%d)\n", rv);
    return 2;
  }

  /* get response */
  tbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  for (;;) {
    uint8_t *p;
    uint32_t l;

    rv=GWEN_Buffer_AllocRoom(tbuf, 1024);
    if (rv<0) {
      fprintf(stderr,
	      "ERROR in check_syncio_http1: Could not allocRoom (%d)\n", rv);
      return 2;
    }

    p=(uint8_t*) GWEN_Buffer_GetPosPointer(tbuf);
    l=GWEN_Buffer_GetMaxUnsegmentedWrite(tbuf);
    do {
      rv=GWEN_SyncIo_Read(sio, p, l-1);
    } while(rv==GWEN_ERROR_INTERRUPTED);
    if (rv==0)
      break;
    else if (rv<0) {
      if (rv==GWEN_ERROR_EOF) {
	if (bodySize!=-1 && bytesRead<bodySize) {
	  fprintf(stderr,
		  "ERROR: Received too few bytes (%d<%d)\n",
		  bytesRead, bodySize);
	  return 2;
	}
      }
      fprintf(stderr,
	      "ERROR: Could not read (%d) [%d / %d]\n",
	      rv, bytesRead, bodySize);
      return 2;
    }
    else {
      GWEN_Buffer_IncrementPos(tbuf, rv);
      GWEN_Buffer_AdjustUsedBytes(tbuf);
      if (firstRead) {
	GWEN_DB_NODE *db;

	db=GWEN_SyncIo_Http_GetDbHeaderIn(sio);
	bodySize=GWEN_DB_GetIntValue(db, "Content-length", 0, -1);
      }
      bytesRead+=rv;
    }

    if (bodySize!=-1 && bytesRead>=bodySize) {
      break;
    }
    firstRead=0;
  }

  rv=writeFile(finName, GWEN_Buffer_GetStart(tbuf), GWEN_Buffer_GetUsedBytes(tbuf));
  if (rv<0) {
    fprintf(stderr,
	    "ERROR in writeFile (%d)\n", rv);
    return 2;
  }

#if 1
  fprintf(stderr, "Received:\n");
  GWEN_Buffer_Dump(tbuf, 2);
#endif
  GWEN_Buffer_free(tbuf);

  rv=GWEN_SyncIo_Disconnect(sio);
  if (rv<0) {
    fprintf(stderr,
	    "ERROR in check_syncio_http1: Could not disconnect (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Finished.\n");

  return 0;
}



int testTresor1(int argc, char **argv) {
  int rv;
  const char *testData="This is the actual test data";
  GWEN_BUFFER *buf1;
  GWEN_BUFFER *buf2;
  int l;

  buf1=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_SmallTresor_Encrypt((const uint8_t*) testData,
			      strlen(testData),
			      "TESTPASSWORD",
			      buf1,
			      1546,
			      1937);
  if (rv<0) {
    fprintf(stderr,
	    "ERROR in checkTresor1: Could not encrypt (%d)\n", rv);
    return 2;
  }

  buf2=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_SmallTresor_Decrypt((const uint8_t*) GWEN_Buffer_GetStart(buf1),
                              GWEN_Buffer_GetUsedBytes(buf1),
			      "TESTPASSWORD",
			      buf2,
			      1546,
			      1937);
  if (rv<0) {
    fprintf(stderr,
	    "ERROR in checkTresor1: Could not decrypt (%d)\n", rv);
    return 2;
  }

  l=strlen(testData);
  if (l!=GWEN_Buffer_GetUsedBytes(buf2)) {
    fprintf(stderr, "Bad result (len):\n");
    GWEN_Buffer_Dump(buf2, 2);
    return 2;
  }

  if (strcmp(testData, GWEN_Buffer_GetStart(buf2))!=0) {
    fprintf(stderr, "Bad result (content):\n");
    GWEN_Buffer_Dump(buf2, 2);
    return 2;
  }

  fprintf(stderr, "Finished.\n");

  return 0;
}



int testTresor2(int argc, char **argv) {
  int i;

  for (i=0; i<100; i++) {
    int rv;
    const char *testData="This is the actual test data";
    GWEN_BUFFER *buf1;
    GWEN_BUFFER *buf2;
    int l;

    fprintf(stderr, "Round %d...\n", i);
    buf1=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_SmallTresor_Encrypt((const uint8_t*) testData,
                                strlen(testData),
                                "TESTPASSWORD",
                                buf1,
                                1546,
                                1937);
    if (rv<0) {
      fprintf(stderr,
              "ERROR in checkTresor1: Could not encrypt (%d)\n", rv);
      return 2;
    }
  
    buf2=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_SmallTresor_Decrypt((const uint8_t*) GWEN_Buffer_GetStart(buf1),
                                GWEN_Buffer_GetUsedBytes(buf1),
                                "TESTPASSWORD",
                                buf2,
                                1546,
                                1937);
    if (rv<0) {
      fprintf(stderr,
              "ERROR in checkTresor1: Could not decrypt (%d)\n", rv);
      return 2;
    }
  
    l=strlen(testData);
    if (l!=GWEN_Buffer_GetUsedBytes(buf2)) {
      fprintf(stderr, "Bad result (len):\n");
      GWEN_Buffer_Dump(buf2, 2);
      return 2;
    }
  
    if (strcmp(testData, GWEN_Buffer_GetStart(buf2))!=0) {
      fprintf(stderr, "Bad result (content):\n");
      GWEN_Buffer_Dump(buf2, 2);
      return 2;
    }
    fprintf(stderr, "Round %d... Ok.\n", i);
  }

  fprintf(stderr, "Finished.\n");

  return 0;
}



int testHashTree(int argc, char **argv) {
  int rv;
  GWEN_MDIGEST *md;
  GWEN_STRINGLIST *sl;
  GWEN_STRINGLISTENTRY *se;

  if (argc<3) {
    fprintf(stderr, "Folder needed\n");
    return 1;
  }

  sl=GWEN_StringList_new();
  md=GWEN_MDigest_Rmd160_new();
  rv=GWEN_MDigest_HashFileTree(md, argv[2], NULL, sl);
  if (rv<0) {
    fprintf(stderr, "ERROR on HASHTREE (%d)\n", rv);
    return 2;
  }

  se=GWEN_StringList_FirstEntry(sl);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s && *s)
      fprintf(stderr, " %s\n", s);
    se=GWEN_StringListEntry_Next(se);
  }

  return 0;
}



int testDate1(int argc, char **argv) {
  GWEN_DATE *ti1;
  int j;
  int i;

  ti1=GWEN_Date_CurrentDate();
  assert(ti1);

  j=GWEN_Date_GetJulian(ti1);
  DBG_NOTICE(0, "Current Julian date: %d (%s)", j, GWEN_Date_GetString(ti1));

  for (i=1; i<20; i++) {
    GWEN_DATE  *ti2;

    ti2=GWEN_Date_fromJulian(j+i);
    if (ti2==NULL) {
      DBG_ERROR(0, "Bad julian date value %d", j+i);
    }
    else {
      DBG_NOTICE(0, "  - Julian date %2d: %s (%d)", j+i, GWEN_Date_GetString(ti2), GWEN_Date_WeekDay(ti1));
      GWEN_Date_free(ti2);
    }
  }

  for (i=1; i<4; i++) {
    GWEN_DATE  *ti2;

    ti2=GWEN_Date_fromJulian(j+(i*7));
    if (ti2==NULL) {
      DBG_ERROR(0, "Bad julian date value %d", j+(i*7));
    }
    else {
      DBG_NOTICE(0, "  + Julian date %2d: %s (%d)", j+(i*7), GWEN_Date_GetString(ti2), GWEN_Date_WeekDay(ti2));
      GWEN_Date_free(ti2);
    }
  }


  return 0;
}



int testDate2(int argc, char **argv) {
  GWEN_DATE *ti1;
  GWEN_BUFFER *dbuf;

  ti1=GWEN_Date_CurrentDate();
  assert(ti1);

  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Date_toStringWithTemplate(ti1, "w*, DDDD.MM.YYYY", dbuf);
  DBG_NOTICE(0, "Current date: %s", GWEN_Buffer_GetStart(dbuf));

  GWEN_Buffer_Reset(dbuf);
  GWEN_Date_toStringWithTemplate(ti1, "www, DD.M*.YYYY (W)", dbuf);
  DBG_NOTICE(0, "or like this: %s", GWEN_Buffer_GetStart(dbuf));

  GWEN_Buffer_Reset(dbuf);
  GWEN_Date_toStringWithTemplate(ti1, "w*, D.M.YYYY (W)", dbuf);
  DBG_NOTICE(0, "or like this: %s", GWEN_Buffer_GetStart(dbuf));

  return 0;
}



int testSar1(int argc, char **argv) {
  GWEN_SAR *sr;
  const char *aname;
  const char *fname;
  int rv;

  if (argc<4) {
    fprintf(stderr, "%s sar1 archive file\n", argv[0]);
    return 1;
  }

  aname=argv[2];
  fname=argv[3];

  sr=GWEN_Sar_new();
  rv=GWEN_Sar_CreateArchive(sr, aname);
  if (rv<0) {
    fprintf(stderr, "Error creating archive (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Adding file \"%s\"\n", fname);
  rv=GWEN_Sar_AddFile(sr, fname);
  if (rv<0) {
    fprintf(stderr, "Error adding file \"%s\" to archive \"%s\" (%d)\n",
            fname, aname, rv);
    return 2;
  }

  rv=GWEN_Sar_CloseArchive(sr, 0);
  if (rv<0) {
    fprintf(stderr, "Error closing archive (%d)\n", rv);
    return 2;
  }

  return 0;
}



int testSar2(int argc, char **argv) {
  GWEN_SAR *sr;
  const char *aname;
  int rv;
  const GWEN_SAR_FILEHEADER_LIST *fhl;

  if (argc<3) {
    fprintf(stderr, "%s sar2 archive\n", argv[0]);
    return 1;
  }

  aname=argv[2];

  sr=GWEN_Sar_new();
  rv=GWEN_Sar_OpenArchive(sr, aname,
                          GWEN_SyncIo_File_CreationMode_OpenExisting,
                          GWEN_SYNCIO_FILE_FLAGS_READ);
  if (rv<0) {
    fprintf(stderr, "Error opening archive (%d)\n", rv);
    return 2;
  }

  fhl=GWEN_Sar_GetHeaders(sr);
  if (fhl) {
    const GWEN_SAR_FILEHEADER *fh;

    fh=GWEN_SarFileHeader_List_First(fhl);
    while(fh) {
      const char *s;

      s=GWEN_SarFileHeader_GetPath(fh);
      rv=GWEN_Sar_CheckFile(sr, fh);
      if (rv<0) {
        fprintf(stderr, "%s: CRC error\n", s?s:"(noname)");
      }
      else {
        fprintf(stderr, "%s: Ok\n", s?s:"(noname)");
      }

      fh=GWEN_SarFileHeader_List_Next(fh);
    }
  }

  rv=GWEN_Sar_CloseArchive(sr, 0);
  if (rv<0) {
    fprintf(stderr, "Error closing archive (%d)\n", rv);
    return 2;
  }

  return 0;
}



int testSar3(int argc, char **argv) {
  GWEN_SAR *sr;
  const char *aname;
  int rv;
  const GWEN_SAR_FILEHEADER_LIST *fhl;

  if (argc<3) {
    fprintf(stderr, "%s sar2 archive\n", argv[0]);
    return 1;
  }

  aname=argv[2];

  sr=GWEN_Sar_new();
  rv=GWEN_Sar_OpenArchive(sr, aname,
                          GWEN_SyncIo_File_CreationMode_OpenExisting,
                          GWEN_SYNCIO_FILE_FLAGS_READ);
  if (rv<0) {
    fprintf(stderr, "Error opening archive (%d)\n", rv);
    return 2;
  }

  fhl=GWEN_Sar_GetHeaders(sr);
  if (fhl) {
    const GWEN_SAR_FILEHEADER *fh;

    fh=GWEN_SarFileHeader_List_First(fhl);
    while(fh) {
      const char *s;

      s=GWEN_SarFileHeader_GetPath(fh);
      rv=GWEN_Sar_ExtractFile(sr, fh);
      if (rv<0) {
        fprintf(stderr, "%s: CRC error\n", s?s:"(noname)");
      }
      else {
        fprintf(stderr, "%s: Ok\n", s?s:"(noname)");
      }

      fh=GWEN_SarFileHeader_List_Next(fh);
    }
  }

  rv=GWEN_Sar_CloseArchive(sr, 0);
  if (rv<0) {
    fprintf(stderr, "Error closing archive (%d)\n", rv);
    return 2;
  }

  return 0;
}



int testSar4(int argc, char **argv) {
  GWEN_SAR *sr;
  const char *aname;
  const char *fname;
  int rv;

  if (argc<4) {
    fprintf(stderr, "%s sar1 archive file\n", argv[0]);
    return 1;
  }

  aname=argv[2];
  fname=argv[3];

  sr=GWEN_Sar_new();
  rv=GWEN_Sar_OpenArchive(sr, aname,
                          GWEN_SyncIo_File_CreationMode_OpenExisting,
                          GWEN_SYNCIO_FILE_FLAGS_READ|
                          GWEN_SYNCIO_FILE_FLAGS_WRITE);
  if (rv<0) {
    fprintf(stderr, "Error opening archive (%d)\n", rv);
    return 2;
  }

  fprintf(stderr, "Adding file \"%s\"\n", fname);
  rv=GWEN_Sar_AddFile(sr, fname);
  if (rv<0) {
    fprintf(stderr, "Error adding file \"%s\" to archive \"%s\" (%d)\n",
            fname, aname, rv);
    return 2;
  }

  rv=GWEN_Sar_CloseArchive(sr, 0);
  if (rv<0) {
    fprintf(stderr, "Error closing archive (%d)\n", rv);
    return 2;
  }

  return 0;
}



int testStringListFromString(int argc, char **argv) {
  GWEN_STRINGLIST *sl;
#if 1
  const char delims[]="\n";
  const char td[]="test1\ntest2\ntest3";
#else
  const char delims[]=",";
  const char td[]="test1,test2,test3";
#endif
  sl=GWEN_StringList_fromString(td, delims, 1);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      const char *s;

      s=GWEN_StringListEntry_Data(se);
      fprintf(stderr, "Got this: [%s]\n", s?s:"(empty)");
      se=GWEN_StringListEntry_Next(se);
    }
  }
  return 0;
}



int testPasswordStore1(int argc, char **argv) {
  GWEN_PASSWD_STORE *sto;
  const char *pw1="Secret1";
  char pw[256];
  int rv;
  GWEN_GUI *gui;

  fprintf(stderr, "Creating gui.\n");
  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);

  sto=GWEN_PasswordStore_new("/tmp/pwstore.pw");
  rv=GWEN_PasswordStore_SetPassword(sto, "TestVar1", pw1);
  if (rv<0) {
    DBG_ERROR(0, "Error setting password: %d", rv);
    return 2;
  }
  rv=GWEN_PasswordStore_GetPassword(sto, "TestVar1", pw, 4, sizeof(pw)-1);
  if (rv<0) {
    DBG_ERROR(0, "Error getting password: %d", rv);
    return 2;
  }
  if (strcmp(pw1, pw)!=0) {
    DBG_ERROR(0, "Bad password, expected [%s], got [%s].", pw1, pw);
    return 2;
  }
  GWEN_PasswordStore_free(sto);

  return 0;
}



int testPasswordStore2(int argc, char **argv) {
  GWEN_PASSWD_STORE *sto;
  const char *pw1="Secret1";
  char pw[256];
  int rv;
  GWEN_GUI *gui;

  fprintf(stderr, "Creating gui.\n");
  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGui(gui);

  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Verbous);

  sto=GWEN_PasswordStore_new("/tmp/pwstore.pw");
  rv=GWEN_PasswordStore_GetPassword(sto, "TestVar1", pw, 4, sizeof(pw)-1);
  if (rv<0) {
    DBG_ERROR(0, "Error getting password: %d", rv);
    return 2;
  }
  if (strcmp(pw1, pw)!=0) {
    DBG_ERROR(0, "Bad password, expected [%s], got [%s].", pw1, pw);
    return 2;
  }
  GWEN_PasswordStore_free(sto);

  return 0;
}



int main(int argc, char **argv) {
  int rv;

  GWEN_Init();
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevel_Debug);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevel_Info);

  if (argc<2) {
    fprintf(stderr, "Usage: %s <test>\n  where <test> is one of db, dbfile, dbfile2, list, key, mkkey, cpkey, xml, xml2, sn, ssl, accept, connect\n", argv[0]);
    GWEN_Fini();
    return 1;
  }


  if (strcasecmp(argv[1], "dbfile")==0)
    rv=testDBfile(argc, argv);
  else if (strcasecmp(argv[1], "des")==0)
    rv=testDES(argc, argv);
  else if (strcasecmp(argv[1], "des2")==0)
    rv=testDES2(argc, argv);
  else if (strcasecmp(argv[1], "des3")==0)
    rv=testDES3(argc, argv);
  else if (strcasecmp(argv[1], "des4")==0)
    rv=testDES4(argc, argv);
  else if (strcasecmp(argv[1], "db")==0)
    rv=testDB(argc, argv);
  else if (strcasecmp(argv[1], "db2")==0)
    rv=testDB2(argc, argv);
  else if (strcasecmp(argv[1], "dbfile2")==0)
    rv=testDBfile2(argc, argv);
  else if (strcasecmp(argv[1], "dbfile3")==0)
    rv=testDBfile3(argc, argv);
  else if (strcasecmp(argv[1], "dbfile4")==0)
    rv=testDBfile4(argc, argv);
  else if (strcasecmp(argv[1], "msg")==0)
    rv=testMsg(argc, argv);
  else if (strcasecmp(argv[1], "list")==0)
    rv=testListMsg(argc, argv);
  else if (strcasecmp(argv[1], "xml")==0)
    rv=testXML(argc, argv);
  else if (strcasecmp(argv[1], "xml2")==0)
    rv=testXML2(argc, argv);
  else if (strcasecmp(argv[1], "xml3")==0)
    rv=testXML3(argc, argv);
  else if (strcasecmp(argv[1], "xml4")==0)
    rv=testXML4(argc, argv);
  else if (strcasecmp(argv[1], "xml5")==0)
    rv=testXML5(argc, argv);
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
  else if (strcasecmp(argv[1], "time1")==0)
    rv=testTimeToString(argc, argv);
  else if (strcasecmp(argv[1], "olddb")==0)
    rv=testOldDbImport(argc, argv);
  else if (strcasecmp(argv[1], "822")==0)
    rv=testRfc822Import(argc, argv);
  else if (strcasecmp(argv[1], "822x")==0)
    rv=testRfc822Export(argc, argv);
  else if (strcasecmp(argv[1], "xmldb1")==0)
    rv=testXmlDbExport(argc, argv);
  else if (strcasecmp(argv[1], "xmldb2")==0)
    rv=testXmlDbImport(argc, argv);
  else if (strcasecmp(argv[1], "fslock")==0)
    rv=testFsLock(argc, argv);
  else if (strcasecmp(argv[1], "fslock2")==0)
    rv=testFsLock2(argc, argv);
  else if (strcasecmp(argv[1], "ptr")==0)
    rv=testPtr(argc, argv);
  else if (strcasecmp(argv[1], "sl2")==0)
    rv=testStringList2(argc, argv);
  else if (strcasecmp(argv[1], "fuzzy")==0)
    rv=testFuzzy(argc, argv);
  else if (strcasecmp(argv[1], "sort")==0)
    rv=testSort(argc, argv);
  else if (strcasecmp(argv[1], "buf2")==0)
    rv=testBuffer2(argc, argv);
  else if (strcasecmp(argv[1], "mem")==0)
    rv=testMem(argc, argv);
  else if (strcasecmp(argv[1], "floatdouble")==0)
    rv=testFloatDouble(argc, argv);
  else if (strcasecmp(argv[1], "map")==0)
    rv=testMap(argc, argv);
  else if (strcasecmp(argv[1], "map2")==0)
    rv=testMap2(argc, argv);
  else if (strcasecmp(argv[1], "map3")==0)
    rv=testMap3(argc, argv);
  else if (strcasecmp(argv[1], "map4")==0)
    rv=testMap4(argc, argv);
  else if (strcasecmp(argv[1], "idlist")==0)
    rv=testIdList(argc, argv);
  else if (strcasecmp(argv[1], "signals1")==0)
    rv=testSignals1(argc, argv);
  else if (strcasecmp(argv[1], "signals2")==0)
    rv=testSignals2(argc, argv);
  else if (strcasecmp(argv[1], "signals3")==0)
    rv=testSignals3(argc, argv);
  else if (strcasecmp(argv[1], "url")==0)
    rv=testUrl(argc, argv);
  else if (strcasecmp(argv[1], "newxml")==0)
    rv=testNewXML(argc, argv);
  else if (strcasecmp(argv[1], "3rsa")==0)
    rv=testCrypt3Rsa(argc, argv);
  else if (strcasecmp(argv[1], "3rsa2")==0)
    rv=testCrypt3Rsa2(argc, argv);
  else if (strcasecmp(argv[1], "3rsa3")==0)
    rv=testCrypt3Rsa3(argc, argv);
  else if (strcasecmp(argv[1], "3rsa4")==0)
    rv=testCrypt3Rsa4(argc, argv);
  else if (strcasecmp(argv[1], "gtls")==0)
    rv=testGnutls(argc, argv);
  else if (strcasecmp(argv[1], "httpsession")==0)
    rv=testHttpSession(argc, argv);
  else if (strcasecmp(argv[1], "rsa")==0) {
    check_rsa_keys();
    rv=0;
  }
  else if (strcasecmp(argv[1], "cryptmgr1")==0) {
    rv=testCryptMgr1(argc, argv);
  }
  else if (strcasecmp(argv[1], "cryptmgr2")==0) {
    rv=testCryptMgr2(argc, argv);
  }
  else if (strcasecmp(argv[1], "cryptmgr3")==0) {
    rv=testCryptMgr3(argc, argv);
  }
  else if (strcasecmp(argv[1], "pss1")==0) {
    rv=testPss1(argc, argv);
  }
  else if (strcasecmp(argv[1], "pss2")==0) {
    rv=testPss2(argc, argv);
  }
  else if (strcasecmp(argv[1], "dlg")==0) {
    rv=testDialog(argc, argv);
  }
  else if (strcasecmp(argv[1], "sio1")==0) {
    rv=testSyncIo1(argc, argv);
  }
  else if (strcasecmp(argv[1], "sio2")==0) {
    rv=testSyncIo2(argc, argv);
  }
  else if (strcasecmp(argv[1], "sio3")==0) {
    rv=testSyncIo3(argc, argv);
  }
  else if (strcasecmp(argv[1], "http1")==0) {
    rv=testHttp1(argc, argv);
  }
  else if (strcasecmp(argv[1], "http2")==0) {
    rv=testHttp2(argc, argv);
  }
  else if (strcasecmp(argv[1], "tresor1")==0) {
    rv=testTresor1(argc, argv);
  }
  else if (strcasecmp(argv[1], "tresor2")==0) {
    rv=testTresor2(argc, argv);
  }
  else if (strcasecmp(argv[1], "hashtree")==0) {
    rv=testHashTree(argc, argv);
  }
  else if (strcasecmp(argv[1], "date1")==0) {
    rv=testDate1(argc, argv);
  }
  else if (strcasecmp(argv[1], "date2")==0) {
    rv=testDate2(argc, argv);
  }
  else if (strcasecmp(argv[1], "sar1")==0) {
    rv=testSar1(argc, argv);
  }
  else if (strcasecmp(argv[1], "sar2")==0) {
    rv=testSar2(argc, argv);
  }
  else if (strcasecmp(argv[1], "sar3")==0) {
    rv=testSar3(argc, argv);
  }
  else if (strcasecmp(argv[1], "sar4")==0) {
    rv=testSar4(argc, argv);
  }
  else if (strcasecmp(argv[1], "sl")==0) {
    rv=testStringListFromString(argc, argv);
  }
  else if (strcasecmp(argv[1], "pw1")==0) {
    rv=testPasswordStore1(argc, argv);
  }
  else if (strcasecmp(argv[1], "pw2")==0) {
    rv=testPasswordStore2(argc, argv);
  }
  else {
    fprintf(stderr, "Unknown command \"%s\"\n", argv[1]);
    GWEN_Fini();
    return 1;
  }

  GWEN_Fini();
  return rv;
}

