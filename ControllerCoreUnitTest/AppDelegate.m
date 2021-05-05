//
//  AppDelegate.m
//  ControllerCoreUnitTest
//
//  Created by Developer on 26.03.13.
//  Copyright (c) 2013 HMDP. All rights reserved.
//

#include "uniBench.h"

//unit testing
#include "minunit.h"

#import "AppDelegate.h"

void allTests(void)
{
    char *result = do_bench();
    
    if (result != 0) {
        NSLog(@"%s\n", result);
    }
    else {
        NSLog(@"ALL TESTS PASSED!\n");
    }
    NSLog(@"Tests run: %d\n", tests_run);
    
    return;
}

@implementation AppDelegate
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    allTests();
}

@end

#pragma mark -
// this is a faceless background application
int main(int argc, char * argv[]) {
    @autoreleasepool {
        allTests();
    }
    
    return EXIT_SUCCESS;
}
