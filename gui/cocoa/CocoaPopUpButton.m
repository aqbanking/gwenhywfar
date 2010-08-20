//
//  CocoaPopUpButton.m
//  
//
//  Created by Samuel Strupp on 10.08.10.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#import "CocoaPopUpButton.h"

#ifndef COCOA_POPUP_BUTTON_MM
#define COCOA_POPUP_BUTTON_MM

@implementation CocoaPopUpButton

@synthesize fillX;
@synthesize fillY;

- (id)initWithFrame:(NSRect)frameRect pullsDown:(BOOL)flag {
    self = [super initWithFrame:frameRect pullsDown:flag];
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
	if ([self numberOfItems] > 0) {
		NSSize maxSize = NSZeroSize;
		NSDictionary *attributes = [NSDictionary dictionaryWithObjectsAndKeys:
									[NSColor blackColor], NSForegroundColorAttributeName,
									[self font], NSFontAttributeName,
									nil];
		
		NSArray *titles = [self itemTitles];
		for (NSString *title in titles) {
			NSSize titleSize = [title sizeWithAttributes:attributes];
			if (maxSize.width < titleSize.width) maxSize.width = titleSize.width;
			if (maxSize.height < titleSize.height) maxSize.height = titleSize.height;
		}
		
		return maxSize;
	}
	return NSZeroSize;
}

-(void) computeMinWidth {
	NSSize size = [self neededTextSize];
	minWidth = size.width + 40.0;
}

-(void) setC_PopUpActionPtr:(gwenPopUpActionPtr)ptr Data:(void*)data {
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

- (void)addItemWithTitle:(NSString *)title {
	[super addItemWithTitle:title];
	[self computeMinWidth];
}

#pragma mark Protocoll Methods

- (NSSize) minSize {
	return NSMakeSize(minWidth, 24.0);
}

@end

#endif
