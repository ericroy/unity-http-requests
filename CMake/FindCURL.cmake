find_path(CURL_INCLUDE_DIRS curl/curl.h PATHS ${CMAKE_INSTALL_PREFIX}/include NO_DEFAULT_PATH)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_library(CURL_LIBRARY libcurl-d.a PATHS ${CMAKE_INSTALL_PREFIX}/lib NO_DEFAULT_PATH)
else()
    find_library(CURL_LIBRARY libcurl.a PATHS ${CMAKE_INSTALL_PREFIX}/lib NO_DEFAULT_PATH)
endif()

set(CURL_LIBRARIES "${CURL_LIBRARY}")
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CURL DEFAULT_MSG CURL_INCLUDE_DIRS CURL_LIBRARY)
mark_as_advanced(CURL_INCLUDE_DIRS CURL_LIBRARY)
