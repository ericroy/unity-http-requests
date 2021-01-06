#import <Foundation/Foundation.h>

@interface HeaderStorage : NSObject {}
@property (nonatomic, readonly, retain) NSString *key;
@property (nonatomic, readonly, retain) NSString *value;
- (id)initWithKey:(NSString *)aKey andValue:(NSString *)aValue;
- (void)dealloc;
@end
