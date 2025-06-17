/***************************************************************************
 begin       : Fri Feb 07 2003
 copyright   : (C) 2003-2010 by Martin Preuss
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

#define DISABLE_DEBUGLOG


#include "nogui_p.h"




GWEN_INHERIT(GWEN_GUI, GWEN_GUI_NOGUI)



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _freeData(GWEN_UNUSED void *bp, void *p);

static int GWENHYWFAR_CB _messageBox(GWEN_GUI *gui,
                                     uint32_t flags,
                                     const char *title,
                                     const char *text,
                                     const char *b1,
                                     const char *b2,
                                     const char *b3,
                                     uint32_t guiid);

static int GWENHYWFAR_CB _inputBox(GWEN_GUI *gui,
                                   uint32_t flags,
                                   const char *title,
                                   const char *text,
                                   char *buffer,
                                   int minLen,
                                   int maxLen,
                                   uint32_t guiid);


static uint32_t GWENHYWFAR_CB _showBox(GWEN_GUI *gui,
                                       uint32_t flags,
                                       const char *title,
                                       const char *text,
                                       uint32_t guiid);
static void GWENHYWFAR_CB _hideBox(GWEN_GUI *gui, uint32_t id);


static uint32_t GWENHYWFAR_CB _progressStart(GWEN_GUI *gui,
                                             uint32_t progressFlags,
                                             const char *title,
                                             const char *text,
                                             uint64_t total,
                                             uint32_t guiid);
static int GWENHYWFAR_CB _progressAdvance(GWEN_GUI *gui, uint32_t pid, uint64_t progress);
static int GWENHYWFAR_CB _progressSetTotal(GWEN_GUI *gui, uint32_t pid, uint64_t total);
static int GWENHYWFAR_CB _progressLog(GWEN_GUI *gui, uint32_t pid, GWEN_LOGGER_LEVEL level, const char *text);
static int GWENHYWFAR_CB _progressEnd(GWEN_GUI *gui, uint32_t pid);

static int GWENHYWFAR_CB _print(GWEN_GUI *gui,
                                const char *docTitle,
                                const char *docType,
                                const char *descr,
                                const char *text,
                                uint32_t guiid);


static int GWENHYWFAR_CB _getPassword(GWEN_GUI *gui,
                                      uint32_t flags,
                                      const char *token,
                                      const char *title,
                                      const char *text,
                                      char *buffer,
                                      int minLen,
                                      int maxLen,
                                      GWEN_GUI_PASSWORD_METHOD methodId,
                                      GWEN_DB_NODE *methodParams,
                                      uint32_t guiid);

static int GWENHYWFAR_CB _setPasswordStatus(GWEN_GUI *gui,
                                            const char *token,
                                            const char *pin,
                                            GWEN_GUI_PASSWORD_STATUS status,
                                            GWEN_UNUSED uint32_t guiid);

static int GWENHYWFAR_CB _logHook(GWEN_GUI *gui, const char *logDomain, GWEN_LOGGER_LEVEL priority, const char *s);


static int GWENHYWFAR_CB _waitForSockets(GWEN_GUI *gui,
                                         GWEN_SOCKET_LIST2 *readSockets,
                                         GWEN_SOCKET_LIST2 *writeSockets,
                                         int msecs,
                                         uint32_t guiid);

static int GWENHYWFAR_CB _checkCert(GWEN_GUI *gui,
                                    const GWEN_SSLCERTDESCR *cert,
                                    GWEN_SYNCIO *sio,
                                    uint32_t guiid);

static int GWENHYWFAR_CB _keyDataFromTextOpenSsl(GWEN_GUI *gui,
                                                 const char *text,
                                                 unsigned char *buffer,
                                                 unsigned int bufLength);


static int GWENHYWFAR_CB _execDialog(GWEN_GUI *gui,
                                     GWEN_DIALOG *dlg,
                                     uint32_t guiid);


static int GWENHYWFAR_CB _openDialog(GWEN_GUI *gui,
                                     GWEN_DIALOG *dlg,
                                     uint32_t guiid);

static int GWENHYWFAR_CB _closeDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg);

static int GWENHYWFAR_CB _runDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, int timeout);


static int GWENHYWFAR_CB _readDialogPrefs(GWEN_GUI *gui,
                                          const char *groupName,
                                          const char *altName,
                                          GWEN_DB_NODE **pDb);

static int GWENHYWFAR_CB _writeDialogPrefs(GWEN_GUI *gui,
                                           const char *groupName,
                                           GWEN_DB_NODE *db);


static int GWENHYWFAR_CB _getFilename(GWEN_GUI *gui,
                                      const char *caption,
                                      GWEN_GUI_FILENAME_TYPE fnt,
                                      uint32_t flags,
                                      const char *patterns,
                                      GWEN_BUFFER *pathBuffer,
                                      uint32_t guiid);


static int GWENHYWFAR_CB _getSyncio(GWEN_GUI *gui,
                                    const char *url,
                                    const char *defaultProto,
                                    int defaultPort,
                                    GWEN_SYNCIO **pSio);



/* ------------------------------------------------------------------------------------------------
 * code
 * ------------------------------------------------------------------------------------------------
 */




GWEN_GUI *GWEN_NoGui_new(void)
{
  GWEN_GUI *gui;
  GWEN_GUI_NOGUI *xgui;

  gui=GWEN_Gui_new();
  GWEN_NEW_OBJECT(GWEN_GUI_NOGUI, xgui);

  GWEN_INHERIT_SETDATA(GWEN_GUI, GWEN_GUI_NOGUI, gui, xgui, _freeData);

  xgui->messageBoxFn=GWEN_Gui_SetMessageBoxFn(gui, _messageBox);
  xgui->inputBoxFn=GWEN_Gui_SetInputBoxFn(gui, _inputBox);
  xgui->showBoxFn=GWEN_Gui_SetShowBoxFn(gui, _showBox);
  xgui->hideBoxFn=GWEN_Gui_SetHideBoxFn(gui, _hideBox);
  xgui->progressStartFn=GWEN_Gui_SetProgressStartFn(gui, _progressStart);
  xgui->progressAdvanceFn=GWEN_Gui_SetProgressAdvanceFn(gui, _progressAdvance);
  xgui->progressSetTotalFn=GWEN_Gui_SetProgressSetTotalFn(gui, _progressSetTotal);
  xgui->progressLogFn=GWEN_Gui_SetProgressLogFn(gui, _progressLog);
  xgui->progressEndFn=GWEN_Gui_SetProgressEndFn(gui, _progressEnd);
  xgui->printFn=GWEN_Gui_SetPrintFn(gui, _print);
  xgui->getPasswordFn=GWEN_Gui_SetGetPasswordFn(gui, _getPassword);
  xgui->setPasswordStatusFn=GWEN_Gui_SetSetPasswordStatusFn(gui, _setPasswordStatus);
  xgui->logHookFn=GWEN_Gui_SetLogHookFn(gui, _logHook);
  xgui->waitForSocketsFn=GWEN_Gui_SetWaitForSocketsFn(gui, _waitForSockets);
  xgui->checkCertFn=GWEN_Gui_SetCheckCertFn(gui, _checkCert);
  xgui->keyDataFromTextOpenSslFn=GWEN_Gui_SetKeyDataFromTextOpenSslFn(gui, _keyDataFromTextOpenSsl);
  xgui->execDialogFn=GWEN_Gui_SetExecDialogFn(gui, _execDialog);
  xgui->openDialogFn=GWEN_Gui_SetOpenDialogFn(gui, _openDialog);
  xgui->closeDialogFn=GWEN_Gui_SetCloseDialogFn(gui, _closeDialog);
  xgui->runDialogFn=GWEN_Gui_SetRunDialogFn(gui, _runDialog);
  xgui->readDialogPrefsFn=GWEN_Gui_SetReadDialogPrefsFn(gui, _readDialogPrefs);
  xgui->writeDialogPrefsFn=GWEN_Gui_SetWriteDialogPrefsFn(gui, _writeDialogPrefs);
  xgui->getFileNameFn=GWEN_Gui_SetGetFileNameFn(gui, _getFilename);
  xgui->getSyncIoFn=GWEN_Gui_SetGetSyncIoFn(gui, _getSyncio);

  return gui;
}



void GWENHYWFAR_CB _freeData(GWEN_UNUSED void *bp, void *p)
{
  GWEN_GUI_NOGUI *xgui;

  xgui=(GWEN_GUI_NOGUI*) p;

  GWEN_FREE_OBJECT(xgui);
}



int GWENHYWFAR_CB _messageBox(GWEN_UNUSED GWEN_GUI *gui,
                GWEN_UNUSED uint32_t flags,
                GWEN_UNUSED const char *title,
                GWEN_UNUSED const char *text,
                GWEN_UNUSED const char *b1,
                GWEN_UNUSED const char *b2,
                GWEN_UNUSED const char *b3,
                GWEN_UNUSED uint32_t guiid)
{
  return 0;
}



int GWENHYWFAR_CB _inputBox(GWEN_UNUSED GWEN_GUI *gui,
              GWEN_UNUSED uint32_t flags,
              GWEN_UNUSED const char *title,
              GWEN_UNUSED const char *text,
              GWEN_UNUSED char *buffer,
              GWEN_UNUSED int minLen,
              GWEN_UNUSED int maxLen,
              GWEN_UNUSED uint32_t guiid)
{
  return 0;
}



uint32_t GWENHYWFAR_CB _showBox(GWEN_UNUSED GWEN_GUI *gui,
                  GWEN_UNUSED uint32_t flags,
                  GWEN_UNUSED const char *title,
                  GWEN_UNUSED const char *text,
                  GWEN_UNUSED uint32_t guiid)
{
  return 0;
}



void GWENHYWFAR_CB _hideBox(GWEN_UNUSED GWEN_GUI *gui, GWEN_UNUSED uint32_t id)
{
}



uint32_t GWENHYWFAR_CB _progressStart(GWEN_UNUSED GWEN_GUI *gui,
                        GWEN_UNUSED uint32_t progressFlags,
                        GWEN_UNUSED const char *title,
                        GWEN_UNUSED const char *text,
                        GWEN_UNUSED uint64_t total,
                        GWEN_UNUSED uint32_t guiid)
{
  return 0;
}



int GWENHYWFAR_CB _progressAdvance(GWEN_UNUSED GWEN_GUI *gui,
                     GWEN_UNUSED uint32_t pid,
                     GWEN_UNUSED uint64_t progress)
{
  return 0;
}



int GWENHYWFAR_CB _progressSetTotal(GWEN_UNUSED GWEN_GUI *gui,
                      GWEN_UNUSED uint32_t pid,
                      GWEN_UNUSED uint64_t total)
{
  return 0;
}



int GWENHYWFAR_CB _progressLog(GWEN_UNUSED GWEN_GUI *gui,
                 GWEN_UNUSED uint32_t pid,
                 GWEN_UNUSED GWEN_LOGGER_LEVEL level,
                 GWEN_UNUSED const char *text)
{
  return 0;
}



int GWENHYWFAR_CB _progressEnd(GWEN_UNUSED GWEN_GUI *gui,
                 GWEN_UNUSED uint32_t pid)
{
  return 0;
}



int GWENHYWFAR_CB _print(GWEN_UNUSED GWEN_GUI *gui,
           GWEN_UNUSED const char *docTitle,
           GWEN_UNUSED const char *docType,
           GWEN_UNUSED const char *descr,
           GWEN_UNUSED const char *text,
           GWEN_UNUSED uint32_t guiid)
{
  return 0;
}



int GWENHYWFAR_CB _getPassword(GWEN_UNUSED GWEN_GUI *gui,
                 GWEN_UNUSED uint32_t flags,
                 GWEN_UNUSED const char *token,
                 GWEN_UNUSED const char *title,
                 GWEN_UNUSED const char *text,
                 GWEN_UNUSED char *buffer,
                 GWEN_UNUSED int minLen,
                 GWEN_UNUSED int maxLen,
                 GWEN_UNUSED GWEN_GUI_PASSWORD_METHOD methodId,
                 GWEN_UNUSED GWEN_DB_NODE *methodParams,
                 GWEN_UNUSED uint32_t guiid)
{
  return 0;
}



int GWENHYWFAR_CB _setPasswordStatus(GWEN_UNUSED GWEN_GUI *gui,
                       GWEN_UNUSED const char *token,
                       GWEN_UNUSED const char *pin,
                       GWEN_UNUSED GWEN_GUI_PASSWORD_STATUS status,
                       GWEN_UNUSED uint32_t guiid)
{
  return 0;
}



int GWENHYWFAR_CB _logHook(GWEN_UNUSED GWEN_GUI *gui,
             GWEN_UNUSED const char *logDomain,
             GWEN_UNUSED GWEN_LOGGER_LEVEL priority,
             GWEN_UNUSED const char *s)
{
  return 0;
}



int GWENHYWFAR_CB _waitForSockets(GWEN_UNUSED GWEN_GUI *gui,
                    GWEN_UNUSED GWEN_SOCKET_LIST2 *readSockets,
                    GWEN_UNUSED GWEN_SOCKET_LIST2 *writeSockets,
                    GWEN_UNUSED int msecs,
                    GWEN_UNUSED uint32_t guiid)
{
  return 0;
}



int GWENHYWFAR_CB _checkCert(GWEN_UNUSED GWEN_GUI *gui,
               GWEN_UNUSED const GWEN_SSLCERTDESCR *cert,
               GWEN_UNUSED GWEN_SYNCIO *sio,
               GWEN_UNUSED uint32_t guiid)
{
  return 0;
}



int GWENHYWFAR_CB _keyDataFromTextOpenSsl(GWEN_UNUSED GWEN_GUI *gui,
                            GWEN_UNUSED const char *text,
                            GWEN_UNUSED unsigned char *buffer,
                            GWEN_UNUSED unsigned int bufLength)
{
  return 0;
}



int GWENHYWFAR_CB _execDialog(GWEN_UNUSED GWEN_GUI *gui,
                GWEN_UNUSED GWEN_DIALOG *dlg,
                GWEN_UNUSED uint32_t guiid)
{
  return 0;
}



int GWENHYWFAR_CB _openDialog(GWEN_UNUSED GWEN_GUI *gui,
                GWEN_UNUSED GWEN_DIALOG *dlg,
                GWEN_UNUSED uint32_t guiid)
{
  return 0;
}



int GWENHYWFAR_CB _closeDialog(GWEN_UNUSED GWEN_GUI *gui,
                 GWEN_UNUSED GWEN_DIALOG *dlg)
{
  return 0;
}



int GWENHYWFAR_CB _runDialog(GWEN_UNUSED GWEN_GUI *gui,
               GWEN_UNUSED GWEN_DIALOG *dlg,
               GWEN_UNUSED int timeout)
{
  return 0;
}



int GWENHYWFAR_CB _readDialogPrefs(GWEN_UNUSED GWEN_GUI *gui,
                     GWEN_UNUSED const char *groupName,
                     GWEN_UNUSED const char *altName,
                     GWEN_UNUSED GWEN_DB_NODE **pDb)
{
  return 0;
}



int GWENHYWFAR_CB _writeDialogPrefs(GWEN_UNUSED GWEN_GUI *gui,
                      GWEN_UNUSED const char *groupName,
                      GWEN_UNUSED GWEN_DB_NODE *db)
{
  return 0;
}



int GWENHYWFAR_CB _getFilename(GWEN_UNUSED GWEN_GUI *gui,
                 GWEN_UNUSED const char *caption,
                 GWEN_UNUSED GWEN_GUI_FILENAME_TYPE fnt,
                 GWEN_UNUSED uint32_t flags,
                 GWEN_UNUSED const char *patterns,
                 GWEN_UNUSED GWEN_BUFFER *pathBuffer,
                 GWEN_UNUSED uint32_t guiid)
{
  return 0;
}



int GWENHYWFAR_CB _getSyncio(GWEN_UNUSED GWEN_GUI *gui,
               GWEN_UNUSED const char *url,
               GWEN_UNUSED const char *defaultProto,
               GWEN_UNUSED int defaultPort,
               GWEN_UNUSED GWEN_SYNCIO **pSio)
{
  return 0;
}





