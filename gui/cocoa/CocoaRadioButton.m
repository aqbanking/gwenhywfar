//
//  CocoaRadioButton.m
//  CocoaGwenGuiTest
//
//  Created by Samuel Strupp on 12.08.10.
//  Copyright 2010 Synium Software GmbH. All rights reserved.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#import "CocoaRadioButton.h"


@implementation CocoaRadioButton

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		[self setBezelStyle:NSRoundedBezelStyle];
		[self setButtonType:NSRadioButton];
		_groupManager = nil;
    }
    return self;
}

-(void) dealloc {
	if (_groupManager) {
		[_groupManager removeRadioButton:self];
		[_groupManager release];
	}
	[super dealloc];
}

-(void) computeMinWidth {
	NSSize size = [self neededTextSize];
	minWidth = size.width + 22.0;
}

-(void) createNewGroupManagerWithGroupID:(NSInteger)newGroupID {
	if (_groupManager) {
		[_groupManager removeRadioButton:self];
		[_groupManager release];
		_groupManager = nil;
	}
	_groupManager = [[CocoaRadioGroupManager alloc] init];
	_groupManager.groupeID = newGroupID;
	[_groupManager addRadioButton:self];
}

-(CocoaRadioGroupManager*) getGroupManager {
	return _groupManager;
}

-(void) setGroupManager:(CocoaRadioGroupManager*) newGroupManager {
	if (_groupManager) {
		[_groupManager removeRadioButton:self];
		[_groupManager release];
		_groupManager = nil;
	}
	if (newGroupManager) {
		_groupManager = [newGroupManager retain];
		[_groupManager addRadioButton:self];
	}
}

- (void)setState:(NSInteger)value {
	if (value == NSOnState && _groupManager) {
		[_groupManager newOnStateButtonIs:self];
	}
	[super setState:value];
}

-(void) clicked:(id)sender {
	if ([self state] == NSOnState && _groupManager) {
		[_groupManager newOnStateButtonIs:self];
	}
	[super clicked:sender];
}

/*- (void)drawRect:(NSRect)dirtyRect {
	//debug colors
    [[NSColor blueColor] set];
	NSRectFill(dirtyRect);
	[super drawRect:dirtyRect];
}*/

#pragma mark Protocoll Methods

- (NSSize) minSize {
	return NSMakeSize(minWidth, 16.0);
}

@end



@implementation CocoaRadioGroupManager

@synthesize groupeID;

- (id)init {
    self = [super init];
    if (self) {
		memberRadioButtons = [[NSPointerArray pointerArrayWithWeakObjects] retain];
    }
    return self;
}

-(void) dealloc {
	[memberRadioButtons release];
	[super dealloc];
}

-(void) removeRadioButton:(CocoaRadioButton*)buttonToRemove {
	NSUInteger i;
	NSUInteger count = [memberRadioButtons count];
	NSUInteger foundIndex = count; //ungültiger Index
	for (i=0; i<count; i++) {
		if ([memberRadioButtons pointerAtIndex:i] == buttonToRemove) {
			foundIndex = i;
			i = count;
		}
	}
	if (foundIndex < count) {
		[memberRadioButtons removePointerAtIndex:foundIndex];
	}
}

-(void) addRadioButton:(CocoaRadioButton*)buttonToAdd {
	[memberRadioButtons addPointer:buttonToAdd];
}

-(void) newOnStateButtonIs:(CocoaRadioButton*)newOnStateButton {
	for (CocoaRadioButton* r in memberRadioButtons) {
		if (r != newOnStateButton) {
			[r setState:NSOffState];
		}
	}
}

@end

