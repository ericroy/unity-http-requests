set(curl_version 7.75.0)
set(curl_url https://curl.se/download/curl-${curl_version}.tar.gz)
set(curl_hash fbd1e354a5e4e9a4ac07db3d1222d19f84a5e751)
set(curl_basename curl-${curl_version})
set(curl_archive_name ${curl_basename}.tar.gz)

set(ca_bundle_url https://curl.se/ca/cacert.pem)
set(ca_bundle_hash 82fc9ac5a1fd4f55fa3d34d8da6b430d3ab1e7fc)

set(mbedtls_version 2.25.0)
set(mbedtls_url https://github.com/ARMmbed/mbedtls/archive/v${mbedtls_version}.tar.gz)
set(mbedtls_hash 746ab10dec0aaadba746c2ecb3df24807329dce1)
set(mbedtls_basename mbedtls-${mbedtls_version})
set(mbedtls_archive_name ${mbedtls_basename}.tar.gz)

# Download curl ca bundle (configure-time)
if(NOT EXISTS cacert.pem)
	file(DOWNLOAD ${ca_bundle_url} ${CMAKE_CURRENT_SOURCE_DIR}/cacert.pem
		TIMEOUT 60
		EXPECTED_HASH SHA1=${ca_bundle_hash}
		TLS_VERIFY ON)
	set(CA_BUNDLE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cacert.pem PARENT_SCOPE)
endif()

# Download curl (configure-time)
if(NOT EXISTS ${curl_archive_name})
	file(DOWNLOAD ${curl_url} ${CMAKE_CURRENT_SOURCE_DIR}/${curl_archive_name}
		TIMEOUT 60
		EXPECTED_HASH SHA1=${curl_hash}
		TLS_VERIFY ON)
endif()

# Extract curl
if(NOT EXISTS ${curl_basename})
	file(ARCHIVE_EXTRACT
		INPUT ${CMAKE_CURRENT_SOURCE_DIR}/${curl_archive_name}
		DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
	)
endif()

if(NOT WIN32)
	# Download mbed (configure-time)
	if(NOT EXISTS ${mbedtls_archive_name})
		file(DOWNLOAD ${mbedtls_url} ${CMAKE_CURRENT_SOURCE_DIR}/${mbedtls_archive_name}
			TIMEOUT 60
			EXPECTED_HASH SHA1=${mbedtls_hash}
			TLS_VERIFY ON)
	endif()

	# Extract mbedtls
	if(NOT EXISTS ${mbedtls_basename})
		file(ARCHIVE_EXTRACT
			INPUT ${CMAKE_CURRENT_SOURCE_DIR}/${mbedtls_archive_name}
			DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}
		)
	endif()

	# mbedtls
	set(ENABLE_TESTING false CACHE BOOL "" FORCE)
	set(ENABLE_PROGRAMS false CACHE BOOL "" FORCE)
	add_subdirectory(${mbedtls_basename})
endif()

# curl
set(BUILD_SHARED_LIBS false CACHE BOOL "" FORCE)
set(BUILD_CURL_EXE false CACHE BOOL "" FORCE)
set(BUILD_TESTING false CACHE BOOL "" FORCE)
set(HTTP_ONLY true CACHE BOOL "" FORCE)
set(CMAKE_USE_LIBSSH2 false CACHE BOOL "" FORCE)
set(CMAKE_USE_OPENSSL false CACHE BOOL "" FORCE)
set(CMAKE_USE_MBEDTLS false CACHE BOOL "" FORCE)
set(CMAKE_USE_SCHANNEL true CACHE BOOL "" FORCE)
if(NOT WIN32)
	set(CMAKE_USE_MBEDTLS true CACHE BOOL "" FORCE)
	set(CMAKE_USE_SCHANNEL false CACHE BOOL "" FORCE)

	# This allows curl's find_package() call to locate mbedtls
	set(CMAKE_PREFIX_PATH 
		${CMAKE_CURRENT_BINARY_DIR}/${mbedtls_basename}/include
		${CMAKE_CURRENT_BINARY_DIR}/${mbedtls_basename}/library
	)
endif()

add_subdirectory(${curl_basename})

if(NOT WIN32)
	add_dependencies(libcurl mbedtls mbedx509 mbedcrypto)
endif()
