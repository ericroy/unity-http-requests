#import <Foundation/Foundation.h>
#include "UnityHttpRequests.h"

@interface Result : NSObject {}
@property UHR_RequestId rid;
@property (readonly, retain) NSData *data;
@property (readonly, retain) NSHTTPURLResponse *response;
@property (readonly, retain) NSError *error;
- (id)initWithRid:(NSInteger)aRid response:(NSHTTPURLResponse *)aResponse data:(NSData *)aData error:(NSError *)aError;
- (void)dealloc;
@end