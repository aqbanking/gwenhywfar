//
//  CocoaComboBox.h
//
//
//  Created by Samuel Strupp on 10.08.10.
//



#ifndef COCOA_COMBOBOX_H
#define COCOA_COMBOBOX_H

#import <Cocoa/Cocoa.h>
#import "CocoaGwenGUIProtocol.h"

typedef void (*gwenComboBoxActionPtr)(NSComboBox *comboBox, void *data);


@interface CocoaComboBox : NSComboBox <CocoaGwenGUIProtocol>
{

  BOOL fillX;
  BOOL fillY;

  CGFloat minWidth;

  gwenComboBoxActionPtr c_actionPtr;
  void *c_actionData;
}

@property BOOL fillX;
@property BOOL fillY;

-(void) setC_ComboBoxActionPtr:(gwenComboBoxActionPtr)ptr Data:(void *)data;

@end

#endif
