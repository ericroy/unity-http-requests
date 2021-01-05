#import <Foundation/Foundation.h>

@interface HeaderStorage : NSObject {
    NSString *key;
    NSString *value;
}
- (id)initWithKey:(NSString *)aKey andValue:(NSString *)aValue;
- (void)dealloc;
@end
