/***************************************************************************
    begin       : Wed Feb 15 2022
    copyright   : (C) 2022 by Ralf Habacker
    email       : ralf.habacker@freenet.de

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


#include "funcs.h"

#include <stdio.h>

int GWEN_Funcs_Has_Call(const GWEN_FUNCS *func)
{
  return func->func1!=NULL;
}

int GWEN_Funcs_Has_Call_Args(const GWEN_FUNCS *func)
{
  return func->func2!=NULL;
}

int GWEN_Funcs_Has_Call_DB_NODE_Args(const GWEN_FUNCS *func)
{
  return func->func3!=NULL;
}

int GWEN_Funcs_Call(const GWEN_FUNCS *func)
{
   return func->func1();
}

int GWEN_Funcs_Call_Args(const GWEN_FUNCS *func, int argc, char **argv)
{
  return func->func2(argc, argv);
}

int GWEN_Funcs_Call_DB_NODE_Args(const GWEN_FUNCS *func, GWEN_DB_NODE *node, int argc, char **argv)
{
  return func->func3(node, argc, argv);
}

/**
 * This function prints out a space separated list of all defined functions
 */
void GWEN_Funcs_Usage(const GWEN_FUNCS *funcs)
{
    const GWEN_FUNCS *p;

    for (p=funcs; p->name; p++) {
      fprintf(stderr, " %s", p->name);
      if (p->description)
        fprintf(stderr, " (%s)", p->description);
    }
    fprintf(stderr, "\n");
}

/**
 * This function prints out list of all defined functions including the description
 */
void GWEN_Funcs_Usage_With_Help(const GWEN_FUNCS *funcs)
{
    const GWEN_FUNCS *p;

    for (p=funcs; p->name; p++) {
      fprintf(stderr, "  %s:\n    %s\n\n", p->name, p->description ? p->description : "");
    }
}

/**
 * This function returns a pointer to the function identified by \p name
 * or NULL if function has not been found.
 */
const GWEN_FUNCS* GWEN_Funcs_Find(const GWEN_FUNCS *funcs, const char *name)
{
  const GWEN_FUNCS *p;

  for (p=funcs; p->name; p++) {
    if (strcasecmp(name, p->name)==0)
      return p;
  }
  return NULL;
}



