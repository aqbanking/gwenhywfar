/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: error.h 1104 2007-01-03 09:21:32Z martin $
    begin       : Tue Oct 02 2002
    copyright   : (C) 2002 by Martin Preuss
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

#ifndef GWENHYWFAR_GUI_GUI_H
#define GWENHYWFAR_GUI_GUI_H



#include <gwenhywfar/inherit.h>
#include <gwenhywfar/logger.h>
#include <gwenhywfar/inetsocket.h>
#include <gwenhywfar/ssl_cert_descr.h>
#include <gwenhywfar/iolayer.h>

#include <inttypes.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct GWEN_GUI GWEN_GUI;
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_GUI, GWENHYWFAR_API)


#define GWEN_GUI_CPU_TIMEOUT 200

#define GWEN_GUI_CHECK_PERIOD 750
#define GWEN_GUI_DELAY_SECS   2


/** @name Flags For GWEN_Gui_ProgressStart
 *
 * These flags are given to @ref GWEN_Gui_ProgressStart to modify its
 * behaviour.
 */
/*@{*/
#define GWEN_GUI_PROGRESS_DELAY            0x00000001
#define GWEN_GUI_PROGRESS_SHOW_LOG         0x00000002
#define GWEN_GUI_PROGRESS_SHOW_ABORT       0x00000004
#define GWEN_GUI_PROGRESS_ALLOW_SUBLEVELS  0x00000008
#define GWEN_GUI_PROGRESS_ALLOW_EMBED      0x00000010
#define GWEN_GUI_PROGRESS_SHOW_PROGRESS    0x00000020
#define GWEN_GUI_PROGRESS_KEEP_OPEN        0x00000040
#define GWEN_GUI_PROGRESS_ALWAYS_SHOW_LOG  0x00000080
/*@}*/



/** @name Flags For GWEN_Gui_InputBox
 *
 * These flags are given to @ref GWEN_Gui_InputBox to modify its
 * behaviour.
 */
/*@{*/
/** input must be confirmed (e.g. by asking for the same input twice) */
#define GWEN_GUI_INPUT_FLAGS_CONFIRM        0x00000001
/** input may be shown (otherwise it should be hidden, e.g. for passwords) */
#define GWEN_GUI_INPUT_FLAGS_SHOW           0x00000002
/** numeric input is requested (e.g. for PINs) */
#define GWEN_GUI_INPUT_FLAGS_NUMERIC        0x00000004
/** if set then this is a retry (esp. when getting a PIN) */
#define GWEN_GUI_INPUT_FLAGS_RETRY          0x00000008
/** allow a default value to be used instead of an entered one.
 * A graphical UI should add a "default" button to the dialog. */
#define GWEN_GUI_INPUT_FLAGS_ALLOW_DEFAULT  0x00000010
/** The input is a TAN (this is used by @ref GWEN_Gui_GetPassword) */
#define GWEN_GUI_INPUT_FLAGS_TAN            0x00000020
/*@}*/


/** @name Flags For GWEN_Gui_MessageBox
 *
 * These flags are given to @ref GWEN_Gui_MessageBox to modify its
 * behaviour. You may OR-combine the flags.<br>
 * Examples:
 * <ul>
 *  <li>
 *    normal error message, multiple buttons, first button confirms
 *    @code
 *      (GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
 *      GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
 *      GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL)
 *    @endcode
 *  </li>
 *  <li>
 *    dangerous error message, multiple buttons, first button confirms
 *    @code
 *      (GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
 *      GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
 *      GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS)
 *    @endcode
 *  </li>
 * </ul>
 * <p>
 * A note about <i>confirmation buttons</i>: AqBanking has been designed with
 * non-interactive applications in mind. For such an application it is
 * important to know what button-press it has to simulate upon catching of a
 * messagebox callback. This is what the confimation button flags are for.
 * For informative messages the application may simply return the number of
 * the confirmation button and be done.
 * </p>
 * <p>
 * However, non-interactive applications should return an error (value 0)
 * for messages classified as <b>dangerous</b>
 * (see @ref GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS) to avoid data loss.
 * </p>
 */
/*@{*/
/**
 * Defines the mask to catch the message type. E.g. a check whether a
 * message is a warning could be performed by:
 * @code
 * if ( ( flags & GWEN_GUI_MSG_FLAGS_TYPE_MASK) ==
 *      GWEN_GUI_MSG_FLAGS_TYPE_WARN) {
 *      fprintf(stderr, "This is a warning.\n");
 * }
 * @endcode
 */
#define GWEN_GUI_MSG_FLAGS_TYPE_MASK           0x07
/** The message is a simple information. */
#define GWEN_GUI_MSG_FLAGS_TYPE_INFO         0
/** check whether the given flags represent an info message */
#define GWEN_GUI_MSG_FLAGS_TYPE_IS_INFO(fl) \
  ((fl & GWEN_GUI_MSG_FLAGS_TYPE_MASK)==GWEN_GUI_MSG_FLAGS_TYPE_INFO)

/** The message is a warning */
#define GWEN_GUI_MSG_FLAGS_TYPE_WARN         1
/** check whether the given flags represent a warning message */
#define GWEN_GUI_MSG_FLAGS_TYPE_IS_WARN(fl)  \
  ((fl & GWEN_GUI_MSG_FLAGS_TYPE_MASK)==GWEN_GUI_MSG_FLAGS_TYPE_WARN)

/** The message is a error message */
#define GWEN_GUI_MSG_FLAGS_TYPE_ERROR        2
/** check whether the given flags represent an error message */
#define GWEN_GUI_MSG_FLAGS_TYPE_IS_ERROR     \
  ((fl & GWEN_GUI_MSG_FLAGS_TYPE_MASK)==GWEN_GUI_MSG_FLAGS_TYPE_ERROR)

/** button 1 is the confirmation button */
#define GWEN_GUI_MSG_FLAGS_CONFIRM_B1         (1<<3)
/** button 2 is the confirmation button */
#define GWEN_GUI_MSG_FLAGS_CONFIRM_B2         (2<<3)
/** button 3 is the confirmation button */
#define GWEN_GUI_MSG_FLAGS_CONFIRM_B3         (3<<3)
/** Determine which button is the confirmation button */
#define GWEN_GUI_MSG_FLAGS_CONFIRM_BUTTON(fl) (((fl)>>3) & 0x3)


/**
 * <p>
 * Check for the severity of the message. This allows non-interactive
 * backends to react on the message accordingly.
 * The backend calling this function thus allows the frontend to detect
 * when the message is important regarding data security.
 * E.g. a message like "Shall I delete this file" should be considered
 * dangerous (since this might result in a data loss). However, the messae
 * "Just started" is not that dangerous ;-)
 * </p>
 * <p>
 * The following example allows to determine whether a message is
 * dangerous:
 * @code
 * if ( ( flags & GWEN_GUI_MSG_FLAGS_SEVERITY_MASK) ==
 *      GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS) {
 *      fprintf(stderr, "This is dangerous.\n");
 * }
 * @endcode
 * </p>
 */
#define GWEN_GUI_MSG_FLAGS_SEVERITY_MASK       (0x7<<5)
/** Message does not affect security or induce any problem to the system */
#define GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL      (0x0<<5)
#define GWEN_GUI_MSG_FLAGS_SEVERITY_IS_NORMAL(fl) \
  ((fl & GWEN_GUI_MSG_FLAGS_SEVERITY_MASK)==\
  GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL)
/** Message is considered dangerous and thus should be attended to by a
 * humanoid ;-) */
#define GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS   (0x1<<5)
#define GWEN_GUI_MSG_FLAGS_SEVERITY_IS_DANGEROUS(fl)  \
  ((fl & GWEN_GUI_MSG_FLAGS_SEVERITY_MASK)==\
  GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS)
/*@}*/


/** @name Flags For GWEN_Gui_ShowBox
 *
 */
/*@{*/
/**
 * Make the frontend beep. This should rarely be used, and only in situations
 * where it is very important to get the users attention (e.g. when asking
 * for a PIN on a card reader).
 */
#define GWEN_GUI_SHOWBOX_FLAGS_BEEP 0x00000001
/*@}*/



/** @name Special Progress Values for GWEN_Gui_ProgressAdvance
 *
 */
/*@{*/
/**
 * This value is used with @ref GWEN_Gui_ProgressAdvance to flag that
 * there really was no progress since the last call to that function but
 * that that function should simply check for user interaction (without
 * the need of updating the progress bar).
 */
#define GWEN_GUI_PROGRESS_NONE (0xffffffffUL)

/**
 * This value is used when the total number of steps is not known to the
 * caller and he just wants to advance the progress by one (e.g. backends
 * use this value when a job has been finished, but the backends do not know
 * how many jobs there still are in AqBanking's queue).
 */
#define GWEN_GUI_PROGRESS_ONE  (0xfffffffeUL)
/*@}*/



/**
 * This status is used for passwords, pins and tans, e.g. by the CryptToken
 * code.
 * It can be used by implementations to mark bad pins, used/unused tans etc.
 */
typedef enum {
  GWEN_Gui_PasswordStatus_Bad=-1,
  GWEN_Gui_PasswordStatus_Unknown,
  GWEN_Gui_PasswordStatus_Ok,
  GWEN_Gui_PasswordStatus_Used,
  GWEN_Gui_PasswordStatus_Unused,
  GWEN_Gui_PasswordStatus_Remove
} GWEN_GUI_PASSWORD_STATUS;


/** @name Constructor, Destructor etc
 *
 */
/*@{*/
GWENHYWFAR_API 
GWEN_GUI *GWEN_Gui_new();

GWENHYWFAR_API 
void GWEN_Gui_free(GWEN_GUI *gui);

GWENHYWFAR_API 
void GWEN_Gui_Attach(GWEN_GUI *gui);

GWENHYWFAR_API 
void GWEN_Gui_SetGui(GWEN_GUI *gui);

GWENHYWFAR_API 
GWEN_GUI *GWEN_Gui_GetGui();

/*@}*/






/** @name Virtual User Interaction Functions
 *
 * <p>
 * All text passed to the frontend via one of the following functions
 * is expected to be an UTF-8 string which may contain newlines but no other
 * control characters.
 * Text delivered as argument called <i>text</i> throughout the documentation
 * in this group may contain HTML tags.
 * If it does a non-HTML version must be supplied, too.
 * The text MUST begin with the non-HTML version, so that a frontend not
 * capable of parsing HTML can simply exclude the HTML part by cutting
 * before "<html".
 *
 * </p>
 * <p>
 * This is an example for HTML and non-HTML text:
 * </p>
 * @code
 * const char *text;
 *
 * text="This is the non-HTML text"
 *      "<html>"
 *      "And this is the <b>HTML</b> version."
 *      "</html>"
 * @endcode
 * <p>
 * Frontends capable of parsing HTML (such as the KDE frontend) will
 * extract the HTML information and show only that part of the string.
 * </p>
 * <p>
 * Other frontends have to extract the non-HTML information and show only
 * that.
 * </p>
 */
/*@{*/
/**
 * <p>
 * Show a message box with optional buttons.
 * The message box may either contain 1, 2 or three buttons.
 * If only one button is wanted then b1 should hold a pointer to the button
 * text (b2 and b3 must be NULL)
 * In two-button mode b1 and b2 must be valid (b3 must be NULL)
 * In three-button-mode b1, b2 and b3 must be valid pointers.
 * The return value tells which button the user pressed:
 * <ul>
 *  <li>1: button 1</li>
 *  <li>2: button 2</li>
 *  <li>3: button 3</li>
 * </ul>
 * If the frontend can not determine which button has been pressed (e.g. if
 * no button was pressed but the user rather aborted the dialog by simply
 * closing the box) it should return @b 0.
 * </p>
 * <p>
 *  This function is blocking.
 * </p>
 * @return the number of the button pressed (1=b1, 2=b2, 3=b3), any other
 *  value should be considered an error, including 0)
 * @param flags flags, see @ref GWEN_GUI_MSG_FLAGS_TYPE_MASK ff.
 * @param title title of the message box
 * @param text Text of the box: UTF-8, with both a normal text and a HTML variant of the text in the same string. See text restrictions note above.
 * @param b1 text for the first button (required), should be something
 *  like "Ok" (see text restrictions note above)
 * @param b2 text for the optional second button
 * @param b3 text for the optional third button
 */
GWENHYWFAR_API 
int GWEN_Gui_MessageBox(uint32_t flags,
			const char *title,
			const char *text,
			const char *b1,
			const char *b2,
			const char *b3,
			uint32_t guiid);

/**
 * <p>
 * Ask the user for input.
 * </p>
 * <p>
 *  This function is blocking.
 * </p>
 * @param ab banking interface
 * @param flags flags, see @ref GWEN_GUI_INPUT_FLAGS_CONFIRM ff.
 * @param title title of the input box
 * @param text Text of the box: UTF-8, with both a normal text and a HTML variant of the text in the same string. See text restrictions note above.
 * @param buffer buffer to store the response in. Must have at least room for
 *  @b maxLen bytes
 * @param minLen minimal length of input (if 0 then there is no low limit)
 * @param maxLen size of the buffer including the trailing NULL character.
 * This means that if you want to ask the user for a PIN of at most 4
 * characters you need to supply a buffer of at least @b 5 bytes and provide
 * a 5 as maxLen.
 *
 * @return Zero on success, nonzero when the user requested abort or there was
 * any error. The special value AB_ERROR_DEFAULT_VALUE should be returned if
 * the flag GWEN_GUI_INPUT_FLAGS_ALLOW_DEFAULT was given and the user has
 * chosen to use the default value (e.g. pressed the "default" button in a
 * GUI).
 */
GWENHYWFAR_API 
int GWEN_Gui_InputBox(uint32_t flags,
		      const char *title,
		      const char *text,
		      char *buffer,
		      int minLen,
		      int maxLen,
		      uint32_t guiid);

/**
 * <p>
 * Shows a box with the given text. This function should return immediately,
 * it should especially NOT wait for user input. This is used to show very
 * important notices the user should see but which don't need user
 * interaction. The message box will be removed later via
 * @ref GWEN_Gui_HideBox. It is ok to allow the user to prematurely
 * close the box.
 * </p>
 * <p>
 * It is required for every call to this function to be followed later
 * by a corresponding call to @ref GWEN_Gui_HideBox.
 * </p>
 * <p>
 * This function MUST return immediately (non-blocking).
 * </p>
 * @return returns an id to be presented to @ref GWEN_Gui_HideBox.
 * @param ab banking interface
 * @param flags flags, see @ref GWEN_GUI_SHOWBOX_FLAGS_BEEP ff
 * @param title title of the box
 * @param text Text of the box: UTF-8, with both a normal text and a HTML variant of the text in the same string. See text restrictions note above.
 */
GWENHYWFAR_API 
uint32_t GWEN_Gui_ShowBox(uint32_t flags,
			  const char *title,
			  const char *text,
			  uint32_t guiid);

/**
 * Hides a message box previously shown by a call to @ref GWEN_Gui_ShowBox.
 * <p>
 * This function MUST return immediately (non-blocking).
 * </p>
 * @param ab banking interface
 * @param id id returned by @ref GWEN_Gui_ShowBox. If @b 0 then the last
 * message shown is referred to.
 */
GWENHYWFAR_API 
void GWEN_Gui_HideBox(uint32_t id);


/**
 * <p>
 * This function is called when a long term operation is started.
 * Theoretically nesting is allowed, however, you should refrain from
 * opening multiple progress dialogs to avoid confusion of the user.
 * </p>
 * <p>
 * This function must return immediately (i.e. it MUST NOT wait for
 * user interaction).
 * </p>
 * <p>
 * On graphical user interfaces such a dialog should contain a widget
 * for the text presented here, a progress bar, a text widget to
 * collect the log messages received via @ref GWEN_Gui_ProgressLog and
 * a button to allow the user to abort the current operation monitored by
 * this dialog window.
 * </p>
 * <p>
 * Between a call to this function and one to @ref GWEN_Gui_ProgressEnd
 * the user should not be allowed to close the dialog window.
 * </p>
 * <p>
 * This function MUST return immediately (non-blocking).
 * </p>
 * @return id to be used with the other GWEN_Gui_Progress functions.
 * @param title title of the dialog
 * @param text Text of the box: UTF-8, with both a normal text and a HTML variant of the text in the same string. See text restrictions note above.
 * @param total total number of steps of the operation started (i.e. value
 *  which represents 100%)
 */
GWENHYWFAR_API 
uint32_t GWEN_Gui_ProgressStart(uint32_t progressFlags,
				const char *title,
				const char *text,
				uint64_t total,
				uint32_t guiid);

/**
 * <p>
 * Advances the progress bar an application might present to the user and
 * checks whether the user wants to abort the operation currently in progress.
 * </p>
 * <p>
 * On graphical user interfaces this function should also check for user
 * interaction and/or update the GUI (e.g. by calling qApp->processEvents()
 * when using QT/KDE).
 * </p>
 * <p>
 * This function MUST return immediately (non-blocking).
 * </p>
 * @return 0 if ok, !=0 if the current operation is to be aborted
 * @param id id assigned by @ref GWEN_Gui_ProgressStart (if 0 then the
 * last started progress dialog is referred to)
 * @param progress new value for progress. A special value is
 *  GWEN_GUI_PROGRESS_NONE which means that the progress is unchanged.
 * This might be used as a keepalive call to a GUI.
 */
GWENHYWFAR_API 
int GWEN_Gui_ProgressAdvance(uint32_t id, uint32_t progress);

/**
 * Adds a log message to the referred process dialog.
 * <p>
 * This function MUST return immediately (non-blocking).
 * </p>
 * @param id id assigned by @ref GWEN_Gui_ProgressStart (if 0 then the
 * last started progress dialog is referred to)
 * @param level log level (see @ref GWEN_Gui_LogLevelPanic ff.)
 * @param text Text of the box: UTF-8, with both a normal text and a HTML variant of the text in the same string. See text restrictions note above.
 */
GWENHYWFAR_API 
int GWEN_Gui_ProgressLog(uint32_t id,
			 GWEN_LOGGER_LEVEL level,
			 const char *text);

/**
 * <p>
 * Flags the end of the current operation. In graphical user interfaces
 * this call should allow the user to close the progress dialog window.
 * </p>
 * <p>
 * On graphical user interfaces a call to this function should disable the
 * <i>abort</i> button. It would be best not to close the dialog on
 * receiption of this call but to simply enable a dialog closing (otherwise
 * the user will not be able to see the log messages).
 * </p>
 * <p>
 * This function MUST return immediately (non-blocking).
 * </p>
 * @param id id assigned by @ref GWEN_Gui_ProgressStart (if 0 then the
 * last started progress dialog is referred to)
 */
GWENHYWFAR_API 
int GWEN_Gui_ProgressEnd(uint32_t id);


/**
 * This function makes the application print something.
 * @param docTitle title of the document. This might be presented to the user
 * @param docType an unique identifier of the document to be printed. This can
 *   be used by the application to separate printer settings for different
 *   document types. The name itself has no meaning and can be choosen freely
 *   by the caller. However, backends should append their name and a colon
 *   to keep this argument unique. This argument should not be translated.
 * @param descr an optional description about what the document contains. This
 *   might be shown to the user (see text restriction notes above).
 * @param text text to be printed (see text restriction notes above).
 */
GWENHYWFAR_API 
int GWEN_Gui_Print(const char *docTitle,
		   const char *docType,
		   const char *descr,
		   const char *text,
		   uint32_t guiid);


GWENHYWFAR_API
int GWEN_Gui_GetPassword(uint32_t flags,
			 const char *token,
			 const char *title,
			 const char *text,
			 char *buffer,
			 int minLen,
			 int maxLen,
			 uint32_t guiid);

/**
 * This functions sets the status of a password.
 */
GWENHYWFAR_API 
int GWEN_Gui_SetPasswordStatus(const char *token,
			       const char *pin,
			       GWEN_GUI_PASSWORD_STATUS status,
			       uint32_t guiid);

/**
 * This function is called internally by @ref GWEN_Logger_Log.
 */
GWENHYWFAR_API 
int GWEN_Gui_LogHook(const char *logDomain,
		     GWEN_LOGGER_LEVEL priority, const char *s);


/**
 * This function waits for activity on the given sockets. The default implementation uses
 * GWEN_Socket_Select() for this purpose.
 */
GWENHYWFAR_API 
int GWEN_Gui_WaitForSockets(GWEN_SOCKET_LIST2 *readSockets,
			    GWEN_SOCKET_LIST2 *writeSockets,
                            uint32_t guiid,
			    int msecs);


GWENHYWFAR_API 
int GWEN_Gui_CheckCert(const GWEN_SSLCERTDESCR *cert,
		       GWEN_IO_LAYER *io,
		       uint32_t guiid);


/**
 * This function is not officially part of the API but is needed for some ancient OpenHBCI
 * keyfiles.
 * License issues forbid us to link against OpenSSL so we leave it up to the application
 * to implement this function. A converter tool might use this function once to convert
 * an anciant OpenHBCI key file.
 */
GWENHYWFAR_API
int GWEN_Gui_KeyDataFromText_OpenSSL(const char *text,
				     unsigned char *buffer,
				     unsigned int bufLength);


/*@}*/






#ifdef __cplusplus
}
#endif


#endif




