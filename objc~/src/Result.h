#import <Foundation/Foundation.h>
#include "UnityHttpRequests.h"

@interface Result : NSObject {
    UHR_RequestId rid;
    NSData *data;
    NSHTTPURLResponse *response;
    NSError *error;
}
- (id)init;
- (void)dealloc;
@end