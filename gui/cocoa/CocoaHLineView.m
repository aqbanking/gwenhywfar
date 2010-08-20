//
//  CocoaHLineView.m
//  
//
//  Created by Samuel Strupp on 10.08.10.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#import "CocoaHLineView.h"


@implementation CocoaHLineView

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

- (void)drawRect:(NSRect)dirtyRect {
	NSRect bounds = [self bounds];
    [[NSColor grayColor] set];
	NSBezierPath *line = [NSBezierPath bezierPath];
	CGFloat y = bounds.origin.y+bounds.size.height/2.0;
	[line moveToPoint:NSMakePoint(bounds.origin.x, y)];
	[line lineToPoint:NSMakePoint(bounds.origin.x + bounds.size.width, y)];
	[line stroke];
}

#pragma mark Protocoll Methods

- (NSSize) minSize {
	return NSMakeSize(3.0, 3.0);
}

- (BOOL) fillX {
	return fillX;
}

- (BOOL) fillY {
	return fillY;
}

@end
