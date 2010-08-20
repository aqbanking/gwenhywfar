//
//  CocoaProgressbar.m
//  
//
//  Created by Samuel Strupp on 10.08.10.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#import "CocoaProgressbar.h"


@implementation CocoaProgressbar

@synthesize fillX;
@synthesize fillY;

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
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
	return NSMakeSize(5.0, 20.0);
}

- (BOOL) fillX {
	return fillX;
}

- (BOOL) fillY {
	return fillY;
}

@end
