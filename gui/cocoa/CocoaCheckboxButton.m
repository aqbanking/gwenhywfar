//
//  CocoaCheckboxButton.m
//
//  Created by Samuel Strupp on 12.08.10.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#import "CocoaCheckboxButton.h"


@implementation CocoaCheckboxButton

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		[self setBezelStyle:NSRoundedBezelStyle];
		[self setButtonType:NSSwitchButton];
    }
    return self;
}

-(void) dealloc {
	[super dealloc];
}

-(void) computeMinWidth {
	NSSize size = [self neededTextSize];
	minWidth = size.width + 22.0;
}

/*- (void)drawRect:(NSRect)dirtyRect {
	//debug colors
    [[NSColor orangeColor] set];
	NSRectFill(dirtyRect);
	[super drawRect:dirtyRect];
}*/

#pragma mark Protocoll Methods

- (NSSize) minSize {
	return NSMakeSize(minWidth, 16.0);
}

@end