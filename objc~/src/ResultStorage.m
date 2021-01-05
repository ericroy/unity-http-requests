#import "ResultStorage.h"
#import "HeaderStorage.h"

@implementation ResultStorage
@synthesize body;
@synthesize headers;
@synthesize headerRefs;
- (id)initWithResult:(Result *)result {
    self = [super init];
    if (self) {
        body = [result.data retain];

        NSDictionary *allHeaderFields = result.response.allHeaderFields;
        headers = [[NSMutableArray alloc] initWithCapacity:allHeaderFields.count];
        headerRefs = (UHR_Header *)malloc(sizeof(UHR_Header) * allHeaderFields.count);

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
