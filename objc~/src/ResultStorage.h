#import <Foundation/Foundation.h>
#include "UnityHttpRequests.h"
#include "Result.h"

@interface ResultStorage : NSObject {}
@property (readonly, retain) NSData *body;
@property (readonly, retain) NSMutableArray *headers; // HeaderStorage[]
@property (readonly) UHR_Header *headerRefs; // UHR_Header[]
- (id)initWithResult:(Result *)result;
- (void)dealloc;
@end
