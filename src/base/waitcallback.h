/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Sat Jan 17 2004
 copyright   : (C) 2004 by Martin Preuss
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


#ifndef GWEN_WAIT_CALLBACK_H
#define GWEN_WAIT_CALLBACK_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/logger.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct GWEN_WAITCALLBACK GWEN_WAITCALLBACK;
GWEN_LIST_FUNCTION_LIB_DEFS(GWEN_WAITCALLBACK, GWEN_WaitCallback, GWENHYWFAR_API)
GWEN_INHERIT_FUNCTION_LIB_DEFS(GWEN_WAITCALLBACK, GWENHYWFAR_API)
/* No trailing semicolon here because this is a macro call */
#ifdef __cplusplus
}
#endif


#include <gwenhywfar/types.h>


#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup MOD_WAITCALLBACK Long Term Operation Wait Callback
 * @ingroup MOD_BASE
 *
 * @brief This file contains the definition of a GWEN_WAITCALLBACK.
 *
 * WaitCallbacks are called during long term operations to give the user
 * the opportunity to abort the operation.
 * The default implementation does nothing. But an application can use such
 * a callback to:
 * <ul>
 *   <li>Check whether the user wants to abort</li>
 *   <li>Update a GUI thus preventing X toolkits such as QT from freezing
 * </ul>
 * <p>
 * Wait callbacks are organized in callback-contexts which must be entered
 * and left.
 * </p>
 */
/*@{*/


#include <time.h>

/**
 * This value tells the callback mechanism that there has been no progress.
 * It is used for keep-alive calls.
 */
//#define GWEN_WAITCALLBACK_PROGRESS_NONE
//  ((GWEN_TYPE_UINT64)(0xffffffffffffffffLL))

#define GWEN_WAITCALLBACK_PROGRESS_NONE \
  ((GWEN_TYPE_UINT64)(-1))


/**
 * This depth level is used upon entering of a waitcallback which is reused
 * (because there already is a waitcallback with the enter-id).
 * In such a case Gwen calls the the log callback function with this level
 * and the text given to @ref @GWEN_WaitCallback_EnterWithText.
 */
#define GWEN_WAITCALLBACK_LEVEL_REUSED 9999

/**
 * Normally Gwen tries to reuse already open WaitCallbacks instead of
 * creating new ones. If this flag is given then a new WaitCallback will be
 * created in any case.
 */
#define GWEN_WAITCALLBACK_FLAGS_NO_REUSE 0x00000001


/**
 * This id can be used for any simple progress operation.
 * GUI implementations for this callback might choose to show the associated
 * text (@ref GWEN_WaitCallback_GetText) and provide a simple progress bar.
 */
#define GWEN_WAITCALLBACK_ID_SIMPLE_PROGRESS \
  "GWEN_WAITCALLBACK_ID_SIMPLE_PROGRESS"


/**
 * Use this callback id for operations which do not need a GUI.
 * Applications should implement this callback without opening a window.
 */
#define GWEN_WAITCALLBACK_ID_FAST \
  "GWEN_WAITCALLBACK_ID_FAST"



/**
 * Result of @ref GWEN_WAITCALLBACK_CHECKABORTFN.
 */
typedef enum {
  /** tells the caller that it is ok to continue */
  GWEN_WaitCallbackResult_Continue=0,
  /** tells the caller to abort */
  GWEN_WaitCallbackResult_Abort
} GWEN_WAITCALLBACK_RESULT;


/** @name Prototypes For Virtual Functions
 *
 */
/*@{*/

/**
 * Checks whether the user wants to abort the current action.
 * @param ctx context
 * This function is optional.
 * @param level If the context given to @ref GWEN_WaitCallback_Enter did
 * not exist then a new default context has been created which uses the
 * functions (like this one) of the at that time active context. For such an
 * artificially derived context the level represents the current level
 * below the context given as parameter <i>ctx</i>. So if the level is 0 then
 * the given context actually <b>is</b> the currently active one.
 */
typedef
  GWEN_WAITCALLBACK_RESULT
  (*GWEN_WAITCALLBACK_CHECKABORTFN)(GWEN_WAITCALLBACK *ctx,
				    unsigned int level);


/**
 * Uses the given context as a template to instantiate a new context.
 * For GUI callbacks this function can be used to open a window (such as
 * progress dialogs etc).
 * This function <b>must</b> be set.
 * @param ctx context
 */
typedef
  GWEN_WAITCALLBACK*
  (*GWEN_WAITCALLBACK_INSTANTIATEFN)(GWEN_WAITCALLBACK *ctx);


/**
 * Logs a message to the given callback.
 * A GUI program could use this function to write the given string to an
 * open window.
 * This function is optional.
 * @param ctx context
 * @param level see @ref GWEN_WAITCALLBACK_CHECKABORTFN. If the level value
 * is GWEN_WAITCALLBACK_LEVEL_REUSED then the log message is the text
 * given to @ref GWEN_WaitCallback_EnterWithText for reused callbacks.
 * @param loglevel a higher level results in a more detailed output. Loglevels
 * are defined from 0 (the most important level) and 10 (the least important
 * level). Libgwenhywfar itself does not use this function.
 * @param s log string
 */
typedef
  void
  (*GWEN_WAITCALLBACK_LOGFN)(GWEN_WAITCALLBACK *ctx,
                             unsigned int level,
                             GWEN_LOGGER_LEVEL loglevel,
			     const char *s);
/*@}*/


/** @name Virtual Functions
 *
 */
/*@{*/

/**
 * Checks whether the user wants to abort the current action.
 * This in fact calls the function @ref GWEN_WAITCALLBACK_CHECKABORTFN
 * stored in the current waitcallback context.
 */
GWENHYWFAR_API
  GWEN_WAITCALLBACK_RESULT GWEN_WaitCallback();

GWENHYWFAR_API
void GWEN_WaitCallback_free(GWEN_WAITCALLBACK *ctx);

/**
 * This is basically a combination of the functions
 * @ref GWEN_WaitCallback_SetProgressPos and
 * @ref GWEN_WaitCallback.
 * @param pos current progress position
 */
GWENHYWFAR_API
  GWEN_WAITCALLBACK_RESULT GWEN_WaitCallbackProgress(GWEN_TYPE_UINT64 pos);


/**
 * Don't call this function directly, better use the macro
 * @ref GWEN_WaitCallback_Enter
 */
GWENHYWFAR_API
  void GWEN_WaitCallback_Enter_u(const char *id,
                                 const char *file,
                                 int line);

/**
 * Don't call this function directly, better use the macro
 * @ref GWEN_WaitCallback_EnterWithText
 */
GWENHYWFAR_API
  void GWEN_WaitCallback_EnterWithText_u(const char *id,
                                         const char *txt,
                                         const char *units,
                                         GWEN_TYPE_UINT32 flags,
					 const char *file,
					 int line);

/**
 * Enters the callback of the given name. If there is no callback of that name
 * below the currently active callback context then it will be faked.
 * This simply means that a new default context is created which still calls
 * the currently active context (but with level!=0).
 * If the context does exist its function
 * @ref GWEN_WAITCALLBACK_INSTANTIATEFN will be called and the resulting
 * context becomes the active one.
 */
#define GWEN_WaitCallback_Enter(id) \
  GWEN_WaitCallback_Enter_u(id, __FILE__, __LINE__)


/**
 * This macro internally calls @ref GWEN_WaitCallback_EnterWithText_u.
 * It therefore does the same as @ref GWEN_WaitCallback_Enter, but it
 * stores the given text internally to be retrieved by the implemention of
 * a specific callback and presented to the user.
 */
#define GWEN_WaitCallback_EnterWithText(id, txt, units, flags) \
  GWEN_WaitCallback_EnterWithText_u(id, txt, units, flags, __FILE__, __LINE__)


/**
 * Leaves the currently active context (by freeing it) and reinstates the
 * previously active one.
 */
GWENHYWFAR_API
  void GWEN_WaitCallback_Leave();

/**
 * Sends the given log string to the currently active context.
 * Internally calls @ref GWEN_WAITCALLBACK_LOGFN.
 */
GWENHYWFAR_API
  void GWEN_WaitCallback_Log(GWEN_LOGGER_LEVEL logLevel,
                             const char *s);
/*@}*/


/** @name Informational Functions
 *
 */
/*@{*/

/**
 * You can use this to initialize a progress dialog. This value can later be
 * used by the callback function to properly display a progress bar.
 * if the special value GWEN_WAITCALLBACK_PROGRESS_NONE is used here then
 * no progress bar is requested. Instead a text box might be implemented by
 * the application which shows the ProgressPos as an absolute number.
 */
GWENHYWFAR_API
  void GWEN_WaitCallback_SetProgressTotal(GWEN_TYPE_UINT64 total);

/**
 * You can use this to feed a progress dialog. This value can later be
 * used by the callback function to properly display a progress bar.
 * @param pos current progress position
 */
GWENHYWFAR_API
  void GWEN_WaitCallback_SetProgressPos(GWEN_TYPE_UINT64 pos);

/**
 * Returns the proposed distance in milliseconds between two
 * calls to the callback (defaults to 0).
 * You can use the value returned here for a call to select(2).
 * @param ctx context to inspect (if 0 then the currently active one will
 * be inspected)
 */
GWENHYWFAR_API
  int GWEN_WaitCallback_GetDistance(const GWEN_WAITCALLBACK *ctx);

/**
 * Resturns the nesting level of the given context
 *
 */
GWENHYWFAR_API
int GWEN_WaitCallback_GetNestingLevel(const GWEN_WAITCALLBACK *ctx);

/*@}*/


GWENHYWFAR_API
  int GWEN_WaitCallback_Register(GWEN_WAITCALLBACK *ctx);

GWENHYWFAR_API
  int GWEN_WaitCallback_Unregister(GWEN_WAITCALLBACK *ctx);


#if (defined(GWEN_EXTEND_WAITCALLBACK) || defined(DOXYGEN))

/** @name Functions To Be Used By Inheritors
 *
 */
/*@{*/

GWENHYWFAR_API
  const char *GWEN_WaitCallback_GetId(const GWEN_WAITCALLBACK *ctx);

/**
 * Creates a new callback. This function should only be used by inheriting
 * "classes".
 */
GWENHYWFAR_API
  GWEN_WAITCALLBACK *GWEN_WaitCallback_new(const char *id);

/**
 * Returns the current progress position (as set by
 * @ref GWEN_WaitCallback_SetProgressPos).
 * This can be used by the callback context to correctly display a
 * progress bar.
 */
GWENHYWFAR_API
  GWEN_TYPE_UINT64
  GWEN_WaitCallback_GetProgressPos(const GWEN_WAITCALLBACK *ctx);

/**
 * Returns the progress total (as set by
 * @ref GWEN_WaitCallback_SetProgressTotal).
 * This can be used by the callback context to correctly display a
 * progress bar.
 */
GWENHYWFAR_API
  GWEN_TYPE_UINT64
  GWEN_WaitCallback_GetProgressTotal(const GWEN_WAITCALLBACK *ctx);

/**
 *
 */
GWENHYWFAR_API
  const char *GWEN_WaitCallback_GetText(const GWEN_WAITCALLBACK *ctx);

/**
 *
 */
GWENHYWFAR_API
  const char *GWEN_WaitCallback_GetUnits(const GWEN_WAITCALLBACK *ctx);

/**
 * Sets the checkAbort function (see @ref GWEN_WAITCALLBACK_CHECKABORTFN).
 */
GWENHYWFAR_API
  void
  GWEN_WaitCallback_SetCheckAbortFn(GWEN_WAITCALLBACK *ctx,
				    GWEN_WAITCALLBACK_CHECKABORTFN fn);

/**
 * Sets the instantiate function (see @ref GWEN_WAITCALLBACK_INSTANTIATEFN).
 */
GWENHYWFAR_API
  void
  GWEN_WaitCallback_SetInstantiateFn(GWEN_WAITCALLBACK *ctx,
				     GWEN_WAITCALLBACK_INSTANTIATEFN fn);

/**
 * Sets the log function (see @ref GWEN_WAITCALLBACK_LOGFN).
 */
GWENHYWFAR_API
  void
  GWEN_WaitCallback_SetLogFn(GWEN_WAITCALLBACK *ctx,
			     GWEN_WAITCALLBACK_LOGFN fn);

/**
 * Returns the time when the callback function was last called
 * (or 0 if it has never been called)
 */
GWENHYWFAR_API
  time_t GWEN_WaitCallback_LastCalled(const GWEN_WAITCALLBACK *ctx);


/**
 * Returns the time when the callback context was last entered (or 0 if
 * it never has been).
 */
GWENHYWFAR_API
  time_t GWEN_WaitCallback_LastEntered(const GWEN_WAITCALLBACK *ctx);


/**
 * Sets the proposed distance in milliseconds between two
 * calls to the callback. This value is never enforced by the callback
 * mechanism itself.
 */
GWENHYWFAR_API
  void GWEN_WaitCallback_SetDistance(GWEN_WAITCALLBACK *ctx,
				     int d);
/*@}*/

#endif


/*@}*/


#ifdef __cplusplus
}
#endif


#endif

