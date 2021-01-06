#import <Foundation/Foundation.h>
#include "UnityHttpRequests.h"
#include "Result.h"

@interface ResultStorage : NSObject {}
@property (nonatomic, readonly, retain) NSData *body;
@property (nonatomic, readonly, retain) NSMutableArray *headers; // HeaderStorage[]
@property (nonatomic, readonly) UHR_Header *headerRefs; // UHR_Header[]
- (id)initWithResult:(Result *)result;
- (void)dealloc;
@end
