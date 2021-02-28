cmake_minimum_required(VERSION 3.18)
project(uhr)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN true)
list(APPEND CMAKE_SHARED_LINKER_FLAGS "-Wl,--exclude-libs,ALL")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

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
target_link_libraries(uhr PRIVATE CURL::libcurl)
target_compile_definitions(uhr PRIVATE -DUHR_BUILDING_SHARED)
if(WIN32)
	target_compile_definitions(uhr PRIVATE -DNOMINMAX)
endif()

install(TARGETS uhr RUNTIME)
