#import <Foundation/Foundation.h>
#include <stdint.h>
#include "UnityHttpRequests.h"

@interface Result : NSObject {}
@property (nonatomic) UHR_RequestId rid;
@property (nonatomic, readonly, retain) NSData *data;
@property (nonatomic, readonly, retain) NSHTTPURLResponse *response;
@property (nonatomic, readonly, retain) NSError *error;
- (id)initWithRid:(int32_t)aRid response:(NSHTTPURLResponse *)aResponse data:(NSData *)aData error:(NSError *)aError;
- (void)dealloc;
@end
