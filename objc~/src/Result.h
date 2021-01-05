#import <Foundation/Foundation.h>
#include "UnityHttpRequests.h"

@interface Result : NSObject {
    UHR_RequestId rid;
    NSData *data;
    NSHTTPURLResponse *response;
    NSError *error;
}
- (id)initWithRid:(NSInteger)aRid response:(NSHTTPURLResponse *)aResponse data:(NSData *)aData error:(NSError *)aError;
- (void)dealloc;
@end