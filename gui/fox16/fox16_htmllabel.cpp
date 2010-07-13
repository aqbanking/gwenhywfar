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

#include "fox16_htmllabel.hpp"
#include "fox16_htmlctx.hpp"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/i18n.h>

#include <assert.h>


#define MAX_DEFAULT_WIDTH 400
#define ICON_SPACE        4



FXDEFMAP(FOX16_HtmlLabel) FOX16_HtmlLabelMap[]={
  FXMAPFUNC(SEL_PAINT,0,FOX16_HtmlLabel::onPaint),
};



// Object implementation
FXIMPLEMENT(FOX16_HtmlLabel, FXFrame, FOX16_HtmlLabelMap, ARRAYNUMBER(FOX16_HtmlLabelMap))




FOX16_HtmlLabel::FOX16_HtmlLabel(FXComposite* p, const FXString& text,
				 FXuint opts,
				 FXint x, FXint y, FXint w, FXint h,
				 FXint pl, FXint pr,
				 FXint pt, FXint pb)
:FXFrame(p, opts, x, y, w, h, pl, pr, pt, pb)
,m_htmlCtx(NULL)
,m_minWidth(0)
,m_maxDefaultWidth(MAX_DEFAULT_WIDTH)
,m_haveDefaultDims(false)
,m_icon(NULL)
{
  setText(text);
  flags|=FLAG_ENABLED|FLAG_DIRTY|FLAG_RECALC;
}



FOX16_HtmlLabel::FOX16_HtmlLabel()
:FXFrame()
,m_htmlCtx(NULL)
,m_minWidth(0)
,m_icon(NULL)
{
  flags|=FLAG_ENABLED;
}



FOX16_HtmlLabel::~FOX16_HtmlLabel() {
  if (m_htmlCtx)
    delete m_htmlCtx;
}



void FOX16_HtmlLabel::setText(const FXString& text) {
  m_haveDefaultDims=false;
  m_text=text;
  updateHtml();
  flags|=FLAG_DIRTY;
  layout();
  recalc();
  update();
}



void FOX16_HtmlLabel::setIcon(FXIcon *ic) {
  m_icon=ic;
  flags|=FLAG_DIRTY;
  layout();
  recalc();
  update();
}



void FOX16_HtmlLabel::calcDefaultDims() {
#if 0
  int w;
  int wNeeded;

  m_htmlCtx->layout(-1, -1);
  wNeeded=m_htmlCtx->getWidth();
  w=wNeeded;
  if (w>m_maxDefaultWidth)
    w=m_maxDefaultWidth;
  if (w<width)
    w=width;
  if (w<wNeeded) {
    m_htmlCtx->layout(w-border*2, -1);
  }
  m_defaultWidth=m_htmlCtx->getWidth();
  m_defaultHeight=m_htmlCtx->getHeight();
  m_haveDefaultDims=true;
#else
  int w;

  if (options & FLAGS_NO_WORDWRAP)
    w=-1;
  else if (options & FLAGS_USE_FULL_WIDTH)
    w=width;
  else
    w=m_maxDefaultWidth;
  m_htmlCtx->layout(w-border*2, -1);
  m_defaultWidth=m_htmlCtx->getWidth();
  m_defaultHeight=m_htmlCtx->getHeight();
  m_haveDefaultDims=true;
#endif
}



FXint FOX16_HtmlLabel::getDefaultWidth() {
  int w;

  if (m_htmlCtx==NULL)
    updateHtml();
  if (!m_haveDefaultDims)
    calcDefaultDims();

  w=m_defaultWidth;
  if (m_icon)
    w+=m_icon->getWidth()+ICON_SPACE;
  return w;
}



FXint FOX16_HtmlLabel::getDefaultHeight() {
  int h;

  if (m_htmlCtx==NULL)
    updateHtml();
  if (!m_haveDefaultDims)
    calcDefaultDims();
  h=m_defaultHeight;
  if (m_icon) {
    int ih;

    ih=m_icon->getHeight();
    if (ih>h)
      h=ih;
  }

  return h;
}



long FOX16_HtmlLabel::onPaint(FXObject*, FXSelector, void *ptr) {
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this, ev);

  dc.setForeground(backColor);
  dc.fillRectangle(border, border, width-(border*2), height-(border*2));

  if (m_htmlCtx) {
    if (m_icon) {
      int th;
      int ih;
      int ty=border;

      if(isEnabled())
	dc.drawIcon(m_icon, border, border);
      else
	dc.drawIconSunken(m_icon, border, border);

      ih=m_icon->getHeight();
      th=m_htmlCtx->getHeight();
      if (ih>th)
	ty+=(ih-th)/2;
      m_htmlCtx->paint(&dc, border+ICON_SPACE+m_icon->getWidth(), ty);
    }
    else {
      m_htmlCtx->paint(&dc, border, border);
    }
  }
  else {
    DBG_ERROR(0, "No HtmlContext");
  }

  drawFrame(dc, 0, 0, width, height);
  return 1;
}



void FOX16_HtmlLabel::create() {
  FXFrame::create();
  if (m_icon)
    m_icon->create();
  updateHtml();
  recalc();
}



void FOX16_HtmlLabel::layout() {
#if 0
  int w;

  m_haveDefaultDims=false;
  if (options & FLAGS_NO_WORDWRAP)
    w=-1;
  else
    w=width;

  if (m_htmlCtx==NULL)
    updateHtml();
  m_htmlCtx->layout(w-border*2, height-border*2);
  update();
  flags&=~FLAG_DIRTY;
#else
  int w;

  m_haveDefaultDims=false;
  if (options & FLAGS_NO_WORDWRAP)
    w=-1;
  else if (options & FLAGS_USE_FULL_WIDTH) {
    w=width;
    if (m_icon)
      w-=(m_icon->getWidth()+ICON_SPACE);
  }
  else
    w=m_maxDefaultWidth;

  if (m_htmlCtx==NULL)
    updateHtml();
  m_htmlCtx->layout(w-border*2, height-border*2);
  update();
  flags&=~FLAG_DIRTY;
#endif
}



void FOX16_HtmlLabel::updateHtml() {
  if (m_htmlCtx)
    delete m_htmlCtx;
  m_haveDefaultDims=false;
  m_htmlCtx=new FOX16_HtmlCtx(0);
  m_htmlCtx->setBackgroundColor(backColor);
  m_htmlCtx->setForegroundColor(fxcolorfromname("black"));
  m_htmlCtx->setText(m_text.text());
  flags|=FLAG_DIRTY;
}







