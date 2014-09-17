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

#ifndef GWENHYWFAR_PARAM_FNS_H
#define GWENHYWFAR_PARAM_FNS_H


GWENHYWFAR_API int GWEN_Param_GetCurrentValueAsInt(const GWEN_PARAM *param);
GWENHYWFAR_API void GWEN_Param_SetCurrentValueAsInt(GWEN_PARAM *param, int v);

GWENHYWFAR_API double GWEN_Param_GetCurrentValueAsDouble(const GWEN_PARAM *param);
GWENHYWFAR_API void GWEN_Param_SetCurrentValueAsDouble(GWEN_PARAM *param, double v);



#endif

