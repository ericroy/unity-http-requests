#import <Foundation/Foundation.h>
#include "UnityHttpRequests.h"

@interface ResultStorage : NSObject {}
@property (nonatomic) UHR_RequestId rid;
@property (nonatomic, readonly, retain) NSError *error;
@property (nonatomic) uint32_t status;
@property (nonatomic, readonly, retain) NSMutableArray *headers; // HeaderStorage[]
@property (nonatomic, readonly) UHR_Header *headerRefs; // UHR_Header[]
@property (nonatomic, readonly, retain) NSData *body;
- (id)initWithRid:(UHR_RequestId)aRid response:(NSHTTPURLResponse *)response body:(NSData *)aBody error:(NSError *)aError;
- (void)dealloc;
@end
