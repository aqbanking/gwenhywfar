//
//  CocoaSpinbox.h
//  
//
//  Created by Samuel Strupp on 19.08.10.
//



#ifndef COCOA_SPINBOX_H
#define COCOA_SPINBOX_H

#import <Cocoa/Cocoa.h>
#import "CocoaGwenGUIProtocol.h"

typedef void (*gwenSpinBoxActionPtr)(NSView *spinbox, void* data);


@interface CocoaSpinbox : NSView <CocoaGwenGUIProtocol> {
	NSStepper *stepper;
	NSTextField *textfield;
	
	CGFloat minWidth;
	
	BOOL fillX;
	BOOL fillY;
	
	NSInteger minValue;
	NSInteger maxValue;
	
	gwenSpinBoxActionPtr c_actionPtr;
	void* c_actionData;
}

@property BOOL fillX;
@property BOOL fillY;

-(void) setC_ActionPtr:(gwenSpinBoxActionPtr)ptr Data:(void*)data;

-(void) makeFirstResponder;
-(BOOL) isFirstResponder;

-(void) setEnabled:(BOOL)value;
-(BOOL) isEnabled;

-(void) setIntegerValue:(NSInteger)new_value;
-(NSInteger) integerValue;
-(void) setMinValue:(NSInteger)new_min_value;
-(NSInteger) minValue;
-(void) setMaxValue:(NSInteger)new_max_value;
-(NSInteger) maxValue;

-(void)setStringValue:(NSString *)aString;
-(NSString*) stringValue;

@end

#endif
