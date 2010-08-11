/***************************************************************************
    begin       : Tue Aug 10 2010
    copyright   : (C) 2010 by Samuel Strupp

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#import "CocoaHLineView.h"


@implementation CocoaHLineView

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    return self;
}

-(void) dealloc {
	[super dealloc];
}

- (void)drawRect:(NSRect)dirtyRect {
	//nur zum debuggen
	NSRect bounds = [self bounds];
    [[NSColor yellowColor] set];
	NSBezierPath *line = [NSBezierPath bezierPath];
	CGFloat y = bounds.origin.y+bounds.size.height/2.0;
	[line moveToPoint:NSMakePoint(bounds.origin.x, y)];
	[line lineToPoint:NSMakePoint(bounds.origin.x + bounds.size.width, y)];
	[line stroke];
}

@end
