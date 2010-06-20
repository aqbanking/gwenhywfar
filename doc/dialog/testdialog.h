/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

/** @file dialog/testdialog.h
 * This is the public header file for the dialog. It is to be included
 * by the sources using this dialog.
 */

#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/dialog.h>



/**
 * This is the constructor for the test dialog.
 * It reads the dialog description from the XML file
 * "testdialog.dlg". It also sets the signal handler.
 * As a result of this call the dialog is completely
 * setup for use with @ref GWEN_Gui_ExecDialog().
 */
GWEN_DIALOG *TestDialog_new();



#endif

