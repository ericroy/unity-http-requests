#import "src/Context.h"

@implementation Context
- (id)init {
    self = [super init];
    if (self) {
        NSURLSessionConfiguration *config = [NSURLSessionConfiguration defaultSessionConfiguration];
        self.tasks = [[NSMutableDictionary alloc] initWithCapacity:16];
        self.session = [[NSURLSession alloc] initWithConfiguration: config
                                         delegate: nil
                                         delegateQueue: [NSOperationQueue mainQueue]];
        self.resultsLock = [[NSLock alloc] init];
        self.results = [[NSMutableArray alloc] initWithCapacity:16];
        self.resultStorage = [[NSMutableDictionary alloc] initWithCapacity:16];
        self.nextRequestID = 1;
    }
    return self;
}

- (void)dealloc {
    [self.tasks release];
    self.tasks = nil;

    [self.session invalidateAndCancel];
    [self.session release];
    self.session = nil;

    [self.resultsLock release];
    self.resultsLock = nil;

    [self.results release];
    self.results = nil;

    [self.resultStorage release];
    self.resultStorage = nil;

    [super dealloc];
}
@end