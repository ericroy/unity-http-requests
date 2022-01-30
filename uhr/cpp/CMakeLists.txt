cmake_minimum_required(VERSION 3.20)
project(uhr)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN true)
if(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	string(APPEND CMAKE_CXX_FLAGS " /Zc:__cplusplus")
else()
	string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--exclude-libs,ALL")
endif()

# All of these packages should be found in our install prefix.
find_package(UTF8CPP REQUIRED)
find_package(CURL REQUIRED)

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
	${UTF8CPP_INCLUDE_DIRS}
	${CURL_INCLUDE_DIRS}
)
target_link_libraries(uhr PRIVATE ${CURL_LIBRARY})
target_compile_definitions(uhr PRIVATE -DUHR_BUILDING_SHARED)
if(WIN32)
	target_compile_definitions(uhr PRIVATE -DNOMINMAX)
endif()
if(MSVC)
  target_compile_options(uhr PRIVATE /W4 /WX)
else()
  target_compile_options(uhr PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

install(TARGETS uhr RUNTIME)
