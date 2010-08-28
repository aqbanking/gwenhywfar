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

#include "fox16_gui_sortinglist_l.hpp"

#include <gwenhywfar/debug.h>





FXDEFMAP(FOX16_GuiSortingList) FOX16_GuiSortingListMap[]={
  FXMAPFUNC(SEL_COMMAND, FXFoldingList::ID_HEADER_CHANGE, FOX16_GuiSortingList::onHeaderClicked)
};


            
FXIMPLEMENT(FOX16_GuiSortingList, FXFoldingList,
            FOX16_GuiSortingListMap, ARRAYNUMBER(FOX16_GuiSortingListMap))


FOX16_GuiSortingList::FOX16_GuiSortingList(FXComposite *p, FXObject* tgt, FXSelector sel, FXuint opts,
			       FXint x, FXint y, FXint w, FXint h)
:FXFoldingList(p, tgt, sel, opts, x, y, w, h)
{

}



FOX16_GuiSortingList::FOX16_GuiSortingList()
:FXFoldingList() {
}



FOX16_GuiSortingList::~FOX16_GuiSortingList() {
}



FXFoldingItem *FOX16_GuiSortingList::getItem(int index) {
  FXFoldingItem *ti;
  int i=index;

  ti=getFirstItem();
  while(ti && i) {
    ti=ti->getNext();
    i--;
  }

  return ti;
}



FXbool FOX16_GuiSortingList::getHeaderArrowDir(int c) const {
  return header->getArrowDir(c);
}



void FOX16_GuiSortingList::setHeaderArrowDir(int c, FXbool b) {
  header->setArrowDir(c, b);
}



long FOX16_GuiSortingList::onHeaderClicked(FXObject*, FXSelector, void* ptr) {
  FXint idx=(FXint)(FXival)ptr;
  FXbool b;
  int i;

  b=header->getArrowDir(idx);
  for (i=0; i<header->getNumItems(); i++)
    header->setArrowDir(i, MAYBE);

  if (b==TRUE) {
    /* was up, becomes down */
    header->setArrowDir(idx, FALSE);
    sortByColumn(idx, false);
  }
  else if (b==FALSE) {
    /* was down, becomes MAYBE */
    header->setArrowDir(idx, MAYBE);
  }
  else {
    /* was MAYBE, becomes up */
    header->setArrowDir(idx, TRUE);
    sortByColumn(idx, true);
  }
  return 1;
}



void FOX16_GuiSortingList::sortByColumn(int c, bool up) {
  switch(c) {
  case 0:
    if (up) setSortFunc(sortFuncCol0a);
    else setSortFunc(sortFuncCol0d);
    break;
  case 1:
    if (up) setSortFunc(sortFuncCol1a);
    else setSortFunc(sortFuncCol1d);
    break;

  case 2:
    if (up) setSortFunc(sortFuncCol2a);
    else setSortFunc(sortFuncCol2d);
    break;

  case 3:
    if (up) setSortFunc(sortFuncCol3a);
    else setSortFunc(sortFuncCol3d);
    break;

  case 4:
    if (up) setSortFunc(sortFuncCol4a);
    else setSortFunc(sortFuncCol4d);
    break;

  case 5:
    if (up) setSortFunc(sortFuncCol5a);
    else setSortFunc(sortFuncCol5d);
    break;

  case 6:
    if (up) setSortFunc(sortFuncCol6a);
    else setSortFunc(sortFuncCol6d);
    break;

  case 7:
    if (up) setSortFunc(sortFuncCol7a);
    else setSortFunc(sortFuncCol7d);
    break;

  case 8:
    if (up) setSortFunc(sortFuncCol8a);
    else setSortFunc(sortFuncCol8d);
    break;

  case 9:
    if (up) setSortFunc(sortFuncCol9a);
    else setSortFunc(sortFuncCol9d);
    break;

  default:
    DBG_ERROR(GWEN_LOGDOMAIN, "Column %d is out of range (no more functions)", c);
    return;
  }

  sortItems();
}




FXint FOX16_GuiSortingList::sortFuncCol0a(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(a->getText().text(),b->getText().text(), 0);
}



FXint FOX16_GuiSortingList::sortFuncCol0d(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(b->getText().text(),a->getText().text(), 0);
}



FXint FOX16_GuiSortingList::sortFuncCol1a(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(a->getText().text(),b->getText().text(), 1);
}



FXint FOX16_GuiSortingList::sortFuncCol1d(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(b->getText().text(),a->getText().text(), 1);
}



FXint FOX16_GuiSortingList::sortFuncCol2a(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(a->getText().text(),b->getText().text(), 2);
}



FXint FOX16_GuiSortingList::sortFuncCol2d(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(b->getText().text(),a->getText().text(), 2);
}



FXint FOX16_GuiSortingList::sortFuncCol3a(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(a->getText().text(),b->getText().text(), 3);
}



FXint FOX16_GuiSortingList::sortFuncCol3d(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(b->getText().text(),a->getText().text(), 3);
}



FXint FOX16_GuiSortingList::sortFuncCol4a(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(a->getText().text(),b->getText().text(), 4);
}



FXint FOX16_GuiSortingList::sortFuncCol4d(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(b->getText().text(),a->getText().text(), 4);
}



FXint FOX16_GuiSortingList::sortFuncCol5a(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(a->getText().text(),b->getText().text(), 5);
}



FXint FOX16_GuiSortingList::sortFuncCol5d(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(b->getText().text(),a->getText().text(), 5);
}



FXint FOX16_GuiSortingList::sortFuncCol6a(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(a->getText().text(),b->getText().text(), 6);
}



FXint FOX16_GuiSortingList::sortFuncCol6d(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(b->getText().text(),a->getText().text(), 6);
}



FXint FOX16_GuiSortingList::sortFuncCol7a(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(a->getText().text(),b->getText().text(), 7);
}



FXint FOX16_GuiSortingList::sortFuncCol7d(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(b->getText().text(),a->getText().text(), 7);
}



FXint FOX16_GuiSortingList::sortFuncCol8a(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(a->getText().text(),b->getText().text(), 8);
}



FXint FOX16_GuiSortingList::sortFuncCol8d(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(b->getText().text(),a->getText().text(), 8);
}



FXint FOX16_GuiSortingList::sortFuncCol9a(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(a->getText().text(),b->getText().text(), 9);
}



FXint FOX16_GuiSortingList::sortFuncCol9d(const FXFoldingItem *a, const FXFoldingItem *b) {
  return compareSectionCase(b->getText().text(),a->getText().text(), 9);
}











