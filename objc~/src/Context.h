#import <Foundation/Foundation.h>
#include "src/UnityHttpRequests.h"

@interface Context : NSObject
NSMutableDictionary *tasks;
NSURLSession *session;
NSLock *resultsLock;
NSMutableArray *results;
NSMutableDictionary *resultStorage;
UHR_RequestId nextRequestID;
- (id)init;
- (void)dealloc;
@end
