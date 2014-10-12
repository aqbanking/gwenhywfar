/***************************************************************************
    begin       : Tue Feb 23 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "fox16_htmltext.hpp"
#include "fox16_htmlctx.hpp"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/i18n.h>

#include <assert.h>


#define MAX_DEFAULT_WIDTH 400
#define BORDER 2



FXDEFMAP(FOX16_HtmlText) FOX16_HtmlTextMap[]= {
  FXMAPFUNC(SEL_PAINT,0,FOX16_HtmlText::onPaint),
};



// Object implementation
FXIMPLEMENT(FOX16_HtmlText, FXScrollArea, FOX16_HtmlTextMap, ARRAYNUMBER(FOX16_HtmlTextMap))




FOX16_HtmlText::FOX16_HtmlText(FXComposite* p, const FXString& text,
                               FXuint opts,
                               FXint x, FXint y, FXint w, FXint h)
  :FXScrollArea(p, opts, x, y, w, h)
  ,m_htmlCtx(NULL)
  ,m_minWidth(0)
  ,m_maxDefaultWidth(MAX_DEFAULT_WIDTH)
  ,m_haveDefaultDims(false)
  ,margintop(BORDER)
  ,marginbottom(BORDER)
  ,marginleft(BORDER)
  ,marginright(BORDER)
  ,barwidth(8) {
  setText(text);
  flags|=FLAG_ENABLED;
}



FOX16_HtmlText::FOX16_HtmlText()
  :FXScrollArea()
  ,m_htmlCtx(NULL)
  ,m_minWidth(0) {
  flags|=FLAG_ENABLED;
}



FOX16_HtmlText::~FOX16_HtmlText() {
  if (m_htmlCtx)
    delete m_htmlCtx;
}



void FOX16_HtmlText::setText(const FXString& text) {
  m_haveDefaultDims=false;
  m_text=text;
  updateHtml();
  flags|=FLAG_DIRTY;
  layout();
  recalc();
  update();
}



void FOX16_HtmlText::calcDefaultDims() {
  int w;
  int wNeeded;

  m_htmlCtx->layout(-1, -1);
  wNeeded=m_htmlCtx->getWidth();
  w=wNeeded;
  if (w>m_maxDefaultWidth)
    w=m_maxDefaultWidth;
  if (w<viewport_w)
    w=viewport_w;
  //if (w<wNeeded) {
  m_htmlCtx->layout(w-BORDER*2, -1);
  //}
  m_defaultWidth=m_htmlCtx->getWidth();
  m_defaultHeight=m_htmlCtx->getHeight();
  m_haveDefaultDims=true;
}



FXint FOX16_HtmlText::getContentWidth() {
  if (m_htmlCtx==NULL)
    updateHtml();

  if (!m_haveDefaultDims)
    calcDefaultDims();

  m_htmlCtx->layout(viewport_w-(marginleft+marginright+barwidth), -1);
  return m_htmlCtx->getWidth();
}



FXint FOX16_HtmlText::getContentHeight() {
  if (m_htmlCtx==NULL)
    updateHtml();

  if (!m_haveDefaultDims)
    calcDefaultDims();

  return m_htmlCtx->getHeight();
}



long FOX16_HtmlText::onPaint(FXObject*, FXSelector, void *ptr) {
  FXEvent* event=(FXEvent*)ptr;
  FXDCWindow dc(this,event);
  //dc.setFont(font);
  //dc.setForeground(FXRGB(255,0,0));
  //dc.fillRectangle(event->rect.x,event->rect.y,event->rect.w,event->rect.h);

  dc.setForeground(backColor);
  dc.fillRectangle(event->rect.x, event->rect.y, event->rect.w, event->rect.h);

  // Paint top margin
  if (event->rect.y<=margintop) {
    dc.setForeground(backColor);
    dc.fillRectangle(0, 0, viewport_w, margintop);
  }

  // Paint bottom margin
  if (event->rect.y+event->rect.h>=viewport_h-marginbottom) {
    dc.setForeground(backColor);
    dc.fillRectangle(0, viewport_h-marginbottom, viewport_w, marginbottom);
  }

  // Paint left margin
  if(event->rect.x<marginleft) {
    dc.setForeground(backColor);
    dc.fillRectangle(0, margintop, marginleft, viewport_h-margintop-marginbottom);
  }

  // Paint right margin
  if(event->rect.x+event->rect.w>=viewport_w-marginright) {
    dc.setForeground(backColor);
    dc.fillRectangle(viewport_w-marginright, margintop, marginright, viewport_h-margintop-marginbottom);
  }

  // Paint text
  //dc.setClipRectangle(marginleft, margintop, viewport_w-marginright-marginleft, viewport_h-margintop-marginbottom);

  //drawContents(dc, event->rect.x, event->rect.y, event->rect.w, event->rect.h);

  if (m_htmlCtx) {
#if 0 /* this doesn work */
    m_htmlCtx->paintAt(&dc, -marginleft-pos_x, -margintop-pos_y,
                       event->rect.x,
                       event->rect.y,
                       event->rect.w,
                       event->rect.h);
#else
    m_htmlCtx->paintAt(&dc, -marginleft-pos_x, -margintop-pos_y,
                       0, 0,
                       viewport_w-(marginleft+marginright+barwidth),
                       viewport_h-(margintop+marginbottom));
#endif
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "No HtmlContext");
  }

  return 1;
}




void FOX16_HtmlText::layout() {
  int w;

  m_haveDefaultDims=false;
  if (options & FLAGS_NO_WORDWRAP)
    w=-1;
  else
    w=viewport_w;

  if (m_htmlCtx==NULL)
    updateHtml();
  m_htmlCtx->layout(w-(marginleft+marginright+barwidth), height-(margintop+marginbottom));

  // Scrollbars adjusted
  FXScrollArea::layout();

  update();
  flags&=~FLAG_DIRTY;
}



void FOX16_HtmlText::updateHtml() {
  if (m_htmlCtx)
    delete m_htmlCtx;
  m_htmlCtx=new FOX16_HtmlCtx(0);
  m_htmlCtx->setBackgroundColor(backColor);
  m_htmlCtx->setForegroundColor(fxcolorfromname("black"));
  m_htmlCtx->setText(m_text.text());
  flags|=FLAG_DIRTY;
}


void FOX16_HtmlText::makePositionVisible(FXint pos) {
  FXint xPos, yPos;

  xPos=pos_x;
  yPos=getContentHeight()-viewport_h;
  if (xPos>=0 && yPos>=0)
    setPosition(-xPos, -yPos);
}





