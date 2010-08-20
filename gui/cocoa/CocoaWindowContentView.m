//
//  CocoaVLayout.m
//  
//
//  Created by Samuel Strupp on 10.08.10.
//  Copyright 2010 Synium Software GmbH. All rights reserved.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif



#import "CocoaWindowContentView.h"
#import "CocoaGwenGUIProtocol.h"


#ifndef COCOA_WINDOW_CONTENT_VIEW_MM
#define COCOA_WINDOW_CONTENT_VIEW_MM

@implementation CocoaWindowContentView

@synthesize fillX;
@synthesize fillY;

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
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

//#define borderDistance 0.0
//#define cellDistance 0.0

-(void) layoutSubviews {
	NSRect bounds = [self bounds];
	
	NSUInteger numOfSubViews = [subviewsInOrder count];
	
	if (numOfSubViews > 0) {
		//Prepass to compute the sizes
		
		CGFloat sizesHeight[numOfSubViews];
		CGFloat sizesWidth[numOfSubViews];
		CGFloat exclusiveHeight = 0.0;
		NSUInteger exclusiveChilds = 0;
		
		NSUInteger i;
		for (i=0; i<numOfSubViews; i++) {
			NSView* subview = [subviewsInOrder objectAtIndex:i];
			if ([subview conformsToProtocol:@protocol(CocoaGwenGUIProtocol)]) {
				if ([(<CocoaGwenGUIProtocol>)subview fillX]) sizesWidth[i] = -1.0;
				else {
					CGFloat neededWidth = [(<CocoaGwenGUIProtocol>)subview minSize].width;
					sizesWidth[i] = neededWidth;
				}
				if ([(<CocoaGwenGUIProtocol>)subview fillY]) sizesHeight[i] = -1.0;
				else {
					CGFloat neededHeight = [(<CocoaGwenGUIProtocol>)subview minSize].height;
					sizesHeight[i] = neededHeight;
					exclusiveHeight += neededHeight;
					exclusiveChilds++;
				}
			}
			else {
				sizesWidth[i] = -1.0;
				sizesHeight[i] = -1.0;
			}
		}
		
		
		//Compute standard Sizes for Subviews
		
		CGFloat stdHeight = 0.0;
		if (numOfSubViews > exclusiveChilds) {
			CGFloat fillHeight = bounds.size.height-exclusiveHeight;
			stdHeight = (fillHeight/*-(borderDistance+borderDistance)-((numOfSubViews-1)*cellDistance)*/)/(numOfSubViews-exclusiveChilds);
		}
		else {
			CGFloat fillHeight = bounds.size.height;
			stdHeight = (fillHeight/*-(borderDistance+borderDistance)-((numOfSubViews-1)*cellDistance)*/)/(numOfSubViews);
		}
		
		CGFloat stdWidth = bounds.size.width/*-(borderDistance+borderDistance)*/;
		
		
		//change Subviews Frame
		NSRect actualFrame = bounds;
		actualFrame.origin.x = 0.0; //borderDistance;
		actualFrame.origin.y += bounds.size.height; //-borderDistance;
		for (i=0; i<numOfSubViews; i++) {
			
			CGFloat usedHeight = sizesHeight[i];
			if (usedHeight < 0.0) usedHeight = stdHeight;
			actualFrame.origin.y -= usedHeight;
			actualFrame.size.height = usedHeight;
			
			CGFloat usedWidth = sizesWidth[i];
			if (usedWidth < 0.0) usedWidth = stdWidth;
			NSView* subview = [subviewsInOrder objectAtIndex:i];
			actualFrame.size.width = usedWidth;
			
			[subview setFrame:actualFrame];
			//actualFrame.origin.y -= cellDistance;
		}
	}
	
}

-(void) addLayoutSubview:(NSView*)new_subview {
	[subviewsInOrder addObject:new_subview];
	[self addSubview:new_subview];
	[self layoutSubviews];
}

#pragma mark Protocoll Methods


- (NSSize) minSize {
	NSUInteger numOfSubViews = [subviewsInOrder count];
	//CGFloat borderWidth = borderDistance+borderDistance;
	NSSize size = NSMakeSize(0,0/*borderWidth, borderWidth*/);
	if (numOfSubViews > 0) {
		NSUInteger i;
		for (i=0; i<numOfSubViews; i++) {
			NSView* subview = [subviewsInOrder objectAtIndex:i];
			if ([subview conformsToProtocol:@protocol(CocoaGwenGUIProtocol)]) {
				NSSize subViewMinSize = [(<CocoaGwenGUIProtocol>)subview minSize];
				if (subViewMinSize.width/*+borderWidth*/ > size.width) {
					size.width = subViewMinSize.width/*+borderWidh*/;
				}
				size.height += subViewMinSize.height;
				//if (i>0) size.height += cellDistance;
			}
		}
	}
	return size;
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
