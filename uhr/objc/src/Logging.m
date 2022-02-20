#import "Logging.h"

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
        _callback = callback;
        _userData = userData;
    }
}

- (void)log:(NSString*)str {
    @synchronized(self) {
        if (_callback != nil) {
            UHR_StringRef ref;
            ref.characters = (const uint16_t*)CFStringGetCharactersPtr((__bridge CFStringRef)str);
            ref.length = (uint32_t)str.length;
            _callback(ref, _userData);
        }
    }
}

- (void)dealloc {
    _callback = nil;
    _userData = nil;
    [super dealloc];
}

@end
