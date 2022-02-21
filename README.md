# Zero-GC HTTP requests for Unity

## Goals

- To make HTTP requests in Unity without allocating heap garbage at runtime
- To support all major platforms (windows, linux, mac, ios, android)
- To minimize the number of calls over the native boundary

## Constituent parts

- Unity
  - C# abstraction on top of the native API
- Native plugin implementations
  - ObjC / NSURLSession implementation for Mac and iOS
  - C++ / CURL implementation for all other platforms

## Installing the package

Add the package dependency to your `Packages/manifest.json`:

```js
// To get the latest:
"com.ericroy.unity-http-requests": "https://github.com/ericroy/unity-http-requests.git",

// Or, to pin at a certain version:
"com.ericroy.unity-http-requests": "https://github.com/ericroy/unity-http-requests.git#v1.0.13",
```

## Basic usage

- [QuickStart.cs](examples/quick_start/QuickStart.cs) demonstrates how to create an `HTTPSession` and use it.

- [ResponseDispatcher.cs](examples/response_dispatcher/ResponseDispatcher.cs) demonstrates how you can deliver responses directly to the party that initiated the request, instead of processing responses centrally.

## Platform-specific details

### Windows

On Windows, the C++ implementation statically links CURL and zlib.  It dynamically links against SChannel as its TLS/Crypto provider since SChannel ships with windows.  SChannel exists as far back as Windows Vista, so that's likely the lowest supported version of Windows, though I have not verified this.

Only the x86_64 architecture is supported, there is no 32 bit library.

### Linux / Android

On Linux and Android, the C++ implementation statically links CURL, zlib, and MbedTLS.

For linux, only the x86_64 architecture is supported, there is no 32 bit library.

For Android, only the armv7a architecture is supported.  Support for additional architectures is planned, especially arm64.

### Mac / iOS

On Mac and iOS, the ObjC implementation is used, which relies on NSURLSession.  I've read that this is preferrable to CURL on Apple platforms, since NSURLSession is intelligent about how it manages power (waking the radio, etc).

On Mac, x86_64 and arm64 architectures are supported, built as a fat dylib.

On iOS, armv7, armv7s, and arm64 architectures are supported, built as a fat dylib.


## Third-party dependencies, and their licenses

- [CURL](https://curl.se/)
  - License: [Permissive](https://github.com/curl/curl/blob/master/COPYING)
- [mbedtls](https://www.trustedfirmware.org/projects/mbed-tls/)
  - License: [Apache 2.0](https://github.com/ARMmbed/mbedtls/blob/development/LICENSE)
- [utf8cpp](https://github.com/nemtrif/utfcpp)
  - License: [Boost 1.0](https://github.com/nemtrif/utfcpp/blob/master/LICENSE)
- [zlib](http://zlib.net/)
  - License: [Permissive](http://zlib.net/zlib_license.html)
