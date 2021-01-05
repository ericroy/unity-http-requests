#import <Foundation/Foundation.h>
#include "UnityHttpRequests.h"

@interface Context : NSObject {}
@property (readonly, retain) NSMutableDictionary *tasks;
@property (readonly, retain) NSURLSession *session;
@property (readonly, retain) NSLock *resultsLock;
@property (readonly, retain) NSMutableArray *results;
@property (readonly, retain) NSMutableDictionary *resultStorage;
@property UHR_RequestId nextRequestID;
- (id)init;
- (void)dealloc;
@end
