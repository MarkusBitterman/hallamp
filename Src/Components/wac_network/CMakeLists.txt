set(WAC_NETWORK_SOURCES
    Headers.cpp
    main.cpp
    util.cpp
    wac_network_connection.cpp
    wac_network_connection_api.cpp
    wac_network_connection_factory.cpp
    wac_network_dns.cpp
    wac_network_dns_api.cpp
    wac_network_dns_factory.cpp
    wac_network_http_receiver.cpp
    wac_network_http_receiver_api.cpp
    wac_network_http_receiver_factory.cpp
    wac_network_http_server.cpp
    wac_network_http_server_api.cpp
    wac_network_http_server_factory.cpp
    wac_network_onconncb_api.cpp
    wac_network_page_generator_api.cpp
    wac_network_ssl_connection.cpp
    wac_network_ssl_connection_factory.cpp
    wac_network_web_server.cpp
    wac_network_web_server_api.cpp
    wac_network_web_server_factory.cpp
    wac_network_web_server_listen.cpp

    # nu dependency pulled in via ../nu/RingBuffer.h (wac_network_connection).
    # A SHARED lib silently tolerated this as an undefined symbol; an executable
    # (the smoke test) does not, so compile it into the shared object set.
    ${CMAKE_SOURCE_DIR}/Src/nu/RingBuffer.cpp
)

set(WAC_NETWORK_INCLUDES
    ${CMAKE_SOURCE_DIR}/Src/Wasabi       # bfc/platform/..., api/syscb/...
    ${CMAKE_SOURCE_DIR}/Src/replicant    # foundation/error.h (util.cpp direct include)
    ${CMAKE_SOURCE_DIR}/Src/Components   # resolves ../Agave/... and ../nu/... via path arithmetic
)

# Compile the sources once into an object library. The same objects feed both
# the shared library (hidden visibility -> clean dynamic export surface) and the
# Milestone 2 smoke-test executable (static link ignores ELF visibility, so the
# internal JNL classes resolve there without being exported from the .so).
add_library(wac_network_obj OBJECT ${WAC_NETWORK_SOURCES})

target_compile_definitions(wac_network_obj PRIVATE
    WAC_NETWORK_LIBRARY
    USE_SSL
)

target_compile_options(wac_network_obj PRIVATE
    -Wno-template-body
)

target_include_directories(wac_network_obj PRIVATE ${WAC_NETWORK_INCLUDES})

# Object library gets the deps for their compile usage requirements (headers);
# PIC because these objects land in a shared library.
target_link_libraries(wac_network_obj PRIVATE
    Qt6::Core
    Qt6::Network
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
)

set_target_properties(wac_network_obj PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
)

# --- Shared library (Milestone 1 artifact) ---
add_library(wac_network SHARED $<TARGET_OBJECTS:wac_network_obj>)

target_link_libraries(wac_network PRIVATE
    Qt6::Core
    Qt6::Network
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
)

set_target_properties(wac_network PROPERTIES
    OUTPUT_NAME wac_network
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# --- Milestone 2: live network smoke test ---
add_executable(wac_network_smoketest
    test/smoke_test.cpp
    $<TARGET_OBJECTS:wac_network_obj>
)

target_include_directories(wac_network_smoketest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${WAC_NETWORK_INCLUDES}
)

target_compile_definitions(wac_network_smoketest PRIVATE USE_SSL)

target_link_libraries(wac_network_smoketest PRIVATE
    Qt6::Core
    Qt6::Network
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
)

add_test(NAME wac_network_smoke COMMAND wac_network_smoketest)
