add_library(wac_network SHARED
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
)

target_compile_definitions(wac_network PRIVATE
    WAC_NETWORK_LIBRARY
    USE_SSL
)

target_compile_options(wac_network PRIVATE
    -Wno-template-body
)

target_include_directories(wac_network PRIVATE
    ${CMAKE_SOURCE_DIR}/Src/Wasabi       # bfc/platform/..., api/syscb/...
    ${CMAKE_SOURCE_DIR}/Src/replicant    # foundation/error.h (util.cpp direct include)
    ${CMAKE_SOURCE_DIR}/Src/Components  # resolves ../Agave/... and ../nu/... via path arithmetic
)

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
