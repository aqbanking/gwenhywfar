//
//  CocoaButton.m
//  
//
//  Created by Samuel Strupp on 10.08.10.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#import "CocoaButton.h"

#ifndef COCOA_GWEN_BUTTON_MM
#define COCOA_GWEN_BUTTON_MM

@implementation CocoaButton

@synthesize fillX;
@synthesize fillY;

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		[self setTarget:self];
		[self setAction:@selector(clicked:)];
		c_actionPtr = nil;
		c_actionData = nil;
		fillX = NO;
		fillY = NO;
		minWidth = 40.0;
    }
    return self;
}

-(void) dealloc {
	[super dealloc];
}


-(NSSize) neededTextSize {
	NSString *title = [self title];
	if (title && [title length]>0) {
		NSDictionary *attributes = [NSDictionary dictionaryWithObjectsAndKeys:
									[NSColor blackColor], NSForegroundColorAttributeName,
									[self font], NSFontAttributeName,
									nil];
		return [title sizeWithAttributes:attributes];
	}
	return NSZeroSize;
}

-(void) computeMinWidth {
	NSSize size = [self neededTextSize];
	minWidth = size.width + 40.0;
	if ([self image]) {
		minWidth += [[self image] size].width;
	}
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

- (void)setTitle:(NSString *)aString {
	[super setTitle:aString];
	[self computeMinWidth];
}

- (void)setImage:(NSImage *)anImage {
	[super setImage:anImage];
	[self computeMinWidth];
}

#pragma mark Protocoll Methods

- (NSSize) minSize {
	return NSMakeSize(minWidth, 32.0);
}

@end

#endif
