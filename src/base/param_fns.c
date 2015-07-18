/***************************************************************************
 begin       : Wed Sep 17 2014
 copyright   : (C) 2014 by Martin Preuss
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




#include <gwenhywfar/text.h>
#include <gwenhywfar/buffer.h>





int GWEN_Param_GetCurrentValueAsInt(const GWEN_PARAM *param) {
  const char *s;

  assert(param);
  s=param->currentValue;
  if (s && *s) {
    int b;
    int v;

    b=sscanf(s, "%i", &v);
    if (b==1)
      return v;
    /* fall through */
  }

  s=param->defaultValue;
  if (s && *s) {
    int b;
    int v;

    b=sscanf(s, "%i", &v);
    if (b==1)
      return v;
    /* fall through */
  }

  return 0;
}



void GWEN_Param_SetCurrentValueAsInt(GWEN_PARAM *param, int v) {
  char numbuf[64];

  snprintf(numbuf, sizeof(numbuf)-1, "%i", v);
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_Param_SetCurrentValue(param, numbuf);
}




double GWEN_Param_GetCurrentValueAsDouble(const GWEN_PARAM *param) {
  const char *s;

  assert(param);
  s=param->currentValue;
  if (s && *s) {
    double v;
    int b;

    b=GWEN_Text_StringToDouble(s, &v);
    if (b>=0)
      return v;
    /* fall through */
  }

  s=param->defaultValue;
  if (s && *s) {
    double v;
    int b;

    b=GWEN_Text_StringToDouble(s, &v);
    if (b>=0)
      return v;
    /* fall through */
  }

  return 0.0;
}



void GWEN_Param_SetCurrentValueAsDouble(GWEN_PARAM *param, double v) {
  GWEN_BUFFER *tbuf;

  tbuf=GWEN_Buffer_new(0, 64, 0, 1);
  GWEN_Text_DoubleToBuffer(v, tbuf);
  GWEN_Param_SetCurrentValue(param, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}






int GWEN_Param_List_GetCurrentValueAsInt(const GWEN_PARAM_LIST *pl, const char *name, int defVal) {
  GWEN_PARAM *p;

  p=GWEN_Param_List_GetByName(pl, name);
  if (p)
    return GWEN_Param_GetCurrentValueAsInt(p);
  return defVal;
}



void GWEN_Param_List_SetCurrentValueAsInt(GWEN_PARAM_LIST *pl, const char *name, int v) {
  GWEN_PARAM *p;

  p=GWEN_Param_List_GetByName(pl, name);
  if (p)
    GWEN_Param_SetCurrentValueAsInt(p, v);
}



double GWEN_Param_List_GetCurrentValueAsDouble(const GWEN_PARAM_LIST *pl, const char *name, double defVal) {
  GWEN_PARAM *p;

  p=GWEN_Param_List_GetByName(pl, name);
  if (p)
    return GWEN_Param_GetCurrentValueAsDouble(p);
  return defVal;
}



void GWEN_Param_List_SetCurrentValueAsDouble(GWEN_PARAM_LIST *pl, const char *name, double v) {
  GWEN_PARAM *p;

  p=GWEN_Param_List_GetByName(pl, name);
  if (p)
    GWEN_Param_SetCurrentValueAsDouble(p, v);
}



void GWEN_Param_List_WriteXml(const GWEN_PARAM_LIST *pl, GWEN_XMLNODE *xn) {
  const GWEN_PARAM *p;

  p=GWEN_Param_List_First(pl);
  while(p) {
    GWEN_XMLNODE *n;

    n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "param");
    GWEN_Param_WriteXml(p, n);
    GWEN_XMLNode_AddChild(xn, n);

    /* next */
    p=GWEN_Param_List_Next(p);
  }
}



void GWEN_Param_List_ReadXml(GWEN_PARAM_LIST *pl, GWEN_XMLNODE *xn) {
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_FindFirstTag(xn, "param", NULL, NULL);
  while(n) {
    GWEN_PARAM *p;

    p=GWEN_Param_fromXml(n);
    if (p)
      GWEN_Param_List_Add(p, pl);

    n=GWEN_XMLNode_FindNextTag(n, "param", NULL, NULL);
  }
}




void GWEN_Param_List_WriteValuesToXml(const GWEN_PARAM_LIST *pl, GWEN_XMLNODE *xn) {
  const GWEN_PARAM *p;

  p=GWEN_Param_List_First(pl);
  while(p) {
    const char *sName;
    const char *sValue;

    sName=GWEN_Param_GetName(p);
    sValue=GWEN_Param_GetCurrentValue(p);
    if (sName && *sName && sValue && *sValue) {
      GWEN_XMLNODE *n;

      n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "param");
      GWEN_XMLNode_SetCharValue(n, "name", sName);
      GWEN_XMLNode_SetCharValue(n, "currentValue", sValue);
      GWEN_XMLNode_AddChild(xn, n);
    }

    /* next */
    p=GWEN_Param_List_Next(p);
  }
}



void GWEN_Param_List_ReadValuesFromXml(GWEN_PARAM_LIST *pl, GWEN_XMLNODE *xn) {
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_FindFirstTag(xn, "param", NULL, NULL);
  while(n) {
    const char *sName;
    const char *sValue;

    sName=GWEN_XMLNode_GetCharValue(n, "name", NULL);
    sValue=GWEN_XMLNode_GetCharValue(n, "currentValue", NULL);

    if (sName && *sName) {
      GWEN_PARAM *p;

      p=GWEN_Param_List_GetByName(pl, sName);
      if (p) {
	GWEN_Param_SetCurrentValue(p, sValue);
      }
      else {
	DBG_WARN(GWEN_LOGDOMAIN, "Param \"%s\" not found, ignoring", sName);
      }
    }

    n=GWEN_XMLNode_FindNextTag(n, "param", NULL, NULL);
  }
}







