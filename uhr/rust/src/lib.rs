//use std::os::raw::{c_char};
//use std::ffi::{CString, CStr};

use std::any::Any;
use lazy_static::lazy_static;
use std::collections::HashMap;


#[allow(non_upper_case_globals)]
#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
mod bindings { include!("bindings.rs"); }
pub use bindings::*;

// #[no_mangle]
// pub extern fn rust_greeting(to: *const c_char) -> *mut c_char {
//     let c_str = unsafe { CStr::from_ptr(to) };
//     let recipient = match c_str.to_str() {
//         Err(_) => "there",
//         Ok(string) => string,
//     };

//     CString::new("Hello ".to_owned() + recipient).unwrap().into_raw()
// }

lazy_static! {
    static ref ERROR_STRINGS: HashMap<UHR_Error, Vec<u16>> = [
        (UHR_ERR_OK,                         "Ok".encode_utf16().collect()),
        (UHR_ERR_INVALID_CONTEXT,            "The context handle was invalid".encode_utf16().collect()),
        (UHR_ERR_MISSING_REQUIRED_PARAMETER, "A required function parameter was missing or null".encode_utf16().collect()),
        (UHR_ERR_INVALID_HTTP_METHOD,        "Invalid HTTP method".encode_utf16().collect()),
        (UHR_ERR_FAILED_TO_CREATE_REQUEST,   "Failed to create request".encode_utf16().collect()),
        (UHR_ERR_UNKNOWN_ERROR_CODE,         "Unknown error code".encode_utf16().collect()),
    ].iter().cloned().collect();

    static ref METHOD_STRINGS: HashMap<UHR_Method, Vec<u16>> = [
        (UHR_METHOD_GET,     "GET".encode_utf16().collect()),
        (UHR_METHOD_HEAD,    "HEAD".encode_utf16().collect()),
        (UHR_METHOD_POST,    "POST".encode_utf16().collect()),
        (UHR_METHOD_PUT,     "PUT".encode_utf16().collect()),
        (UHR_METHOD_PATCH,   "PATCH".encode_utf16().collect()),
        (UHR_METHOD_DELETE,  "DELETE".encode_utf16().collect()),
        (UHR_METHOD_CONNECT, "CONNECT".encode_utf16().collect()),
        (UHR_METHOD_OPTIONS, "OPTIONS".encode_utf16().collect()),
        (UHR_METHOD_TRACE,   "TRACE".encode_utf16().collect()),
    ].iter().cloned().collect();
}

struct Context {
    
}

#[no_mangle]
pub extern "C" fn UHR_ErrorToString(err: UHR_Error, error_message_out: *mut UHR_StringRef) -> UHR_Error {
    let msg = &ERROR_STRINGS[&err];
    let out = unsafe { &mut *error_message_out };
    out.characters = msg.as_ptr();
    out.length = msg.len() as u32;
    UHR_ERR_OK
}

#[no_mangle]
pub extern "C" fn UHR_CreateHTTPContext(http_context_handle_out: *mut UHR_HttpContext) -> UHR_Error {
    unsafe {
        let context = http_context_handle_out as *mut *mut Context;
        *context = Box::into_raw(Box::new(Context{}));
        UHR_ERR_OK
    }
}

#[no_mangle]
pub extern "C" fn UHR_DestroyHTTPContext(http_context_handle: UHR_HttpContext) -> UHR_Error {
    unsafe {
        let context = http_context_handle as *mut Context;
        if !context.is_null() {
            Box::from_raw(context);
        }
        UHR_ERR_OK
    }
}

#[no_mangle]
pub extern "C" fn UHR_CreateRequest(
    http_context_handle: UHR_HttpContext,
    url: UHR_StringRef,
    method: UHR_Method,
    headers: *mut UHR_Header,
    headers_count: u32,
    body: *mut ::std::os::raw::c_char,
    body_length: u32,
    rid_out: *mut UHR_RequestId,
) -> UHR_Error {
    let context = http_context_handle as *mut Context;
    if context.is_null() {
        return UHR_ERR_INVALID_CONTEXT
    }


    UHR_ERR_OK
}

#[no_mangle]
pub extern "C" fn UHR_Update(
    http_context_handle: UHR_HttpContext,
    responses_out: *mut UHR_Response,
    responses_capacity: u32,
    response_count_out: *mut u32,
) -> UHR_Error {
    let context = http_context_handle as *mut Context;
    if context.is_null() {
        return UHR_ERR_INVALID_CONTEXT
    }


    UHR_ERR_OK
}

#[no_mangle]
pub extern "C" fn UHR_DestroyRequests(
    http_context_handle: UHR_HttpContext,
    request_ids: *mut UHR_RequestId,
    request_ids_count: u32,
) -> UHR_Error {
    let context = http_context_handle as *mut Context;
    if context.is_null() {
        return UHR_ERR_INVALID_CONTEXT
    }


    UHR_ERR_OK
}
