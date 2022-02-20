#import "HeaderStorage.h"

@implementation HeaderStorage

- (id)initWithKey:(NSString *)key andValue:(NSString *)value {
    self = [super init];
    if (self != nil) {
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
