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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define GWEN_EXTEND_WAITCALLBACK
#include "waitcallback.h"


typedef struct GWENPP_WAITCALLBACK GWENPP_WAITCALLBACK;
GWEN_INHERIT(GWEN_WAITCALLBACK, GWENPP_WAITCALLBACK)





namespace GWEN {

  GWEN_WAITCALLBACK*
    WaitCallback::_instantiate(GWEN_WAITCALLBACK *ctx){
      GWEN::WaitCallback *wcb;
      GWEN::WaitCallback *newwcb;

      wcb=(GWEN::WaitCallback*)GWEN_INHERIT_GETDATA(GWEN_WAITCALLBACK,
                                                    GWENPP_WAITCALLBACK,
                                                    ctx);
      assert(wcb);
      newwcb=wcb->instantiate();
      assert(newwcb);
      return newwcb->_ctx;
    }

  GWEN_WAITCALLBACK_RESULT
    WaitCallback::_checkAbort(GWEN_WAITCALLBACK *ctx,
                              unsigned int level){
      GWEN::WaitCallback *wcb;
  
      wcb=(GWEN::WaitCallback*)GWEN_INHERIT_GETDATA(GWEN_WAITCALLBACK,
                                                    GWENPP_WAITCALLBACK,
                                                    ctx);
      assert(wcb);
      return wcb->checkAbort(level);
    }
  
  
  
  void WaitCallback::_log(GWEN_WAITCALLBACK *ctx,
                          unsigned int level,
                          GWEN_LOGGER_LEVEL loglevel,
                          const char *s){
    GWEN::WaitCallback *wcb;

    wcb=(GWEN::WaitCallback*)GWEN_INHERIT_GETDATA(GWEN_WAITCALLBACK,
                                                  GWENPP_WAITCALLBACK,
                                                  ctx);
    assert(wcb);
    wcb->log(level, loglevel, s);
  }
  
  
  void WaitCallback::_freeData(void *bp, void *p) {
    assert(p);
    delete (GWEN::WaitCallback*)p;
  }





  WaitCallback::WaitCallback(const char *id){
    GWENPP_WAITCALLBACK *pp;

    _ctx=GWEN_WaitCallback_new(id);
    pp=(GWENPP_WAITCALLBACK*)this;
    GWEN_INHERIT_SETDATA(GWEN_WAITCALLBACK, GWENPP_WAITCALLBACK,
                         _ctx, pp, _freeData);
    GWEN_WaitCallback_SetCheckAbortFn(_ctx, _checkAbort);
    GWEN_WaitCallback_SetInstantiateFn(_ctx, _instantiate);
    GWEN_WaitCallback_SetLogFn(_ctx, _log);
  }



  WaitCallback::~WaitCallback(){
  }



  WaitCallback *WaitCallback::instantiate(){
    return 0;
  }



  GWEN_WAITCALLBACK_RESULT WaitCallback::checkAbort(unsigned int level){
    return GWEN_WaitCallbackResult_Continue;
  }



  void WaitCallback::log(unsigned int level,
                         unsigned int loglevel,
                         const char *s){
  }





  int WaitCallback::getDistance(){
    return GWEN_WaitCallback_GetDistance(_ctx);
  }



  int WaitCallback::registerCallback(){
    return GWEN_WaitCallback_Register(_ctx);
  }


  int WaitCallback::unregisterCallback(){
    return GWEN_WaitCallback_Unregister(_ctx);
  }


  const char *WaitCallback::getId(){
    return GWEN_WaitCallback_GetId(_ctx);
  }



  GWEN_TYPE_UINT64 WaitCallback::getProgressPos(){
    return GWEN_WaitCallback_GetProgressPos(_ctx);
  }



  GWEN_TYPE_UINT64 WaitCallback::getProgressTotal(){
    return GWEN_WaitCallback_GetProgressTotal(_ctx);
  }



  time_t WaitCallback::lastCalled(){
    return GWEN_WaitCallback_LastCalled(_ctx);
  }



  time_t WaitCallback::lastEntered(){
    return GWEN_WaitCallback_LastEntered(_ctx);
  }



  void WaitCallback::setDistance(int d){
    GWEN_WaitCallback_SetDistance(_ctx, d);
  }







}; /* namespace */




