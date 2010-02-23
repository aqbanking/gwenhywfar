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
,m_text(text)
,m_minWidth(0)
{
  flags|=FLAG_ENABLED;
}



FOX16_HtmlLabel::FOX16_HtmlLabel()
:FXFrame()
,m_htmlCtx(NULL)
,m_minWidth(0)
{
  flags|=FLAG_ENABLED;
}



FOX16_HtmlLabel::~FOX16_HtmlLabel() {
  if (m_htmlCtx)
    delete m_htmlCtx;
}



void FOX16_HtmlLabel::setText(const FXString& text) {
  m_text=text;
  updateHtml();
  recalc();
  layout();
}



FXint FOX16_HtmlLabel::getDefaultWidth() {
  if (m_htmlCtx==NULL)
    updateHtml();

  if (flags & FLAG_DIRTY)
    layout();

  return m_htmlCtx->getWidth();
}



FXint FOX16_HtmlLabel::getDefaultHeight() {
  if (m_htmlCtx==NULL)
    updateHtml();

  if (flags & FLAG_DIRTY)
    layout();

  return m_htmlCtx->getHeight();
}



long FOX16_HtmlLabel::onPaint(FXObject*, FXSelector, void *ptr) {
  FXEvent *ev=(FXEvent*)ptr;
  FXDCWindow dc(this, ev);

  dc.setForeground(backColor);
  dc.fillRectangle(border, border, width-(border*2), height-(border*2));

  if (m_htmlCtx==NULL)
    updateHtml();
  m_htmlCtx->paint(&dc, 0, 0);

  drawFrame(dc, 0, 0, width, height);
  return 1;
}



void FOX16_HtmlLabel::layout() {
  int w;

  w=width;
  if (w<m_minWidth)
    w=m_minWidth;

  if (m_htmlCtx==NULL)
    updateHtml();
  m_htmlCtx->layout(w-border*2, height-border*2);
  flags&=~FLAG_DIRTY;
}



void FOX16_HtmlLabel::updateHtml() {
  HTML_PROPS *pr;

  if (m_htmlCtx)
    delete m_htmlCtx;
  m_htmlCtx=new FOX16_HtmlCtx(0, 0, 2000);
  pr=m_htmlCtx->getStandardProps();
  HtmlProps_SetForegroundColor(pr, fxcolorfromname("black"));

  m_htmlCtx->setText(m_text.text());
}







