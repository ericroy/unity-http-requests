add_subdirectory(deps EXCLUDE_FROM_ALL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

add_executable(uhr
    ca_certs.cc ca_certs.h
    session.cc session.h
    init.cc init.h
    log_sink.cc log_sink.h
    main.cc
    request.cc request.h
    request_builder.cc request_builder.h
    util.cc util.h
)
target_include_directories(uhr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_link_libraries(uhr clue libcurl)
if(WIN32)
    target_compile_definitions(uhr PRIVATE -DNOMINMAX)
endif()

install(TARGETS uhr RUNTIME)
install(FILES ${CA_BUNDLE_FILE} TYPE BIN)
