/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sat Jan 24 2004
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


#ifndef GWEN_NL_LOG_H
#define GWEN_NL_LOG_H

#include <gwenhywfar/netlayer.h>
#include <gwenhywfar/inetsocket.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/url.h>


#define GWEN_NL_LOG_NAME "Log"


GWENHYWFAR_API
GWEN_NETLAYER *GWEN_NetLayerLog_new(GWEN_NETLAYER *baseLayer,
                                    const char *fileNameBase);



#endif /* GWEN_NL_LOG_H */


