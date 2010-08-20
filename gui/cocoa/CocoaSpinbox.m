//
//  CocoaSpinbox.m
//  
//
//  Created by Samuel Strupp on 19.08.10.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#import "CocoaSpinbox.h"

#ifndef COCOA_SPINBOX_MM
#define COCOA_SPINBOX_MM

@implementation CocoaSpinbox

@synthesize fillX;
@synthesize fillY;

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		
		textfield = [[NSTextField alloc] initWithFrame:NSMakeRect(0.0, 0.0, 40.0, 27.0)];
		[textfield setAlignment:NSRightTextAlignment];
		[textfield setTarget:self];
		[textfield setAction:@selector(getValueFromTextField:)];
		[self addSubview:textfield];
		
		stepper = [[NSStepper alloc] initWithFrame:NSMakeRect(40.0, 0.0, 19.0, 27.0)];
		[stepper setTarget:self];
		[stepper setAction:@selector(getValueFromStepper:)];
		[stepper setValueWraps:NO];
		[self addSubview:stepper];
		
		fillX = NO;
		fillY = NO;
		
		minValue = NSIntegerMin;
		maxValue = NSIntegerMax;
		
		[stepper setMinValue:minValue];
		[stepper setMaxValue:maxValue];
		
		c_actionPtr = nil;
		c_actionData = nil;
		
		minWidth = 40.0;
    }
    return self;
}

-(void) dealloc {
	[stepper release];
	[textfield release];
	[super dealloc];
}

-(void) setC_ActionPtr:(gwenSpinBoxActionPtr)ptr Data:(void*)data {
	c_actionPtr = ptr;
	c_actionData = data;
}

-(void) valueDidChange {
	if (c_actionPtr) {
		c_actionPtr(self, c_actionData);
	}
}


-(void) setFrame:(NSRect)frameRect {
	if (frameRect.size.height < 22.0) frameRect.size.height = 22.0;
	[super setFrame:frameRect];
	NSRect bounds = [self bounds];
	NSRect stepperFrame = NSMakeRect(bounds.origin.x+bounds.size.width-16.0, bounds.origin.y-3.0, 19.0, 27.0);
	[stepper setFrame:stepperFrame];
	
	NSRect textfieldFrame = NSMakeRect(bounds.origin.x, bounds.origin.y, bounds.size.width-16.0, 22.0);
	[textfield setFrame:textfieldFrame];
}

-(void) makeFirstResponder {
	if ([textfield window]) {
		[[textfield window] makeFirstResponder:textfield];
	}
}

-(BOOL) isFirstResponder {
	if ([textfield window]) {
		if ([[textfield window] firstResponder] == textfield) return YES;
	}
	return NO;
}

-(void) setEnabled:(BOOL)value {
	[textfield setEnabled:value];
	[stepper setEnabled:value];
}

-(BOOL) isEnabled {
	return [textfield isEnabled];
}


-(void) setIntegerValue:(NSInteger)new_value {
	if (new_value >= minValue && new_value <= maxValue) {
		[textfield setIntegerValue:new_value];
		[stepper setIntegerValue:new_value];
	}
}

-(NSInteger) integerValue {
	return [textfield integerValue];
}

-(void) setMinValue:(NSInteger)new_min_value {
	minValue = new_min_value;
	[stepper setMinValue:minValue];
	if (maxValue < minValue) maxValue = minValue;
	if ([self integerValue] < minValue) [self setIntegerValue:minValue];
}

-(NSInteger) minValue {
	return minValue;
}

-(void) setMaxValue:(NSInteger)new_max_value {
	maxValue = new_max_value;
	[stepper setMaxValue:maxValue];
	if (maxValue < minValue) minValue = maxValue;
	if ([self integerValue] > maxValue) [self setIntegerValue:maxValue];
}

-(NSInteger) maxValue {
	return maxValue;
}


- (void)setStringValue:(NSString *)aString {
	NSInteger value = 0;
	if (aString && [aString length] > 0) {
		value = [aString integerValue];
	}
	[self setIntegerValue:value];
}

-(NSString*) stringValue {
	return [textfield stringValue];
}

-(void) getValueFromStepper:(id)sender {
	NSInteger new_value = [stepper integerValue];
	if (new_value >= minValue && new_value <= maxValue) {
		[textfield setIntegerValue:new_value];
		[self valueDidChange];
	}
}

-(void) getValueFromTextField:(id)sender {
	NSInteger new_value = [textfield integerValue];
	if (new_value < minValue) new_value = minValue;
	if (new_value > maxValue) new_value = maxValue;
	[textfield setIntegerValue:new_value];
	[stepper setIntegerValue:new_value];
	[self valueDidChange];
}

#pragma mark Protocol Methods


- (NSSize) minSize {
	return NSMakeSize(minWidth, 22.0);
}

@end

#endif
