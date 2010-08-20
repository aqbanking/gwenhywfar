//
//  CocoaGridLayout.m
//  
//
//  Created by Samuel Strupp on 10.08.10.
//  Copyright 2010 Synium Software GmbH. All rights reserved.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#import "CocoaGridLayout.h"
#import "CocoaGwenGUIProtocol.h"


@implementation CocoaGridLayout

@synthesize fillX;
@synthesize fillY;

@synthesize columns;
@synthesize rows;

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
    [[NSColor redColor] set];
	NSRectFill(dirtyRect);
}*/

#define borderDistance 8.0
#define cellDistance 4.0

-(void) layoutSubviews {
	NSRect bounds = [self bounds];
	
	NSUInteger numOfSubViews = [subviewsInOrder count];
	
	if (numOfSubViews > 0) {
		//Prepass to compute the sizes
		BOOL columnMode = (columns > 0);
		
		NSInteger neededColumns, neededRows;
		
		if (columnMode) {
			neededColumns = columns;
			neededRows = numOfSubViews/columns + numOfSubViews%columns;
		}
		else {
			if (rows > 0) {
				neededColumns = numOfSubViews/rows + numOfSubViews%rows;
				neededRows = rows;
			}
			else {
				//We choose column mode with one column
				neededColumns = 1;
				neededRows = numOfSubViews;
			}
		}
		
		
		CGFloat minWidthNeededForColumn[neededColumns];
		CGFloat minHeightNeededForRow[neededRows];
		char fillXFlags[neededColumns];
		char fillYFlags[neededRows];
		
		NSUInteger i;
		for (i=0; i<neededColumns; i++) {
			minWidthNeededForColumn[i] = 0.0;
			fillXFlags[i] = 0;
		}
		for (i=0; i<neededRows; i++) {
			minHeightNeededForRow[i] = 0.0;
			fillYFlags[i] = 0;
		}
		
		NSInteger actualRow, actualColumn;
		
		for (i=0; i<numOfSubViews; i++) {
			if (columnMode) {
				actualRow = i/neededColumns;
				actualColumn = i%neededColumns;
			}
			else {
				actualRow = i%neededRows;
				actualColumn = i/neededRows;
			}
			
			NSView* subview = [subviewsInOrder objectAtIndex:i];
			if ([subview conformsToProtocol:@protocol(CocoaGwenGUIProtocol)]) {
				NSSize minSize = [(<CocoaGwenGUIProtocol>)subview minSize];
				
				if (minWidthNeededForColumn[actualColumn] < minSize.width)
					minWidthNeededForColumn[actualColumn] = minSize.width;
				
				if (minHeightNeededForRow[actualRow] < minSize.height)
					minHeightNeededForRow[actualRow] = minSize.height;
				
				if ([(<CocoaGwenGUIProtocol>)subview fillX]) fillXFlags[actualColumn] = 1;
				if ([(<CocoaGwenGUIProtocol>)subview fillY]) fillYFlags[actualRow] = 1;
				
			}
		}
		
		
		//compute exact sizes
		CGFloat maxWidth = bounds.size.width-borderDistance-borderDistance-(neededColumns-1)*cellDistance;
		NSInteger flexibleCells = 0;
		for(i=0; i<neededColumns; i++) {
			if (fillXFlags[i] == 1) flexibleCells++;
			else maxWidth -= minWidthNeededForColumn[i];
		}
		if (maxWidth > 0.0 && flexibleCells > 0) {
			CGFloat flexibleStdWidth = maxWidth/flexibleCells;
			for (i=0; i<neededColumns; i++) {
				if (fillXFlags[i] == 1) minWidthNeededForColumn[i] = flexibleStdWidth;
			}
		}
		
		CGFloat maxHeight = bounds.size.height-borderDistance-borderDistance-(neededRows-1)*cellDistance;
		flexibleCells = 0;
		for(i=0; i<neededRows; i++) {
			if (fillYFlags[i] == 1) flexibleCells++;
			else maxHeight -= minHeightNeededForRow[i];
		}
		if (maxHeight > 0.0 && flexibleCells > 0) {
			CGFloat flexibleStdHeight = maxHeight/flexibleCells;
			for (i=0; i<neededRows; i++) {
				if (fillYFlags[i] == 1) minHeightNeededForRow[i] = flexibleStdHeight;
			}
		}
		
		
		
		
		//Set the sizes to the view
		NSRect actualFrame = bounds;
		actualFrame.origin.x = borderDistance;
		actualFrame.origin.y += bounds.size.height-borderDistance;
		
		
		NSInteger oldIndex = -1;
		
		for (i=0; i<numOfSubViews; i++) {
			if (columnMode) {
				actualRow = i/neededColumns;
				actualColumn = i%neededColumns;
				
				if (oldIndex != actualRow) {
					actualFrame.origin.x = borderDistance;
					actualFrame.origin.y -= minHeightNeededForRow[actualRow]+cellDistance;
					oldIndex = actualRow;
				}
			}
			else {
				actualRow = i%neededRows;
				actualColumn = i/neededRows;
				
				if (oldIndex != actualColumn) {
					if (oldIndex >= 0) actualFrame.origin.x += minWidthNeededForColumn[oldIndex]+cellDistance;
					actualFrame.origin.y = bounds.origin.y+bounds.size.height-borderDistance-minHeightNeededForRow[actualRow];
					oldIndex = actualColumn;
				}
			}
			
			NSView* subview = [subviewsInOrder objectAtIndex:i];
			
			actualFrame.size.height = minHeightNeededForRow[actualRow];
			actualFrame.size.width = minWidthNeededForColumn[actualColumn];
			NSRect realFrame = actualFrame;
			if ([subview conformsToProtocol:@protocol(CocoaGwenGUIProtocol)]) {
				BOOL flexWidth = [(<CocoaGwenGUIProtocol>)subview fillX];
				BOOL flexHeight = [(<CocoaGwenGUIProtocol>)subview fillY];
				if (!flexWidth || !flexHeight) {
					NSSize minSize = [(<CocoaGwenGUIProtocol>)subview minSize];
					if (!flexWidth && minSize.width < realFrame.size.width) realFrame.size.width = minSize.width;
					if (!flexHeight && minSize.height < realFrame.size.height) {
						realFrame.origin.y += realFrame.size.height-minSize.height;
						realFrame.size.height = minSize.height;
					}
				}
			}
			[subview setFrame:realFrame];
			//NSLog(@"frame = %@", NSStringFromRect(actualFrame));
			
			if (columnMode) actualFrame.origin.x += actualFrame.size.width+cellDistance;
			else if (actualRow+1 < neededRows) actualFrame.origin.y -= minHeightNeededForRow[actualRow+1]+cellDistance;
		}
		
		/*CGFloat sizesHeight[numOfSubViews];
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
		 stdHeight = (fillHeight-(borderDistance+borderDistance)-((numOfSubViews-1)*cellDistance))/(numOfSubViews-exclusiveChilds);
		 }
		 else {
		 CGFloat fillHeight = bounds.size.height;
		 stdHeight = (fillHeight-(borderDistance+borderDistance)-((numOfSubViews-1)*cellDistance))/(numOfSubViews);
		 }
		 
		 CGFloat stdWidth = bounds.size.width-(borderDistance+borderDistance);
		 
		 
		 //change Subviews Frame
		 NSRect actualFrame = bounds;
		 actualFrame.origin.x = borderDistance;
		 actualFrame.origin.y += bounds.size.height-borderDistance;
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
		 actualFrame.origin.y -= cellDistance;
		 }*/
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
	
	if (numOfSubViews > 0) {
		
		NSInteger neededColumns, neededRows;
		BOOL columnMode = (columns > 0);
		if (columnMode) {
			neededColumns = columns;
			neededRows = numOfSubViews/columns + numOfSubViews%columns;
		}
		else {
			if (rows > 0) {
				neededColumns = numOfSubViews/rows + numOfSubViews%rows;
				neededRows = rows;
			}
			else {
				//We choose column mode with one column
				neededColumns = 1;
				neededRows = numOfSubViews;
			}
		}
		
		
		CGFloat minWidthNeededForColumn[neededColumns];
		CGFloat minHeightNeededForRow[neededRows];
		
		NSUInteger i;
		for (i=0; i<neededColumns; i++) {
			minWidthNeededForColumn[i] = 0.0;
		}
		for (i=0; i<neededRows; i++) {
			minHeightNeededForRow[i] = 0.0;
		}
		
		NSInteger actualRow, actualColumn;
		
		for (i=0; i<numOfSubViews; i++) {
			if (columnMode) {
				actualRow = i/neededColumns;
				actualColumn = i%neededColumns;
			}
			else {
				actualRow = i%neededRows;
				actualColumn = i/neededRows;
			}
			
			NSView* subview = [subviewsInOrder objectAtIndex:i];
			if ([subview conformsToProtocol:@protocol(CocoaGwenGUIProtocol)]) {
				NSSize minSize = [(<CocoaGwenGUIProtocol>)subview minSize];
				
				if (minWidthNeededForColumn[actualColumn] < minSize.width)
					minWidthNeededForColumn[actualColumn] = minSize.width;
				
				if (minHeightNeededForRow[actualRow] < minSize.height)
					minHeightNeededForRow[actualRow] = minSize.height;
			}
		}
		CGFloat minNeededWidth = borderDistance+borderDistance+(neededColumns-1)*cellDistance;
		for (i=0; i<neededColumns; i++) {
			minNeededWidth += minWidthNeededForColumn[i];
		}
		CGFloat minNeededHeight = borderDistance+borderDistance+(neededRows-1)*cellDistance;
		for (i=0; i<neededRows; i++) {
			minNeededHeight += minHeightNeededForRow[i];
		}
		return NSMakeSize(minNeededWidth, minNeededHeight);
	}
	return NSZeroSize;
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
