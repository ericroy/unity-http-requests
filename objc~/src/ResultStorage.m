#import "ResultStorage.h"

@implementation ResultStorage
- (id)initWithResult:(Result *)result {
    self = [super init];
    if (self) {
        body = [result.body retain];

        NSDictionary *allHeaderFields = result.response.allHeaderFields;
        headers = [[NSMutableArray alloc] initWithCapacity:allHeaderFields.length];
        headerRefs = (UHR_Header *)malloc(sizeof(UHR_Header) * allHeaderFields.length);

        int i = 0;
        for(id key in allHeaderFields) {
            HeaderStorage *storage = [[HeaderStorage alloc]
                initWithKey:(NSString *)key
                andValue:(NSString *)[allHeaderFields objectForKey:key]];
            
            UHR_Header headerRef;
            headerRef.name.length = storage.key.length;
            headerRef.name.characters = (*uint16_t)CFStringGetCharactersPtr((__bridge CFStringRef) storage.key);
            headerRef.value.length = storage.value.length;
            headerRef.value.characters = (*uint16_t)CFStringGetCharactersPtr((__bridge CFStringRef) storage.value);
            headerRefs[i++] = headerRef;

            [headers addObject:storage];
            [storage release];
        }
    }
    return self;
}
- (void)dealloc {
    [headers release];
    headers = nil;

    free(headerRefs);
    headerRefs = nil;

    [body release];
    body = nil;

    [super dealloc];
}
@end
