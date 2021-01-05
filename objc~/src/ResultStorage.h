#import <Foundation/Foundation.h>
#include "UnityHttpRequests.h"
#include "Result.h"

@interface ResultStorage : NSObject {
    NSMutableArray *headers; // HeaderStorage[]
    UHR_Header *headerRefs; // UHR_Header[]
    NSData *body;
}
- (id)initWithResult:(Result *)result;
- (void)dealloc;
@end
