#import "Session.h"
#import "Logging.h"

@implementation Session

- (id)init {
    self = [super init];
    if (self) {
        NSURLSessionConfiguration *config = [NSURLSessionConfiguration defaultSessionConfiguration];
        tasks = [[NSMutableDictionary alloc] initWithCapacity:16];

        // This is stupid.
        // NSURLSession can't be manually alloc'ed and initialized, so there's no option but to
        // have it owned by the nearest containing autorelease pool.  I want this class to own the
        // instance and explicitly destroy the NSURLSession when this class is dealloced.
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
            UHR_LOG_DEBUG(@"Created NSURLSession");
        }

        resultsLock = [[NSLock alloc] init];
        results = [[NSMutableArray alloc] initWithCapacity:16];
        resultStorage = [[NSMutableDictionary alloc] initWithCapacity:16];
        nextRequestID = 1;
    }
    return self;
}

- (void)dealloc {
    // Apple docs:
    // "Once invalidated, references to the delegate and callback objects are broken. After invalidation, session objects cannot be reused."
    // So it sounds like the completionHandler blocks for in-flight tasks will not be called after this,
    // which is a good thing because they're borrowing pointers to this Session instance.
    [session invalidateAndCancel];

    [tasks release];
    tasks = nil;

    [session release];
    session = nil;

    [resultsLock release];
    resultsLock = nil;

    [results release];
    results = nil;

    [resultStorage release];
    resultStorage = nil;

    [super dealloc];
    UHR_LOG_DEBUG(@"Destroyed NSURLSession");
}

@end
