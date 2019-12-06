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

@implementation AppDelegate

- (void)dealloc
{
    [super dealloc];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    char *result = do_bench();
    
    if (result != 0) {
        NSLog(@"%s\n", result);
    }
    else {
        NSLog(@"ALL TESTS PASSED!\n");
    }
    NSLog(@"Tests run: %d\n", tests_run);
    
    //[NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
}

@end
