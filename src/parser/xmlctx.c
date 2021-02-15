/***************************************************************************
 begin       : Sat Jun 28 2003
 copyright   : (C) 2021 by Martin Preuss
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


#include "xmlctx_p.h"
#include "gwenhywfar/debug.h"
#include "gwenhywfar/misc.h"
#include "gwenhywfar/text.h"
#include "gwenhywfar/path.h"
#include "i18n_l.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>



GWEN_INHERIT_FUNCTIONS(GWEN_XML_CONTEXT)



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static GWEN_XMLNODE *_findTagForClosingTagAndcloseAndMoveSubTags(GWEN_XML_CONTEXT *ctx, GWEN_XMLNODE *currNode, const char *tagName);
static void _moveSubTagsUpOneLevel(GWEN_XMLNODE *currNode);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



GWEN_XML_CONTEXT *GWEN_XmlCtx_new(uint32_t flags)
{
  GWEN_XML_CONTEXT *ctx;

  GWEN_NEW_OBJECT(GWEN_XML_CONTEXT, ctx);
  ctx->_refCount=1;
  GWEN_INHERIT_INIT(GWEN_XML_CONTEXT, ctx);

  ctx->flags=flags;

  return ctx;
}



void GWEN_XmlCtx_free(GWEN_XML_CONTEXT *ctx)
{
  if (ctx) {
    assert(ctx->_refCount);
    if (ctx->_refCount==1) {
      GWEN_INHERIT_FINI(GWEN_XML_CONTEXT, ctx);
      if (ctx->encoding)
        free(ctx->encoding);
      ctx->_refCount=0;
      GWEN_FREE_OBJECT(ctx);
    }
    else
      ctx->_refCount--;
  }
}



void GWEN_XmlCtx_Attach(GWEN_XML_CONTEXT *ctx)
{
  assert(ctx);
  assert(ctx->_refCount);
  ctx->_refCount++;
}



uint32_t GWEN_XmlCtx_GetFlags(const GWEN_XML_CONTEXT *ctx)
{
  assert(ctx);
  return ctx->flags;
}



void GWEN_XmlCtx_SetFlags(GWEN_XML_CONTEXT *ctx, uint32_t f)
{
  assert(ctx);
  ctx->flags=f;
}



const char *GWEN_XmlCtx_GetEncoding(const GWEN_XML_CONTEXT *ctx)
{
  assert(ctx);
  return ctx->encoding;
}



void GWEN_XmlCtx_SetEncoding(GWEN_XML_CONTEXT *ctx, const char *encoding)
{
  char *s;

  assert(ctx);
  if (encoding) {
    s=strdup(encoding);
    assert(s);
  }
  else
    s=NULL;
  if (ctx->encoding)
    free(ctx->encoding);
  ctx->encoding=s;
}



int GWEN_XmlCtx_GetDepth(const GWEN_XML_CONTEXT *ctx)
{
  assert(ctx);
  return ctx->depth;
}



void GWEN_XmlCtx_SetDepth(GWEN_XML_CONTEXT *ctx, int i)
{
  assert(ctx);
  ctx->depth=i;
}



void GWEN_XmlCtx_IncDepth(GWEN_XML_CONTEXT *ctx)
{
  assert(ctx);
  ctx->depth++;
}



int GWEN_XmlCtx_DecDepth(GWEN_XML_CONTEXT *ctx)
{
  assert(ctx);
  if (ctx->depth<1)
    return -1;
  ctx->depth--;
  return 0;
}



uint32_t GWEN_XmlCtx_GetFinishedElement(const GWEN_XML_CONTEXT *ctx)
{
  assert(ctx);
  return ctx->finishedElements;
}



void GWEN_XmlCtx_IncFinishedElement(GWEN_XML_CONTEXT *ctx)
{
  assert(ctx);
  ctx->finishedElements++;
}



void GWEN_XmlCtx_ResetFinishedElement(GWEN_XML_CONTEXT *ctx)
{
  assert(ctx);
  ctx->finishedElements=0;
}



void GWEN_XmlCtx_SetCurrentNode(GWEN_XML_CONTEXT *ctx, GWEN_XMLNODE *n)
{
  assert(ctx);
  ctx->currentNode=n;
}



GWEN_XMLNODE *GWEN_XmlCtx_GetCurrentNode(const GWEN_XML_CONTEXT *ctx)
{
  assert(ctx);
  return ctx->currentNode;
}



void GWEN_XmlCtx_SetCurrentHeader(GWEN_XML_CONTEXT *ctx, GWEN_XMLNODE *n)
{
  assert(ctx);
  ctx->currentHeader=n;
}



GWEN_XMLNODE *GWEN_XmlCtx_GetCurrentHeader(const GWEN_XML_CONTEXT *ctx)
{
  assert(ctx);
  return ctx->currentHeader;
}



GWEN_XMLCTX_STARTTAG_FN GWEN_XmlCtx_SetStartTagFn(GWEN_XML_CONTEXT *ctx,
                                                  GWEN_XMLCTX_STARTTAG_FN f)
{
  GWEN_XMLCTX_STARTTAG_FN of;

  assert(ctx);
  of=ctx->startTagFn;
  ctx->startTagFn=f;
  return of;
}



GWEN_XMLCTX_ENDTAG_FN GWEN_XmlCtx_SetEndTagFn(GWEN_XML_CONTEXT *ctx,
                                              GWEN_XMLCTX_ENDTAG_FN f)
{
  GWEN_XMLCTX_ENDTAG_FN of;

  assert(ctx);
  of=ctx->endTagFn;
  ctx->endTagFn=f;
  return of;
}



GWEN_XMLCTX_ADDDATA_FN GWEN_XmlCtx_SetAddDataFn(GWEN_XML_CONTEXT *ctx,
                                                GWEN_XMLCTX_ADDDATA_FN f)
{
  GWEN_XMLCTX_ADDDATA_FN of;

  assert(ctx);
  of=ctx->addDataFn;
  ctx->addDataFn=f;
  return of;
}



GWEN_XMLCTX_ADDATTR_FN GWEN_XmlCtx_SetAddAttrFn(GWEN_XML_CONTEXT *ctx,
                                                GWEN_XMLCTX_ADDATTR_FN f)
{
  GWEN_XMLCTX_ADDATTR_FN of;

  assert(ctx);
  of=ctx->addAttrFn;
  ctx->addAttrFn=f;
  return of;
}



GWEN_XMLCTX_ADDCOMMENT_FN GWEN_XmlCtx_SetAddCommentFn(GWEN_XML_CONTEXT *ctx,
                                                      GWEN_XMLCTX_ADDCOMMENT_FN f)
{
  GWEN_XMLCTX_ADDCOMMENT_FN of;

  assert(ctx);
  of=ctx->addCommentFn;
  ctx->addCommentFn=f;
  return of;
}




int GWEN_XmlCtx_StartTag(GWEN_XML_CONTEXT *ctx, const char *tagName)
{
  assert(ctx);

  if (ctx->startTagFn)
    return ctx->startTagFn(ctx, tagName);
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Starting tag: [%s]", tagName);
    return 0;
  }
}



int GWEN_XmlCtx_EndTag(GWEN_XML_CONTEXT *ctx, int closing)
{
  assert(ctx);

  if (ctx->endTagFn)
    return ctx->endTagFn(ctx, closing);
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Ending tag (%s)", closing?"closing":"not closing");
    return 0;
  }
}



int GWEN_XmlCtx_AddData(GWEN_XML_CONTEXT *ctx, const char *data)
{
  assert(ctx);

  if (ctx->addDataFn)
    return ctx->addDataFn(ctx, data);
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Adding data: [%s]", data);
    return 0;
  }
}



int GWEN_XmlCtx_AddComment(GWEN_XML_CONTEXT *ctx, const char *data)
{
  assert(ctx);

  if (ctx->addCommentFn)
    return ctx->addCommentFn(ctx, data);
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Adding comment: [%s]", data);
    return 0;
  }
}



int GWEN_XmlCtx_AddAttr(GWEN_XML_CONTEXT *ctx,
                        const char *attrName,
                        const char *attrData)
{
  assert(ctx);

  if (ctx->addAttrFn)
    return ctx->addAttrFn(ctx, attrName, attrData);
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Adding attribute: [%s]=[%s]",
             attrName, attrData);
    return 0;
  }
}








GWEN_XML_CONTEXT *GWEN_XmlCtxStore_new(GWEN_XMLNODE *n, uint32_t flags)
{
  GWEN_XML_CONTEXT *ctx;

  ctx=GWEN_XmlCtx_new(flags);
  assert(ctx);

  GWEN_XmlCtx_SetCurrentNode(ctx, n);

  GWEN_XmlCtx_SetStartTagFn(ctx, GWEN_XmlCtxStore_StartTag);
  GWEN_XmlCtx_SetEndTagFn(ctx, GWEN_XmlCtxStore_EndTag);
  GWEN_XmlCtx_SetAddDataFn(ctx, GWEN_XmlCtxStore_AddData);
  GWEN_XmlCtx_SetAddCommentFn(ctx, GWEN_XmlCtxStore_AddComment);
  GWEN_XmlCtx_SetAddAttrFn(ctx, GWEN_XmlCtxStore_AddAttr);

  return ctx;
}



int GWEN_XmlCtxStore_StartTag(GWEN_XML_CONTEXT *ctx, const char *tagName)
{
  GWEN_XMLNODE *currNode;
  GWEN_XMLNODE *newNode;

  currNode=GWEN_XmlCtx_GetCurrentNode(ctx);
  if (currNode==NULL)
    return GWEN_ERROR_INVALID;

  if (*tagName=='?' && (GWEN_XmlCtx_GetFlags(ctx) & GWEN_XML_FLAGS_HANDLE_HEADERS)) {
    newNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, tagName);
    assert(newNode);
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Adding header [%s] to [%s]",
                GWEN_XMLNode_GetData(newNode),
                GWEN_XMLNode_GetData(currNode));
    GWEN_XMLNode_AddHeader(currNode, newNode);
    GWEN_XmlCtx_SetCurrentHeader(ctx, newNode);
  }
  else if (strcasecmp(tagName, "!DOCTYPE")==0) {
    newNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, tagName);
    assert(newNode);
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Adding header [%s] to [%s]",
                GWEN_XMLNode_GetData(newNode),
                GWEN_XMLNode_GetData(currNode));
    GWEN_XMLNode_AddHeader(currNode, newNode);
    GWEN_XmlCtx_SetCurrentHeader(ctx, newNode);
  }
  else if (*tagName=='/') {
    const char *s;

    tagName++;
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Finishing tag [%s]", tagName);
    s=GWEN_XMLNode_GetData(currNode);
    if (s==NULL) {
      DBG_INFO(GWEN_LOGDOMAIN, "Current node tag has no name");
      return GWEN_ERROR_BAD_DATA;
    }

    if (strcasecmp(s, tagName)!=0) {
      /* current closing tag doesn't match currently open tag, check further */
      if (!(GWEN_XmlCtx_GetFlags(ctx) & GWEN_XML_FLAGS_TOLERANT_ENDTAGS)) {
        DBG_INFO(GWEN_LOGDOMAIN, "Endtag does not match curent tag (%s != %s)", s, tagName);
        return GWEN_ERROR_BAD_DATA;
      }
      else {
        newNode=_findTagForClosingTagAndcloseAndMoveSubTags(ctx, currNode, tagName);
        if (newNode==NULL) {
          DBG_INFO(GWEN_LOGDOMAIN, "here");
          return GWEN_ERROR_BAD_DATA;
        }
      }
    }
    else {
      newNode=GWEN_XMLNode_GetParent(currNode);
      if (newNode==NULL) {
        DBG_INFO(GWEN_LOGDOMAIN, "No parent node at [%s]", tagName);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_XmlCtx_SetCurrentNode(ctx, newNode);
      GWEN_XmlCtx_DecDepth(ctx);
    }
    /* one more element finished */
    GWEN_XmlCtx_IncFinishedElement(ctx);
  }
  else {
    newNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, tagName);
    assert(newNode);
    GWEN_XMLNode_AddChild(currNode, newNode);
    GWEN_XmlCtx_SetCurrentNode(ctx, newNode);
    GWEN_XmlCtx_IncDepth(ctx);
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Starting tag [%s]", tagName);
  }

  return 0;
}



GWEN_XMLNODE *_findTagForClosingTagAndcloseAndMoveSubTags(GWEN_XML_CONTEXT *ctx, GWEN_XMLNODE *currNode, const char *tagName)
{
  GWEN_XMLNODE *newNode;

  newNode=currNode;

  if (GWEN_XmlCtx_GetFlags(ctx) & GWEN_XML_FLAGS_SGML)
    _moveSubTagsUpOneLevel(currNode);

  while ((newNode=GWEN_XMLNode_GetParent(newNode))) {
    const char *s;

    GWEN_XmlCtx_DecDepth(ctx);
    s=GWEN_XMLNode_GetData(newNode);
    if (strcasecmp(s, tagName)==0)
      break;
    if (GWEN_XmlCtx_GetFlags(ctx) & GWEN_XML_FLAGS_SGML)
      _moveSubTagsUpOneLevel(newNode);
  }
  if (newNode)
    newNode=GWEN_XMLNode_GetParent(newNode);
  if (newNode) {
    GWEN_XmlCtx_SetCurrentNode(ctx, newNode);
    GWEN_XmlCtx_DecDepth(ctx);
    return newNode;
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No matching parent node for [%s]", tagName);
    return NULL;
  }
}



void _moveSubTagsUpOneLevel(GWEN_XMLNODE *currNode)
{
  GWEN_XMLNODE *childNode;
  GWEN_XMLNODE *parentNode;

  parentNode=GWEN_XMLNode_GetParent(currNode);
  if (parentNode) {
    while( (childNode=GWEN_XMLNode_GetFirstTag(currNode)) ) {
      GWEN_XMLNode_UnlinkChild(currNode, childNode);
      GWEN_XMLNode_AddChild(parentNode, childNode);
    }
  }
}



int GWEN_XmlCtxStore_EndTag(GWEN_XML_CONTEXT *ctx, int closing)
{
  GWEN_XMLNODE *currNode;

  currNode=GWEN_XmlCtx_GetCurrentHeader(ctx);
  if (currNode) {
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Ending header [%s]", GWEN_XMLNode_GetData(currNode));
    GWEN_XmlCtx_SetCurrentHeader(ctx, NULL);
  }
  else {
    currNode=GWEN_XmlCtx_GetCurrentNode(ctx);
    if (currNode==NULL)
      return GWEN_ERROR_INVALID;
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Ending tag [%s] (%s)",
                GWEN_XMLNode_GetData(currNode),
                closing?"closing":"not closing");

    if (closing) {
      GWEN_XMLNODE *newNode;

      newNode=GWEN_XMLNode_GetParent(currNode);
      if (newNode==NULL) {
        DBG_INFO(GWEN_LOGDOMAIN, "No parent node at [%s]", GWEN_XMLNode_GetData(currNode));
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_XmlCtx_SetCurrentNode(ctx, newNode);
      /* one more element finished */
      GWEN_XmlCtx_DecDepth(ctx);
      GWEN_XmlCtx_IncFinishedElement(ctx);
    }
  }

  return 0;
}



int GWEN_XmlCtxStore_AddData(GWEN_XML_CONTEXT *ctx, const char *data)
{
  GWEN_XMLNODE *currNode;
  GWEN_BUFFER *buf;
  uint32_t flags;

  flags=GWEN_XmlCtx_GetFlags(ctx);
  currNode=GWEN_XmlCtx_GetCurrentNode(ctx);
  if (currNode==NULL)
    return GWEN_ERROR_INVALID;

  buf=GWEN_Buffer_new(0, 64, 0, 1);
  if (GWEN_Text_UnescapeXmlToBuffer(data, buf)) {
    GWEN_Buffer_free(buf);
    DBG_INFO(GWEN_LOGDOMAIN, "here");
    return GWEN_ERROR_BAD_DATA;
  }

  if (!(flags & GWEN_XML_FLAGS_NO_CONDENSE) ||
      (flags & GWEN_XML_FLAGS_KEEP_CNTRL) ||
      (flags & GWEN_XML_FLAGS_KEEP_BLANKS)) {
    const uint8_t *p;
    uint8_t *dst;
    uint8_t *src;
    unsigned int size;
    unsigned int i;
    int lastWasBlank;
    uint8_t *lastBlankPos;
    uint32_t bStart=0;

    dst=(uint8_t *)GWEN_Buffer_GetStart(buf);
    src=dst;
    if (!(flags & GWEN_XML_FLAGS_KEEP_BLANKS)) {
      if (flags & GWEN_XML_FLAGS_KEEP_CNTRL) {
        while (*src && (*src==32 || *src==9))
          src++;
      }
      else {
        while (*src && *src<33)
          src++;
      }
    }

    p=src;
    bStart=src-((uint8_t *)GWEN_Buffer_GetStart(buf));
    size=GWEN_Buffer_GetUsedBytes(buf)-bStart;
    lastWasBlank=0;
    lastBlankPos=0;

    for (i=0; i<size; i++) {
      uint8_t c;

      c=*p;
      if (!(flags & GWEN_XML_FLAGS_KEEP_CNTRL) && c<32)
        c=32;

      /* remember next loop whether this char was a blank */
      if (!(flags & GWEN_XML_FLAGS_NO_CONDENSE) && c==32) {
        if (!lastWasBlank) {
          /* store only one blank */
          lastWasBlank=1;
          lastBlankPos=dst;
          *(dst++)=c;
        }
      }
      else {
        lastWasBlank=0;
        lastBlankPos=0;
        *(dst++)=c;
      }
      p++;
    }

    /* remove trailing blanks */
    if (lastBlankPos!=0)
      dst=lastBlankPos;

    size=dst-(uint8_t *)GWEN_Buffer_GetStart(buf);
    GWEN_Buffer_Crop(buf, 0, size);
  }

  if (GWEN_Buffer_GetUsedBytes(buf)) {
    GWEN_XMLNODE *newNode;

    newNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeData, GWEN_Buffer_GetStart(buf));
    assert(newNode);
    GWEN_XMLNode_AddChild(currNode, newNode);
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Setting this data: [%s]", GWEN_Buffer_GetStart(buf));
  }
  GWEN_Buffer_free(buf);

  return 0;
}



int GWEN_XmlCtxStore_AddComment(GWEN_UNUSED GWEN_XML_CONTEXT *ctx, GWEN_UNUSED const char *data)
{
  return 0;
}



int GWEN_XmlCtxStore_AddAttr(GWEN_XML_CONTEXT *ctx,
                             const char *attrName,
                             const char *attrData)
{
  GWEN_XMLNODE *currNode;

  currNode=GWEN_XmlCtx_GetCurrentHeader(ctx);
  if (currNode) {
    if ((strcmp(GWEN_XMLNode_GetData(currNode), "?xml")==0)
        && (strcmp(attrName, "encoding")==0)) {
      if (strcasecmp(attrData, "UTF-8")==0)
        GWEN_XmlCtx_SetEncoding(ctx, NULL);
      else
        GWEN_XmlCtx_SetEncoding(ctx, attrData);
    }
    DBG_VERBOUS(GWEN_LOGDOMAIN, "Setting attribute of header [%s]: [%s]=[%s]",
                GWEN_XMLNode_GetData(currNode), attrName, attrData);
    GWEN_XMLNode_SetProperty(currNode, attrName, attrData);
  }
  else {
    int isNormalProperty=1;

    currNode=GWEN_XmlCtx_GetCurrentNode(ctx);
    if (currNode==NULL)
      return GWEN_ERROR_INVALID;
    if (attrData==NULL)
      attrData="";

    if (ctx->flags & GWEN_XML_FLAGS_HANDLE_NAMESPACES) {
      if (strcasecmp(attrName, "xmlns")==0) {
        GWEN_XMLNODE_NAMESPACE *ns;

        DBG_VERBOUS(GWEN_LOGDOMAIN, "Adding namespace [%s] to node [%s]",
                    attrData, GWEN_XMLNode_GetData(currNode));
        ns=GWEN_XMLNode_NameSpace_new("", attrData);
        GWEN_XMLNode_AddNameSpace(currNode, ns);
        GWEN_XMLNode_NameSpace_free(ns);
        isNormalProperty=0;
      }
      else if (strncasecmp(attrName, "xmlns:", 6)==0) {
        const char *name;

        name=strchr(attrName, ':');
        if (name) {
          name++;
          if (*name) {
            GWEN_XMLNODE_NAMESPACE *ns;

            DBG_VERBOUS(GWEN_LOGDOMAIN, "Adding namespace [%s]=[%s]",
                        name, attrData);
            ns=GWEN_XMLNode_NameSpace_new(name, attrData);
            GWEN_XMLNode_AddNameSpace(currNode, ns);
            GWEN_XMLNode_NameSpace_free(ns);
            isNormalProperty=0;
          }
        }
      }
    }

    if (isNormalProperty) {
      GWEN_BUFFER *buf;

      DBG_VERBOUS(GWEN_LOGDOMAIN, "Setting attribute of tag [%s]: [%s]=[%s]",
                  GWEN_XMLNode_GetData(currNode), attrName, attrData);
      buf=GWEN_Buffer_new(0, 64, 0, 1);
      if (GWEN_Text_UnescapeXmlToBuffer(attrData, buf)) {
        GWEN_Buffer_free(buf);
        DBG_INFO(GWEN_LOGDOMAIN, "here");
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_XMLNode_SetProperty(currNode, attrName, GWEN_Buffer_GetStart(buf));
      GWEN_Buffer_free(buf);
    }
  }

  return 0;
}






