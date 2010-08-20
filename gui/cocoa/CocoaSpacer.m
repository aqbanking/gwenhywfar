//
//  CocoaSpacer.m
//  
//
//  Created by Samuel Strupp on 17.08.10.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#import "CocoaSpacer.h"

#ifndef COCOA_SPACER_MM
#define COCOA_SPACER_MM

@implementation CocoaSpacer

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
	return NSMakeSize(0.0, 0.0);
}

- (BOOL) fillX {
	return fillX;
}

- (BOOL) fillY {
	return fillY;
}

@end

#endif
