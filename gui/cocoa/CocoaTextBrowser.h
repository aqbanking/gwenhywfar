//
//  CocoaTextBrowser.h
//
//
//  Created by Samuel Strupp on 17.08.10.
//

#import <Cocoa/Cocoa.h>
#import <WebKit/WebKit.h>
#import "CocoaGwenGUIProtocol.h"

@interface CocoaTextBrowser : WebView <CocoaGwenGUIProtocol>
{
  BOOL fillX;
  BOOL fillY;

  NSString *loadedString;
}

@property BOOL fillX;
@property BOOL fillY;

-(void) setStringValue:(NSString *)value BasePath:(NSString *)base;
-(void) setStringValue:(NSString *)value;
-(NSString *) stringValue;

@end
