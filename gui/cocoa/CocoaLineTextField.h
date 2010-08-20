//
//  CocoaLineTextField.h
//  
//
//  Created by Samuel Strupp on 10.08.10.
//



#ifndef COCOA_LINE_TEXTFIELD_H
#define COCOA_LINE_TEXTFIELD_H

#import <Cocoa/Cocoa.h>
#import "CocoaLabel.h"

typedef void (*gwenTextFieldActionPtr)(NSTextField *textField, void* data);


@interface CocoaLineTextField : CocoaLabel {
	gwenTextFieldActionPtr c_actionPtr;
	void* c_actionData;
	
	gwenTextFieldActionPtr c_textChangedActionPtr;
	void* c_textChangedActionData;
}

-(void) setC_ActionPtr:(gwenTextFieldActionPtr)ptr Data:(void*)data;
-(void) setC_TextChanged_ActionPtr:(gwenTextFieldActionPtr)ptr Data:(void*)data;

@end

#endif
