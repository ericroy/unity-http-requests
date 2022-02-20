#import "Logging.h"

NSString* const kLogTagDebug = @"UHR[DEBUG]: ";
NSString* const kLogTagInfo = @"UHR[INFO]: ";
NSString* const kLogTagError = @"UHR[ERROR]: ";
NSString* const kLogTagCritical = @"UHR[CRITICAL]: ";

static LogSink* gLogSink = nil;

@interface LogSink()
@property (nonatomic) UHR_LoggingCallback callback;
@property (nonatomic) void* userData;
- (id)init;
- (void)set:(UHR_LoggingCallback)callback userData:(void *)userData;
- (void)log:(NSString*)str;
- (void)dealloc;
@end

@implementation LogSink

+ (void)log:(NSString*)str {
    @synchronized(self) {
        if (gLogSink == nil)
            gLogSink = [[self alloc] init];
        [gLogSink log:str];
    }
}

+ (void)set:(UHR_LoggingCallback)callback userData:(void *)userData {
    @synchronized(self) {
        if (gLogSink == nil)
            gLogSink = [[self alloc] init];
        [gLogSink set:callback userData:userData];
    }
}

- (id)init {
    if (self = [super init]) {
        _callback = nil;
        _userData = nil;
    }
    return self;
}

- (void)set:(UHR_LoggingCallback)callback userData:(void *)userData {
    self.callback = callback;
    self.userData = userData;
}

- (void)log:(NSString*)str {
    if (self.callback != nil) {
        UHR_StringRef ref;
        ref.characters = (const uint16_t*)CFStringGetCharactersPtr((__bridge CFStringRef)str);
        ref.length = (uint32_t)str.length;
        self.callback(ref, self.userData);
    }
}

- (void)dealloc {
    _callback = nil;
    _userData = nil;
    [super dealloc];
}

@end
