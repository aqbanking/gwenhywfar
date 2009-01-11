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
#include <errno.h>

#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif

#ifdef HAVE_I18N
# include <libintl.h>
# include <locale.h>
#endif


static GWEN_STRINGLIST *gwen_i18n__localelist=0;
static char *gwen_i18n__currentlocale=0;



int GWEN_I18N_ModuleInit(){
  const char *localedir;
  GWEN_STRINGLIST *slist;

  gwen_i18n__localelist=GWEN_StringList_new();

  slist=GWEN_PathManager_GetPaths(GWEN_PM_LIBNAME, GWEN_PM_LOCALEDIR);
  if (slist) {
    if (GWEN_StringList_Count(slist) > 0) {
      int rv;

      localedir=GWEN_StringList_FirstString(slist);
      rv=GWEN_I18N_BindTextDomain_Dir(PACKAGE, localedir);
      if (rv) {
	DBG_WARN(GWEN_LOGDOMAIN, "Could not bind textdomain (%d)", rv);
      }
      else {
	rv=GWEN_I18N_BindTextDomain_Codeset(PACKAGE, "UTF-8");
	if (rv) {
	  DBG_WARN(GWEN_LOGDOMAIN, "Could not set codeset (%d)", rv);
	}
      }

      /* set locale */
      if (GWEN_I18N_SetLocale("")) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Could not set locale");
      }
    }
    else {
      DBG_ERROR(GWEN_LOGDOMAIN, "Empty locale path list");
    }
    GWEN_StringList_free(slist);
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "No locale path list");
  }
  return 0;
}



int GWEN_I18N_ModuleFini(){
  GWEN_StringList_free(gwen_i18n__localelist);
  free(gwen_i18n__currentlocale);
  return 0;
}



int GWEN_I18N_SetLocale(const char *s){
  const char *realLocale;
  char *p;
  char *cs;

  assert(s);

#ifdef HAVE_I18N
  realLocale=setlocale(LC_ALL, s);
  if (realLocale==NULL) {
    DBG_INFO(GWEN_LOGDOMAIN, "Unable to set locale [%s]", s);
    realLocale=s;
  }
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "Real locale is [%s]", realLocale);
  }
#else
  realLocale=s;
#endif

  cs=strdup(realLocale);
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
  gwen_i18n__currentlocale=strdup(realLocale);
  return 0;
}



GWEN_STRINGLIST *GWEN_I18N_GetCurrentLocaleList(){
  return gwen_i18n__localelist;
}



const char *GWEN_I18N_GetCurrentLocale() {
  return gwen_i18n__currentlocale;
}



const char *GWEN_I18N_Translate(const char *textdomain, const char *text) {
#ifdef HAVE_I18N
  const char *p;

  p=strchr(text, '|');
  if (p) {
    const char *s;

    s=dgettext(textdomain, text);
    if (strcmp(s, text)==0)
      return ++p;
  }
  else
    return dgettext(textdomain, text);
#else
  const char *p;

  p=strchr(text, '|');
  if (p)
    return ++p;
  return text;
#endif
}



int GWEN_I18N_BindTextDomain_Dir(const char *textdomain, const char *folder) {
#ifdef HAVE_I18N
  if (NULL==bindtextdomain(textdomain, folder)) {
    DBG_INFO(GWEN_LOGDOMAIN, "bindtextdomain(): %s", strerror(errno));
    return GWEN_ERROR_GENERIC;
  }
  return 0;
#else
  return GWEN_ERROR_NOT_SUPPORTED;
#endif
}



int GWEN_I18N_BindTextDomain_Codeset(const char *textdomain, const char *cs) {
#ifdef HAVE_I18N
  if (NULL==bind_textdomain_codeset(textdomain, cs)) {
    DBG_INFO(GWEN_LOGDOMAIN, "bind_textdomain_codeset(): %s", strerror(errno));
    return GWEN_ERROR_GENERIC;
  }
  return 0;
#else
  return GWEN_ERROR_NOT_SUPPORTED;
#endif
}








