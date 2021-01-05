#import <Foundation/Foundation.h>
#include "UnityHttpRequests.h"
#import "Result.h"
#import "ResultStorage.h"
#import "HeaderStorage.h"
#import "Context.h"

#if ARC
#error UnityHttpRequests implementation does not support ARC
#endif

NSString* lastError = @"";

void UHR_GetLastError(UHR_StringRef* errorOut) {
    errorOut->characters = (uint16_t* )CFStringGetCharactersPtr((__bridge CFStringRef) lastError);
	errorOut->length = (int32_t)lastError.length;
}

UHR_HttpContext UHR_CreateHTTPContext() {
    Context* context = [[Context alloc] init];
    return (UHR_HttpContext)context;
}

void UHR_DestroyHTTPContext(UHR_HttpContext httpContextHandle) {
    Context* context = (Context* )httpContextHandle;
    [context release];
}

UHR_RequestId UHR_CreateRequest(UHR_HttpContext httpContextHandle, UHR_StringRef url, int32_t method, UHR_Header* headers, int32_t headersCount, char* body, int32_t bodyLength) {
    @autoreleasepool {
        __block Context* context = (Context* )httpContextHandle;
        if (context == nil) {
            lastError = @"Invalid HTTPContext handle";
            return UHR_REQUEST_ID_INVALID;
        }
        
        UHR_RequestId rid = context.nextRequestID++;

        NSMutableURLRequest* request = [NSMutableURLRequest
            requestWithURL: [NSURL
                URLWithString: [NSString
                    stringWithCharacters:url.characters length:url.length]]];

        request.HTTPMethod = (method == UHR_METHOD_POST) ? @"POST" : @"GET";
        if (body != nil && bodyLength > 0) {
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

        __block NSURLSessionDataTask* task = [context.session
            dataTaskWithRequest:request
            completionHandler:^(NSData* data, NSURLResponse* response, NSError* error) {
                Result* result = [[Result alloc]
                    initWithRid:rid
                    response:(NSHTTPURLResponse* )response
                    data:data
                    error:error];
                [context.resultsLock lock];
                [context.results insertObject:result atIndex:0];
                [context.resultsLock unlock];
                [result release];
            }];
        [task resume];
        [context.tasks setObject:task forKey:[NSNumber numberWithInt:rid]];

        return rid;
    } // autoreleasepool
}

int32_t UHR_Update(UHR_HttpContext httpContextHandle, UHR_Response* responsesOut, int32_t responsesCapacity) {
    @autoreleasepool {
        Context* context = (Context* )httpContextHandle;
        if (context == nil) {
            lastError = @"Invalid HTTPContext handle";
            return -1;
        }

        int32_t resultCount = 0;

        [context.resultsLock lock];
        for (; resultCount < responsesCapacity && context.results.count > 0; ++resultCount) {
            Result* result = (Result* )[context.results lastObject];
            ResultStorage* storage = [[ResultStorage alloc] initWithResult:result];
            [context.results removeObjectAtIndex:context.results.count-1];

            UHR_Response res;
            res.request_id = result.rid;
            res.http_status = result.response.statusCode,
            res.response_headers = &storage.headerRefs[0];
            res.response_headers_count = storage.headers.count;
            res.response_body = (char*)[storage.body bytes];
            res.response_body_length = storage.body.length;
            responsesOut[resultCount] = res;

            NSNumber* ridKey = [NSNumber numberWithInt:result.rid];
            [context.resultStorage setObject:storage forKey:ridKey];
            [storage release];
        }
        [context.resultsLock unlock];
        return resultCount;
    } // autoreleasepool
}

int32_t UHR_DestroyRequests(UHR_HttpContext httpContextHandle, UHR_RequestId* requestIDs, int32_t requestIDsCount) {
    @autoreleasepool {
        Context* context = (Context* )httpContextHandle;
        if (context == nil) {
            lastError = @"Invalid HTTPContext handle";
            return -1;
        }
        for (int32_t i = 0; i < requestIDsCount; ++i) {
            NSNumber* ridKey = [NSNumber numberWithInt:requestIDs[i]];
            NSURLSessionDataTask* task = [context.tasks objectForKey:ridKey];
            if (task != nil) {
                [task cancel];
                [context.tasks removeObjectForKey:ridKey];
            }
            [context.resultStorage removeObjectForKey:ridKey];
        }
        return 0;
    } // autoreleasepool
}
