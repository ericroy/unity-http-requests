#import "HeaderStorage.h"

@implementation HeaderStorage
@synthesize key;
@synthesize value;
- (id)initWithKey:(NSString *)aKey andValue:(NSString *)aValue {
    self = [super init];
    if (self) {
        _key = [aKey retain];
        _value = [aValue retain];
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