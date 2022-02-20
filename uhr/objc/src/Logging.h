#import <Foundation/Foundation.h>
#include "UnityHttpRequests.h"

#define uhr_LOG_EXPRESSION( prefix, fmt, ... ) [::gLogSink log:[NSString stringWithFormat:fmt, __VA_ARGS__]]

#define UHR_LOG_DEBUG(fmt, ...) uhr_LOG_EXPRESSION("UHR[DEBUG]: ", fmt, __VA_ARGS__)
#define UHR_LOG_INFO(fmt, ...) uhr_LOG_EXPRESSION("UHR[INFO]: ", fmt, __VA_ARGS__)
#define UHR_LOG_ERROR(fmt, ...) uhr_LOG_EXPRESSION("UHR[ERROR]: ", fmt, __VA_ARGS__)
#define UHR_LOG_CRITICAL(fmt, ...) uhr_LOG_EXPRESSION("UHR[CRITICAL]: ", fmt, __VA_ARGS__)

@interface LogSink : NSObject {}
- (id)init;
- (void)set:(UHR_LoggingCallback)callback userData:(void *)userData;
- (void)log:(NSString*)str;
- (void)dealloc;
@end

extern LogSink* gLogSink;
