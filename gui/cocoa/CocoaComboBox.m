//
//  CocoaComboBox.m
//  
//
//  Created by Samuel Strupp on 10.08.10.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#import "CocoaComboBox.h"

#ifndef COCOA_COMBOBOX_MM
#define COCOA_COMBOBOX_MM

@implementation CocoaComboBox

@synthesize fillX;
@synthesize fillY;

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		[self setTarget:self];
		[self setAction:@selector(textChanged:)];
		c_actionPtr = nil;
		c_actionData = nil;
		
		c_actionPtr = nil;
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
		NSArray *objectValues = [self objectValues];
		
		for (NSString *s in objectValues) {
			NSSize titleSize = [s sizeWithAttributes:attributes];
			if (maxSize.width < titleSize.width) maxSize.width = titleSize.width;
			if (maxSize.height < titleSize.height) maxSize.height = titleSize.height;
		}
		
		return maxSize;
	}
	return NSZeroSize;
}

-(void) computeMinWidth {
	NSSize size = [self neededTextSize];
	minWidth = size.width+44.0;
}

-(void) setC_ComboBoxActionPtr:(gwenComboBoxActionPtr)ptr Data:(void*)data {
	c_actionPtr = ptr;
	c_actionData = data;
}

-(void) textChanged:(id)sender {
	if (c_actionPtr) {
		c_actionPtr(self, c_actionData);
	}
}

- (void)setStringValue:(NSString *)aString {
	[super setStringValue:aString]; //damit hebeln wir die automatische Größen Berechnung vom CocoaLabel aus.
}

- (void)addItemWithObjectValue:(id)anObject {
	[super addItemWithObjectValue:anObject];
	[self computeMinWidth];
}

#pragma mark Protocoll Methods

- (NSSize) minSize {
	return NSMakeSize(minWidth, 24.0);
}

@end

#endif
