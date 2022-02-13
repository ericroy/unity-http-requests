find_path(CURL_INCLUDE_DIRS curl/curl.h)

set(LIB_NAME "libcurl${CMAKE_STATIC_LIBRARY_SUFFIX}")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(LIB_NAME "libcurl-d${CMAKE_STATIC_LIBRARY_SUFFIX}")
endif()

find_library(CURL_LIBRARY ${LIB_NAME})

set(CURL_LIBRARIES "${CURL_LIBRARY}")
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CURL DEFAULT_MSG CURL_INCLUDE_DIRS CURL_LIBRARY)
mark_as_advanced(CURL_INCLUDE_DIRS CURL_LIBRARY)
