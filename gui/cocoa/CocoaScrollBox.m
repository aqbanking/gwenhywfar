//
//  CocoaScrollBox.m
//  
//
//  Created by Samuel Strupp on 17.08.10.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#import "CocoaScrollBox.h"

#ifndef COCOA_SCROLL_BOX_M
#define COCOA_SCROLL_BOX_M

@implementation CocoaScrollBox

@synthesize fillX;
@synthesize fillY;

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		fillX = NO;
		fillY = NO;
		//subviewsInOrder = [[NSMutableArray alloc] init];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(layoutSubviews) name:NSViewFrameDidChangeNotification object:self];
    }
    return self;
}

-(void) dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	//[subviewsInOrder release];
	[super dealloc];
}

/*- (void)drawRect:(NSRect)dirtyRect {
	//debug colors
    [[NSColor greenColor] set];
	NSRectFill(dirtyRect);
}*/

#define borderDistance 8.0
#define cellDistance 4.0

-(void) layoutSubviews {
	
	NSView *documentView = [self documentView];
	
	if (documentView && [documentView conformsToProtocol:@protocol(CocoaGwenGUIProtocol)]) {
		NSSize neededMinSize = [(<CocoaGwenGUIProtocol>)documentView minSize];
		NSSize contentSize = [self contentSize];
		
		NSRect newFrame = NSMakeRect(0.0, 0.0, neededMinSize.width, neededMinSize.height);
		if ([(<CocoaGwenGUIProtocol>)documentView fillX] && neededMinSize.width < contentSize.width)
			newFrame.size.width = contentSize.width;
		if ([(<CocoaGwenGUIProtocol>)documentView fillY] && neededMinSize.height < contentSize.height)
			newFrame.size.height = contentSize.height;
		
		[documentView setFrame:newFrame];
	}
}

-(void) setLayoutedDocumentView:(NSView*)new_documentView {
	[self setDocumentView:new_documentView];
	[self layoutSubviews];
}

#pragma mark Protocoll Methods

- (NSSize) minSize {
	return NSMakeSize(50.0, 50.0);
}

/*- (void)setFrame:(NSRect)frameRect {
	NSSize minSize = [self minSize];
	if (frameRect.size.height < minSize.height) {
		frameRect.size.height = minSize.height;
	}
	if (frameRect.size.width < minSize.width) {
		frameRect.size.width = minSize.width;
	}
	[super setFrame:frameRect];
}*/

@end

#endif
