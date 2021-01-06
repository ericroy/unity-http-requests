package main

// #cgo CFLAGS: -DUHR_CGO -I../../../include
// #include <UnityHttpRequests.h>
import "C"

import (
	"bytes"
	"context"
	"io"
	"net/http"
	"time"
	"unicode/utf16"
	"unsafe"
)

var errorStrings = map[C.UHR_Error][]uint16{
	C.UHR_ERR_OK:                         utf16.Encode([]rune("Ok")),
	C.UHR_ERR_INVALID_CONTEXT:            utf16.Encode([]rune("The context handle was invalid")),
	C.UHR_ERR_MISSING_REQUIRED_PARAMETER: utf16.Encode([]rune("A required function parameter was missing or null")),
	C.UHR_ERR_INVALID_HTTP_METHOD:        utf16.Encode([]rune("Invalid HTTP method")),
	C.UHR_ERR_FAILED_TO_CREATE_REQUEST:   utf16.Encode([]rune("Failed to create request")),
	C.UHR_ERR_UNKNOWN_ERROR_CODE:         utf16.Encode([]rune("Unknown error code")),
}

var methodStrings = map[C.UHR_Method]string{
	C.UHR_METHOD_GET:     "GET",
	C.UHR_METHOD_HEAD:    "HEAD",
	C.UHR_METHOD_POST:    "POST",
	C.UHR_METHOD_PUT:     "PUT",
	C.UHR_METHOD_PATCH:   "PATCH",
	C.UHR_METHOD_DELETE:  "DELETE",
	C.UHR_METHOD_CONNECT: "CONNECT",
	C.UHR_METHOD_OPTIONS: "OPTIONS",
	C.UHR_METHOD_TRACE:   "TRACE",
}

// This holds a reference to the HTTPContext structures which have been
// returned to C-land as a pointer.  It keeps them from being collected prematurely.
var httpContexts = make(map[C.UHR_HttpContext]*HTTPContext, 4)

var scratchString = make([]uint16, 0, 128)

// RequestID is an identifier for an http request
type RequestID int32

// Result is for tracking the result of an http request
type Result struct {
	rid  int
	err  error
	resp *http.Response
}

// HeaderStorage is the backing memory of a header that can be referenced by UHR_StringRefs
type HeaderStorage struct {
	key   []uint16
	value []uint16
}

// ResultStorage is the backing memory for a response that can be referenced by a UHR_Response
type ResultStorage struct {
	headers    []HeaderStorage
	headerRefs []C.UHR_Header
	body       []byte
}

// HTTPContext manages a set of requests and responses.
type HTTPContext struct {
	client        *http.Client
	cancelFuncs   map[int]context.CancelFunc // Keyed by request id
	resultStorage map[int]*ResultStorage     // Keyed by request id
	results       chan Result
	nextRequestID int
}

func stringRefToString(sr C.UHR_StringRef) string {
	charactersSlice := (*[1 << 30]uint16)(unsafe.Pointer(sr.characters))[:uint(sr.length)]
	scratchString = append(scratchString[:0], charactersSlice...)
	return string(utf16.Decode(scratchString))
}

func newResultStorage(body []byte, headers http.Header) *ResultStorage {
	storage := &ResultStorage{
		headers:    make([]HeaderStorage, 0, len(headers)),
		headerRefs: make([]C.UHR_Header, 0, len(headers)),
		body:       body,
	}
	for k, v := range headers {
		if len(v) > 0 {
			// NOTE: We don't support multiple headers with the same name.
			storage.headers = append(storage.headers, HeaderStorage{
				key:   utf16.Encode([]rune(k)),
				value: utf16.Encode([]rune(v[0])),
			})
		}
	}
	for _, header := range storage.headers {
		storage.headerRefs = append(storage.headerRefs, C.UHR_Header{
			name: C.UHR_StringRef{
				characters: (*C.uint16_t)(unsafe.Pointer(&header.key[0])),
				length:     C.uint32_t(len(header.key)),
			},
			value: C.UHR_StringRef{
				characters: (*C.uint16_t)(unsafe.Pointer(&header.value[0])),
				length:     C.uint32_t(len(header.value)),
			},
		})
	}
	return storage
}

//export UHR_ErrorToString
func UHR_ErrorToString(err C.UHR_Error, errorMessageOut *C.UHR_StringRef) C.UHR_Error {
	if errorMessageOut == nil {
		return C.UHR_ERR_MISSING_REQUIRED_PARAMETER
	}
	str, ok := errorStrings[err]
	if !ok {
		return C.UHR_ERR_UNKNOWN_ERROR_CODE
	}
	errorMessageOut.characters = (*C.uint16_t)(unsafe.Pointer(&str[0]))
	errorMessageOut.length = C.uint32_t(len(str))
	return C.UHR_ERR_OK
}

//export UHR_CreateHTTPContext
func UHR_CreateHTTPContext(httpContextOut *C.UHR_HttpContext) C.UHR_Error {
	if httpContextOut == nil {
		return C.UHR_ERR_MISSING_REQUIRED_PARAMETER
	}
	httpContext := &HTTPContext{
		client: &http.Client{
			Transport: &http.Transport{
				MaxIdleConns:       10,
				IdleConnTimeout:    30 * time.Second,
				DisableCompression: true,
			},
		},
		cancelFuncs:   make(map[int]context.CancelFunc, 16),
		resultStorage: make(map[int]*ResultStorage, 16),
		results:       make(chan Result, 16),
		nextRequestID: 1,
	}
	opaqueHandle := (C.UHR_HttpContext)(uintptr(unsafe.Pointer(httpContext)))
	httpContexts[opaqueHandle] = httpContext
	*httpContextOut = opaqueHandle
	return C.UHR_ERR_OK
}

//export UHR_DestroyHTTPContext
func UHR_DestroyHTTPContext(httpContextHandle C.UHR_HttpContext) C.UHR_Error {
	httpContext, ok := httpContexts[httpContextHandle]
	if !ok {
		return C.UHR_ERR_INVALID_CONTEXT
	}
	close(httpContext.results)
	for _, cancel := range httpContext.cancelFuncs {
		cancel()
	}
	httpContext.client.CloseIdleConnections()
	delete(httpContexts, httpContextHandle)
	return C.UHR_ERR_OK
}

//export UHR_CreateRequest
func UHR_CreateRequest(
	httpContextHandle C.UHR_HttpContext,
	url C.UHR_StringRef,
	method C.UHR_Method,
	headers *C.UHR_Header,
	headersCount C.uint32_t,
	body *C.char,
	bodyLength C.uint32_t,
	ridOut *C.UHR_RequestId,
) C.UHR_Error {
	if ridOut == nil {
		return C.UHR_ERR_MISSING_REQUIRED_PARAMETER
	}

	if headersCount > 0 && headers == nil {
		return C.UHR_ERR_MISSING_REQUIRED_PARAMETER
	}

	if bodyLength > 0 && body == nil {
		return C.UHR_ERR_MISSING_REQUIRED_PARAMETER
	}

	httpContext, ok := httpContexts[httpContextHandle]
	if !ok {
		return C.UHR_ERR_INVALID_CONTEXT
	}

	methodStr, ok := methodStrings[method]
	if !ok {
		return C.UHR_ERR_INVALID_HTTP_METHOD
	}

	var bodyReader io.Reader
	if bodyLength > 0 {
		buf := make([]byte, int(bodyLength))
		copy(buf, (*[1 << 30]byte)(unsafe.Pointer(body))[:uint(bodyLength)])
		bodyReader = bytes.NewBuffer(buf)
	}

	ctx, cancel := context.WithCancel(context.Background())
	req, err := http.NewRequestWithContext(ctx, methodStr, stringRefToString(url), bodyReader)
	if err != nil {
		cancel()
		return C.UHR_ERR_FAILED_TO_CREATE_REQUEST
	}

	headerRefs := (*[1 << 30]C.UHR_Header)(unsafe.Pointer(headers))[:uint(headersCount)]
	for _, header := range headerRefs {
		req.Header.Add(stringRefToString(header.name), stringRefToString(header.value))
	}

	// Advance rid, handle wraparound
	rid := httpContext.nextRequestID
	httpContext.nextRequestID++
	if httpContext.nextRequestID <= 0 {
		httpContext.nextRequestID = 1
	}

	httpContext.cancelFuncs[rid] = cancel

	go func() {
		resp, err := httpContext.client.Do(req)
		httpContext.results <- Result{
			rid:  rid,
			resp: resp,
			err:  err,
		}
	}()

	*ridOut = C.UHR_RequestId(rid)
	return C.UHR_ERR_OK
}

//export UHR_Update
func UHR_Update(httpContextHandle C.UHR_HttpContext, responsesOut *C.UHR_Response, responsesCapacity C.uint32_t, responseCountOut *C.uint32_t) C.int32_t {
	if responseCountOut == nil {
		return C.UHR_ERR_MISSING_REQUIRED_PARAMETER
	}

	httpContext, ok := httpContexts[httpContextHandle]
	if !ok {
		return C.UHR_ERR_INVALID_CONTEXT
	}

	count := uint(0)
	responsesOutSlice := (*[1 << 30]C.UHR_Response)(unsafe.Pointer(responsesOut))[:uint(responsesCapacity)]

ForLoop:
	for ; count < uint(responsesCapacity); count++ {
		select {
		case res := <-httpContext.results:
			if _, ok := httpContext.cancelFuncs[res.rid]; !ok {
				// Already cancelled
				continue ForLoop
			}

			httpStatus := C.int32_t(res.resp.StatusCode)

			var body []byte
			if res.err != nil {
				httpStatus = C.int32_t(-1)
				body = []byte(res.err.Error())
			} else {
				// Use the content length if it's non-negative
				// Otherwise, start with an empty slice and let it grow.
				var capacity = res.resp.ContentLength
				if capacity < 0 {
					capacity = 0
				}
				bodyBuffer := bytes.NewBuffer(make([]byte, 0, capacity))
				if _, err := io.Copy(bodyBuffer, res.resp.Body); err != nil {
					httpStatus = C.int32_t(-1)
				}
				body = bodyBuffer.Bytes()
			}

			storage := newResultStorage(body, res.resp.Header)
			httpContext.resultStorage[res.rid] = storage

			responsesOutSlice[count] = C.UHR_Response{
				request_id:  C.int32_t(res.rid),
				http_status: httpStatus,
				headers: C.UHR_HeadersData{
					headers: &storage.headerRefs[0],
					count:   C.uint32_t(len(storage.headerRefs)),
				},
				body: C.UHR_BodyData{
					body:   (*C.char)(unsafe.Pointer(&storage.body[0])),
					length: C.uint32_t(len(storage.body)),
				},
			}
		default:
			break ForLoop
		}
	}

	*responseCountOut = C.uint32_t(count)
	return C.UHR_ERR_OK
}

//export UHR_DestroyRequests
func UHR_DestroyRequests(httpContextHandle C.UHR_HttpContext, requestIDs *C.UHR_RequestId, requestIDsCount C.uint32_t) C.UHR_Error {
	httpContext, ok := httpContexts[httpContextHandle]
	if !ok {
		return C.UHR_ERR_INVALID_CONTEXT
	}
	requestIDsSlice := (*[1 << 30]C.int32_t)(unsafe.Pointer(requestIDs))[:uint(requestIDsCount)]
	for _, rid := range requestIDsSlice {
		if cancel, ok := httpContext.cancelFuncs[int(rid)]; ok {
			cancel()
			delete(httpContext.cancelFuncs, int(rid))
			delete(httpContext.resultStorage, int(rid))
		}
	}
	return C.UHR_ERR_OK
}

func main() {}
