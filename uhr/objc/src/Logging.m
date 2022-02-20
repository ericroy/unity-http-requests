#import "Logging.h"

NSString* const kLogTagDebug = @"UHR[DEBUG]: ";
NSString* const kLogTagInfo = @"UHR[INFO]: ";
NSString* const kLogTagError = @"UHR[ERROR]: ";
NSString* const kLogTagCritical = @"UHR[CRITICAL]: ";

static UHR_LoggingCallback gLogCallback = nil;
static void* gLogUserData = nil;

@implementation LogSink

+ (void)set:(UHR_LoggingCallback)callback userData:(void*)userData {
    @synchronized(self) {
        gLogCallback = callback;
        gLogUserData = userData;
    }
}

+ (void)log:(NSString*)str {
    @synchronized(self) {
        if (gLogCallback != nil) {
            UHR_StringRef ref;
            ref.characters = (const uint16_t*)CFStringGetCharactersPtr((__bridge CFStringRef)str);
            ref.length = (uint32_t)str.length;
            gLogCallback(ref, gLogUserData);
        }
    }
}

@end
