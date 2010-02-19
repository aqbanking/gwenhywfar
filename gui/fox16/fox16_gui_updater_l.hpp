/***************************************************************************
    begin       : Fri Jan 22 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef FOX16_GUI_UPDATER_L_HPP
#define FOX16_GUI_UPDATER_L_HPP

#include <fx.h>

#include <list>




class FOX16_GuiUpdater: public FXObject {
  FXDECLARE(FOX16_GuiUpdater)

public:
  enum {
    ID_CHORE=1
  };

  FOX16_GuiUpdater();
  ~FOX16_GuiUpdater();

  void guiUpdate();

  long onChore(FXObject*, FXSelector, void*);

protected:
  FXuint m_guiIdleFlag;

};



#endif

