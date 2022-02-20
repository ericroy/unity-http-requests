#import "Logging.h"

LogSink* gLogSink = [[LogSink alloc] init];

@implementation LogSink {
    UHR_LoggingCallback _callback;
	void* _userData;
}

- (id)init {
    self = [super init];
    if (self) {
        self._callback = nil;
        self._userData = nil;
    }
    return self;
}

- (void)set:(UHR_LoggingCallback)callback userData:(void *)userData {
    @synchronized(self) {
        self._callback = callback;
        self._userData = userData;
    }
}

- (void)log:(NSString*)str {
    @synchronized(self) {
        if (self._callback != nil) {
            UHR_StringRef ref;
            ref->characters = (const uint16_t*)CFStringGetCharactersPtr((__bridge CFStringRef)str);
            ref->length = (uint32_t)str.length;
            self._callback(ref, self._userData);
        }
    }
}

- (void)dealloc {
    [super dealloc];
}

@end
