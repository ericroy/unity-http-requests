#import "Result.h"

@implementation Result
@synthesize rid;
@synthesize data;
@synthesize response;
@synthesize error;

- (id)initWithRid:(NSInteger)aRid response:(NSHTTPURLResponse *)aResponse data:(NSData *)aData error:(NSError *)aError {
    self = [super init];
    if (self) {
        _rid = aRid;
        _data = [data retain];
        _response = [response retain];
        _error = [error retain];
    }
    return self;
}
- (void)dealloc {
    [_data release];
    _data = nil;
    [_response release];
    _response = nil;
    [_error release];
    _error = nil;
    [super dealloc];
}
@end