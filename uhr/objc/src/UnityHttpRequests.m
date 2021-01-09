#import <Foundation/Foundation.h>
#include "UnityHttpRequests.h"
#import "Result.h"
#import "ResultStorage.h"
#import "HeaderStorage.h"
#import "Context.h"

#ifdef ARC
#error UnityHttpRequests implementation does not support ARC
#endif

#define LITERALIZE_HELPER(x) #x
#define LITERALIZE(x) @LITERALIZE_HELPER(x)

#define kErrorStrings @{
    /* UHR_ERR_OK */                            @"Ok",
    /* UHR_ERR_INVALID_CONTEXT */               @"The context handle was invalid",
    /* UHR_ERR_MISSING_REQUIRED_PARAMETER */    @"A required function parameter was missing or null",
    /* UHR_ERR_INVALID_HTTP_METHOD */           @"Invalid HTTP method",
    /* UHR_ERR_FAILED_TO_CREATE_REQUEST */      @"Failed to create request",
    /* UHR_ERR_UNKNOWN_ERROR_CODE */            @"Unknown error code",
}

#define kMethodStrings @{
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

UHR_Error UHR_ErrorToString(UHR_Error err, UHR_StringRef* errorMessageOut) {
    if (errorMessageOut == nil) {
        return UHR_ERR_MISSING_REQUIRED_PARAMETER;
    }
    if (err >= kErrorStrings.length) {
        return UHR_ERR_UNKNOWN_ERROR_CODE;
    }
    NSString* str = kErrorStrings[err];
    errorMessageOut->characters = (const uint16_t*)CFStringGetCharactersPtr((__bridge CFStringRef)str);
	errorMessageOut->length = (uint32_t)str.length;
    return UHR_ERR_OK;
}

UHR_Error UHR_CreateHTTPContext(UHR_HttpContext* httpContextHandleOut) {
    if (httpContextHandleOut == nil) {
        return UHR_ERR_MISSING_REQUIRED_PARAMETER;
    }
    Context* context = [[Context alloc] init];
    *httpContextHandleOut = (UHR_HttpContext)context;
    return UHR_ERR_OK;
}

UHR_Error UHR_DestroyHTTPContext(UHR_HttpContext httpContextHandle) {
    Context* context = (Context*)httpContextHandle;
    if (context == nil) {
        return UHR_ERR_INVALID_CONTEXT;
    }
    [context release];
    return UHR_ERR_OK;
}

UHR_Error UHR_CreateRequest(UHR_HttpContext httpContextHandle,
    UHR_StringRef url,
    UHR_Method method,
    UHR_Header* headers,
    uint32_t headersCount,
    char* body,
    uint32_t bodyLength,
    UHR_RequestId* ridOut
) {
    @autoreleasepool {
        if (ridOut == nil) {
            return UHR_ERR_MISSING_REQUIRED_PARAMETER;
        }

        if (headersCount > 0 && headers == nil) {
            return UHR_ERR_MISSING_REQUIRED_PARAMETER;
        }

        if (bodyLength > 0 && body == nil) {
            return UHR_ERR_MISSING_REQUIRED_PARAMETER;
        }

        Context* __block context = (Context*)httpContextHandle;
        if (context == nil) {
            return UHR_ERR_INVALID_CONTEXT;
        }

        if (method >= kMethodStrings.length) {
            return UHR_ERR_INVALID_HTTP_METHOD;
        }
        NSString* methodStr = kMethodStrings[method];
        
        // Advance rid, handle wraparound
        UHR_RequestId rid = context.nextRequestID++;
        if (context.nextRequestID == 0) {
            context.nextRequestID = 1;
        }

        NSMutableURLRequest* request = [NSMutableURLRequest
            requestWithURL: [NSURL
                URLWithString: [NSString
                    stringWithCharacters:url.characters length:url.length]]];
        
        request.HTTPMethod = methodStr;

        if (bodyLength > 0) {
            request.HTTPBody = [NSData dataWithBytes:body length:bodyLength];
        }

        for (int32_t i = 0; i < headersCount; ++i) {
            [request
                setValue: [NSString
                    stringWithCharacters:headers[i].name.characters
                    length:headers[i].name.length]
                forHTTPHeaderField: [NSString
                    stringWithCharacters:headers[i].value.characters
                    length:headers[i].value.length]];
        }

        NSURLSessionDataTask* task = [context.session
            dataTaskWithRequest:request
            completionHandler:^(NSData* body, NSURLResponse* response, NSError* error) {
                ResultStorage* result = [[ResultStorage alloc]
                    initWithRid:rid
                    response:(NSHTTPURLResponse* )response
                    body:body
                    error:error];
                [context.resultsLock lock];
                [context.results insertObject:result atIndex:0];
                [context.resultsLock unlock];
                [result release];
            }];
        [task resume];
        [context.tasks setObject:task forKey:[NSNumber numberWithUnsignedInt:rid]];

        *ridOut = rid;
        return UHR_ERR_OK;
    } // autoreleasepool
}

UHR_Error UHR_Update(UHR_HttpContext httpContextHandle, UHR_Response* responsesOut, uint32_t responsesCapacity, uint32_t* responseCountOut) {
    @autoreleasepool {
        if (responseCountOut == nil) {
            return UHR_ERR_MISSING_REQUIRED_PARAMETER;
        }

        if (responsesOut == nil || responsesCapacity == 0) {
            return UHR_ERR_MISSING_REQUIRED_PARAMETER;
        }
        
        Context* context = (Context*)httpContextHandle;
        if (context == nil) {
            return UHR_ERR_INVALID_CONTEXT;
        }

        uint32_t count = 0;

        [context.resultsLock lock];
        for (; count < responsesCapacity && context.results.count > 0; ++count) {
            ResultStorage* result = (ResultStorage*)[context.results lastObject];

            UHR_Response res;
            res.request_id = result.rid;
            res.http_status = result.status;
            res.headers.headers = &result.headerRefs[0];
            res.headers.count = result.headers.count;
            res.body.body = (char*)[result.body bytes];
            res.body.length = result.body.length;
            responsesOut[count] = res;

            NSNumber* ridKey = [NSNumber numberWithUnsignedInt:result.rid];
            [context.resultStorage setObject:result forKey:ridKey];
            [context.results removeObjectAtIndex:context.results.count-1];
        }
        [context.resultsLock unlock];

        *responseCountOut = count;
        return UHR_ERR_OK;
    } // autoreleasepool
}

UHR_Error UHR_DestroyRequests(UHR_HttpContext httpContextHandle, UHR_RequestId* requestIDs, uint32_t requestIDsCount) {
    @autoreleasepool {
        if (requestIDsCount > 0 && requestIDs == nil) {
            return UHR_ERR_MISSING_REQUIRED_PARAMETER;
        }

        Context* context = (Context*)httpContextHandle;
        if (context == nil) {
            return UHR_ERR_INVALID_CONTEXT;
        }
        for (int32_t i = 0; i < requestIDsCount; ++i) {
            NSNumber* ridKey = [NSNumber numberWithInt:requestIDs[i]];
            NSURLSessionDataTask* task = context.tasks[ridKey];
            if (task != nil) {
                [task cancel];
                [context.tasks removeObjectForKey:ridKey];
            }
            [context.resultStorage removeObjectForKey:ridKey];
        }
        return 0;
    } // autoreleasepool
}
