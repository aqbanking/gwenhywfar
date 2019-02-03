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


#ifdef OS_WIN32

struct gwen_i18n_tabletype {
  const char *win_name;
  const char *nls_name;
};


static struct gwen_i18n_tabletype gwen_i18n___localetable[]= {
  { "German_Germany", "de_DE" },
  { "English_UK", "en_GB" },
  { "English_US", "en_US" },
  { "French_France", "fr_FR" },
  { NULL, NULL }
};



static const char *gwen_i18n_transwinlocale(const char *s)
{
  char *cs;
  char *p;
  struct gwen_i18n_tabletype *tt;

  cs=strdup(s);

  /* find complete */
  tt=gwen_i18n___localetable;
  while (tt->win_name) {
    if (strcasecmp(tt->win_name, cs)==0) {
      free(cs);
      return tt->nls_name;
    }
    tt++;
  }

  p=strrchr(cs, '.');
  if (p) {
    *p=0;
    /* find partial string */
    tt=gwen_i18n___localetable;
    while (tt->win_name) {
      if (strcasecmp(tt->win_name, cs)==0) {
        free(cs);
        return tt->nls_name;
      }
      tt++;
    }
  }

  p=strrchr(cs, '_');
  if (p) {
    *p=0;
    /* find partial string */
    tt=gwen_i18n___localetable;
    while (tt->win_name) {
      if (strcasecmp(tt->win_name, cs)==0) {
        free(cs);
        return tt->nls_name;
      }
      tt++;
    }
  }
  free(cs);
  DBG_ERROR(GWEN_LOGDOMAIN, "No translation found for WIN32 locale [%s]", s);
  return s;
}


#endif



int GWEN_I18N_ModuleInit(void)
{
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



int GWEN_I18N_ModuleFini(void)
{
  GWEN_StringList_free(gwen_i18n__localelist);
  free(gwen_i18n__currentlocale);
  return 0;
}



int GWEN_I18N_SetLocale(const char *s)
{
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
#ifdef OS_WIN32
    const char *t;

    t=gwen_i18n_transwinlocale(realLocale);
    DBG_INFO(GWEN_LOGDOMAIN, "Real locale is [%s] (from [%s])", t, realLocale);
    realLocale=t;
#else
    DBG_INFO(GWEN_LOGDOMAIN, "Real locale is [%s]", realLocale);
#endif
  }
#else
  realLocale=s;
#endif

  cs=strdup(realLocale);
  GWEN_StringList_Clear(gwen_i18n__localelist);
  GWEN_StringList_AppendString(gwen_i18n__localelist, cs, 0, 1);

  p=strrchr(cs, '@');
  if (p) {
    *p=0;
    GWEN_StringList_AppendString(gwen_i18n__localelist, cs, 0, 1);
  }
  p=strrchr(cs, '.');
  if (p) {
    *p=0;
    GWEN_StringList_AppendString(gwen_i18n__localelist, cs, 0, 1);
  }

  p=strrchr(cs, '_');
  if (p) {
    *p=0;
    GWEN_StringList_AppendString(gwen_i18n__localelist, cs, 0, 1);
  }
  free(cs);

  free(gwen_i18n__currentlocale);
  gwen_i18n__currentlocale=strdup(realLocale);
  return 0;
}



GWEN_STRINGLIST *GWEN_I18N_GetCurrentLocaleList(void)
{
  return gwen_i18n__localelist;
}



const char *GWEN_I18N_GetCurrentLocale(void)
{
  return gwen_i18n__currentlocale;
}



const char *GWEN_I18N_Translate(const char *txtdom, const char *text)
{
#ifdef HAVE_I18N
  const char *p;

  p=strchr(text, '|');
  if (p) {
    const char *s;

    s=dgettext(txtdom, text);
    if (strcmp(s, text)==0)
      return ++p;
    else
      return s;
  }
  else
    return dgettext(txtdom, text);
#else
  const char *p;

  p=strchr(text, '|');
  if (p)
    return ++p;
  return text;
#endif
}



int GWEN_I18N_BindTextDomain_Dir(const char *txtdom, const char *folder)
{
#ifdef HAVE_I18N
  if (NULL==bindtextdomain(txtdom, folder)) {
    DBG_INFO(GWEN_LOGDOMAIN, "bindtextdomain(): %s", strerror(errno));
    return GWEN_ERROR_GENERIC;
  }
  return 0;
#else
  return GWEN_ERROR_NOT_SUPPORTED;
#endif
}



int GWEN_I18N_BindTextDomain_Codeset(const char *txtdom, const char *cs)
{
#ifdef HAVE_I18N
  if (NULL==bind_textdomain_codeset(txtdom, cs)) {
    DBG_INFO(GWEN_LOGDOMAIN, "bind_textdomain_codeset(): %s", strerror(errno));
    return GWEN_ERROR_GENERIC;
  }
  return 0;
#else
  return GWEN_ERROR_NOT_SUPPORTED;
#endif
}








