#import <Foundation/Foundation.h>

@interface HeaderStorage : NSObject {}
@property (readonly, retain) NSString *key;
@property (readonly, retain) NSString *value;
- (id)initWithKey:(NSString *)aKey andValue:(NSString *)aValue;
- (void)dealloc;
@end
