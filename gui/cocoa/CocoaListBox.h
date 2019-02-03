//
//  CocoaListBox.h
//
//
//  Created by Samuel Strupp on 17.08.10.
//

#import <Cocoa/Cocoa.h>
#import "CocoaGwenGUIProtocol.h"

typedef void (*gwenListBoxActionPtr)(NSTableView *listbox, void *data);

@interface CocoaListBox : NSTableView <CocoaGwenGUIProtocol>
{
  gwenListBoxActionPtr c_actionPtr;
  void *c_actionData;

  BOOL fillX;
  BOOL fillY;

  NSMutableArray *dataArray;
}

@property BOOL fillX;
@property BOOL fillY;

-(void) setC_ActionPtr:(gwenListBoxActionPtr)ptr Data:(void *)data;

-(void) setTitelsAndCreateColumns:(NSString *)titleString;
-(NSString *) titlesString;
-(void) addDataRowString:(NSString *)dataRowString;
-(NSString *) dataRowStringForRow:(NSUInteger)index;
-(void) clearDataRows;

-(BOOL) setColumnWidthTo:(NSInteger)widthInPixels forColumn:(NSInteger)index;
-(NSInteger) widthOfColumn:(NSInteger)index;
-(BOOL) setSortOrderTo:(BOOL)ascending ForColumnWithIndex:(NSUInteger)index;
-(NSInteger) sortOrderForColumnAtIndex:(NSUInteger)index;

@end
