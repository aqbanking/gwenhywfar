/***************************************************************************
    begin       : Tue Aug 10 2010
    copyright   : (C) 2010 by Samuel Strupp

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#import "CocoaGWENButton.h"

#ifndef COCOA_GWEN_BUTTON_MM
#define COCOA_GWEN_BUTTON_MM

@implementation CocoaGWENButton

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		[self setTarget:self];
		[self setAction:@selector(clicked:)];
		c_actionPtr = nil;
		c_actionData = nil;
    }
    return self;
}

-(void) dealloc {
	[super dealloc];
}

-(void) setC_ActionPtr:(gwenActionPtr)ptr Data:(void*)data {
	c_actionPtr = ptr;
	c_actionData = data;
}

-(void) clicked:(id) sender {
	if (c_actionPtr) {
		c_actionPtr(self, c_actionData);
	}
}

@end

#endif
