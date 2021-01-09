#import "ResultStorage.h"
#import "HeaderStorage.h"

@implementation ResultStorage

@synthesize rid;
@synthesize error;
@synthesize status;
@synthesize headers;
@synthesize headerRefs;
@synthesize body;


- (id)initWithRid:(UHR_RequestId)aRid response:(NSHTTPURLResponse *)response body:(NSData *)aBody error:(NSError *)aError {
    self = [super init];
    if (self) {
        rid = aRid;
        error = [aError retain];
        status = (uint32_t)response.statusCode;
        body = [aBody retain];

        NSDictionary *allHeaderFields = response.allHeaderFields;
        headers = [[NSMutableArray alloc] initWithCapacity:allHeaderFields.count];
        headerRefs = (UHR_Header *)malloc(sizeof(UHR_Header) * allHeaderFields.count);

        int i = 0;
        for(id key in allHeaderFields) {
            HeaderStorage *storage = [[HeaderStorage alloc]
                initWithKey:(NSString *)key
                andValue:(NSString *)allHeaderFields[key]];
            
            UHR_Header headerRef;
            headerRef.name.length = (uint32_t)storage.key.length;
            headerRef.name.characters = (const uint16_t*)CFStringGetCharactersPtr((__bridge CFStringRef) storage.key);
            headerRef.value.length = (uint32_t)storage.value.length;
            headerRef.value.characters = (const uint16_t*)CFStringGetCharactersPtr((__bridge CFStringRef) storage.value);
            headerRefs[i++] = headerRef;

            [headers addObject:storage];
            [storage release];
        }
    }
    return self;
}

- (void)dealloc {
    [error release];
    error = nil;
    
    [headers release];
    headers = nil;

    [body release];
    body = nil;

    free(headerRefs);
    headerRefs = nil;

    [super dealloc];
}

@end
