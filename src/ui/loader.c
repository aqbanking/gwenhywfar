/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Feb 09 2004
    copyright   : (C) 2004 by Martin Preuss
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


#include "loader_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/bio_buffer.h>

#include <gwenhywfar/ui/button.h>
#include <gwenhywfar/ui/window.h>
#include <gwenhywfar/ui/scrollwidget.h>
#include <gwenhywfar/ui/editbox.h>
#include <gwenhywfar/ui/checkbox.h>
#include <gwenhywfar/ui/dropdownbox.h>
#include <gwenhywfar/ui/textwidget.h>
#include <gwenhywfar/ui/tablewidget.h>



int GWEN_UILoader__ExtractText(GWEN_XMLNODE *n,
                               GWEN_DB_NODE *db,
                               const char *name) {
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;
  GWEN_BUFFER *buf;
  GWEN_XMLNODE *nn;

  nn=GWEN_XMLNode_FindFirstTag(n, name, 0, 0);
  if (!nn) {
    DBG_NOTICE(0, "XML node not found");
    return 0;
  }
  buf=GWEN_Buffer_new(0, 256, 0, 1);
  bio=GWEN_BufferedIO_Buffer2_new(buf, 0);
  GWEN_BufferedIO_SetWriteBuffer(bio, 0, 256);
  if (GWEN_XMLNode_WriteToStream(nn, bio, 0)) {
    DBG_ERROR(0, "Error writing text");
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    GWEN_Buffer_free(buf);
    return -1;
  }
  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO_ERR(0, err);
    GWEN_BufferedIO_free(bio);
    GWEN_Buffer_free(buf);
    return -1;
  }

  GWEN_BufferedIO_free(bio);
  DBG_NOTICE(0, "Setting text [%s]=\"%s\"", name, GWEN_Buffer_GetStart(buf));
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       name, GWEN_Buffer_GetStart(buf));
  GWEN_Buffer_free(buf);

  return 0;
}




int GWEN_UILoader__ParseWidget(GWEN_XMLNODE *n,
                               GWEN_DB_NODE *db,
                               int *x, int *y,
                               int width, int height) {
  GWEN_XMLNODE *nn;
  int localX;
  int localY;
  int localW;
  int localH;
  int childX;
  int childY;
  int xoffs;
  int yoffs;
  int woffs;
  int hoffs;
  GWEN_DB_NODE *dbW;
  GWEN_TYPE_UINT32 flags;

  assert(n);
  assert(db);
  assert(x);
  assert(y);

  flags=0;

  GWEN_DB_ClearGroup(db, 0);

  localX=GWEN_XMLNode_GetIntValue(n, "x", *x);
  localY=GWEN_XMLNode_GetIntValue(n, "y", *y);
  localW=GWEN_XMLNode_GetIntValue(n, "width", width-localX);
  localH=GWEN_XMLNode_GetIntValue(n, "height", height-localY);
  DBG_NOTICE(0, "Parsing Widget [%s] (%d/%d, %d/%d)",
             GWEN_XMLNode_GetData(n),
             localX, localY, localW, localH);

  *x+=localW;
  *y+=localH;

  if ((localX+localW)>width) {
    DBG_ERROR(0,
              "Widget does not fit into parent (x=%d, w=%d, width=%d)",
              localX, localW, width);
    return -1;
  }

  if ((localY+localH)>height) {
    DBG_ERROR(0,
              "Widget does not fit into parent (y=%d, h=%d, height=%d)",
              localY, localH, height);
    return -1;
  }

  woffs=hoffs=0;
  xoffs=yoffs=0;

  /* store variables in DB */
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "x", localX);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "y", localY);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "width", localW);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "height", localH);
  GWEN_DB_SetPtrValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "xmlpointer", (void*)n);

  /* parse WIDGET flags */
  nn=GWEN_XMLNode_FindFirstTag(n, "flags", 0, 0);
  if (nn) {
    nn=GWEN_XMLNode_FindFirstTag(nn, "flag", 0, 0);
    while(nn) {
      const char *p;
      GWEN_XMLNODE *dn;

      dn=GWEN_XMLNode_GetFirstData(nn);
      if (dn) {
        p=GWEN_XMLNode_GetData(dn);
        if (p) {
          int not;
          GWEN_TYPE_UINT32 flag = 0;
  
          if (*p=='-' || *p=='!') {
            p++;
            not=1;
          }
          else if (*p=='+') {
            p++;
            not=0;
          }
          else
            not=0;
          DBG_NOTICE(0, "Parsing flag \"%s\"", p);
          if (strcasecmp(p, "WIDGET_FLAGS_DEFAULT")==0) {
            flag=GWEN_WIDGET_FLAGS_DEFAULT;
          }
          else if (strcasecmp(p, "WIDGET_FLAGS_IGN_HELP")==0) {
            flag=GWEN_WIDGET_FLAGS_IGN_HELP;
          }
          else if (strcasecmp(p, "WIDGET_FLAGS_KEYPAD")==0) {
            flag=GWEN_WIDGET_FLAGS_KEYPAD;
          }
          else if (strcasecmp(p, "WIDGET_FLAGS_SCROLL")==0) {
            flag=GWEN_WIDGET_FLAGS_SCROLL;
          }
          else if (strcasecmp(p, "WIDGET_FLAGS_ACTIVE")==0) {
            flag=GWEN_WIDGET_FLAGS_ACTIVE;
          }
          else if (strcasecmp(p, "WIDGET_FLAGS_FOCUSABLE")==0) {
            flag=GWEN_WIDGET_FLAGS_FOCUSABLE;
          }
          else if (strcasecmp(p, "WIDGET_FLAGS_BORDER")==0) {
            flag=GWEN_WIDGET_FLAGS_BORDER;
            woffs+=2;
            xoffs++;
            if (localH>1) {
              hoffs++;
              yoffs++;
            }
          }
          else if (strcasecmp(p, "WIDGET_FLAGS_NEEDCURSOR")==0) {
            flag=GWEN_WIDGET_FLAGS_NEEDCURSOR;
          }
          else if (strcasecmp(p, "WIDGET_FLAGS_HCENTER")==0) {
            flag=GWEN_WIDGET_FLAGS_HCENTER;
          }
          else if (strcasecmp(p, "WIDGET_FLAGS_VCENTER")==0) {
            flag=GWEN_WIDGET_FLAGS_VCENTER;
          }
          else if (strcasecmp(p, "WIDGET_FLAGS_HIGHLIGHT")==0) {
            flag=GWEN_WIDGET_FLAGS_HIGHLIGHT;
          }
          else if (strcasecmp(p, "WIDGET_FLAGS_MODAL")==0) {
            flag=GWEN_WIDGET_FLAGS_MODAL;
          }
          else if (strcasecmp(p, "WINDOW_FLAGS_TITLE")==0) {
            yoffs++;
            hoffs++;
          }
          else if (strcasecmp(p, "SCROLLWIN_FLAGS_HSLIDER")==0) {
            hoffs++;
          }
          else if (strcasecmp(p, "SCROLLWIN_FLAGS_VSLIDER")==0) {
            woffs++;
          }
          else
            flag=0;
  
          if (flag) {
            if (not)
              flags&=~flag;
            else
              flags|=flag;
          }
        }
      }
      nn=GWEN_XMLNode_FindNextTag(nn, "flag", 0, 0);
    }
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "flags", flags);
  }

  /* read config specific data */
  nn=GWEN_XMLNode_FindFirstTag(n, "config", 0, 0);
  if (nn) {
    const char *p;
    GWEN_DB_NODE *dbConfig;
    int i;

    dbConfig=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_DEFAULT, "config");
    assert(dbConfig);
    p=GWEN_XMLNode_GetCharValue(nn, "path", 0);
    if (p)
      GWEN_DB_SetCharValue(dbConfig, GWEN_DB_FLAGS_DEFAULT, "path", p);
    p=GWEN_XMLNode_GetCharValue(nn, "type", 0);
    if (p)
      GWEN_DB_SetCharValue(dbConfig, GWEN_DB_FLAGS_DEFAULT, "type", p);
    i=GWEN_XMLNode_GetIntValue(nn, "minlen", 0);
    if (i)
      GWEN_DB_SetIntValue(dbConfig, GWEN_DB_FLAGS_DEFAULT, "minlen", i);
    i=GWEN_XMLNode_GetIntValue(nn, "maxlen", 0);
    if (i)
      GWEN_DB_SetIntValue(dbConfig, GWEN_DB_FLAGS_DEFAULT, "maxlen", i);
    i=GWEN_XMLNode_GetIntValue(nn, "minvalue", 0);
    if (i)
      GWEN_DB_SetIntValue(dbConfig, GWEN_DB_FLAGS_DEFAULT, "minvalue", i);
    i=GWEN_XMLNode_GetIntValue(nn, "maxvalue", 0);
    if (i)
      GWEN_DB_SetIntValue(dbConfig, GWEN_DB_FLAGS_DEFAULT, "maxvalue", i);
  }

  /* check for children */
  childX=0;
  childY=0;

  localW-=woffs;
  localH-=hoffs;

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "xoffs", xoffs);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "yoffs", yoffs);

  nn=GWEN_XMLNode_GetFirstTag(n);
  while(nn) {
    const char *name;

    name=GWEN_XMLNode_GetData(nn);
    if (name) {
      int rv;

      localX=childX;
      localY=childY;
      dbW=0;
      if (strcasecmp(name, "hgroup")==0) {
        dbW=GWEN_DB_GetGroup(db,
                             GWEN_DB_FLAGS_DEFAULT |
                             GWEN_PATH_FLAGS_CREATE_GROUP,
                             "hgroup");
        assert(dbW);
        rv=GWEN_UILoader__ParseHGroup(nn, dbW, &localX, &localY,
                                      localW, localH);
      }
      else if (strcasecmp(name, "vgroup")==0) {
        dbW=GWEN_DB_GetGroup(db,
                             GWEN_DB_FLAGS_DEFAULT |
                             GWEN_PATH_FLAGS_CREATE_GROUP,
                             "vgroup");
        assert(dbW);
        rv=GWEN_UILoader__ParseVGroup(nn, dbW, &localX, &localY,
                                      localW, localH);
      }
      else if (strcasecmp(name, "widget")==0) {
        dbW=GWEN_DB_GetGroup(db,
                             GWEN_DB_FLAGS_DEFAULT |
                             GWEN_PATH_FLAGS_CREATE_GROUP,
                             "widget");
        assert(dbW);
        rv=GWEN_UILoader__ParseWidget(nn, dbW, &localX, &localY,
                                      localW, localH);
      }
      else {
        rv=0;
      }
      if (rv) {
        DBG_ERROR(0, "Error parsing child node");
        return -1;
      }
    } /* if name */
    nn=GWEN_XMLNode_GetNextTag(nn);
  } /* while */

  return 0;
}



int GWEN_UILoader__ParseVGroup(GWEN_XMLNODE *n,
                               GWEN_DB_NODE *db,
                               int *x, int *y,
                               int width, int height) {
  GWEN_XMLNODE *nn;
  int localX;
  int localY;
  int localW;
  int localH;
  int maxX;
  int elements;
  const char *align;
  int gap;
  int offset;
  int currY = 0;
  int useAll = 1;
  GWEN_DB_NODE *dbW;

  assert(n);
  assert(db);
  assert(x);
  assert(y);
  assert(width);
  assert(height);

  maxX=0;
  elements=0;

  GWEN_DB_ClearGroup(db, 0);

  DBG_NOTICE(0, "Parsing VGroup (%d/%d, %d/%d", *x, *y, width, height);

  align=GWEN_XMLNode_GetProperty(n, "align", "none");
  gap=atoi(GWEN_XMLNode_GetProperty(n, "gap", "0"));
  localX=GWEN_XMLNode_GetIntValue(n, "x", *x);
  localY=GWEN_XMLNode_GetIntValue(n, "y", *y);
  localW=GWEN_XMLNode_GetIntValue(n, "width", width-localX);
  localH=GWEN_XMLNode_GetIntValue(n, "height", height-localY);

  if ((localX+localW)>width) {
    DBG_ERROR(0,
              "Widget does not fit into parent (x=%d, w=%d, width=%d)",
              localX, localW, width);
    return -1;
  }

  if ((localY+localH)>height) {
    DBG_ERROR(0,
              "Widget does not fit into parent (y=%d, h=%d, height=%d)",
              localY, localH, height);
    return -1;
  }

  /* store variables in DB */
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "x", localX);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "y", localY);
  GWEN_DB_SetPtrValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "xmlpointer", (void*)n);

  localX=0;
  localY=0;

  /* gather relative positions and sizes */
  nn=GWEN_XMLNode_GetFirstTag(n);
  while(nn) {
    const char *name;

    name=GWEN_XMLNode_GetData(nn);
    if (name) {
      int rv;

      localX=0;
      dbW=0;
      elements++;
      if (strcasecmp(name, "hgroup")==0) {
        dbW=GWEN_DB_GetGroup(db,
                             GWEN_DB_FLAGS_DEFAULT |
                             GWEN_PATH_FLAGS_CREATE_GROUP,
                             "hgroup");
        assert(dbW);
        rv=GWEN_UILoader__ParseHGroup(nn, dbW, &localX, &localY,
                                      localW, localH);
      }
      else if (strcasecmp(name, "vgroup")==0) {
        dbW=GWEN_DB_GetGroup(db,
                             GWEN_DB_FLAGS_DEFAULT |
                             GWEN_PATH_FLAGS_CREATE_GROUP,
                             "vgroup");
        assert(dbW);
        rv=GWEN_UILoader__ParseVGroup(nn, dbW, &localX, &localY,
                                      localW, localH);
      }
      else if (strcasecmp(name, "widget")==0) {
        dbW=GWEN_DB_GetGroup(db,
                             GWEN_DB_FLAGS_DEFAULT |
                             GWEN_PATH_FLAGS_CREATE_GROUP,
                             "widget");
        assert(dbW);
        rv=GWEN_UILoader__ParseWidget(nn, dbW, &localX, &localY,
                                      localW, localH);
      }
      else {
        elements--; /* has been falsely incremented above */
        rv=0;
      }
      if (rv) {
        DBG_ERROR(0, "Error parsing child node");
        return -1;
      }
      maxX=(maxX<localX)?localX:maxX;
    } /* if name */
    nn=GWEN_XMLNode_GetNextTag(nn);
  } /* while */

  if (elements) {
    if (localY>localH) {
      DBG_ERROR(0, "Sub-elements do not fit into this group (width)");
      return-1;
    }

    if (maxX>localW) {
      DBG_ERROR(0, "Sub-elements do not fit into this group (height)");
      return-1;
    }

    /* calculate gap size according to align mode */
    offset=0;
    useAll=1;
    if (strcasecmp(align, "center")==0) {
      offset=(localH-localY)/2;
    }
    else if (strcasecmp(align, "top")==0) {
      offset=0;
    }
    else if (strcasecmp(align, "bottom")==0) {
      offset=(localH-(localY+(elements-1)*gap))-1;
    }
    else if (strcasecmp(align, "block")==0) {
      if (elements>1)
        gap=(localH-localY)/(elements-1);
      else
        gap=0;
      offset=0;
    }
    else {
      useAll=0;
    }

    /* adjust relative values (according to alignment) */
    currY=offset;
    dbW=GWEN_DB_GetFirstGroup(db);
    while(dbW) {
      int v;

      assert(elements);
      v=GWEN_DB_GetIntValue(dbW, "height", 0, height);
      GWEN_DB_SetIntValue(dbW, GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "y", currY);
      GWEN_DB_SetIntValue(dbW, GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "x", 0);
      currY+=v;
      elements--;
      if (elements)
        currY+=gap;
      dbW=GWEN_DB_GetNextGroup(dbW);
    }

  } /* if elements */

  /* final check */
  if (currY>localH) {
    DBG_ERROR(0,
              "Sub-elements do not fit into this group (cy=%d, localh=%d)",
              currY, localH);
    return-1;
  }


  *x+=maxX;
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "width", maxX);
  if (useAll) {
    *y+=localH;
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "height", localH);
  }
  else {
    *y+=currY;
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "height", currY);
  }
  return 0;
}




int GWEN_UILoader__ParseHGroup(GWEN_XMLNODE *n,
                               GWEN_DB_NODE *db,
                               int *x, int *y,
                               int width, int height) {
  GWEN_XMLNODE *nn;
  int localX;
  int localY;
  int localW;
  int localH;
  int maxY;
  int elements;
  const char *align;
  int gap;
  int offset;
  int currX = 0;
  int useAll = 1;
  GWEN_DB_NODE *dbW;

  assert(n);
  assert(db);
  assert(x);
  assert(y);
  assert(width);
  assert(height);

  maxY=0;
  elements=0;

  GWEN_DB_ClearGroup(db, 0);
  align=GWEN_XMLNode_GetProperty(n, "align", "none");
  gap=atoi(GWEN_XMLNode_GetProperty(n, "gap", "0"));
  localX=GWEN_XMLNode_GetIntValue(n, "x", *x);
  localY=GWEN_XMLNode_GetIntValue(n, "y", *y);
  localW=GWEN_XMLNode_GetIntValue(n, "width", width-localX);
  localH=GWEN_XMLNode_GetIntValue(n, "height", height-localY);

  if ((localX+localW)>width) {
    DBG_ERROR(0,
              "Widget does not fit into parent (x=%d, w=%d, width=%d)",
              localX, localW, width);
    return -1;
  }

  if ((localY+localH)>height) {
    DBG_ERROR(0,
              "Widget does not fit into parent (y=%d, h=%d, height=%d)",
              localY, localH, height);
    return -1;
  }

  /* store variables in DB */
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "x", localX);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "y", localY);

  GWEN_DB_SetPtrValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "xmlpointer", (void*)n);

  localX=0;
  localY=0;

  /* gather relative positions and sizes */
  nn=GWEN_XMLNode_GetFirstTag(n);
  while(nn) {
    const char *name;

    name=GWEN_XMLNode_GetData(nn);
    if (name) {
      int rv;

      localY=0;
      dbW=0;
      elements++;
      if (strcasecmp(name, "hgroup")==0) {
        dbW=GWEN_DB_GetGroup(db,
                             GWEN_DB_FLAGS_DEFAULT |
                             GWEN_PATH_FLAGS_CREATE_GROUP,
                             "hgroup");
        assert(dbW);
        rv=GWEN_UILoader__ParseHGroup(nn, dbW, &localX, &localY,
                                      localW, localH);
      }
      else if (strcasecmp(name, "vgroup")==0) {
        dbW=GWEN_DB_GetGroup(db,
                             GWEN_DB_FLAGS_DEFAULT |
                             GWEN_PATH_FLAGS_CREATE_GROUP,
                             "vgroup");
        assert(dbW);
        rv=GWEN_UILoader__ParseVGroup(nn, dbW, &localX, &localY,
                                      localW, localH);
      }
      else if (strcasecmp(name, "widget")==0) {
        dbW=GWEN_DB_GetGroup(db,
                             GWEN_DB_FLAGS_DEFAULT |
                             GWEN_PATH_FLAGS_CREATE_GROUP,
                             "widget");
        assert(dbW);
        rv=GWEN_UILoader__ParseWidget(nn, dbW, &localX, &localY,
                                      localW, localH);
      }
      else {
        elements--; /* has been falsely incremented above */
        rv=0;
      }
      if (rv) {
        DBG_ERROR(0, "Error parsing child node");
        return -1;
      }
      maxY=(maxY<localY)?localY:maxY;
    } /* if name */
    nn=GWEN_XMLNode_GetNextTag(nn);
  } /* while */

  if (elements) {
    if (localX>localW) {
      DBG_ERROR(0, "Sub-elements do not fit into this group (width)");
      return-1;
    }

    if (maxY>localH) {
      DBG_ERROR(0, "Sub-elements do not fit into this group (height)");
      return-1;
    }

    /* calculate gap size according to align mode */
    offset=0;
    useAll=1;
    if (strcasecmp(align, "center")==0) {
      offset=(localW-localX)/2;
    }
    else if (strcasecmp(align, "left")==0) {
      offset=0;
    }
    else if (strcasecmp(align, "right")==0) {
      offset=localW-(localX+(elements-1)*gap);
    }
    else if (strcasecmp(align, "block")==0) {
      if (elements>1)
        gap=(localW-localX)/(elements-1);
      else
        gap=0;
      offset=0;
    }
    else {
      useAll=0;
    }

    /* adjust relative values (according to alignment) */
    currX=offset;
    dbW=GWEN_DB_GetFirstGroup(db);
    while(dbW) {
      int v;

      assert(elements);
      v=GWEN_DB_GetIntValue(dbW, "width", 0, width);
      GWEN_DB_SetIntValue(dbW, GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "x", currX);
      GWEN_DB_SetIntValue(dbW, GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "y", 0);
      currX+=v;
      elements--;
      if (elements)
        currX+=gap;
      dbW=GWEN_DB_GetNextGroup(dbW);
    }

  } /* if elements */

  /* final check */
  if (currX>localW) {
    DBG_ERROR(0,
              "Sub-elements do not fit into this group (cx=%d, w=%d",
              currX, localW);
    return-1;
  }

  *y+=maxY;
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "height", maxY);
  if (useAll) {
    *x+=localW;
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "width", localW);
  }
  else {
    *x+=currX;
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "width", currX);
  }
  return 0;
}



int GWEN_UILoader__AdjustWidgetsPos(GWEN_DB_NODE *db,
                                    int x, int y) {
  GWEN_DB_NODE *dbW;
  int nx, ny;

  nx=GWEN_DB_GetIntValue(db, "x", 0, 0)+x;
  ny=GWEN_DB_GetIntValue(db, "y", 0, 0)+y;

  if (x || y) {
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "x", nx);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "y", ny);
  }
  else {
  }

  if (strcasecmp(GWEN_DB_GroupName(db), "widget")==0) {
    nx=0;
    ny=0;
  }

  dbW=GWEN_DB_GetFirstGroup(db);
  while(dbW) {
    if (GWEN_UILoader__AdjustWidgetsPos(dbW, nx, ny)){
      DBG_ERROR(0, "Error adjusting child");
      return -1;
    }
    dbW=GWEN_DB_GetNextGroup(dbW);
  } /* while */

  return 0;
}



int GWEN_UILoader_ParseWidget(GWEN_XMLNODE *n,
                              GWEN_DB_NODE *db,
                              int x, int y,
                              int width, int height) {
  int localX, localY;

  localX=0;
  localY=0;

  /* get defaults for with and height */
  if (width==0)
    width=GWEN_UI_GetCols();
  if (height==0)
    height=GWEN_UI_GetLines();

  if (GWEN_UILoader__ParseWidget(n, db, &localX, &localY, width, height)) {
    return -1;
  }

  DBG_NOTICE(0, "width=%d, height=%d", localX, localY);

  if (x==-1) {
    x=(width-localX)/2;
    DBG_NOTICE(0, "Changed X to %d", x);
  }

  if (y==-1) {
    y=(height-localY)/2;
    DBG_NOTICE(0, "Changed Y to %d", y);
  }

  if (GWEN_UILoader__AdjustWidgetsPos(db, x, y)) {
    return -1;
  }

  return 0;
}




GWEN_WIDGET *GWEN_UILoader_LoadButton(GWEN_WIDGET *parent,
                                      GWEN_DB_NODE *db,
                                      GWEN_XMLNODE *n) {
  GWEN_WIDGET *widget;
  int x, y, w, h;
  const char *text, *helpText, *name;
  GWEN_TYPE_UINT32 flags;
  GWEN_XMLNODE *nn;
  int commandId;

  x=GWEN_DB_GetIntValue(db, "x", 0, 0);
  y=GWEN_DB_GetIntValue(db, "y", 0, 0);
  w=GWEN_DB_GetIntValue(db, "width", 0, 0);
  h=GWEN_DB_GetIntValue(db, "height", 0, 0);
  name=GWEN_XMLNode_GetCharValue(n, "name", 0);
  flags=GWEN_DB_GetIntValue(db, "flags", 0, 0);
  if (GWEN_UILoader__ExtractText(n, db, "text")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  text=GWEN_DB_GetCharValue(db, "text", 0, 0);
  if (GWEN_UILoader__ExtractText(n, db, "helptext")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  helpText=GWEN_DB_GetCharValue(db, "helpText", 0, 0);

  nn=GWEN_XMLNode_FindFirstTag(n, "flags", 0, 0);
  if (nn) {
    nn=GWEN_XMLNode_FindFirstTag(nn, "flag", 0, 0);
    while(nn) {
      const char *p;
      GWEN_XMLNODE *dn;

      dn=GWEN_XMLNode_GetFirstData(nn);
      if (dn) {
        p=GWEN_XMLNode_GetData(dn);
        if (p) {
          int not;
          GWEN_TYPE_UINT32 flag;

          if (*p=='-' || *p=='!') {
            p++;
            not=1;
          }
          else if (*p=='+') {
            p++;
            not=0;
          }
          else
            not=0;
          DBG_NOTICE(0, "Parsing flag \"%s\"", p);
          if (strcasecmp(p, "BUTTON_FLAGS_CHECKBOX")==0) {
            flag=GWEN_BUTTON_FLAGS_CHECKBOX;
          }
          else
            flag=0;
  
          if (flag) {
            if (not)
              flags&=~flag;
            else
              flags|=flag;
          }
        }
      }
      nn=GWEN_XMLNode_FindNextTag(nn, "flag", 0, 0);
    }
  }

  commandId=GWEN_DB_GetIntValue(db, "commandId", 0, 0);
  widget=GWEN_Button_new(parent, flags, name, text, commandId,
                         x, y, w, h);
  assert(widget);
  if (helpText)
    GWEN_Widget_SetHelpText(widget, helpText);
  GWEN_DB_DeleteVar(db, "text");
  GWEN_DB_DeleteVar(db, "helpText");
  return widget;
}



GWEN_WIDGET *GWEN_UILoader_LoadWindow(GWEN_WIDGET *parent,
                                      GWEN_DB_NODE *db,
                                      GWEN_XMLNODE *n) {
  GWEN_WIDGET *widget;
  int x, y, w, h;
  const char *text, *helpText, *name, *title;
  GWEN_TYPE_UINT32 flags;
  GWEN_XMLNODE *nn;

  x=GWEN_DB_GetIntValue(db, "x", 0, 0);
  y=GWEN_DB_GetIntValue(db, "y", 0, 0);
  w=GWEN_DB_GetIntValue(db, "width", 0, 0);
  h=GWEN_DB_GetIntValue(db, "height", 0, 0);
  name=GWEN_XMLNode_GetCharValue(n, "name", 0);
  flags=GWEN_DB_GetIntValue(db, "flags", 0, 0);
  title=GWEN_XMLNode_GetCharValue(n, "title", 0);
  if (GWEN_UILoader__ExtractText(n, db, "text")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  text=GWEN_DB_GetCharValue(db, "text", 0, 0);
  if (GWEN_UILoader__ExtractText(n, db, "helptext")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  helpText=GWEN_DB_GetCharValue(db, "helpText", 0, 0);

  nn=GWEN_XMLNode_FindFirstTag(n, "flags", 0, 0);
  if (nn) {
    nn=GWEN_XMLNode_FindFirstTag(nn, "flag", 0, 0);
    while(nn) {
      const char *p;
      GWEN_XMLNODE *dn;

      dn=GWEN_XMLNode_GetFirstData(nn);
      if (dn) {
        p=GWEN_XMLNode_GetData(dn);
        if (p) {
          int not;
          GWEN_TYPE_UINT32 flag;

          if (*p=='-' || *p=='!') {
            p++;
            not=1;
          }
          else if (*p=='+') {
            p++;
            not=0;
          }
          else
            not=0;
          DBG_NOTICE(0, "Parsing flag \"%s\"", p);
          if (strcasecmp(p, "WINDOW_FLAGS_TITLE")==0) {
            flag=GWEN_WINDOW_FLAGS_TITLE;
          }
          else
            flag=0;
  
          if (flag) {
            if (not)
              flags&=~flag;
            else
              flags|=flag;
          }
        }
      }
      nn=GWEN_XMLNode_FindNextTag(nn, "flag", 0, 0);
    }
  }

  DBG_NOTICE(0, "Flags: %08x", flags);
  widget=GWEN_Window_new(parent, flags, name, title,
                         x, y, w, h);
  assert(widget);
  if (helpText)
    GWEN_Widget_SetHelpText(widget, helpText);
  GWEN_DB_DeleteVar(db, "text");
  GWEN_DB_DeleteVar(db, "helpText");
  return widget;
}



GWEN_WIDGET *GWEN_UILoader_LoadScrollWidget(GWEN_WIDGET *parent,
                                            GWEN_DB_NODE *db,
                                            GWEN_XMLNODE *n) {
  GWEN_WIDGET *widget;
  int x, y, w, h;
  const char *text, *helpText, *name, *title;
  GWEN_TYPE_UINT32 flags;
  GWEN_XMLNODE *nn;

  x=GWEN_DB_GetIntValue(db, "x", 0, 0);
  y=GWEN_DB_GetIntValue(db, "y", 0, 0);
  w=GWEN_DB_GetIntValue(db, "width", 0, 0);
  h=GWEN_DB_GetIntValue(db, "height", 0, 0);
  name=GWEN_XMLNode_GetCharValue(n, "name", 0);
  flags=GWEN_DB_GetIntValue(db, "flags", 0, 0);
  title=GWEN_XMLNode_GetCharValue(n, "title", 0);
  if (GWEN_UILoader__ExtractText(n, db, "text")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  text=GWEN_DB_GetCharValue(db, "text", 0, 0);
  if (GWEN_UILoader__ExtractText(n, db, "helptext")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  helpText=GWEN_DB_GetCharValue(db, "helpText", 0, 0);

  nn=GWEN_XMLNode_FindFirstTag(n, "flags", 0, 0);
  if (nn) {
    nn=GWEN_XMLNode_FindFirstTag(nn, "flag", 0, 0);
    while(nn) {
      const char *p;
      GWEN_XMLNODE *dn;

      dn=GWEN_XMLNode_GetFirstData(nn);
      if (dn) {
        p=GWEN_XMLNode_GetData(dn);
        if (p) {
          int not;
          GWEN_TYPE_UINT32 flag;

          if (*p=='-' || *p=='!') {
            p++;
            not=1;
          }
          else if (*p=='+') {
            p++;
            not=0;
          }
          else
            not=0;
          DBG_NOTICE(0, "Parsing flag \"%s\"", p);
          if (strcasecmp(p, "SCROLLWIN_FLAGS_HSLIDER")==0) {
            flag=GWEN_SCROLLWIN_FLAGS_HSLIDER;
          }
          else if (strcasecmp(p, "SCROLLWIN_FLAGS_VSLIDER")==0) {
            flag=GWEN_SCROLLWIN_FLAGS_VSLIDER;
          }
          else if (strcasecmp(p, "SCROLLWIN_FLAGS_PASSIVE_SLIDERS")==0) {
            flag=GWEN_SCROLLWIN_FLAGS_PASSIVE_SLIDERS;
          }
          else
            flag=0;
  
          if (flag) {
            if (not)
              flags&=~flag;
            else
              flags|=flag;
          }
        }
      }
      nn=GWEN_XMLNode_FindNextTag(nn, "flag", 0, 0);
    }
  }

  widget=GWEN_ScrollWidget_new(parent, flags, name,
                               x, y, w, h);
  assert(widget);
  if (helpText)
    GWEN_Widget_SetHelpText(widget, helpText);

  GWEN_DB_DeleteVar(db, "text");
  GWEN_DB_DeleteVar(db, "helpText");
  return widget;
}



GWEN_WIDGET *GWEN_UILoader_LoadEditBox(GWEN_WIDGET *parent,
                                       GWEN_DB_NODE *db,
                                       GWEN_XMLNODE *n) {
  GWEN_WIDGET *widget;
  int x, y, w, h, maxLen;
  const char *text, *helpText, *name;
  GWEN_TYPE_UINT32 flags;
  GWEN_XMLNODE *nn;

  x=GWEN_DB_GetIntValue(db, "x", 0, 0);
  y=GWEN_DB_GetIntValue(db, "y", 0, 0);
  w=GWEN_DB_GetIntValue(db, "width", 0, 0);
  h=GWEN_DB_GetIntValue(db, "height", 0, 0);
  name=GWEN_XMLNode_GetCharValue(n, "name", 0);
  maxLen=GWEN_DB_GetIntValue(db, "maxlen", 0, 0);
  flags=GWEN_DB_GetIntValue(db, "flags", 0, 0);
  if (GWEN_UILoader__ExtractText(n, db, "text")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  text=GWEN_DB_GetCharValue(db, "text", 0, 0);
  if (GWEN_UILoader__ExtractText(n, db, "helptext")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  helpText=GWEN_DB_GetCharValue(db, "helpText", 0, 0);

  nn=GWEN_XMLNode_FindFirstTag(n, "flags", 0, 0);
  if (nn) {
    nn=GWEN_XMLNode_FindFirstTag(nn, "flag", 0, 0);
    while(nn) {
      const char *p;
      GWEN_XMLNODE *dn;

      dn=GWEN_XMLNode_GetFirstData(nn);
      if (dn) {
        p=GWEN_XMLNode_GetData(dn);
        if (p) {
          int not;
          GWEN_TYPE_UINT32 flag;

          if (*p=='-' || *p=='!') {
            p++;
            not=1;
          }
          else if (*p=='+') {
            p++;
            not=0;
          }
          else
            not=0;
          DBG_NOTICE(0, "Parsing flag \"%s\"", p);
          if (strcasecmp(p, "EDITBOX_FLAGS_MULTILINE")==0) {
            flag=GWEN_EDITBOX_FLAGS_MULTILINE;
          }
          else if (strcasecmp(p, "EDITBOX_FLAGS_DYNAMIC")==0) {
            flag=GWEN_EDITBOX_FLAGS_DYNAMIC;
          }
          else if (strcasecmp(p, "EDITBOX_FLAGS_CLEARALL")==0) {
            flag=GWEN_EDITBOX_FLAGS_CLEARALL;
          }
          else if (strcasecmp(p, "EDITBOX_FLAGS_INSERT")==0) {
            flag=GWEN_EDITBOX_FLAGS_INSERT;
          }
          else if (strcasecmp(p, "EDITBOX_FLAGS_EDIT")==0) {
            flag=GWEN_EDITBOX_FLAGS_EDIT;
          }
          else
            flag=0;
  
          if (flag) {
            if (not)
              flags&=~flag;
            else
              flags|=flag;
          }
        }
      }
      nn=GWEN_XMLNode_FindNextTag(nn, "flag", 0, 0);
    }
  }

  widget=GWEN_EditBox_new(parent, flags, name, text,
                          x, y, w, h, maxLen);
  assert(widget);
  if (helpText)
    GWEN_Widget_SetHelpText(widget, helpText);

  GWEN_DB_DeleteVar(db, "text");
  GWEN_DB_DeleteVar(db, "helpText");
  return widget;
}



GWEN_WIDGET *GWEN_UILoader_LoadCheckBox(GWEN_WIDGET *parent,
                                        GWEN_DB_NODE *db,
                                        GWEN_XMLNODE *n) {
  GWEN_WIDGET *widget;
  int x, y, w, h;
  const char *text, *helpText, *name;
  GWEN_TYPE_UINT32 flags;

  x=GWEN_DB_GetIntValue(db, "x", 0, 0);
  y=GWEN_DB_GetIntValue(db, "y", 0, 0);
  w=GWEN_DB_GetIntValue(db, "width", 0, 0);
  h=GWEN_DB_GetIntValue(db, "height", 0, 0);
  name=GWEN_XMLNode_GetCharValue(n, "name", 0);
  flags=GWEN_DB_GetIntValue(db, "flags", 0, 0);
  if (GWEN_UILoader__ExtractText(n, db, "text")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  text=GWEN_DB_GetCharValue(db, "text", 0, 0);
  if (GWEN_UILoader__ExtractText(n, db, "helptext")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  helpText=GWEN_DB_GetCharValue(db, "helpText", 0, 0);

  widget=GWEN_CheckBox_new(parent, flags, name, text,
                           x, y, w, h);
  assert(widget);
  if (helpText)
    GWEN_Widget_SetHelpText(widget, helpText);

  GWEN_DB_DeleteVar(db, "text");
  GWEN_DB_DeleteVar(db, "helpText");
  return widget;
}



GWEN_WIDGET *GWEN_UILoader_LoadDropDownBox(GWEN_WIDGET *parent,
                                           GWEN_DB_NODE *db,
                                           GWEN_XMLNODE *n) {
  GWEN_WIDGET *widget;
  int x, y, w, h, maxLen;
  const char *text, *helpText, *name;
  GWEN_TYPE_UINT32 flags;
  GWEN_XMLNODE *nn;
  GWEN_STRINGLIST *sl;

  x=GWEN_DB_GetIntValue(db, "x", 0, 0);
  y=GWEN_DB_GetIntValue(db, "y", 0, 0);
  w=GWEN_DB_GetIntValue(db, "width", 0, 0);
  h=GWEN_DB_GetIntValue(db, "height", 0, 0);
  name=GWEN_XMLNode_GetCharValue(n, "name", 0);
  maxLen=GWEN_DB_GetIntValue(db, "maxlen", 0, 0);
  flags=GWEN_DB_GetIntValue(db, "flags", 0, 0);
  if (GWEN_UILoader__ExtractText(n, db, "text")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  text=GWEN_DB_GetCharValue(db, "text", 0, 0);
  if (GWEN_UILoader__ExtractText(n, db, "helptext")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  helpText=GWEN_DB_GetCharValue(db, "helpText", 0, 0);

  nn=GWEN_XMLNode_FindFirstTag(n, "flags", 0, 0);
  if (nn) {
    nn=GWEN_XMLNode_FindFirstTag(nn, "flag", 0, 0);
    while(nn) {
      const char *p;
      GWEN_XMLNODE *dn;

      dn=GWEN_XMLNode_GetFirstData(nn);
      if (dn) {
        p=GWEN_XMLNode_GetData(dn);
        if (p) {
          int not;
          GWEN_TYPE_UINT32 flag;

          if (*p=='-' || *p=='!') {
            p++;
            not=1;
          }
          else if (*p=='+') {
            p++;
            not=0;
          }
          else
            not=0;
          DBG_NOTICE(0, "Parsing flag \"%s\"", p);
          if (strcasecmp(p, "DROPDOWNBOX_FLAGS_EDIT")==0) {
            flag=GWEN_DROPDOWNBOX_FLAGS_EDIT;
          }
          else
            flag=0;

          if (flag) {
            if (not)
              flags&=~flag;
            else
              flags|=flag;
          }
        }
      }
      nn=GWEN_XMLNode_FindNextTag(nn, "flag", 0, 0);
    }
  }

  sl=GWEN_StringList_new();
  nn=GWEN_XMLNode_FindFirstTag(n, "choices", 0, 0);
  if (nn) {
    nn=GWEN_XMLNode_FindFirstTag(nn, "choice", 0, 0);
    while(nn) {
      const char *p;
      GWEN_XMLNODE *dn;

      dn=GWEN_XMLNode_GetFirstData(nn);
      if (dn) {
        p=GWEN_XMLNode_GetData(dn);
        if (p)
          GWEN_StringList_AppendString(sl, p, 0, 1);
      }
      nn=GWEN_XMLNode_FindNextTag(nn, "choice", 0, 0);
    }
  }

  widget=GWEN_DropDownBox_new(parent, flags, name,
                              x, y, w, h, sl);
  assert(widget);
  GWEN_StringList_free(sl);
  if (helpText)
    GWEN_Widget_SetHelpText(widget, helpText);

  GWEN_DB_DeleteVar(db, "text");
  GWEN_DB_DeleteVar(db, "helpText");
  return widget;
}



GWEN_WIDGET *GWEN_UILoader_LoadTextWidget(GWEN_WIDGET *parent,
                                          GWEN_DB_NODE *db,
                                          GWEN_XMLNODE *n) {
  GWEN_WIDGET *widget;
  int x, y, w, h;
  const char *text, *helpText, *name;
  GWEN_TYPE_UINT32 flags;
  GWEN_XMLNODE *nn;

  x=GWEN_DB_GetIntValue(db, "x", 0, 0);
  y=GWEN_DB_GetIntValue(db, "y", 0, 0);
  w=GWEN_DB_GetIntValue(db, "width", 0, 0);
  h=GWEN_DB_GetIntValue(db, "height", 0, 0);
  name=GWEN_XMLNode_GetCharValue(n, "name", 0);
  flags=GWEN_DB_GetIntValue(db, "flags", 0, 0);
  if (GWEN_UILoader__ExtractText(n, db, "text")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  text=GWEN_DB_GetCharValue(db, "text", 0, 0);
  if (GWEN_UILoader__ExtractText(n, db, "helptext")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  helpText=GWEN_DB_GetCharValue(db, "helpText", 0, 0);

  nn=GWEN_XMLNode_FindFirstTag(n, "flags", 0, 0);
  if (nn) {
    nn=GWEN_XMLNode_FindFirstTag(nn, "flag", 0, 0);
    while(nn) {
      const char *p;
      GWEN_XMLNODE *dn;

      dn=GWEN_XMLNode_GetFirstData(nn);
      if (dn) {
        p=GWEN_XMLNode_GetData(dn);
        if (p) {
          int not;
          GWEN_TYPE_UINT32 flag;

          if (*p=='-' || *p=='!') {
            p++;
            not=1;
          }
          else if (*p=='+') {
            p++;
            not=0;
          }
          else
            not=0;
          DBG_NOTICE(0, "Parsing flag \"%s\"", p);
          if (strcasecmp(p, "TEXTWIDGET_FLAGS_BREAKLINES")==0) {
            flag=GWEN_TEXTWIDGET_FLAGS_BREAKLINES;
          }
          else if (strcasecmp(p, "TEXTWIDGET_FLAGS_LINEMODE")==0) {
            flag=GWEN_TEXTWIDGET_FLAGS_LINEMODE;
          }
          else if (strcasecmp(p, "TEXTWIDGET_FLAGS_HIGHLIGHT")==0) {
            flag=GWEN_TEXTWIDGET_FLAGS_HIGHLIGHT;
          }
          else if (strcasecmp(p, "TEXTWIDGET_FLAGS_DATACHANGE")==0) {
            flag=GWEN_TEXTWIDGET_FLAGS_DATACHANGE;
          }
          else if (strcasecmp(p, "TEXTWIDGET_FLAGS_FIXED")==0) {
            flag=GWEN_TEXTWIDGET_FLAGS_FIXED;
          }
          else if (strcasecmp(p, "TEXTWIDGET_FLAGS_CONST")==0) {
            flag=GWEN_TEXTWIDGET_FLAGS_CONST;
          }
          else
            flag=0;
  
          if (flag) {
            if (not)
              flags&=~flag;
            else
              flags|=flag;
          }
        }
      }
      nn=GWEN_XMLNode_FindNextTag(nn, "flag", 0, 0);
    }
  }

  widget=GWEN_TextWidget_new(parent, flags, name, text,
                             x, y, w, h);
  assert(widget);
  if (helpText)
    GWEN_Widget_SetHelpText(widget, helpText);

  GWEN_DB_DeleteVar(db, "text");
  GWEN_DB_DeleteVar(db, "helpText");
  return widget;
}



GWEN_WIDGET *GWEN_UILoader_LoadTableWidget(GWEN_WIDGET *parent,
                                           GWEN_DB_NODE *db,
                                           GWEN_XMLNODE *n) {
  GWEN_WIDGET *widget;
  int x, y, w, h;
  const char *text, *helpText, *name;
  GWEN_TYPE_UINT32 flags;
  GWEN_XMLNODE *nn;

  x=GWEN_DB_GetIntValue(db, "x", 0, 0);
  y=GWEN_DB_GetIntValue(db, "y", 0, 0);
  w=GWEN_DB_GetIntValue(db, "width", 0, 0);
  h=GWEN_DB_GetIntValue(db, "height", 0, 0);
  name=GWEN_XMLNode_GetCharValue(n, "name", 0);
  flags=GWEN_DB_GetIntValue(db, "flags", 0, 0);
  if (GWEN_UILoader__ExtractText(n, db, "text")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  text=GWEN_DB_GetCharValue(db, "text", 0, 0);
  if (GWEN_UILoader__ExtractText(n, db, "helptext")) {
    DBG_ERROR(0, "Error extracting text");
    return 0;
  }
  helpText=GWEN_DB_GetCharValue(db, "helpText", 0, 0);

  nn=GWEN_XMLNode_FindFirstTag(n, "flags", 0, 0);
  if (nn) {
    nn=GWEN_XMLNode_FindFirstTag(nn, "flag", 0, 0);
    while(nn) {
      const char *p;
      GWEN_XMLNODE *dn;

      dn=GWEN_XMLNode_GetFirstData(nn);
      if (dn) {
        p=GWEN_XMLNode_GetData(dn);
        if (p) {
          int not;
          GWEN_TYPE_UINT32 flag;

          if (*p=='-' || *p=='!') {
            p++;
            not=1;
          }
          else if (*p=='+') {
            p++;
            not=0;
          }
          else
            not=0;
          DBG_NOTICE(0, "Parsing flag \"%s\"", p);
          if (strcasecmp(p, "TABLEWIDGET_FLAGS_BREAKLINES")==0) {
            flag=GWEN_TABLEWIDGET_FLAGS_BREAKLINES;
          }
          else if (strcasecmp(p, "TABLEWIDGET_FLAGS_COLBORDER")==0) {
            flag=GWEN_TABLEWIDGET_FLAGS_COLBORDER;
          }
          else if (strcasecmp(p, "TABLEWIDGET_FLAGS_ROWBORDER")==0) {
            flag=GWEN_TABLEWIDGET_FLAGS_ROWBORDER;
          }
          else if (strcasecmp(p, "TABLEWIDGET_FLAGS_LINEMODE")==0) {
            flag=GWEN_TABLEWIDGET_FLAGS_LINEMODE;
          }
          else if (strcasecmp(p, "TABLEWIDGET_FLAGS_FIXED")==0) {
            flag=GWEN_TABLEWIDGET_FLAGS_FIXED;
          }
          else if (strcasecmp(p, "TABLEWIDGET_FLAGS_HIGHLIGHT")==0) {
            flag=GWEN_TABLEWIDGET_FLAGS_HIGHLIGHT;
          }
          else if (strcasecmp(p, "TABLEWIDGET_FLAGS_SELECTION")==0) {
            flag=GWEN_TABLEWIDGET_FLAGS_SELECTION;
          }
          else
            flag=0;
  
          if (flag) {
            if (not)
              flags&=~flag;
            else
              flags|=flag;
          }
        }
      }
      nn=GWEN_XMLNode_FindNextTag(nn, "flag", 0, 0);
    }
  }

  widget=GWEN_TableWidget_new(parent, flags, name,
                              x, y, w, h);
  assert(widget);
  if (helpText)
    GWEN_Widget_SetHelpText(widget, helpText);

  nn=GWEN_XMLNode_FindFirstTag(n, "columns", 0, 0);
  if (nn) {
    nn=GWEN_XMLNode_FindFirstTag(nn, "column", 0, 0);
    while(nn) {
      int size;

      size=atoi(GWEN_XMLNode_GetProperty(nn, "width", "0"));
      if (GWEN_TableWidget_AddColumn(widget, size)) {
        DBG_ERROR(0, "Could not add column");
        GWEN_Widget_free(widget);
        return 0;
      }
      nn=GWEN_XMLNode_FindNextTag(nn, "flag", 0, 0);
    }
  }

  GWEN_DB_DeleteVar(db, "text");
  GWEN_DB_DeleteVar(db, "helpText");
  return widget;
}




GWEN_WIDGET *GWEN_UILoader_LoadWidget(GWEN_WIDGET *parent,
                                      GWEN_DB_NODE *db) {
  GWEN_DB_NODE *dbW;
  GWEN_WIDGET *widget;
  GWEN_WIDGET *newParent;

  DBG_NOTICE(0, "Loading child of %08x (%s)",
             (unsigned int)parent, GWEN_DB_GroupName(db));
  widget=0;
  newParent=0;
  if (strcasecmp(GWEN_DB_GroupName(db), "widget")==0) {
    const char *typeName;
    GWEN_XMLNODE *n;

    n=(GWEN_XMLNODE*)GWEN_DB_GetPtrValue(db, "xmlpointer", 0, 0);
    assert(n);

    typeName=GWEN_XMLNode_GetProperty(n, "type", 0);
    if (typeName) {
      if (strcasecmp(typeName, "button")==0) {
        DBG_NOTICE(0, "Loading Button");
        widget=GWEN_UILoader_LoadButton(parent, db, n);
        newParent=widget;
      }
      else if (strcasecmp(typeName, "window")==0) {
        DBG_NOTICE(0, "Loading Window");
        widget=GWEN_UILoader_LoadWindow(parent, db, n);
        if (widget)
          newParent=GWEN_Window_GetViewPort(widget);
      }
      else if (strcasecmp(typeName, "scrollwidget")==0) {
        DBG_NOTICE(0, "Loading ScrollWidget");
        widget=GWEN_UILoader_LoadScrollWidget(parent, db, n);
        if (widget)
          newParent=GWEN_ScrollWidget_GetViewPort(widget);
      }
      else if (strcasecmp(typeName, "editbox")==0) {
        DBG_NOTICE(0, "Loading EditBox");
        widget=GWEN_UILoader_LoadEditBox(parent, db, n);
        newParent=widget;
      }
      else if (strcasecmp(typeName, "checkbox")==0) {
        DBG_NOTICE(0, "Loading CheckBox");
        widget=GWEN_UILoader_LoadCheckBox(parent, db, n);
        newParent=widget;
      }
      else if (strcasecmp(typeName, "dropdownbox")==0) {
        DBG_NOTICE(0, "Loading DropDownBox");
        widget=GWEN_UILoader_LoadDropDownBox(parent, db, n);
        newParent=widget;
      }
      else if (strcasecmp(typeName, "textwidget")==0) {
        DBG_NOTICE(0, "Loading TextWidget");
        widget=GWEN_UILoader_LoadTextWidget(parent, db, n);
        newParent=widget;
      }
      else if (strcasecmp(typeName, "tablewidget")==0) {
        DBG_NOTICE(0, "Loading TableWidget");
        widget=GWEN_UILoader_LoadTableWidget(parent, db, n);
        newParent=widget;
      }
      else {
        DBG_ERROR(0, "Unknown widget type \"%s\"", typeName);
        GWEN_Widget_free(widget);
        return 0;
      }
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
                           "type", typeName);
      DBG_NOTICE(0, "Loaded a widget (%08x, %08x)",
                 (unsigned int)widget,
                 (unsigned int)newParent);
    }
    if (!widget) {
      DBG_ERROR(0, "Could not load widget");
      return 0;
    }
    GWEN_DB_SetPtrValue(db, GWEN_DB_FLAGS_DEFAULT,
                        "widgetPointer", (void*)widget);
  } /* if widget */
  else {
    newParent=parent;
    widget=parent;
    DBG_NOTICE(0, "Not a widget");
  }

  /* load all children */
  DBG_NOTICE(0, "Loading children of widget %08x (%s)",
             (unsigned int)widget,
             GWEN_DB_GroupName(db));
  dbW=GWEN_DB_GetFirstGroup(db);
  while(dbW) {
    GWEN_WIDGET *subw;

    DBG_NOTICE(0, "Loading now %s", GWEN_DB_GroupName(dbW));
    subw=GWEN_UILoader_LoadWidget(newParent, dbW);
    if (!subw) {
      DBG_NOTICE(0, "here");
      GWEN_Widget_free(widget);
      return 0;
    }
    dbW=GWEN_DB_GetNextGroup(dbW);
  } /* while */

  return widget;
}



GWEN_WIDGET *GWEN_UILoader_LoadDialog(GWEN_WIDGET *parent,
                                      GWEN_XMLNODE *n,
                                      GWEN_DB_NODE *db){
  GWEN_WIDGET *w;
  GWEN_DB_NODE *dbDialog;

  dbDialog=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_CREATE_GROUP, "widget");
  if (GWEN_UILoader_ParseWidget(n, dbDialog, -1, -1, 0, 0)) {
    DBG_ERROR(0, "Could not parse dialog");
    return 0;
  }

  w=GWEN_UILoader_LoadWidget(parent, dbDialog);
  if (!w) {
    DBG_NOTICE(0, "Could not load dialog");
    return 0;
  }

  return w;
}



int GWEN_UILoader_Populate(GWEN_DB_NODE *dbDialog,
                           GWEN_DB_NODE *dbData){
  GWEN_DB_NODE *dbDialogChild;
  GWEN_DB_NODE *dbDialogCfg;

  dbDialogCfg=GWEN_DB_GetGroup(dbDialog, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                               "config");
  if (dbDialogCfg) {
    const char *wtype;
    const char *path;
    GWEN_WIDGET *w;

    w=GWEN_DB_GetPtrValue(dbDialog, "widgetPointer", 0, 0);
    wtype=GWEN_DB_GetCharValue(dbDialog, "type", 0, 0);
    path=GWEN_DB_GetCharValue(dbDialogCfg, "path", 0, 0);
    if (w && wtype && path) {
      const char *dtype;
      const char *d = 0;
      char numbuf[64];

      /* create data */
      dtype=GWEN_DB_GetCharValue(dbDialogCfg, "type", 0, "char");
      if (strcasecmp(dtype, "char")==0) {
        d=GWEN_DB_GetCharValue(dbData, path, 0, 0);
      }
      else if (strcasecmp(dtype, "int")==0) {
        snprintf(numbuf, sizeof(numbuf), "%i",
                 GWEN_DB_GetIntValue(dbData, path, 0, 0));
        d=numbuf;
      }

      if (!d)
        d="";

      if (strcasecmp(wtype, "EditBox")==0 ||
          strcasecmp(wtype, "DropDownBox")==0 ||
          strcasecmp(wtype, "TextWidget")==0) {
        GWEN_Widget_SetText(w, d, GWEN_EventSetTextMode_Replace);
      }
      else {
        DBG_INFO(0, "Cannot handle widget type \"%s\"", wtype);
        return -1;
      }
    }
  }

  dbDialogChild=GWEN_DB_GetFirstGroup(dbDialog);
  while(dbDialogChild) {
    if (GWEN_UILoader_Populate(dbDialogChild, dbData)) {
      DBG_INFO(0, "here");
      return -1;
    }
    dbDialogChild=GWEN_DB_GetNextGroup(dbDialogChild);
  }

  return 0;
}

































