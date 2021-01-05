#import "Context.h"

@implementation Context
- (id)init {
    self = [super init];
    if (self) {
        NSURLSessionConfiguration *config = [NSURLSessionConfiguration defaultSessionConfiguration];
        tasks = [[NSMutableDictionary alloc] initWithCapacity:16];
        session = [[NSURLSession alloc] initWithConfiguration: config
                                         delegate: nil
                                         delegateQueue: [NSOperationQueue mainQueue]];
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