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
  GWEN_DB_NODE *dbW;
  GWEN_TYPE_UINT32 flags;

  assert(n);
  assert(db);
  assert(x);
  assert(y);
  assert(width);
  assert(height);

  flags=0;

  GWEN_DB_ClearGroup(db, 0);

  DBG_NOTICE(0, "Parsing Widget");
  localX=GWEN_XMLNode_GetIntValue(n, "x", *x);
  localY=GWEN_XMLNode_GetIntValue(n, "y", *y);
  localW=GWEN_XMLNode_GetIntValue(n, "width", width-localX);
  localH=GWEN_XMLNode_GetIntValue(n, "height", height-localY);

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

  /* check for children */

  childX=0;
  childY=0;

  if (flags & GWEN_WIDGET_FLAGS_BORDER) {
    childX++;
    localW-=2;
    if (localH>1) {
      childY++;
      localH-=2;
    }
  }

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

  DBG_NOTICE(0, "Parsing Widget: done");
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
  int currY;
  int useAll;
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

  DBG_NOTICE(0, "Parsing VGroup");

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
      offset=localH-(localY+(elements-1)*gap);
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
  DBG_NOTICE(0, "Parsing VGroup: done");
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
  int currX;
  int useAll;
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
  DBG_NOTICE(0, "Parsing HGroup");

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
  DBG_NOTICE(0, "Parsing HGroup: done");
  return 0;
}



int GWEN_UILoader__AdjustWidgetsPos(GWEN_DB_NODE *db,
                                    int x, int y) {
  GWEN_DB_NODE *dbW;
  int nx, ny;

  DBG_NOTICE(0, "Adjusting relatively to %d, %d", x, y);
  nx=GWEN_DB_GetIntValue(db, "x", 0, 0)+x;
  ny=GWEN_DB_GetIntValue(db, "y", 0, 0)+y;

  if (x || y) {
    DBG_NOTICE(0, "Changing");
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "x", nx);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "y", ny);
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



GWEN_DB_NODE *GWEN_UILoader_ParseWidget(GWEN_XMLNODE *n,
                                        int x, int y,
                                        int width, int height) {
  GWEN_DB_NODE *db;
  int localX, localY;

  localX=0;
  localY=0;
  db=GWEN_DB_Group_new("widget");
  if (GWEN_UILoader__ParseWidget(n, db, &localX, &localY, width, height)) {
    GWEN_DB_Group_free(db);
    return 0;
  }

  if (GWEN_UILoader__AdjustWidgetsPos(db, x, y)) {
    GWEN_DB_Group_free(db);
    return 0;
  }

  return db;
}


































