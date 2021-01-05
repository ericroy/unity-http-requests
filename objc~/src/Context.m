#import "Context.h"

@implementation Context
@synthesize tasks;
@synthesize session;
@synthesize resultsLock;
@synthesize results;
@synthesize resultStorage;
@synthesize nextRequestID;
- (id)init {
    self = [super init];
    if (self) {
        NSURLSessionConfiguration *config = [NSURLSessionConfiguration defaultSessionConfiguration];
        tasks = [[NSMutableDictionary alloc] initWithCapacity:16];

        // This is stupid.
        // NSURLSession can't be manually alloc'ed and initialized, so there's no option but to
        // have it owned by the nearest containing autorelease pool.  I want this class to own the
        // insteance and explicitly destroy the NSURLSession when this class is dealloced.
        // To achieve this, we do the following:
        // - Create it in a small-scope autorelease pool, refcount=1
        // - Retain it, refcount=2
        // - Autorelease pool goes out of scope, refcont=1
        // - In our classes dealloc method, release it, refcount=0
        @autoreleasepool {
            // Retain explicitly.
            session = [[NSURLSession sessionWithConfiguration:config
                                    delegate:nil
                                    delegateQueue:[NSOperationQueue mainQueue]] retain];
        }

        resultsLock = [[NSLock alloc] init];
        results = [[NSMutableArray alloc] initWithCapacity:16];
        resultStorage = [[NSMutableDictionary alloc] initWithCapacity:16];
        nextRequestID = 1;
    }
    return self;
}

- (void)dealloc {
    [tasks release];
    tasks = nil;

    [session invalidateAndCancel];
    [session release];
    session = nil;

    [resultsLock release];
    resultsLock = nil;

    [results release];
    results = nil;

    [resultStorage release];
    resultStorage = nil;

    [super dealloc];
}
@end