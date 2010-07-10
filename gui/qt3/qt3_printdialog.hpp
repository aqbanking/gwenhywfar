/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef QBANKING_PRINTDIALOG_H
#define QBANKING_PRINTDIALOG_H

#include "qt3_printdialog.ui.hpp"

#include <qstring.h>

class QPrinter;


class QT3_PrintDialog : public QT3_PrintDialogUi {
  Q_OBJECT
public:
  QT3_PrintDialog(const char *docTitle,
		  const char *docType,
		  const char *descr,
		  const char *text,
		  QWidget* parent=0,
		  const char* name=0,
		  bool modal=FALSE,
		  WFlags fl=0);
  ~QT3_PrintDialog();

  void accept();

public slots:
  void slotPrint();
  void slotSetup();
  void slotFont();
  void slotHelpClicked();

private:
  const char *_docTitle;
  const char *_docType;
  const char *_descr;
  const char *_text;
  QString _fontFamily;
  int _fontSize;
  int _fontWeight;

  void loadPrinterSetup(QPrinter *printer);
  void savePrinterSetup(QPrinter *printer);

  void loadGuiSetup();
  void saveGuiSetup();

};




#endif
