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


#ifndef GWENPP_WAIT_CALLBACK_H
#define GWENPP_WAIT_CALLBACK_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/misc.h>


namespace GWEN {

  /**
   * This class allows to easily use the WaitCallback framework from
   * within C++.
   */
  class WaitCallback {
  private:
    GWEN_WAITCALLBACK *_ctx;

    static GWEN_WAITCALLBACK *_instantiate(GWEN_WAITCALLBACK *ctx);
    static GWEN_WAITCALLBACK_RESULT _checkAbort(GWEN_WAITCALLBACK *ctx,
                                                unsigned int level);
    static void _log(GWEN_WAITCALLBACK *ctx,
                     unsigned int level,
                     unsigned int loglevel,
                     const char *s);
    static void _freeData(void *bp, void *p);

  protected:
    /**
     * Returns the current progress position (as set by
     * @ref GWEN_WaitCallback_SetProgressPos).
     * This can be used by the callback context to correctly display a
     * progress bar.
     */
    GWEN_TYPE_UINT64 getProgressPos();

    /**
     * Returns the progress total (as set by
     * @ref GWEN_WaitCallback_SetProgressTotal).
     * This can be used by the callback context to correctly display a
     * progress bar.
     */
    GWEN_TYPE_UINT64 getProgressTotal();

    /**
     * Returns the time when the callback function was last called
     * (or 0 if it has never been called)
     */
    time_t lastCalled();

    /**
     * Returns the time when the callback context was last entered (or 0 if
     * it never has been).
     */
    time_t lastEntered();

    /**
     * Sets the proposed distance in milliseconds between two
     * calls to the callback. This value is never enforced by the callback
     * mechanism itself.
     */
    void setDistance(int d);

    /** @name Functions Called by Gwenhywfar
     *
     * These are functions internally called by Gwenhywfar.
     * You should never call one of these functions from within your own
     * code !
     *
     */
    /*@{*/

    /**
     * Uses this callback as a template to instantiate a new one.
     * For GUI callbacks this function can be used to open a window (such as
     * progress dialogs etc).
     */
    virtual WaitCallback *instantiate();


    /**
     * Checks whether the user wants to abort the current action.
     * This function is optional.
     * @param level If the context given to @ref GWEN_WaitCallback_Enter did
     * not exist then a new default context has been created which uses the
     * functions (like this one) of the at that time active context. For such
     * an artificially derived context the level represents the current level
     * below the context given as parameter <i>ctx</i>. So if the level is 0
     * then the given context actually <b>is</b> the currently active one.
     */
    virtual GWEN_WAITCALLBACK_RESULT checkAbort(unsigned int level);

    /**
     * Logs a message to this callback.
     * A GUI program could use this function to write the given string to an
     * open window.
     * This function is optional.
     * @param level see @ref GWEN_WAITCALLBACK_CHECKABORTFN
     * @param loglevel a higher level results in a more detailed output.
     * Loglevels are defined from 0 (the most important level) and 10 (the
     * least important level).
     * @param s log string
     */
    virtual void log(unsigned int level,
		     unsigned int loglevel,
		     const char *s);

    /**
     * Constructor.
     */
    WaitCallback(const char *id);
    virtual ~WaitCallback();

  public:

    /**
     * Registers this callback with Gwenhywfar.
     * After having registered this callback its method @ref instantiate()
     * will be called by Gwenhywfar whenever this a callback of this type
     * is entered via @ref GWEN_WaitCallback_Enter.
     */
    int registerCallback(const char *id);

    /**
     * Returns the Id of this callback.
     */
    const char *getId();

    /*@}*/

    int getDistance();

  };

};




#endif /* GWENPP_WAIT_CALLBACK_H */


