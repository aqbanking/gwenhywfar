//
//  CocoaGroupBox.m
//  
//
//  Created by Samuel Strupp on 10.08.10.
//
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#import "CocoaGroupBox.h"


@implementation CocoaGroupBox

@synthesize fillX;
@synthesize fillY;

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		fillX = NO;
		fillY = NO;
    }
    return self;
}

-(void) dealloc {
	[super dealloc];
}

#pragma mark Protocoll Methods

- (NSSize) minSize {
	if ([[self contentView] conformsToProtocol:@protocol(CocoaGwenGUIProtocol)]) {
		NSSize size = [(<CocoaGwenGUIProtocol>)[self contentView] minSize];
		size.height += 24.0;
		size.width += 14.0;
		return size;
	}
	return NSMakeSize(22.0, 40.0);
}

- (BOOL) fillX {
	return fillX;
}

- (BOOL) fillY {
	return fillY;
}

@end
