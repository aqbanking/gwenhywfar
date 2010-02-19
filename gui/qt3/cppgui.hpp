/***************************************************************************
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef CPPGUI_HPP
#define CPPGUI_HPP



#if defined __GNUC__ && (! defined (__sun)) && (__GNUC__ >= 4 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))
# ifdef BUILDING_QT3_GUI
#   define QT3GUI_API __attribute__ ((visibility("default")))
# else
#   define QT3GUI_API
# endif
#else
# define QT3GUI_API
#endif



#include <gwenhywfar/gui_be.h>
#include <list>
#include <string>

class CppGui;

#include <gwen-gui-qt3/cppdialog.hpp>


/**
 * @brief A C++ binding for the C module @ref GWEN_GUI
 *
 * This class simply is a C++ binding for the C module @ref GWEN_GUI.
 * It redirects C callbacks used by GWEN_GUI to virtual functions in
 * this class.
 *
 * @author Martin Preuss<martin@aquamaniac.de>
 */
class QT3GUI_API CppGui {
  friend class CppGuiLinker;

private:
  GWEN_GUI_CHECKCERT_FN _checkCertFn;

  GWEN_DB_NODE *_dbPasswords;
  std::list<std::string> _badPasswords;

  GWEN_GUI_MESSAGEBOX_FN _messageBoxFn;
  GWEN_GUI_INPUTBOX_FN _inputBoxFn;
  GWEN_GUI_PRINT_FN _printFn;
  GWEN_GUI_GETPASSWORD_FN _getPasswordFn;
  GWEN_GUI_SETPASSWORDSTATUS_FN _setPasswordStatusFn;
  GWEN_GUI_EXEC_DIALOG_FN _execDialogFn;
  GWEN_GUI_OPEN_DIALOG_FN _openDialogFn;
  GWEN_GUI_CLOSE_DIALOG_FN _closeDialogFn;
  GWEN_GUI_RUN_DIALOG_FN _runDialogFn;

  GWEN_GUI_GET_FILENAME_FN _getFileNameFn;


  std::string _getPasswordHash(const char *token, const char *pin);

public:
  CppGui();
  virtual ~CppGui();

  static QT3GUI_API CppGui *getCppGui();

  GWEN_GUI *getCInterface();

protected:
  GWEN_GUI *_gui;

  /** @name User Interaction
   *
   */
  /*@{*/

  /**
   * See @ref CppGui_Print
   */
  virtual int print(const char *docTitle,
                    const char *docType,
                    const char *descr,
		    const char *text,
		    uint32_t guiid);

  virtual int getPassword(uint32_t flags,
			  const char *token,
			  const char *title,
			  const char *text,
			  char *buffer,
			  int minLen,
			  int maxLen,
			  uint32_t guiid);

  virtual int setPasswordStatus(const char *token,
				const char *pin,
				GWEN_GUI_PASSWORD_STATUS status,
				uint32_t guiid);

  virtual int checkCert(const GWEN_SSLCERTDESCR *cert,
			GWEN_IO_LAYER *io,
			uint32_t guiid);

  virtual int execDialog(GWEN_DIALOG *dlg, uint32_t guiid);

  virtual int openDialog(GWEN_DIALOG *dlg, uint32_t guiid);
  virtual int closeDialog(GWEN_DIALOG *dlg);
  virtual int runDialog(GWEN_DIALOG *dlg, int untilEnd);

  virtual int getFileName(const char *caption,
			  GWEN_GUI_FILENAME_TYPE fnt,
			  uint32_t flags,
			  const char *patterns,
			  GWEN_BUFFER *pathBuffer,
			  uint32_t guiid);


  int checkCertBuiltIn(const GWEN_SSLCERTDESCR *cert,
		       GWEN_IO_LAYER *io,
		       uint32_t guiid);

  };




#endif /* CPPGUI_HPP */


