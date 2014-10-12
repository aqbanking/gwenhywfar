/***************************************************************************
    begin       : Fri Jan 22 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef FOX16_GUI_SORTINGLIST_HPP
#define FOX16_GUI_SORTINGLIST_HPP


#include <fx.h>



class FOX16_GuiSortingList: public FXFoldingList {
  FXDECLARE(FOX16_GuiSortingList)
public:

public:
  FOX16_GuiSortingList(FXComposite *p, FXObject* tgt=NULL, FXSelector sel=0, FXuint opts=0,
                       FXint x=0, FXint y=0, FXint w=0, FXint h=0);

  virtual ~FOX16_GuiSortingList();

  FXFoldingItem *getItem(int idx);

  virtual void sortByColumn(int c, bool up);

  FXbool getHeaderArrowDir(int c) const;
  void setHeaderArrowDir(int c, FXbool b);

  long onHeaderClicked(FXObject*, FXSelector, void* ptr);

protected:
  FOX16_GuiSortingList();

  static FXint sortFuncCol0a(const FXFoldingItem*,const FXFoldingItem*);
  static FXint sortFuncCol0d(const FXFoldingItem*,const FXFoldingItem*);

  static FXint sortFuncCol1a(const FXFoldingItem*,const FXFoldingItem*);
  static FXint sortFuncCol1d(const FXFoldingItem*,const FXFoldingItem*);

  static FXint sortFuncCol2a(const FXFoldingItem*,const FXFoldingItem*);
  static FXint sortFuncCol2d(const FXFoldingItem*,const FXFoldingItem*);

  static FXint sortFuncCol3a(const FXFoldingItem*,const FXFoldingItem*);
  static FXint sortFuncCol3d(const FXFoldingItem*,const FXFoldingItem*);

  static FXint sortFuncCol4a(const FXFoldingItem*,const FXFoldingItem*);
  static FXint sortFuncCol4d(const FXFoldingItem*,const FXFoldingItem*);

  static FXint sortFuncCol5a(const FXFoldingItem*,const FXFoldingItem*);
  static FXint sortFuncCol5d(const FXFoldingItem*,const FXFoldingItem*);

  static FXint sortFuncCol6a(const FXFoldingItem*,const FXFoldingItem*);
  static FXint sortFuncCol6d(const FXFoldingItem*,const FXFoldingItem*);

  static FXint sortFuncCol7a(const FXFoldingItem*,const FXFoldingItem*);
  static FXint sortFuncCol7d(const FXFoldingItem*,const FXFoldingItem*);

  static FXint sortFuncCol8a(const FXFoldingItem*,const FXFoldingItem*);
  static FXint sortFuncCol8d(const FXFoldingItem*,const FXFoldingItem*);

  static FXint sortFuncCol9a(const FXFoldingItem*,const FXFoldingItem*);
  static FXint sortFuncCol9d(const FXFoldingItem*,const FXFoldingItem*);

};



#endif




