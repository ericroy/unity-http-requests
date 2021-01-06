#import <Foundation/Foundation.h>
#include "UnityHttpRequests.h"

@interface Result : NSObject {}
@property UHR_RequestId rid;
@property (nonatomic, readonly, retain) NSData *data;
@property (nonatomic, readonly, retain) NSHTTPURLResponse *response;
@property (nonatomic, readonly, retain) NSError *error;
- (id)initWithRid:(NSInteger)aRid response:(NSHTTPURLResponse *)aResponse data:(NSData *)aData error:(NSError *)aError;
- (void)dealloc;
@end