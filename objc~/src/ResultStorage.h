#import <Foundation/Foundation.h>
#include "src/UnityHttpRequests.h"

@interface ResultStorage : NSObject
NSMutableArray *headers; // HeaderStorage[]
UHR_Header *headerRefs; // UHR_Header[]
NSData *body;
- (id)init;
- (void)dealloc;
@end
