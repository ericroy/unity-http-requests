use std::ptr;
use std::collections::HashMap;
use reqwest::Method;
use error_chain::error_chain;
use lazy_static::lazy_static;

#[allow(non_upper_case_globals)]
#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
mod bindings { include!("bindings.rs"); }
pub use bindings::*;

mod uhr;
use crate::uhr::context::Context;

error_chain! {
    foreign_links {
        Io(std::io::Error);
        FromUtf16Error(std::string::FromUtf16Error);
        //ParseIntError(std::num::ParseIntError);
    }
}

lazy_static! {
    static ref ERROR_STRINGS: HashMap<UHR_Error, Vec<u16>> = [
        (UHR_ERR_OK,                         "Ok".encode_utf16().collect()),
        (UHR_ERR_INVALID_CONTEXT,            "The context handle was invalid".encode_utf16().collect()),
        (UHR_ERR_MISSING_REQUIRED_PARAMETER, "A required function parameter was missing or null".encode_utf16().collect()),
        (UHR_ERR_INVALID_HTTP_METHOD,        "Invalid HTTP method".encode_utf16().collect()),
        (UHR_ERR_FAILED_TO_CREATE_REQUEST,   "Failed to create request".encode_utf16().collect()),
        (UHR_ERR_UNKNOWN_ERROR_CODE,         "Unknown error code".encode_utf16().collect()),
        (UHR_ERR_FAILED_TO_CREATE_CONTEXT,   "Failed to create context".encode_utf16().collect()),
        (UHR_ERR_STRING_DECODING_ERROR,      "Failed to decode utf16".encode_utf16().collect()),
    ].iter().cloned().collect();

    static ref METHODS: HashMap<UHR_Method, Method> = [
        (UHR_METHOD_GET,     Method::GET),
        (UHR_METHOD_HEAD,    Method::HEAD),
        (UHR_METHOD_POST,    Method::POST),
        (UHR_METHOD_PUT,     Method::PUT),
        (UHR_METHOD_PATCH,   Method::PATCH),
        (UHR_METHOD_DELETE,  Method::DELETE),
        (UHR_METHOD_CONNECT, Method::CONNECT),
        (UHR_METHOD_OPTIONS, Method::OPTIONS),
        (UHR_METHOD_TRACE,   Method::TRACE),
    ].iter().cloned().collect();
}

unsafe fn string_ref_to_string(sr: UHR_StringRef) -> Result<String> {
    let slice = std::slice::from_raw_parts(sr.characters, sr.length as usize);
    Ok(String::from_utf16(slice)?)
}

#[no_mangle]
pub extern "C" fn UHR_ErrorToString(err: UHR_Error, error_message_out: *mut UHR_StringRef) -> UHR_Error {
    if error_message_out == ptr::null_mut() {
		return UHR_ERR_MISSING_REQUIRED_PARAMETER
	}
    match ERROR_STRINGS.get(&err) {
        Some(msg) => {
            let out = unsafe { &mut *error_message_out };
            out.characters = msg.as_ptr();
            out.length = msg.len() as u32;
            UHR_ERR_OK
        },
        _ => UHR_ERR_UNKNOWN_ERROR_CODE
    }
}

#[no_mangle]
pub extern "C" fn UHR_CreateHTTPContext(http_context_handle_out: *mut UHR_HttpContext) -> UHR_Error {
    unsafe {
        if http_context_handle_out == ptr::null_mut() {
            return UHR_ERR_MISSING_REQUIRED_PARAMETER
        }
        let context = match Context::new() {
            Ok(c) => Box::new(c),
            _ => return UHR_ERR_FAILED_TO_CREATE_CONTEXT
        };        
        let out = http_context_handle_out as *mut *mut Context;
        *out = Box::into_raw(context);
        UHR_ERR_OK
    }
}

#[no_mangle]
pub extern "C" fn UHR_DestroyHTTPContext(http_context_handle: UHR_HttpContext) -> UHR_Error {
    unsafe {
        let context = http_context_handle as *mut Context;
        if context.is_null() {
            return UHR_ERR_INVALID_CONTEXT
        }
        let context = Box::from_raw(context);

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
    if rid_out == ptr::null_mut() {
		return UHR_ERR_MISSING_REQUIRED_PARAMETER
	}

	if headers_count > 0 && headers == ptr::null_mut() {
		return UHR_ERR_MISSING_REQUIRED_PARAMETER
	}

	if body_length > 0 && body == ptr::null_mut() {
		return UHR_ERR_MISSING_REQUIRED_PARAMETER
	}

	let context = http_context_handle as *mut Context;
    if context.is_null() {
        return UHR_ERR_INVALID_CONTEXT
    }
    let context = unsafe { &mut *context };

    let method = match METHODS.get(&method) {
        Some(m) => m.clone(),
        None => return UHR_ERR_INVALID_HTTP_METHOD
    };

    let url = match unsafe { string_ref_to_string(url) } {
        Ok(s) => s,
        _ => return UHR_ERR_STRING_DECODING_ERROR
    };

    context.runtime.enter();
    let builder = context.client.request(method, &url);
    let response = builder.send();


    
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
