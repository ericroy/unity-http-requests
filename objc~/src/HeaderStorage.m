#import "src/HeaderStorage.h"

@implementation HeaderStorage
- (id)initWithKey:(NSString *)aKey andValue:(NSString *)aValue {
    self = [super init];
    if (self) {
        key = [aKey retain];
        value = [aValue retain];
    }
    return self;
}
- (void)dealloc {
    [key release];
    key = nil;
    
    [value release];
    value = nil;

    [super dealloc];
}
@end