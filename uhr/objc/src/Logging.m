#import "Logging.h"

NSString* const kLogTagDebug = @"UHR[DEBUG]: ";
NSString* const kLogTagInfo = @"UHR[INFO]: ";
NSString* const kLogTagError = @"UHR[ERROR]: ";
NSString* const kLogTagCritical = @"UHR[CRITICAL]: ";

LogSink* gLogSink = [[LogSink alloc] init];

@interface LogSink()
@property (nonatomic) UHR_LoggingCallback callback;
@property (nonatomic) void* userData;
@end

@implementation LogSink

- (id)init {
    self = [super init];
    if (self) {
        _callback = nil;
        _userData = nil;
    }
    return self;
}

- (void)set:(UHR_LoggingCallback)callback userData:(void *)userData {
    @synchronized(self) {
        self.callback = callback;
        self.userData = userData;
    }
}

- (void)log:(NSString*)str {
    @synchronized(self) {
        if (self.callback != nil) {
            UHR_StringRef ref;
            ref.characters = (const uint16_t*)CFStringGetCharactersPtr((__bridge CFStringRef)str);
            ref.length = (uint32_t)str.length;
            self.callback(ref, self.userData);
        }
    }
}

- (void)dealloc {
    _callback = nil;
    _userData = nil;
    [super dealloc];
}

@end