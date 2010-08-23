//
//  CocoaRadioButton.h
//
//  Created by Samuel Strupp on 12.08.10.
//

#import <Cocoa/Cocoa.h>
#import "CocoaButton.h"

@class CocoaRadioButton;

@interface CocoaRadioGroupManager : NSObject {
	NSInteger groupeID;
	NSPointerArray *memberRadioButtons;
}

@property NSInteger groupeID;

-(void) removeRadioButton:(CocoaRadioButton*)buttonToRemove;
-(void) addRadioButton:(CocoaRadioButton*)buttonToAdd;
-(void) newOnStateButtonIs:(CocoaRadioButton*)newOnStateButton;

@end

@interface CocoaRadioButton : CocoaButton {
	CocoaRadioGroupManager *_groupManager;
}

-(void) createNewGroupManagerWithGroupID:(NSInteger)newGroupID;
-(CocoaRadioGroupManager*) getGroupManager;
-(void) setGroupManager:(CocoaRadioGroupManager*) newGroupManager;

@end


