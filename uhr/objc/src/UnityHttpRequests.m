#import <Foundation/Foundation.h>
#include "UnityHttpRequests.h"
#import "ResultStorage.h"
#import "HeaderStorage.h"
#import "Session.h"
#import "Logging.h"

#ifdef ARC
#error UnityHttpRequests implementation does not support ARC
#endif

static NSString* const kErrorStrings[] = {
    /* UHR_ERR_OK */                            @"Ok",
    /* UHR_ERR_INVALID_SESSION */               @"The session handle was invalid",
    /* UHR_ERR_MISSING_REQUIRED_PARAMETER */    @"A required function parameter was missing or null",
    /* UHR_ERR_INVALID_HTTP_METHOD */           @"Invalid HTTP method",
    /* UHR_ERR_INVALID_URL */                   @"Invalid URL",
    /* UHR_ERR_FAILED_TO_CREATE_REQUEST */      @"Failed to create request",
    /* UHR_ERR_UNKNOWN_ERROR_CODE */            @"Unknown error code",
    /* UHR_ERR_FAILED_TO_CREATE_SESSION */      @"Failed to create session",
    /* UHR_ERR_FAILED_TO_UPDATE_SESSION */      @"Failed to update session",
};
static const int kErrorStringsCount = sizeof(kErrorStrings) / sizeof(kErrorStrings[0]);

static NSString* const kMethodStrings[] = {
	/* UHR_METHOD_GET */        @"GET",
	/* UHR_METHOD_HEAD */       @"HEAD",
	/* UHR_METHOD_POST */       @"POST",
    /* UHR_METHOD_PUT */        @"PUT",
	/* UHR_METHOD_PATCH */      @"PATCH",
	/* UHR_METHOD_DELETE */     @"DELETE",
	/* UHR_METHOD_CONNECT */    @"CONNECT",
	/* UHR_METHOD_OPTIONS */    @"OPTIONS",
	/* UHR_METHOD_TRACE */      @"TRACE",
};
static const int kMethodStringsCount = sizeof(kMethodStrings) / sizeof(kMethodStrings[0]);


UHR_API void UHR_SetLoggingCallback(UHR_LoggingCallback callback, void* userData) {
	[LogSink set:callback userData:userData];
}

UHR_Error UHR_ErrorToString(UHR_Error err, UHR_StringRef* errorMessageOut) {
    if (errorMessageOut == nil)
        return UHR_ERR_MISSING_REQUIRED_PARAMETER;

    if (err >= kErrorStringsCount)
        return UHR_ERR_UNKNOWN_ERROR_CODE;

    NSString* str = kErrorStrings[err];
    errorMessageOut->characters = (const uint16_t*)CFStringGetCharactersPtr((__bridge CFStringRef)str);
	errorMessageOut->length = (uint32_t)str.length;
    return UHR_ERR_OK;
}

UHR_Error UHR_CreateHTTPSession(UHR_HttpSession* httpSessionHandleOut) {
    if (httpSessionHandleOut == nil)
        return UHR_ERR_MISSING_REQUIRED_PARAMETER;

    Session* session = [[Session alloc] init];
    *httpSessionHandleOut = (UHR_HttpSession)session;
    return UHR_ERR_OK;
}

UHR_Error UHR_DestroyHTTPSession(UHR_HttpSession httpSessionHandle) {
    Session* session = (Session*)httpSessionHandle;
    if (session == nil)
        return UHR_ERR_INVALID_SESSION;

    [session release];
    return UHR_ERR_OK;
}

UHR_Error UHR_CreateRequest(UHR_HttpSession httpSessionHandle,
    UHR_StringRef url,
    UHR_Method method,
    UHR_Header* headers,
    uint32_t headersCount,
    char* body,
    uint32_t bodyLength,
    UHR_RequestId* ridOut
) {
    @autoreleasepool {
        if (ridOut == nil)
            return UHR_ERR_MISSING_REQUIRED_PARAMETER;

        if (headersCount > 0 && headers == nil)
            return UHR_ERR_MISSING_REQUIRED_PARAMETER;

        if (bodyLength > 0 && body == nil)
            return UHR_ERR_MISSING_REQUIRED_PARAMETER;

        Session* __block session = (Session*)httpSessionHandle;
        if (session == nil)
            return UHR_ERR_INVALID_SESSION;

        if (method >= kMethodStringsCount)
            return UHR_ERR_INVALID_HTTP_METHOD;

        NSString* methodStr = kMethodStrings[method];
        
        // Advance rid, handle wraparound
        UHR_RequestId rid = session.nextRequestID++;
        if (session.nextRequestID == 0)
            session.nextRequestID = 1;

        NSURL *parsedURL = [NSURL URLWithString:[NSString stringWithCharacters:url.characters length:url.length]];
        if (parsedURL == nil)
            return UHR_ERR_INVALID_URL;

        NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:parsedURL];
        request.HTTPMethod = methodStr;

        if (bodyLength > 0)
            request.HTTPBody = [NSData dataWithBytes:body length:bodyLength];

        for (uint32_t i = 0; i < headersCount; ++i) {
            [request
                setValue: [NSString
                    stringWithCharacters:headers[i].name.characters
                    length:headers[i].name.length]
                forHTTPHeaderField: [NSString
                    stringWithCharacters:headers[i].value.characters
                    length:headers[i].value.length]];
        }

        NSURLSessionDataTask* task = [session.session
            dataTaskWithRequest:request
            completionHandler:^(NSData* responseBody, NSURLResponse* response, NSError* error) {
                UHR_LOG_DEBUG(@"Requestion complete: %@\n%@\n%@", responseBody, response, error);
                ResultStorage* result = [[ResultStorage alloc]
                    initWithRid:rid
                    response:(NSHTTPURLResponse*)response
                    body:responseBody
                    error:error];
                [session.resultsLock lock];
                [session.results insertObject:result atIndex:0];
                [session.resultsLock unlock];
                [result release];
            }];
        [task resume];
        [session.tasks setObject:task forKey:[NSNumber numberWithUnsignedInt:rid]];

        *ridOut = rid;
        return UHR_ERR_OK;
    } // autoreleasepool
}

UHR_Error UHR_Update(UHR_HttpSession httpSessionHandle, UHR_Response* responsesOut, uint32_t responsesCapacity, uint32_t* responseCountOut) {
    @autoreleasepool {
        if (responseCountOut == nil)
            return UHR_ERR_MISSING_REQUIRED_PARAMETER;

        if (responsesOut == nil || responsesCapacity == 0)
            return UHR_ERR_MISSING_REQUIRED_PARAMETER;
        
        Session* session = (Session*)httpSessionHandle;
        if (session == nil)
            return UHR_ERR_INVALID_SESSION;

        uint32_t count = 0;

        [session.resultsLock lock];
        for (; count < responsesCapacity && session.results.count > 0; ++count) {
            ResultStorage* result = (ResultStorage*)[session.results lastObject];

            UHR_Response res;
            res.request_id = result.rid;
            res.http_status = result.status;
            res.headers.count = (uint32_t)result.headers.count;
            res.headers.headers = res.headers.count > 0 ? &result.headerRefs[0] : nil;
            res.body.length = (uint32_t)result.body.length;
            res.body.body = res.body.length > 0 ? (const char*)[result.body bytes] : nil;
            responsesOut[count] = res;

            NSNumber* ridKey = [NSNumber numberWithUnsignedInt:result.rid];
            [session.resultStorage setObject:result forKey:ridKey];
            [session.results removeObjectAtIndex:session.results.count-1];
        }
        [session.resultsLock unlock];

        *responseCountOut = count;
        return UHR_ERR_OK;
    } // autoreleasepool
}

UHR_Error UHR_DestroyRequests(UHR_HttpSession httpSessionHandle, UHR_RequestId* requestIDs, uint32_t requestIDsCount) {
    @autoreleasepool {
        if (requestIDsCount > 0 && requestIDs == nil)
            return UHR_ERR_MISSING_REQUIRED_PARAMETER;

        Session* session = (Session*)httpSessionHandle;
        if (session == nil)
            return UHR_ERR_INVALID_SESSION;

        for (uint32_t i = 0; i < requestIDsCount; ++i) {
            NSNumber* ridKey = [NSNumber numberWithUnsignedInt:requestIDs[i]];
            NSURLSessionDataTask* task = session.tasks[ridKey];
            if (task != nil) {
                [task cancel];
                [session.tasks removeObjectForKey:ridKey];
            }
            [session.resultStorage removeObjectForKey:ridKey];
        }

        return 0;
    } // autoreleasepool
}
