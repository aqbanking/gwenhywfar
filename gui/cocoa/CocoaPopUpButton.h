//
//  CocoaPopUpButton.h
//
//
//  Created by Samuel Strupp on 10.08.10.
//



#ifndef COCOA_POPUP_BUTTON_H
#define COCOA_POPUP_BUTTON_H

#import <Cocoa/Cocoa.h>
#import "CocoaGwenGUIProtocol.h"

typedef void (*gwenPopUpActionPtr)(NSPopUpButton *button, void *data);


@interface CocoaPopUpButton : NSPopUpButton <CocoaGwenGUIProtocol>
{
  gwenPopUpActionPtr c_actionPtr;
  void *c_actionData;
  BOOL fillX;
  BOOL fillY;

  CGFloat minWidth;
}

@property BOOL fillX;
@property BOOL fillY;

-(void) setC_PopUpActionPtr:(gwenPopUpActionPtr)ptr Data:(void *)data;

@end

#endif
