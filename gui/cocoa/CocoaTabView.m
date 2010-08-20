//
//  CocoaTabView.m
//  
//
//  Created by Samuel Strupp on 10.08.10.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#import "CocoaTabView.h"


@implementation CocoaTabView

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
	NSArray *tabViewItems = [self tabViewItems];
	NSSize maxSize = NSZeroSize;
	for (NSTabViewItem *item in tabViewItems) {
		NSView *subview = [item view];
		if ([subview conformsToProtocol:@protocol(CocoaGwenGUIProtocol)]) {
			NSSize subViewMinSize = [(<CocoaGwenGUIProtocol>)subview minSize];
			if (subViewMinSize.width > maxSize.width) maxSize.width = subViewMinSize.width;
			if (subViewMinSize.height > maxSize.height) maxSize.height = subViewMinSize.height;
		}
	}
	if ([self tabViewType] != NSNoTabsNoBorder) {
		maxSize.width += 22.0;
		maxSize.height += 40.0;
	}
	return maxSize;
}

- (BOOL) fillX {
	return fillX;
}

- (BOOL) fillY {
	return fillY;
}

@end
