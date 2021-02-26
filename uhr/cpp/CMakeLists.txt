cmake_minimum_required(VERSION 3.18)
project(uhr)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Download curl ca bundle (configure-time)
# set(ca_bundle_url https://curl.se/ca/cacert.pem)
# set(ca_bundle_hash 82fc9ac5a1fd4f55fa3d34d8da6b430d3ab1e7fc)
# if(NOT EXISTS cacert.pem)
# 	file(DOWNLOAD ${ca_bundle_url} ${CMAKE_INSTALL_PREFIX}/bin/cacert.pem
# 		TIMEOUT 60
# 		EXPECTED_HASH SHA1=${ca_bundle_hash}
# 		TLS_VERIFY ON)
# endif()

find_package(CURL REQUIRED CONFIG)

add_library(uhr SHARED
	src/ca_certs.cc src/ca_certs.h
	src/session.cc src/session.h
	src/init.cc src/init.h
	src/logging.cc src/logging.h
	src/main.cc
	src/request.cc src/request.h
	src/request_builder.cc src/request_builder.h
	src/util.cc src/util.h
)
target_include_directories(uhr PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../include
	${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(uhr CURL::libcurl)
target_compile_definitions(uhr PRIVATE -DUHR_BUILDING_SHARED)
if(WIN32)
	target_compile_definitions(uhr PRIVATE -DNOMINMAX)
endif()

install(TARGETS uhr RUNTIME)
