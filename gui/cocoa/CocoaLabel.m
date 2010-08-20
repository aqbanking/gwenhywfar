//
//  CocoaLabel.m
//  
//
//  Created by Samuel Strupp on 10.08.10.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#import "CocoaLabel.h"

#ifndef COCOA_LABEL_MM
#define COCOA_LABEL_MM

@implementation CocoaLabel

@synthesize fillX;
@synthesize fillY;

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {		
		fillX = NO;
		fillY = NO;
		minWidth = 0.0;
    }
    return self;
}

-(void) dealloc {
	[super dealloc];
}

-(NSSize) neededTextSize {
	NSString *title = [self stringValue];
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
	minWidth = size.width+4.0;
}

- (void)setStringValue:(NSString *)aString {
	
	/*remove HTML version if available*/
	if (aString) {
		NSRange htmlRange = [aString rangeOfString:@"<html>"];
		if (htmlRange.location != NSNotFound) {
			NSRange endHtmlRange = [aString rangeOfString:@"</html>"];
			if (endHtmlRange.location != NSNotFound) {
				NSString *stringToUse = @"";
				NSRange cutRange = NSUnionRange(htmlRange, endHtmlRange);
				stringToUse = [aString stringByReplacingCharactersInRange:cutRange withString:@""];
				[super setStringValue:stringToUse];
				[self computeMinWidth];
				return;
			}
		}
	}
	[super setStringValue:aString];
	[self computeMinWidth];
}

/*- (void)drawRect:(NSRect)dirtyRect {
	//debug colors
    [[NSColor yellowColor] set];
	NSRectFill(dirtyRect);
	[super drawRect:dirtyRect];
}*/

#pragma mark Protocol Methods

- (NSSize) minSize {
	return NSMakeSize(minWidth, 17.0);
}

@end

#endif
