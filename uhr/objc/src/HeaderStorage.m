#import "HeaderStorage.h"

@implementation HeaderStorage

- (id)initWithKey:(NSString *)key andValue:(NSString *)value {
    self = [super init];
    if (self) {
        self.key = key;
        self.value = value;
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
