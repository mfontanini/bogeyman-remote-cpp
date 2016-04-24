set(SOURCES
    reverse_server_connector.cpp
    command_channel.cpp
    tunnel.cpp
    base64.cpp

    main.cpp
)

set(LIBRARIES
    boost_system boost_log pthread jsoncpp
)

add_definitions("-DBOOST_LOG_DYN_LINK=1")
add_executable(bogeyman-tunnel ${SOURCES})
target_link_libraries(bogeyman-tunnel ${LIBRARIES})