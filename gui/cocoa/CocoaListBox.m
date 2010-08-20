//
//  CocoaListBox.m
//  
//
//  Created by Samuel Strupp on 17.08.10.
//

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#import "CocoaListBox.h"


@implementation CocoaListBox

@synthesize fillX;
@synthesize fillY;

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
		fillX = NO;
		fillY = NO;
		dataArray = [[NSMutableArray alloc] init];
		[self setColumnAutoresizingStyle:NSTableViewLastColumnOnlyAutoresizingStyle];
		[self setUsesAlternatingRowBackgroundColors:YES];
		[self setDataSource:self];
		[self setDelegate:self];
		
		[self setTarget:self];
		[self setAction:@selector(performAction:)];
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(performAction:) name:NSTableViewSelectionDidChangeNotification object:self];
    }
    return self;
}

-(void) dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[dataArray release];
	[super dealloc];
}

#pragma mark Special Methods

-(void) setC_ActionPtr:(gwenListBoxActionPtr)ptr Data:(void*)data {
	c_actionPtr = ptr;
	c_actionData = data;
}

-(void) performAction:(id) sender {
	if (c_actionPtr) {
		c_actionPtr(self, c_actionData);
	}
}

-(void) setTitelsAndCreateColumns:(NSString*)titleString {
	if (!titleString) return;
	
	//first delete old columns 
	NSArray *oldColumns = [self tableColumns];
	for (NSTableColumn *oldColumn in oldColumns) {
		[self removeTableColumn:oldColumn];
	}
	
	
	//now create new columns
	NSArray *titles = [titleString componentsSeparatedByString:@"\t"];
	//NSLog(@"%@", titles);
	NSUInteger numOfColumns = [titles count];
	NSUInteger i;
	for (i=0; i<numOfColumns; i++) {
		NSString *indexString = [NSString stringWithFormat:@"%u",i];
		NSTableColumn *newColumn = [[NSTableColumn alloc] initWithIdentifier:indexString];
		[newColumn setEditable:NO];
		NSSortDescriptor *sortDescriptor = [[NSSortDescriptor alloc] initWithKey:indexString ascending:YES];
		[newColumn setSortDescriptorPrototype:sortDescriptor];
		[sortDescriptor release];
		NSString *newTitle = [titles objectAtIndex:i];
		if (newTitle) [[newColumn headerCell] setStringValue:newTitle];
		[self addTableColumn:newColumn];
	}
	[self sizeLastColumnToFit];
}

-(NSString*) titlesString {
	NSArray *columns = [self tableColumns];
	if (columns) {
		NSMutableString *ouputString = [[NSMutableString alloc] init];
		
		NSUInteger i;
		NSUInteger count = [columns count];
		for (i=0; i<count; i++) {
			NSString *value = [[[columns objectAtIndex:i] headerCell] stringValue];
			if (value) {
				[ouputString appendString:value];
			}
			if (i<count-1) [ouputString appendString:@"\t"];
		}
		return [ouputString autorelease];
	}
	return nil;
}

-(void) addDataRowString:(NSString*)dataRowString {
	if (!dataRowString) return;
	
	NSArray *newDataStrings = [dataRowString componentsSeparatedByString:@"\t"];
	//NSLog(@"%@", newDataStrings);
	NSMutableArray *columnArray = [[NSMutableArray alloc] initWithArray:newDataStrings];
	[dataArray addObject:columnArray];
	[columnArray release];
}

-(NSString*) dataRowStringForRow:(NSUInteger)index {
	if (index < [dataArray count]) {
		NSMutableString *ouputString = [[NSMutableString alloc] init];
		NSArray *rowArray = [dataArray objectAtIndex:index];
		
		NSUInteger i;
		NSUInteger count = [rowArray count];
		for (i=0; i<count; i++) {
			NSString *value = [rowArray objectAtIndex:i];
			if (value) {
				[ouputString appendString:value];
			}
			if (i<count-1) [ouputString appendString:@"\t"];
		}
		return [ouputString autorelease];
	}
	return nil;
}

-(void) clearDataRows {
	[dataArray removeAllObjects];
	[self reloadData];
}



NSInteger positionSort(id element1, id element2, void *context)
{
    NSString *obj1 = nil;
	NSString *obj2 = nil;
	NSDictionary *contextDict = (NSDictionary*)context;
	NSInteger indexOfColumn = [[contextDict objectForKey:@"indexOfColumn"] integerValue];
	BOOL ascending = [[contextDict objectForKey:@"ascending"] boolValue];
	if ([element1 count] > indexOfColumn) {
		obj1 = [element1 objectAtIndex:indexOfColumn];
	}
	if ([element2 count] > indexOfColumn) {
		obj2 = [element2 objectAtIndex:indexOfColumn];
	}
	if (obj1 && obj2) {
		NSComparisonResult result = [obj1 localizedCaseInsensitiveCompare:obj2];
		if (ascending) return result;
		else {
			if (result == NSOrderedAscending) return NSOrderedDescending;
			else if (result == NSOrderedDescending) return NSOrderedAscending;
			else return result;
		}
	}
	return NSOrderedSame;
}


-(void) sortTableView:(NSTableView*)tableView {
	NSArray *sortDescriptors = [tableView sortDescriptors];
	if (!sortDescriptors || [sortDescriptors count] <=0) return;
	NSSortDescriptor *sortDesc = [sortDescriptors objectAtIndex:0];
	
	if (sortDesc) {
		NSNumber *indexOfColumn = [NSNumber numberWithInteger:[[sortDesc key] integerValue]];
		NSNumber *sortOrder = [NSNumber numberWithBool:[sortDesc ascending]];
		//NSLog(@"sort Index = %@ Ascending = %@", [indexOfColumn description], [sortOrder description]);
		NSArray *keys = [NSArray arrayWithObjects:@"indexOfColumn", @"ascending", nil];
		NSArray *objects = [NSArray arrayWithObjects:indexOfColumn, sortOrder, nil];
		NSDictionary *contextDict = [NSDictionary dictionaryWithObjects:objects forKeys:keys];
		
		[dataArray sortUsingFunction:positionSort context:contextDict];
		[self reloadData];
	}
}


-(BOOL) setColumnWidthTo:(NSInteger)widthInPixels forColumn:(NSInteger)index {
	if (index < 0 || widthInPixels < 0) return NO;
	NSTableColumn *tableColumn = [self tableColumnWithIdentifier:[NSString stringWithFormat:@"%i",index]];
	if (tableColumn) {
		[tableColumn setWidth:widthInPixels];
		return YES;
	}
	return NO;
}

-(NSInteger) widthOfColumn:(NSInteger)index {
	if (index >= 0) {
		NSTableColumn *tableColumn = [self tableColumnWithIdentifier:[NSString stringWithFormat:@"%i",index]];
		if (tableColumn) {
			return ceil([tableColumn width]);
		}
	}
	return -1;
}

-(BOOL) setSortOrderTo:(BOOL)ascending ForColumnWithIndex:(NSUInteger)index {
	NSArray *tableColumns = [self tableColumns];
	if (index < [tableColumns count]) {
		NSSortDescriptor *sortDesc = [[NSSortDescriptor alloc] initWithKey:[NSString stringWithFormat:@"%u",index] ascending:ascending];
		[self setSortDescriptors:[NSArray arrayWithObject:sortDesc]];
		[sortDesc release];
		
		[self sortTableView:self];
		return YES;
	}
	return NO;
}

-(NSInteger) sortOrderForColumnAtIndex:(NSUInteger)index {
	NSArray *tableColumns = [self tableColumns];
	if (index < [tableColumns count]) {
		NSArray *sortDescriptors = [self sortDescriptors];
		if (!sortDescriptors || [sortDescriptors count] <=0) return -1;
		NSSortDescriptor *sortDesc = [sortDescriptors objectAtIndex:0];
		
		if ((NSUInteger)[[sortDesc key] integerValue] == index) {
			return [sortDesc ascending];
		}
	}
	return -1;
}

#pragma mark DataSource Methods

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView {
	return [dataArray count];
}

- (id)tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex {
	if (rowIndex >= 0 && rowIndex < [dataArray count]) {
		NSUInteger index = [(NSString*)[aTableColumn identifier] integerValue];
		NSArray *rowArray = [dataArray objectAtIndex:rowIndex];
		if (rowArray && index < [rowArray count]) {
			return [rowArray objectAtIndex:index];
		}
	}
	return nil;
}

- (void)tableView:(NSTableView *)aTableView setObjectValue:(id)anObject forTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex {
	return;
}

#pragma mark Delegate Methods

-  (void)tableView:(NSTableView *)aTableView sortDescriptorsDidChange:(NSArray *)oldDescriptors {
	[self sortTableView:aTableView];
}

#pragma mark Protocoll Methods

- (NSSize) minSize {
	return NSMakeSize(300.0, 300.0);
}

- (BOOL) fillX {
	return fillX;
}

- (BOOL) fillY {
	return fillY;
}

@end
