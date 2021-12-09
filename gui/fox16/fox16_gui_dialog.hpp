/***************************************************************************
    begin       : Fri Jan 22 2010
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef FOX16_GUI_DIALOG_HPP
#define FOX16_GUI_DIALOG_HPP

#include <gwen-gui-cpp/cppdialog.hpp>

#include <fx.h>

#include <gwenhywfar/dialog_be.h>
#include <gwenhywfar/widget_be.h>

#include <list>
#include <string>



class FOX16_GuiDialog: public FXObject, public CppDialog {
  FXDECLARE(FOX16_GuiDialog)

protected:
  class RadioButtonGroup {
  protected:
    FXDataTarget m_radioTarget;
    int m_groupId;
    FXint m_radioValue;
    int m_buttonCount;
    std::list<FXRadioButton*> m_buttonList;

  public:
    RadioButtonGroup(int groupId, FXObject* tgt=NULL, FXSelector sel=0)
      :m_radioTarget(m_radioValue, tgt, sel), m_groupId(groupId), m_radioValue(0), m_buttonCount(0) {};
    ~RadioButtonGroup() {
      std::list<FXRadioButton*>::iterator it;

      for (it=m_buttonList.begin(); it!=m_buttonList.end(); it++)
        /* reset target pointer in buttons because we are about to destroy the FXDataTarget */
        (*it)->setTarget(NULL);
    };

    int getGroupId() { return m_groupId;};

    FXDataTarget *getDataTarget() { return &m_radioTarget;};

    int getButtonCount() { return m_buttonList.size();};

    FXint getRadioValue() const { return m_radioValue;};

    void addButton(FXRadioButton *rb) {
      m_buttonList.push_back(rb);
    };
  };


public:
  enum {
    ID_WIDGET_FIRST=1,
    ID_WIDGET_LAST=1000,
    ID_LAST
  };

  FOX16_GuiDialog(GWEN_DIALOG *dlg);
  virtual ~FOX16_GuiDialog();

  static FOX16_GuiDialog *getDialog(GWEN_DIALOG *dlg);

  long onSelCommand(FXObject *sender, FXSelector sel, void *ptr);
  long onSelChanged(FXObject *sender, FXSelector sel, void *ptr);
  long onSelKeyPress(FXObject *sender, FXSelector sel, void *ptr);
  long onSelKeyRelease(FXObject *sender, FXSelector sel, void *ptr);

  FXIcon *getIcon(const char *fileName);

  bool setup(FXWindow *parentWindow);

  int execute();
  int cont();

  int openDialog();
  int closeDialog();


  FXWindow *getMainWindow() { return m_mainWidget;};
  FXWindow *setupTree(FXWindow *parentWindow, GWEN_WIDGET *w);

protected:
  int _widgetCount;
  FXWindow *m_mainWidget;
  FXIconSource *m_iconSource;
  std::list<FXIcon*> m_iconList;
  std::list<RadioButtonGroup*> m_radioGroups;
  bool m_sizeChanged;

  FOX16_GuiDialog();


  FXDialogBox *getDialogBox() const;

  virtual int setIntProperty(GWEN_WIDGET *w,
                             GWEN_DIALOG_PROPERTY prop,
                             int index,
                             int value,
                             int doSignal);

  virtual int getIntProperty(GWEN_WIDGET *w,
                             GWEN_DIALOG_PROPERTY prop,
                             int index,
                             int defaultValue);

  virtual int setCharProperty(GWEN_WIDGET *w,
                              GWEN_DIALOG_PROPERTY prop,
                              int index,
                              const char *value,
                              int doSignal);

  virtual const char *getCharProperty(GWEN_WIDGET *w,
                                      GWEN_DIALOG_PROPERTY prop,
                                      int index,
                                      const char *defaultValue);

};




#endif /* FOX16_DIALOG_L_HPP */


