//
//  CocoaTextBrowser.m
//  
//
//  Created by Samuel Strupp on 18.08.10.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#import "CocoaTextBrowser.h"


@implementation CocoaTextBrowser

@synthesize fillX;
@synthesize fillY;

- (id) initWithFrame:(NSRect)frameRect frameName:(NSString *)frameName groupName:(NSString *)groupName {
    self = [super initWithFrame:frameRect frameName:frameName groupName:groupName];
    if (self) {
		fillX = NO;
		fillY = NO;
		loadedString = nil;
		//[[[self mainFrame] frameView] setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    }
    return self;
}

-(void) dealloc {
	[loadedString release];
	[super dealloc];
}

#pragma mark Special Methods

-(void) setStringValue:(NSString*)value BasePath:(NSString*)base {
	if (value) {
		[loadedString release];
		loadedString = [value retain];
		
		
		NSString *basePath = base;
		if (!basePath) {
			basePath = [[NSBundle mainBundle] resourcePath];
			if (!basePath) basePath = [[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent];
		}
		NSURL *baseURL = [NSURL fileURLWithPath:basePath];
		
		if (value) {
			NSRange htmlRange = [value rangeOfString:@"<html>"];
			if (htmlRange.location != NSNotFound) {
				NSRange endHtmlRange = [value rangeOfString:@"</html>"];
				if (endHtmlRange.location != NSNotFound) {
					NSString *stringToUse = @"";
					NSRange useRange = NSUnionRange(htmlRange, endHtmlRange);
					stringToUse = [value substringWithRange:useRange];
					[[self mainFrame] loadHTMLString:stringToUse baseURL:baseURL];
					return;
				}
			}
			[[self mainFrame] loadHTMLString:value baseURL:baseURL];
		}
	}
	else {
		[loadedString release];
		loadedString = nil;
	}

}

-(void) setStringValue:(NSString*)value {
	[self setStringValue:value BasePath:nil];
}

-(NSString*) stringValue {
	return loadedString;
}

#pragma mark Protocoll Methods

- (NSSize) minSize {
	return NSMakeSize(30.0, 30.0);
}

- (BOOL) fillX {
	return fillX;
}

- (BOOL) fillY {
	return fillY;
}

- (void)setFrame:(NSRect)frameRect {
	if (frameRect.size.width < 0.0) frameRect.size.width = 0.0;
	if (frameRect.size.height < 0.0) frameRect.size.height = 0.0;
	[super setFrame:frameRect];
}

@end
