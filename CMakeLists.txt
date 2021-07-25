if (NOT TARGET asio)
    message(FATAL_ERROR "requires ASIO ( https://github.com/chriskohlhoff/asio/ ) - otherwise you cannot continue!")
endif()

add_library(TwitchIRCSDK STATIC
    include/clientIRC.h
    include/twitchirc.h
    include/message.h
    include/user.h
    include/commandManager.h
    include/event.h
    include/eventManager.h

    src/clientIRC.cpp
    src/twitchirc.cpp
    src/message.cpp
    src/user.cpp
    src/commandManager.cpp
    src/event.cpp
    src/eventManager.cpp
)

target_include_directories(TwitchIRCSDK PUBLIC include)

target_link_libraries(TwitchIRCSDK PRIVATE asio)