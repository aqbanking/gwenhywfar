/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id: xmldb.c 1020 2006-03-26 17:16:05Z aquamaniac $
 begin       : Thu Oct 30 2003
 copyright   : (C) 2003 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "xmldb_p.h"
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/dbio_be.h>
#include <gwenhywfar/syncio_file.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>






int GWEN_DBIO__XmlDb_ImportGroup(GWEN_DBIO *dbio,
                                 uint32_t flags,
                                 GWEN_DB_NODE *data,
                                 GWEN_DB_NODE *cfg,
                                 GWEN_XMLNODE *node)
{
  GWEN_XMLNODE *n;

  assert(node);
  n=GWEN_XMLNode_GetFirstTag(node);
  while (n) {
    const char *tname;
    const char *s;

    tname=GWEN_XMLNode_GetData(n);
    assert(tname && *tname);
    s=GWEN_XMLNode_GetProperty(n, "type", 0);
    if (s) {
      if (strcasecmp(s, "group")==0) {
        GWEN_DB_NODE *db;
        int rv;

        db=GWEN_DB_GetGroup(data, flags, tname);
        if (db==0) {
          DBG_INFO(GWEN_LOGDOMAIN, "here");
          return GWEN_ERROR_INVALID;
        }
        rv=GWEN_DBIO__XmlDb_ImportGroup(dbio, flags, db, cfg, n);
        if (rv) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          return rv;
        }
      }
      else if (strcasecmp(s, "var")==0) {
        int rv;

        rv=GWEN_DBIO__XmlDb_ImportVar(dbio, flags, data, cfg, n);
        if (rv) {
          DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
          return rv;
        }
      }
      else {
        DBG_ERROR(GWEN_LOGDOMAIN, "Unexpected type \"%s\"", s);
        return GWEN_ERROR_INVALID;
      }
    }

    n=GWEN_XMLNode_GetNextTag(n);
  }

  return 0;
}



void GWEN_DBIO__XmlDb_ReadDataTags(GWEN_XMLNODE *node, GWEN_BUFFER *buf)
{
  GWEN_XMLNODE *ndata;
  GWEN_BUFFER *tbuf;
  int rv;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  ndata=GWEN_XMLNode_GetFirstData(node);
  while (ndata) {
    const char *s;

    s=GWEN_XMLNode_GetData(ndata);
    if (s) {
      if (GWEN_Buffer_GetUsedBytes(tbuf))
        GWEN_Buffer_AppendByte(tbuf, ' ');
      GWEN_Buffer_AppendString(tbuf, s);
    }
    ndata=GWEN_XMLNode_GetNextData(node);
  }
  rv=GWEN_Text_UnescapeXmlToBuffer(GWEN_Buffer_GetStart(tbuf), buf);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
  }
  GWEN_Buffer_free(tbuf);
}



int GWEN_DBIO__XmlDb_ImportVar(GWEN_DBIO *dbio,
                               uint32_t flags,
                               GWEN_DB_NODE *data,
                               GWEN_DB_NODE *cfg,
                               GWEN_XMLNODE *node)
{
  GWEN_XMLNODE *n;
  const char *vname;
  GWEN_BUFFER *tbuf;

  assert(node);
  vname=GWEN_XMLNode_GetData(node);
  assert(vname && *vname);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  n=GWEN_XMLNode_FindFirstTag(node, "value", 0, 0);
  while (n) {
    const char *s;
    const char *d;
    int rv;

    GWEN_DBIO__XmlDb_ReadDataTags(n, tbuf);
    d=GWEN_Buffer_GetStart(tbuf);
    if (*d) {
      s=GWEN_XMLNode_GetData(n);
      assert(s && *s);
      s=GWEN_XMLNode_GetProperty(n, "type", 0);
      if (s) {
        if (strcasecmp(s, "char")==0) {
          rv=GWEN_DB_SetCharValue(data, flags, vname, d);
          if (rv) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            GWEN_Buffer_free(tbuf);
            return rv;
          }
        }
        else if (strcasecmp(s, "int")==0) {
          int val;

          if (1!=sscanf(d, "%i", &val)) {
            DBG_INFO(GWEN_LOGDOMAIN, "Non-integer value [%s]", d);
            GWEN_Buffer_free(tbuf);
            return GWEN_ERROR_INVALID;
          }
          rv=GWEN_DB_SetIntValue(data, flags, vname, val);
          if (rv) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            GWEN_Buffer_free(tbuf);
            return rv;
          }
        }
        else if (strcasecmp(s, "bin")==0) {
          GWEN_BUFFER *xbuf;

          xbuf=GWEN_Buffer_new(0, 256, 0, 1);
          rv=GWEN_Text_FromHexBuffer(d, xbuf);
          if (rv) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            GWEN_Buffer_free(xbuf);
            GWEN_Buffer_free(tbuf);
            return rv;
          }

          rv=GWEN_DB_SetBinValue(data, flags, vname,
                                 GWEN_Buffer_GetStart(xbuf),
                                 GWEN_Buffer_GetUsedBytes(xbuf));
          GWEN_Buffer_free(xbuf);
          if (rv) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            GWEN_Buffer_free(tbuf);
            return rv;
          }
        }
        else {
          DBG_ERROR(GWEN_LOGDOMAIN,
                    "Unhandled value type \"%s\"", s);
          GWEN_Buffer_free(tbuf);
          return GWEN_ERROR_INVALID;
        }
      }
    }
    else {
      DBG_DEBUG(GWEN_LOGDOMAIN, "Empty value");
    }
    GWEN_Buffer_Reset(tbuf);

    n=GWEN_XMLNode_FindNextTag(n, "value", 0, 0);
  }
  GWEN_Buffer_free(tbuf);

  return 0;
}




int GWEN_DBIO_XmlDb_Import(GWEN_DBIO *dbio,
                           GWEN_SYNCIO *sio,
                           GWEN_DB_NODE *data,
                           GWEN_DB_NODE *cfg,
                           uint32_t flags)
{
  int rv;
  GWEN_XMLNODE *root;
  GWEN_XMLNODE *n;
  const char *rootName=0;
  GWEN_XML_CONTEXT *ctx;

  assert(data);

  if (cfg)
    rootName=GWEN_DB_GetCharValue(cfg, "rootElement", 0, 0);

  root=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  ctx=GWEN_XmlCtxStore_new(root,
                           GWEN_XML_FLAGS_DEFAULT |
                           GWEN_XML_FLAGS_HANDLE_HEADERS);
  rv=GWEN_XMLContext_ReadFromIo(ctx, sio);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_XmlCtx_free(ctx);
    GWEN_XMLNode_free(root);
    return rv;
  }
  GWEN_XmlCtx_free(ctx);

  if (rootName) {
    n=GWEN_XMLNode_FindFirstTag(root, rootName, 0, 0);
    if (!n) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "Root node in XML file is not \"%s\"", rootName);
      GWEN_XMLNode_free(root);
      return GWEN_ERROR_BAD_DATA;
    }
  }
  else {
    n=GWEN_XMLNode_GetFirstTag(root);
    if (!n) {
      DBG_ERROR(GWEN_LOGDOMAIN,
                "No root node in XML file");
      GWEN_XMLNode_free(root);
      return GWEN_ERROR_BAD_DATA;
    }
  }

  rv=GWEN_DBIO__XmlDb_ImportGroup(dbio, flags, data, cfg, n);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_XMLNode_free(root);
    return rv;
  }

  GWEN_XMLNode_free(root);
  return 0;
}



int GWEN_DBIO_XmlDb__ExportGroup(GWEN_DBIO *dbio,
                                 GWEN_DB_NODE *data,
                                 GWEN_XMLNODE *node,
                                 const char *newName)
{
  const char *s;
  GWEN_XMLNODE *n;
  GWEN_DB_NODE *dbT;
  int rv;

  if (newName)
    s=newName;
  else
    s=GWEN_DB_GroupName(data);
  assert(s && *s);

  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, s);
  GWEN_XMLNode_SetProperty(n, "type", "group");
  GWEN_XMLNode_AddChild(node, n);

  /* store variables */
  dbT=GWEN_DB_GetFirstVar(data);
  while (dbT) {
    if (!(GWEN_DB_GetNodeFlags(dbT) & GWEN_DB_NODE_FLAGS_VOLATILE)) {
      rv=GWEN_DBIO_XmlDb__ExportVar(dbio, dbT, n);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
    dbT=GWEN_DB_GetNextVar(dbT);
  }

  /* store groups */
  dbT=GWEN_DB_GetFirstGroup(data);
  while (dbT) {
    if (!(GWEN_DB_GetNodeFlags(dbT) & GWEN_DB_NODE_FLAGS_VOLATILE)) {
      rv=GWEN_DBIO_XmlDb__ExportGroup(dbio, dbT, n, 0);
      if (rv) {
        DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }
    dbT=GWEN_DB_GetNextGroup(dbT);
  }

  return 0;
}



int GWEN_DBIO_XmlDb__ExportVar(GWEN_DBIO *dbio,
                               GWEN_DB_NODE *data,
                               GWEN_XMLNODE *node)
{
  const char *s;
  GWEN_XMLNODE *n;
  GWEN_DB_NODE *dbT;
  int rv;

  s=GWEN_DB_VariableName(data);
  assert(s && *s);

  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, s);
  GWEN_XMLNode_SetProperty(n, "type", "var");
  GWEN_XMLNode_AddChild(node, n);

  /* store variables */
  dbT=GWEN_DB_GetFirstValue(data);
  while (dbT) {
    if (!(GWEN_DB_GetNodeFlags(dbT) & GWEN_DB_NODE_FLAGS_VOLATILE)) {
      GWEN_DB_NODE_TYPE vt;
      GWEN_XMLNODE *vn;

      vt=GWEN_DB_GetValueType(dbT);
      switch (vt) {

      case GWEN_DB_NodeType_ValueChar:
        s=GWEN_DB_GetCharValueFromNode(dbT);
        if (s && *s) {
          GWEN_XMLNODE *dn;
          GWEN_BUFFER *tbuf;

          vn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "value");
          GWEN_XMLNode_SetProperty(vn, "type", "char");
          GWEN_XMLNode_AddChild(n, vn);

          tbuf=GWEN_Buffer_new(0, 64, 0, 1);
          rv=GWEN_Text_EscapeXmlToBuffer(s, tbuf);
          if (rv<0) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            GWEN_Buffer_free(tbuf);
            return rv;
          }
          dn=GWEN_XMLNode_new(GWEN_XMLNodeTypeData, GWEN_Buffer_GetStart(tbuf));
          GWEN_Buffer_free(tbuf);
          GWEN_XMLNode_AddChild(vn, dn);
        }
        break;

      case GWEN_DB_NodeType_ValueInt: {
        char nbuf[32];
        GWEN_XMLNODE *dn;

        snprintf(nbuf, sizeof(nbuf), "%i", GWEN_DB_GetIntValueFromNode(dbT));

        vn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "value");
        GWEN_XMLNode_SetProperty(vn, "type", "int");
        GWEN_XMLNode_AddChild(n, vn);

        dn=GWEN_XMLNode_new(GWEN_XMLNodeTypeData, nbuf);
        GWEN_XMLNode_AddChild(vn, dn);

        break;
      }

      case GWEN_DB_NodeType_ValueBin: {
        const void *vp;
        unsigned int vsize;

        vp=GWEN_DB_GetBinValueFromNode(dbT, &vsize);
        if (vp && vsize) {
          GWEN_BUFFER *bbuf;
          GWEN_XMLNODE *dn;

          bbuf=GWEN_Buffer_new(0, vsize*2, 0, 1);
          rv=GWEN_Text_ToHexBuffer((const char *)vp,
                                   vsize,
                                   bbuf,
                                   0, 0, 0);
          if (rv) {
            DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
            GWEN_Buffer_free(bbuf);
            return rv;
          }

          vn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "value");
          GWEN_XMLNode_SetProperty(vn, "type", "bin");
          GWEN_XMLNode_AddChild(n, vn);

          dn=GWEN_XMLNode_new(GWEN_XMLNodeTypeData,
                              GWEN_Buffer_GetStart(bbuf));
          GWEN_Buffer_free(bbuf);
          GWEN_XMLNode_AddChild(vn, dn);
        }

        break;
      }

      case GWEN_DB_NodeType_ValuePtr:
        DBG_DEBUG(GWEN_LOGDOMAIN, "Not storing pointer value");
        break;

      default:
        DBG_ERROR(GWEN_LOGDOMAIN,
                  "Unsupported variable type %d",
                  vt);
        return GWEN_ERROR_INVALID;
      }
    } /* if non-volatile */

    dbT=GWEN_DB_GetNextValue(dbT);
  }

  return 0;
}



int GWEN_DBIO_XmlDb_Export(GWEN_DBIO *dbio,
                           GWEN_SYNCIO *sio,
                           GWEN_DB_NODE *data,
                           GWEN_DB_NODE *cfg,
                           uint32_t flags)
{
  GWEN_XMLNODE *root;
  GWEN_XMLNODE *nh;
  int rv;
  const char *rootName=0;
  GWEN_XML_CONTEXT *ctx;

  if (cfg)
    rootName=GWEN_DB_GetCharValue(cfg, "rootElement", 0, 0);

  root=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  nh=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "?xml");
  GWEN_XMLNode_SetProperty(nh, "version", "1.0");
  GWEN_XMLNode_SetProperty(nh, "encoding", "utf-8");
  GWEN_XMLNode_AddHeader(root, nh);

  rv=GWEN_DBIO_XmlDb__ExportGroup(dbio, data, root, rootName);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_XMLNode_free(root);
    return rv;
  }


  ctx=GWEN_XmlCtxStore_new(root,
                           GWEN_XML_FLAGS_DEFAULT |
                           GWEN_XML_FLAGS_SIMPLE |
                           GWEN_XML_FLAGS_HANDLE_HEADERS);

  rv=GWEN_XMLNode_WriteToStream(root, ctx, sio);
  if (rv) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_XmlCtx_free(ctx);
    GWEN_XMLNode_free(root);
    return rv;
  }
  GWEN_XmlCtx_free(ctx);
  GWEN_XMLNode_free(root);

  return 0;
}



GWEN_DBIO_CHECKFILE_RESULT GWEN_DBIO_XmlDb_CheckFile(GWEN_DBIO *dbio,
                                                     const char *fname)
{
  GWEN_SYNCIO *sio;
  int rv;
  uint8_t tbuf[256];

  assert(dbio);
  assert(fname);

  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return GWEN_DBIO_CheckFileResultNotOk;
  }


  rv=GWEN_SyncIo_Read(sio, tbuf, sizeof(tbuf)-1);
  if (rv<1) {
    DBG_INFO(GWEN_LOGDOMAIN,
             "File \"%s\" is not supported by this plugin",
             fname);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return GWEN_DBIO_CheckFileResultNotOk;
  }
  tbuf[rv-1]=0;
  if (-1!=GWEN_Text_ComparePattern((const char *) tbuf, "*<?xml*", 0)) {
    /* match */
    DBG_INFO(GWEN_LOGDOMAIN,
             "File \"%s\" is supported by this plugin",
             fname);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    /* don't be too sure about this, we *may* support the file,
     * so we dont say we don't support this file */
    return GWEN_DBIO_CheckFileResultUnknown;
  }
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);
  return GWEN_DBIO_CheckFileResultNotOk;
}



GWEN_DBIO *GWEN_DBIO_XmlDb_Factory(GWEN_PLUGIN *pl)
{
  GWEN_DBIO *dbio;

  dbio=GWEN_DBIO_new("XmlDb", "Imports and exports XML data");
  GWEN_DBIO_SetImportFn(dbio, GWEN_DBIO_XmlDb_Import);
  GWEN_DBIO_SetExportFn(dbio, GWEN_DBIO_XmlDb_Export);
  GWEN_DBIO_SetCheckFileFn(dbio, GWEN_DBIO_XmlDb_CheckFile);
  return dbio;
}



GWEN_PLUGIN *dbio_xmldb_factory(GWEN_PLUGIN_MANAGER *pm,
                                const char *modName,
                                const char *fileName)
{
  GWEN_PLUGIN *pl;

  pl=GWEN_DBIO_Plugin_new(pm, modName, fileName);
  assert(pl);

  GWEN_DBIO_Plugin_SetFactoryFn(pl, GWEN_DBIO_XmlDb_Factory);

  return pl;

}



