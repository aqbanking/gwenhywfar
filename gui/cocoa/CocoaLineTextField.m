//
//  CocoaLineTextField.m
//  
//
//  Created by Samuel Strupp on 10.08.10.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#import "CocoaLineTextField.h"

#ifndef COCOA_LINE_TEXTFIELD_MM
#define COCOA_LINE_TEXTFIELD_MM

@implementation CocoaLineTextField

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		[self setTarget:self];
		[self setAction:@selector(textChanged:)];
		c_actionPtr = nil;
		c_actionData = nil;
		
		minWidth = 32.0;
		
		c_textChangedActionPtr = nil;
		c_textChangedActionData = nil;
    }
    return self;
}

-(void) dealloc {
	[super dealloc];
}

-(void) computeMinWidth {
	NSSize size = [self neededTextSize];
	if (size.width > 32.0)
		minWidth = size.width;
	else minWidth = 32.0;
}

-(void) setC_ActionPtr:(gwenTextFieldActionPtr)ptr Data:(void*)data {
	c_actionPtr = ptr;
	c_actionData = data;
}

-(void) setC_TextChanged_ActionPtr:(gwenTextFieldActionPtr)ptr Data:(void*)data {
	c_textChangedActionPtr = ptr;
	c_textChangedActionData = data;
}

-(void) textChanged:(id)sender {
	if (c_actionPtr) {
		c_actionPtr(self, c_actionData);
	}
}

- (void)textDidChange:(NSNotification *)aNotification {
	//NSLog(@"textDidChange");
	if (c_textChangedActionPtr) {
		c_textChangedActionPtr(self, c_textChangedActionData);
	}
	[super textDidChange:aNotification];
}

- (void)setStringValue:(NSString *)aString {
	[super setStringValue:aString]; //damit hebeln wir die automatische Größen Berechnung vom CocoaLabel aus.
}

#pragma mark Protocoll Methods

- (NSSize) minSize {
	return NSMakeSize(minWidth, 22.0);
}

@end

#endif
