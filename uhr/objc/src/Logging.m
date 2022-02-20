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
        self.callback = nil;
        self.userData = nil;
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
            self.callback(ref, self._userData);
        }
    }
}

- (void)dealloc {
    [super dealloc];
}

@end
