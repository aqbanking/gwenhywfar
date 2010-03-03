/***************************************************************************
    begin       : Fri Jan 22 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "fox16_gui_updater_l.hpp"

#include <gwenhywfar/debug.h>




FXDEFMAP(FOX16_GuiUpdater) FOX16_GuiUpdaterMap[]={
  FXMAPFUNC(SEL_CHORE, FOX16_GuiUpdater::ID_CHORE, FOX16_GuiUpdater::onChore),
};


FXIMPLEMENT(FOX16_GuiUpdater, FXObject, FOX16_GuiUpdaterMap, ARRAYNUMBER(FOX16_GuiUpdaterMap))



FOX16_GuiUpdater::FOX16_GuiUpdater()
:FXObject()
, m_guiIdleFlag(0)
{
}



FOX16_GuiUpdater::~FOX16_GuiUpdater() {
}




void FOX16_GuiUpdater::guiUpdate() {
  FXApp *a=FXApp::instance();

  a->addChore(this, ID_CHORE);
  a->flush(true);

  m_guiIdleFlag=0;
  DBG_ERROR(0, "Waiting for GUI to become idle");
  a->runUntil(m_guiIdleFlag);
  DBG_ERROR(0, "GUI is idle now");
}



long FOX16_GuiUpdater::onChore(FXObject*, FXSelector, void*){
  m_guiIdleFlag=1;
  return 1;
}

