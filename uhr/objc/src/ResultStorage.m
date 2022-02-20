#import "ResultStorage.h"
#import "HeaderStorage.h"

@implementation ResultStorage

- (id)initWithRid:(UHR_RequestId)rid response:(NSHTTPURLResponse *)response body:(NSData *)body error:(NSError *)error {
    self = [super init];
    if (self != nil) {
        _rid = rid;
        _error = [error retain];
        _body = [body retain];
        
        if (response == nil) {
            // Inidcating a network error
            _status = 0;
            _headers = [[NSMutableArray alloc] initWithCapacity:0];
            _headerRefs = nil;
        } else {
            _status = (uint32_t)response.statusCode;

            NSDictionary *responseHeaders = response.allHeaderFields;
            _headers = [[NSMutableArray alloc] initWithCapacity:responseHeaders.count];
            _headerRefs = (UHR_Header *)malloc(sizeof(UHR_Header) * responseHeaders.count);

            int i = 0;
            for(id key in responseHeaders) {
                HeaderStorage *storage = [[HeaderStorage alloc]
                    initWithKey:(NSString *)key
                    andValue:(NSString *)responseHeaders[key]];
                
                UHR_Header headerRef;
                headerRef.name.length = (uint32_t)storage.key.length;
                headerRef.name.characters = (const uint16_t*)CFStringGetCharactersPtr((__bridge CFStringRef) storage.key);
                headerRef.value.length = (uint32_t)storage.value.length;
                headerRef.value.characters = (const uint16_t*)CFStringGetCharactersPtr((__bridge CFStringRef) storage.value);
                _headerRefs[i++] = headerRef;

                [_headers addObject:storage];
                [storage release];
            }
        }
    }
    return self;
}

- (void)dealloc {
    [_error release];
    _error = nil;
    
    [_headers release];
    _headers = nil;

    [_body release];
    _body = nil;

    if (_headerRefs != nil) {
        free(_headerRefs);
        _headerRefs = nil;
    }

    [super dealloc];
}

@end
