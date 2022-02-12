find_path(CURL_INCLUDE_DIRS curl/curl.h PATHS ${CMAKE_INSTALL_PREFIX}/include NO_DEFAULT_PATH)
find_library(CURL_LIBRARY_RELEASE libcurl.a PATHS ${CMAKE_INSTALL_PREFIX}/lib NO_DEFAULT_PATH)
find_library(CURL_LIBRARY_DEBUG libcurl-d.a PATHS ${CMAKE_INSTALL_PREFIX}/lib NO_DEFAULT_PATH)

set(CURL_LIBRARIES "${CURL_LIBRARY}")
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MBEDTLS DEFAULT_MSG CURL_INCLUDE_DIRS CURL_LIBRARY_RELEASE CURL_LIBRARY_DEBUG)
mark_as_advanced(CURL_INCLUDE_DIRS CURL_LIBRARY_RELEASE CURL_LIBRARY_DEBUG)