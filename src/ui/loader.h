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

#ifndef GWEN_UI_LOADER_H
#define GWEN_UI_LOADER_H

#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/ui/widget.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>
#include <gwenhywfar/buffer.h>



/**
 * <p>
 * This function loads a dialog from the given XML node using the given
 * GWEN_DB as source and as target for the user data.
 * The XML nodew contains information about which widget uses which GWEN_DB
 * entry.
 * </p>
 * <p>
 * For this to work the dialog must at least include one button with the
 * command code "1". This is assumed to be the OK button.
 * If a button with the command "2" is found it is assumed to be the ABORT
 * button. A button with the command "3" may later be used to restore
 * default values (but is currently not implemented).
 * </p>
 * @return -1 on error, 1 if the user pressed [OK], 2 if the user aborted
 */
int GWEN_UILoader_ExecDialog(GWEN_WIDGET *w,
                             GWEN_XMLNODE *n,
                             GWEN_DB_NODE *dbData);


#endif





