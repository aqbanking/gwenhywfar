/***************************************************************************
 begin       : Fri Apr 18 2014
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

#ifndef GWEN_PARSER_P_H
#define GWEN_PARSER_P_H


#include <gwenhywfar/gwen_parser.h>


static int GWEN_Parser__CheckElement(const GWEN_PARSER_ELEMENT *eDefinitions, const GWEN_PARSER_ELEMENT *eData);
static const GWEN_PARSER_ELEMENT *GWEN_Parser__GetChoice(const GWEN_PARSER_ELEMENT *eDefinitions, const GWEN_PARSER_ELEMENT *eData);
static int GWEN_Parser__CheckElementAndChildren(const GWEN_PARSER_ELEMENT *eDefinitions, const GWEN_PARSER_ELEMENT *eData, int depth);
static int GWEN_Parser__CheckSequence(const GWEN_PARSER_ELEMENT *eDefinitions, const GWEN_PARSER_ELEMENT *eData, int depth);


static int GWEN_Parser__UpdateElement(const GWEN_PARSER_ELEMENT *eDefinitions, GWEN_PARSER_ELEMENT *eData);
static const GWEN_PARSER_ELEMENT *GWEN_Parser__GetAndUpdateChoice(const GWEN_PARSER_ELEMENT *eDefinitions, GWEN_PARSER_ELEMENT *eData);
static int GWEN_Parser__UpdateElementAndChildren(const GWEN_PARSER_ELEMENT *eDefinitions, GWEN_PARSER_ELEMENT *eData, int depth);
static int GWEN_Parser__UpdateSequence(const GWEN_PARSER_ELEMENT *eDefinitions, GWEN_PARSER_ELEMENT *eData, int depth);








#endif
