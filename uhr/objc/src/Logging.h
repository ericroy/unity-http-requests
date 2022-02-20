#import <Foundation/Foundation.h>
#include "UnityHttpRequests.h"

#define uhr_LOG_EXPRESSION( prefix, ... ) [::gLogSink log:[prefix stringByAppendingFormat:__VA_ARGS__]]

#define UHR_LOG_DEBUG(...) uhr_LOG_EXPRESSION(kLogTagDebug, __VA_ARGS__)
#define UHR_LOG_INFO(...) uhr_LOG_EXPRESSION(kLogTagInfo, __VA_ARGS__)
#define UHR_LOG_ERROR(...) uhr_LOG_EXPRESSION(kLogTagError, __VA_ARGS__)
#define UHR_LOG_CRITICAL(...) uhr_LOG_EXPRESSION(kLogTagCritical, __VA_ARGS__)

@interface LogSink : NSObject {}
- (id)init;
- (void)set:(UHR_LoggingCallback)callback userData:(void *)userData;
- (void)log:(NSString*)str;
- (void)dealloc;
@end

extern NSString* const kLogTagDebug;
extern NSString* const kLogTagInfo;
extern NSString* const kLogTagError;
extern NSString* const kLogTagCritical;

extern LogSink* gLogSink;
