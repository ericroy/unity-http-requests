#import "Result.h"

@implementation Result
- (id)initWithRid:(NSInteger)aRid response:(NSHTTPURLResponse *)aResponse data:(NSData *)aData error:(NSError *)aError {
    self = [super init];
    if (self) {
        rid = aRid;
        data = [data retain];
        response = [response retain];
        error = [error retain];
    }
    return self;
}
- (void)dealloc {
    [data release];
    data = nil;
    [response release];
    response = nil;
    [error release];
    error = nil;
    [super dealloc];
}
@end