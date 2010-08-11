/***************************************************************************
    begin       : Tue Aug 10 2010
    copyright   : (C) 2010 by Samuel Strupp

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

 */
//
//  CocoaGWENButton.h
//  
//
//  Created by Samuel Strupp on 10.08.10.
//

#import <Cocoa/Cocoa.h>


typedef void (*gwenActionPtr)(NSButton *button, void* data);


@interface CocoaGWENButton : NSButton {
	gwenActionPtr c_actionPtr;
	void* c_actionData;
}

-(void) setC_ActionPtr:(gwenActionPtr)ptr Data:(void*)data;

@end
