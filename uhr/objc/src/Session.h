#import <Foundation/Foundation.h>
#include "UnityHttpRequests.h"

@interface Session : NSObject {}
@property (nonatomic, readonly, retain) NSMutableDictionary *tasks;
@property (nonatomic, readonly, retain) NSURLSession *session;
@property (nonatomic, readonly, retain) NSMutableArray *results;
@property (nonatomic, readonly, retain) NSMutableDictionary *resultStorage;
@property (nonatomic) UHR_RequestId nextRequestID;
- (id)init;
- (void)dealloc;
@end
