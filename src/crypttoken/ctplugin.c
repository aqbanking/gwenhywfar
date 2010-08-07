/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: crypttoken.h 1113 2007-01-10 09:14:16Z martin $
    begin       : Wed Mar 16 2005
    copyright   : (C) 2005 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "ctplugin_p.h"
#include "i18n_l.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/pathmanager.h>


#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif



GWEN_INHERIT(GWEN_PLUGIN, GWEN_CRYPT_TOKEN_PLUGIN)




int GWEN_Crypt_Token_ModuleInit(){
  GWEN_PLUGIN_MANAGER *pm;
  int err;
  GWEN_STRINGLIST *sl;

  pm=GWEN_PluginManager_new(GWEN_CRYPT_TOKEN_PLUGIN_TYPENAME, GWEN_PM_LIBNAME);
  err=GWEN_PluginManager_Register(pm);
  if (err) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not register CryptToken plugin manager");
    return err;
  }

  /* create plugin paths */
  sl=GWEN_PathManager_GetPaths(GWEN_PM_LIBNAME, GWEN_PM_PLUGINDIR);
  if (sl) {
    GWEN_STRINGLISTENTRY *se;
    GWEN_BUFFER *pbuf;

    pbuf=GWEN_Buffer_new(0, 256, 0, 1);

    se=GWEN_StringList_FirstEntry(sl);
    while(se) {
      GWEN_Buffer_AppendString(pbuf, GWEN_StringListEntry_Data(se));
      GWEN_Buffer_AppendString(pbuf, DIRSEP GWEN_CRYPT_TOKEN_FOLDER);
      DBG_INFO(GWEN_LOGDOMAIN, "Adding plugin path [%s]",
	       GWEN_Buffer_GetStart(pbuf));
      GWEN_PluginManager_AddPath(pm, GWEN_PM_LIBNAME,
				 GWEN_Buffer_GetStart(pbuf));
      GWEN_Buffer_Reset(pbuf);
      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_Buffer_free(pbuf);
    GWEN_StringList_free(sl);
  }

  return 0;
}



int GWEN_Crypt_Token_ModuleFini(){
  GWEN_PLUGIN_MANAGER *pm;

  pm=GWEN_PluginManager_FindPluginManager(GWEN_CRYPT_TOKEN_PLUGIN_TYPENAME);
  if (pm) {
    int rv;

    rv=GWEN_PluginManager_Unregister(pm);
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Could not unregister CryptToken plugin manager (%d)", rv);
      return rv;
    }
    else
      GWEN_PluginManager_free(pm);
  }

  return 0;
}











GWEN_PLUGIN *GWEN_Crypt_Token_Plugin_new(GWEN_PLUGIN_MANAGER *mgr,
					 GWEN_CRYPT_TOKEN_DEVICE devType,
					 const char *typeName,
					 const char *fileName) {
  GWEN_PLUGIN *pl;
  GWEN_CRYPT_TOKEN_PLUGIN *xpl;

  pl=GWEN_Plugin_new(mgr, typeName, fileName);
  GWEN_NEW_OBJECT(GWEN_CRYPT_TOKEN_PLUGIN, xpl);
  GWEN_INHERIT_SETDATA(GWEN_PLUGIN, GWEN_CRYPT_TOKEN_PLUGIN, pl, xpl, GWEN_Crypt_Token_Plugin_FreeData);
  xpl->devType=devType;

  return pl;
}



GWENHYWFAR_CB
void GWEN_Crypt_Token_Plugin_FreeData(GWEN_UNUSED void *bp, void *p) {
  GWEN_CRYPT_TOKEN_PLUGIN *xpl;

  xpl=(GWEN_CRYPT_TOKEN_PLUGIN*)p;

  GWEN_FREE_OBJECT(xpl);
}



GWEN_CRYPT_TOKEN *GWEN_Crypt_Token_Plugin_CreateToken(GWEN_PLUGIN *pl, const char *name) {
  GWEN_CRYPT_TOKEN_PLUGIN *xpl;

  assert(pl);
  xpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_CRYPT_TOKEN_PLUGIN, pl);
  assert(xpl);

  if (xpl->createTokenFn)
    return xpl->createTokenFn(pl, name);
  else {
    DBG_WARN(GWEN_LOGDOMAIN, "No createTokenFn");
    return NULL;
  }
}



int GWEN_Crypt_Token_Plugin_CheckToken(GWEN_PLUGIN *pl, GWEN_BUFFER *name) {
  GWEN_CRYPT_TOKEN_PLUGIN *xpl;

  assert(pl);
  xpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_CRYPT_TOKEN_PLUGIN, pl);
  assert(xpl);

  if (xpl->checkTokenFn)
    return xpl->checkTokenFn(pl, name);
  else {
    DBG_INFO(GWEN_LOGDOMAIN, "No checkTokenFn");
    return GWEN_ERROR_NOT_IMPLEMENTED;
  }
}



GWEN_CRYPT_TOKEN_DEVICE GWEN_Crypt_Token_Plugin_GetDeviceType(const GWEN_PLUGIN *pl) {
  GWEN_CRYPT_TOKEN_PLUGIN *xpl;

  assert(pl);
  xpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_CRYPT_TOKEN_PLUGIN, pl);
  assert(xpl);

  return xpl->devType;
}



GWEN_CRYPT_TOKEN_PLUGIN_CREATETOKEN_FN GWEN_Crypt_Token_Plugin_SetCreateTokenFn(GWEN_PLUGIN *pl,
										GWEN_CRYPT_TOKEN_PLUGIN_CREATETOKEN_FN fn) {
  GWEN_CRYPT_TOKEN_PLUGIN *xpl;
  GWEN_CRYPT_TOKEN_PLUGIN_CREATETOKEN_FN of;

  assert(pl);
  xpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_CRYPT_TOKEN_PLUGIN, pl);
  assert(xpl);

  of=xpl->createTokenFn;
  xpl->createTokenFn=fn;

  return of;
}



GWEN_CRYPT_TOKEN_PLUGIN_CHECKTOKEN_FN GWEN_Crypt_Token_Plugin_SetCheckTokenFn(GWEN_PLUGIN *pl,
									      GWEN_CRYPT_TOKEN_PLUGIN_CHECKTOKEN_FN fn) {
  GWEN_CRYPT_TOKEN_PLUGIN *xpl;
  GWEN_CRYPT_TOKEN_PLUGIN_CHECKTOKEN_FN of;

  assert(pl);
  xpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, GWEN_CRYPT_TOKEN_PLUGIN, pl);
  assert(xpl);

  of=xpl->checkTokenFn;
  xpl->checkTokenFn=fn;

  return of;
}










GWEN_PLUGIN_DESCRIPTION_LIST2 *GWEN_Crypt_Token_PluginManager_GetPluginDescrs(GWEN_PLUGIN_MANAGER *pm,
									      GWEN_CRYPT_TOKEN_DEVICE devt) {
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pl1;

  pl1=GWEN_PluginManager_GetPluginDescrs(pm);
  if (pl1) {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pit;
    GWEN_PLUGIN_DESCRIPTION_LIST2 *pl2;

    pl2=GWEN_PluginDescription_List2_new();
    pit=GWEN_PluginDescription_List2_First(pl1);
    if (pit) {
      GWEN_PLUGIN_DESCRIPTION *pd;
      const char *ts;

      if (devt==GWEN_Crypt_Token_Device_Any)
	ts=NULL;
      else
	ts=GWEN_Crypt_Token_Device_toString(devt);
      pd=GWEN_PluginDescription_List2Iterator_Data(pit);
      while(pd) {
	GWEN_XMLNODE *node;
	const char *nts;
        int match=0;

        node=GWEN_PluginDescription_GetXmlNode(pd);
        assert(node);
	nts=GWEN_XMLNode_GetProperty(node, "device", 0);
	if (nts) {
          if (!ts || (ts && strcasecmp(ts, nts)==0))
            match=1;
        }
        else if (!ts)
          match=1;

        if (match) {
          GWEN_PLUGIN_DESCRIPTION *pd2;

          pd2=GWEN_PluginDescription_dup(pd);
          GWEN_PluginDescription_List2_PushBack(pl2, pd2);
        }

        pd=GWEN_PluginDescription_List2Iterator_Next(pit);
      }
      GWEN_PluginDescription_List2Iterator_free(pit);
    }
    GWEN_PluginDescription_List2_freeAll(pl1);

    if (GWEN_PluginDescription_List2_GetSize(pl2)==0) {
      GWEN_PluginDescription_List2_freeAll(pl2);
      DBG_ERROR(GWEN_LOGDOMAIN,
                "No matching plugin descriptions for the given device type");
      return NULL;
    }
    return pl2;
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "No plugin descriptions at all");
  }
  return NULL;
}



int GWEN_Crypt_Token_PluginManager_CheckToken(GWEN_PLUGIN_MANAGER *pm,
					      GWEN_CRYPT_TOKEN_DEVICE devt,
					      GWEN_BUFFER *typeName,
					      GWEN_BUFFER *tokenName,
					      uint32_t guiid) {
  GWEN_PLUGIN_DESCRIPTION_LIST2 *pdl;

  assert(pm);

  pdl=GWEN_Crypt_Token_PluginManager_GetPluginDescrs(pm, devt);
  if (pdl==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No plugin descriptions found for this device type");
    GWEN_Gui_ProgressLog(guiid,
			 GWEN_LoggerLevel_Error,
			 I18N("No plugin found for this device type"));
    GWEN_Gui_ProgressLog(guiid,
			 GWEN_LoggerLevel_Error,
			 I18N("If you're using a Debian/Ubuntu based system "
				"please consider to install package " 
				LIBCHIPCARD_GWENHYWFAR_PLUGIN_PACKAGE));
    return GWEN_ERROR_NOT_FOUND;
  }
  else {
    GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *pit;

    pit=GWEN_PluginDescription_List2_First(pdl);
    if (pit) {
      GWEN_PLUGIN_DESCRIPTION *pd;
      uint32_t progressId;
      unsigned int pdcount;
      unsigned int cnt=0;

      pdcount=GWEN_PluginDescription_List2_GetSize(pdl);
      progressId=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
					GWEN_GUI_PROGRESS_ALLOW_EMBED |
					GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                                        GWEN_GUI_PROGRESS_SHOW_LOG |
					GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG |
					GWEN_GUI_PROGRESS_SHOW_ABORT,
					I18N("Determining plugin module..."),
					NULL,
					pdcount,
					guiid);

      pd=GWEN_PluginDescription_List2Iterator_Data(pit);
      assert(pd);
      while(pd) {
	GWEN_XMLNODE *n;
	int err;
	GWEN_PLUGIN *pl;
	char logbuffer[256];

	n=GWEN_PluginDescription_GetXmlNode(pd);
	assert(n);

	snprintf(logbuffer, sizeof(logbuffer)-1,
		 I18N("Loading plugin \"%s\""),
		 GWEN_PluginDescription_GetName(pd));
	logbuffer[sizeof(logbuffer)-1]=0;
	GWEN_Gui_ProgressLog(progressId,
			     GWEN_LoggerLevel_Notice,
			     logbuffer);

	/* device type matches, check this plugin */
	pl=GWEN_PluginManager_GetPlugin(pm, GWEN_PluginDescription_GetName(pd));
	if (pl) {
	  GWEN_BUFFER *lTokenName;
	  int rv;

	  lTokenName=GWEN_Buffer_dup(tokenName);

	  snprintf(logbuffer, sizeof(logbuffer)-1,
		   I18N("Checking plugin \"%s\""),
		   GWEN_Plugin_GetName(pl));
	  logbuffer[sizeof(logbuffer)-1]=0;
	  GWEN_Gui_ProgressLog(progressId,
			       GWEN_LoggerLevel_Notice,
			       logbuffer);

	  DBG_INFO(GWEN_LOGDOMAIN,
		   "Checking plugin \"%s\" for [%s]",
		   GWEN_Plugin_GetName(pl),
		   GWEN_Buffer_GetStart(lTokenName));

	  rv=GWEN_Crypt_Token_Plugin_CheckToken(pl, lTokenName);
	  switch(rv) {
	  case 0:
	    /* responsive plugin found */
	    snprintf(logbuffer, sizeof(logbuffer)-1,
		     I18N("Plugin \"%s\" supports this token"),
		     GWEN_Plugin_GetName(pl));
	    logbuffer[sizeof(logbuffer)-1]=0;
	    err=GWEN_Gui_ProgressLog(progressId,
				     GWEN_LoggerLevel_Notice,
				     logbuffer);
	    if (err==GWEN_ERROR_USER_ABORTED) {
	      GWEN_Gui_ProgressEnd(progressId);
	      GWEN_Buffer_free(lTokenName);
	      GWEN_PluginDescription_List2Iterator_free(pit);
	      GWEN_PluginDescription_List2_freeAll(pdl);
	      return err;
	    }

	    GWEN_Buffer_Reset(typeName);
	    GWEN_Buffer_AppendString(typeName, GWEN_Plugin_GetName(pl));
	    GWEN_Buffer_Reset(tokenName);
	    GWEN_Buffer_AppendBuffer(tokenName, lTokenName);
	    GWEN_Buffer_free(lTokenName);
	    GWEN_PluginDescription_List2Iterator_free(pit);
	    GWEN_PluginDescription_List2_freeAll(pdl);
	    GWEN_Gui_ProgressEnd(progressId);
	    return 0;

	  case GWEN_ERROR_NOT_IMPLEMENTED:
	    snprintf(logbuffer, sizeof(logbuffer)-1,
		     I18N("Plugin \"%s\": Function not implemented"),
		     GWEN_Plugin_GetName(pl));
	    logbuffer[sizeof(logbuffer)-1]=0;
	    GWEN_Gui_ProgressLog(progressId,
				 GWEN_LoggerLevel_Notice,
				 logbuffer);
	    break;

	  case GWEN_ERROR_NOT_SUPPORTED:
	    snprintf(logbuffer, sizeof(logbuffer)-1,
		     I18N("Plugin \"%s\" does not support this token"),
		     GWEN_Plugin_GetName(pl));
	    logbuffer[sizeof(logbuffer)-1]=0;
	    GWEN_Gui_ProgressLog(progressId,
				 GWEN_LoggerLevel_Info,
				 logbuffer);
	    break;

	  case GWEN_ERROR_BAD_NAME:
	    snprintf(logbuffer, sizeof(logbuffer)-1,
		     I18N("Plugin \"%s\" supports this token, but the name "
			  "did not match"),
		     GWEN_Plugin_GetName(pl));
	    logbuffer[sizeof(logbuffer)-1]=0;
	    GWEN_Gui_ProgressLog(progressId,
				 GWEN_LoggerLevel_Info,
				 logbuffer);
	    break;

	  default:
	    snprintf(logbuffer, sizeof(logbuffer)-1,
		     I18N("Plugin \"%s\": Unexpected error (%d)"),
		     GWEN_Plugin_GetName(pl), rv);
	    logbuffer[sizeof(logbuffer)-1]=0;
	    GWEN_Gui_ProgressLog(progressId,
				 GWEN_LoggerLevel_Info,
				 logbuffer);
	    break;
	  } /* switch */
	} /* if plugin loaded */
	else {
	  snprintf(logbuffer, sizeof(logbuffer)-1,
		   I18N("Could not load plugin \"%s\""),
		   GWEN_PluginDescription_GetName(pd));
	  logbuffer[sizeof(logbuffer)-1]=0;
	  GWEN_Gui_ProgressLog(progressId,
			       GWEN_LoggerLevel_Warning,
			       logbuffer);
	}

	cnt++;
	err=GWEN_Gui_ProgressAdvance(progressId, cnt);
	if (err) {
	  DBG_INFO(GWEN_LOGDOMAIN, "User aborted");
	  GWEN_Gui_ProgressEnd(progressId);
	  GWEN_PluginDescription_List2Iterator_free(pit);
	  GWEN_PluginDescription_List2_freeAll(pdl);
	  GWEN_Gui_ProgressEnd(progressId);
	  return err;
	}

	pd=GWEN_PluginDescription_List2Iterator_Next(pit);
      }

      GWEN_Gui_ProgressEnd(progressId);
      GWEN_PluginDescription_List2Iterator_free(pit);
    }
    GWEN_PluginDescription_List2_freeAll(pdl);
  }

  return GWEN_ERROR_NOT_SUPPORTED;
}









