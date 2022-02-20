#import "HeaderStorage.h"

@implementation HeaderStorage

- (id)initWithKey:(NSString *)key andValue:(NSString *)value {
    if (self = [super init]) {
        _key = [key retain];
        _value = [value retain];
    }
    return self;
}

- (void)dealloc {
    [_key release];
    _key = nil;
    
    [_value release];
    _value = nil;

    [super dealloc];
}

@end
