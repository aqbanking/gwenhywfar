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

/**
 * Returns the current value of the given param converted to integer.
 * @return current value as integer
 * @param param parameter to read the value from
 */
GWENHYWFAR_API int GWEN_Param_GetCurrentValueAsInt(const GWEN_PARAM *param);

/**
 * Changes the current value of the given param (converted from integer).
 * The given value is converted to char and stored.
 * @param param parameter to change
 * @param v value to set
 */
GWENHYWFAR_API void GWEN_Param_SetCurrentValueAsInt(GWEN_PARAM *param, int v);


/**
 * Returns the current value of the given param converted to floating point.
 * @return current value as floating point
 * @param param parameter to read the value from
 */
GWENHYWFAR_API double GWEN_Param_GetCurrentValueAsDouble(const GWEN_PARAM *param);

/**
 * Changes the current value of the given param (converted from floating point).
 * The given value is converted to char and stored.
 * @param param parameter to change
 * @param v value to set
 */
GWENHYWFAR_API void GWEN_Param_SetCurrentValueAsDouble(GWEN_PARAM *param, double v);




/**
 * Find param with the given name and return its current value. If there is no value (or no
 * valid value) then a default value is returned.
 * This is for integer values.
 * @return value
 * @param pl list of parameters
 * @param name name of the parameter
 * @param defVal default value (returned if param has no value or no valid value).
 *
 */
GWENHYWFAR_API int GWEN_Param_List_GetCurrentValueAsInt(const GWEN_PARAM_LIST *pl, const char *name, int defVal);

/**
 * Find param with the given name and change its current value. If there is no param of that name is
 * found this function does nothing.
 * This is for integer values.
 * @return value
 * @param pl list of parameters
 * @param name name of the parameter
 * @param v value to set.
 *
 */
GWENHYWFAR_API void GWEN_Param_List_SetCurrentValueAsInt(GWEN_PARAM_LIST *pl, const char *name, int v);

/**
 * Find param with the given name and return its current value. If there is no value (or no
 * valid value) then a default value is returned.
 * This is for floating point values.
 * @return value
 * @param pl list of parameters
 * @param name name of the parameter
 * @param defVal default value (returned if param has no value or no valid value).
 *
 */
GWENHYWFAR_API double GWEN_Param_List_GetCurrentValueAsDouble(const GWEN_PARAM_LIST *pl, const char *name, double defVal);

/**
 * Find param with the given name and change its current value. If there is no param of that name is
 * found this function does nothing.
 * This is for floating point values.
 * @return value
 * @param pl list of parameters
 * @param name name of the parameter
 * @param v value to set.
 *
 */
GWENHYWFAR_API void GWEN_Param_List_SetCurrentValueAsDouble(GWEN_PARAM_LIST *pl, const char *name, double v);



/**
 * Write the list of parameters to XML.
 * @param pl list of parameters to write
 * @param xn XML node to store the parameters
 */
GWENHYWFAR_API void GWEN_Param_List_WriteXml(const GWEN_PARAM_LIST *pl, GWEN_XMLNODE *xn);

/**
 * Read a list of parameters from XML.
 * @param pl list to store parameters read
 * @param xn XML node to read the parameters from
 */
GWENHYWFAR_API void GWEN_Param_List_ReadXml(GWEN_PARAM_LIST *pl, GWEN_XMLNODE *xn);


/**
 * Write the current values of a list of parameters to XML.
 *
 * For every member of the given list an XML node "param" is created below the given XML node
 * which only contains the attribute "name" (parameter name) and the param's "currentValue" (the current value)
 * as data node.
 * Example:
 * @code
 *   <param name="var1">data1</param>
 *   <param name="var2">data2</param>
 * @endcode
 *
 * @param pl list of parameters to write
 * @param xn XML node to store the parameters
 */
GWENHYWFAR_API void GWEN_Param_List_WriteValuesToXml(const GWEN_PARAM_LIST *pl, GWEN_XMLNODE *xn);

/**
 * Read the current values for a list of parameters from XML.
 *
 * The XML tree is expected to have been created by @ref GWEN_Param_List_WriteValuesToXml.
 * Every "param" node below the given XML node is expected to have an attribute "name" with the name of
 * the param and a data node which holds the current value to set.
 * Entries for non-existing params are ignored.
 * @param pl list of parameters to read values for
 * @param xn XML node to read values for the parameters
 */
GWENHYWFAR_API void GWEN_Param_List_ReadValuesFromXml(GWEN_PARAM_LIST *pl, GWEN_XMLNODE *xn);





#endif

