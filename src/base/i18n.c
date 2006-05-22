/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Fri Sep 12 2003
    copyright   : (C) 2003 by Martin Preuss
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


#include "i18n_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/gwenhywfar.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#ifdef ENABLE_NLS
# include <libintl.h>
# include <locale.h>
#endif


static GWEN_STRINGLIST *gwen_i18n__localelist=0;
static char *gwen_i18n__currentlocale=0;



GWEN_ERRORCODE GWEN_I18N_ModuleInit(){
  const char *s;
  const char *localedir;
  GWEN_STRINGLIST *slist;

  gwen_i18n__localelist=GWEN_StringList_new();

  slist = GWEN_PathManager_GetPaths(GWEN_PM_LIBNAME, GWEN_PM_LOCALEDIR);
  assert(GWEN_StringList_Count(slist) > 0);
  localedir = GWEN_StringList_FirstString(slist);
#ifdef HAVE_I18N
  s = setlocale(LC_ALL,"");
  if (bindtextdomain(PACKAGE, localedir)==0) {
    DBG_WARN(GWEN_LOGDOMAIN, " Error bindtextdomain()\n");
  } else {
    DBG_DEBUG(GWEN_LOGDOMAIN, "Textdomain bound.");
    bind_textdomain_codeset(PACKAGE, "UTF-8");
  }
#else
  s="C";
#endif
  if (s) {
    if (GWEN_I18N_SetLocale(s)) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Could not set locale");
    }
  }
  GWEN_StringList_free(slist);
  return 0;
}



GWEN_ERRORCODE GWEN_I18N_ModuleFini(){
  GWEN_StringList_free(gwen_i18n__localelist);
  free(gwen_i18n__currentlocale);
  return 0;
}



int GWEN_I18N_SetLocale(const char *s){
  char *p;
  char *cs;

  assert(s);
  cs=strdup(s);
  GWEN_StringList_Clear(gwen_i18n__localelist);
  GWEN_StringList_AppendString(gwen_i18n__localelist, cs, 0, 1);
  /*fprintf(stderr, "Appending locale \"%s\"\n", cs);*/
  p=strrchr(cs, '@');
  if (p) {
    *p=0;
    GWEN_StringList_AppendString(gwen_i18n__localelist, cs, 0, 1);
    /*fprintf(stderr, "Appending locale \"%s\"\n", cs);*/
  }
  p=strrchr(cs, '_');
  if (p) {
    *p=0;
    GWEN_StringList_AppendString(gwen_i18n__localelist, cs, 0, 1);
    /*fprintf(stderr, "Appending locale \"%s\"\n", cs);*/
  }
  free(cs);

  free(gwen_i18n__currentlocale);
  gwen_i18n__currentlocale=strdup(s);
#ifdef ENABLE_NLS
  setlocale(LC_ALL, s);
#endif
  return 0;
}



GWEN_STRINGLIST *GWEN_I18N_GetCurrentLocaleList(){
  return gwen_i18n__localelist;
}



const char *GWEN_I18N_GetCurrentLocale() {
  return gwen_i18n__currentlocale;
}








