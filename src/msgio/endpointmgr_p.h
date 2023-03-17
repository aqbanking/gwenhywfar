/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_MGR_P_H
#define GWEN_MSG_ENDPOINT_MGR_P_H


#include "msgio/endpointmgr.h"



struct GWEN_MSG_ENDPOINT_MGR {
  GWEN_INHERIT_ELEMENT(GWEN_MSG_ENDPOINT_MGR)

  GWEN_MSG_ENDPOINT_LIST *endpointList;
};


#endif


