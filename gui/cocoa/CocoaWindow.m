//
//  CocoaLabel.m
//  
//
//  Created by Samuel Strupp on 10.08.10.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#import "CocoaWindow.h"

#ifndef COCOA_WINDOW_MM
#define COCOA_WINDOW_MM

@implementation CocoaWindow


- (id)initWithContentRect:(NSRect)contentRect styleMask:(NSUInteger)windowStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferCreation {
    self = [super initWithContentRect:contentRect styleMask:windowStyle backing:bufferingType defer:deferCreation];
    if (self) {		
		
    }
    return self;
}

-(void) dealloc {
	[super dealloc];
}

@end

#endif
