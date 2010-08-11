/***************************************************************************
    begin       : Tue Aug 10 2010
    copyright   : (C) 2010 by Samuel Strupp

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#import "CocoaHLayout.h"


@implementation CocoaHLayout

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
		subviewsInOrder = [[NSMutableArray alloc] init];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(layoutSubviews) name:NSViewFrameDidChangeNotification object:self];
    }
    return self;
}

-(void) dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[subviewsInOrder release];
	[super dealloc];
}

- (void)drawRect:(NSRect)dirtyRect {
	//nur zum debuggen
    [[NSColor greenColor] set];
	NSRectFill(dirtyRect);
}

#define borderDistance 8.0
#define cellDistance 2.0

-(void) layoutSubviews {
	NSRect bounds = [self bounds];
	
	NSUInteger numOfSubViews = [subviewsInOrder count];
	
	if (numOfSubViews > 0) {
		CGFloat stdWidth = (bounds.size.width-(borderDistance+borderDistance)-((numOfSubViews-1)*cellDistance))/numOfSubViews;
		
		NSUInteger i;
		NSRect actualFrame = bounds;
		actualFrame.origin.x = borderDistance;
		actualFrame.origin.y = borderDistance;
		actualFrame.size.height -= borderDistance+borderDistance;
		actualFrame.size.width = stdWidth;
		for (i=0; i<numOfSubViews; i++) {
			NSView* subview = [subviewsInOrder objectAtIndex:i];
			[subview setFrame:actualFrame];
			actualFrame.origin.x += stdWidth+cellDistance;
		}
	}
	
}

-(void) addLayoutSubview:(NSView*)new_subview {
	[subviewsInOrder addObject:new_subview];
	[self addSubview:new_subview];
	[self layoutSubviews];
}

@end
