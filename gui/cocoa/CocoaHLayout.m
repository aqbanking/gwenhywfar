//
//  CocoaHLayout.m
//  
//
//  Created by Samuel Strupp on 10.08.10.
//
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#import "CocoaHLayout.h"


@implementation CocoaHLayout

@synthesize fillX;
@synthesize fillY;

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		fillX = NO;
		fillY = NO;
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

/*- (void)drawRect:(NSRect)dirtyRect {
	//debug colors
    [[NSColor greenColor] set];
	NSRectFill(dirtyRect);
}*/

#define borderDistance 8.0
#define cellDistance 4.0

-(void) layoutSubviews {
	NSRect bounds = [self bounds];
	
	NSUInteger numOfSubViews = [subviewsInOrder count];
	
	if (numOfSubViews > 0) {
		//Prepass to compute the sizes
		
		CGFloat sizesHeight[numOfSubViews];
		CGFloat sizesWidth[numOfSubViews];
		CGFloat exclusiveWidth = 0.0;
		NSUInteger exclusiveChilds = 0;
		
		NSUInteger i;
		for (i=0; i<numOfSubViews; i++) {
			NSView* subview = [subviewsInOrder objectAtIndex:i];
			if ([subview conformsToProtocol:@protocol(CocoaGwenGUIProtocol)]) {
				if ([(<CocoaGwenGUIProtocol>)subview fillX]) sizesWidth[i] = -1.0;
				else {
					CGFloat neededWidth = [(<CocoaGwenGUIProtocol>)subview minSize].width;
					sizesWidth[i] = neededWidth;
					exclusiveWidth += neededWidth;
					exclusiveChilds++;
				}
				if ([(<CocoaGwenGUIProtocol>)subview fillY]) sizesHeight[i] = -1.0;
				else {
					CGFloat neededHeight = [(<CocoaGwenGUIProtocol>)subview minSize].height;
					sizesHeight[i] = neededHeight;
				}
			}
			else {
				sizesWidth[i] = -1.0;
				sizesHeight[i] = -1.0;
			}
		}
		
		
		//Compute standard Sizes for Subviews
		
		CGFloat stdWidth = 0.0;
		if (numOfSubViews > exclusiveChilds) {
			CGFloat fillWidth = bounds.size.width-exclusiveWidth;
			stdWidth = (fillWidth-(borderDistance+borderDistance)-((numOfSubViews-1)*cellDistance))/(numOfSubViews-exclusiveChilds);
		}
		else {
			CGFloat fillWidth = bounds.size.width;
			stdWidth = (fillWidth-(borderDistance+borderDistance)-((numOfSubViews-1)*cellDistance))/(numOfSubViews);
		}
		
		CGFloat stdHeight = bounds.size.height-(borderDistance+borderDistance);
		
		//if (numOfSubViews>=4) NSLog(@"view.width = %f", bounds.size.width);
		
		//change Subviews Frame
		NSRect actualFrame = bounds;
		actualFrame.origin.x = borderDistance;
		actualFrame.origin.y += bounds.size.height-borderDistance;
		for (i=0; i<numOfSubViews; i++) {
			
			CGFloat usedHeight = sizesHeight[i];
			if (usedHeight < 0.0) usedHeight = stdHeight;
			actualFrame.origin.y = bounds.size.height-borderDistance-usedHeight;
			actualFrame.size.height = usedHeight;
			
			CGFloat usedWidth = sizesWidth[i];
			if (usedWidth < 0.0) usedWidth = stdWidth;
			actualFrame.size.width = usedWidth;
			
			//if (numOfSubViews>=4) NSLog(@"subview %i width = %f", i, usedWidth);
			
			NSView* subview = [subviewsInOrder objectAtIndex:i];
			[subview setFrame:actualFrame];
			actualFrame.origin.x += usedWidth+cellDistance;
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
	CGFloat borderWidth = borderDistance+borderDistance;
	NSSize size = NSMakeSize(borderWidth, borderWidth);
	if (numOfSubViews > 0) {
		NSUInteger i;
		for (i=0; i<numOfSubViews; i++) {
			NSView* subview = [subviewsInOrder objectAtIndex:i];
			if ([subview conformsToProtocol:@protocol(CocoaGwenGUIProtocol)]) {
				NSSize subViewMinSize = [(<CocoaGwenGUIProtocol>)subview minSize];
				if (subViewMinSize.height+borderWidth > size.height) {
					size.height = subViewMinSize.height+borderWidth;
				}
				size.width += subViewMinSize.width;
				if (i>0) size.width += cellDistance;
			}
		}
	}
	return size;
}

- (void)setFrame:(NSRect)frameRect {
	NSSize minSize = [self minSize];
	if (frameRect.size.height < minSize.height) {
		frameRect.size.height = minSize.height;
	}
	if (frameRect.size.width < minSize.width) {
		frameRect.size.width = minSize.width;
	}
	[super setFrame:frameRect];
}

@end
