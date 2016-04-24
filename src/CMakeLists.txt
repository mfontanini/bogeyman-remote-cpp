set(SOURCES
    reverse_server_connector.cpp
    command_channel.cpp
    tunnel.cpp
    base64.cpp

    main.cpp
)

set(LIBRARIES
    boost_system pthread jsoncpp
)

add_executable(bogeyman-tunnel ${SOURCES})
target_link_libraries(bogeyman-tunnel ${LIBRARIES})