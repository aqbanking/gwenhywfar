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

#ifndef GWEN_PARSER_H
#define GWEN_PARSER_H


#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/gwen_parser_element.h>



GWENHYWFAR_API int GWEN_Parser_CheckTree(const GWEN_PARSER_ELEMENT_TREE *tDefinitions, const GWEN_PARSER_ELEMENT_TREE *tData);
GWENHYWFAR_API int GWEN_Parser_UpdateTree(const GWEN_PARSER_ELEMENT_TREE *tDefinitions, GWEN_PARSER_ELEMENT_TREE *tData);


#endif
