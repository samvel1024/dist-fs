set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")


set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS program_options filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})


add_library(Err
        nio/Error.h nio/Error.cpp
        )

add_library(Nio
        nio/SendBuffer.cpp nio/SendBuffer.h
        nio/Alarm.cpp nio/Alarm.h
        nio/Poll.cpp nio/Poll.h
        nio/TCPServer.cpp nio/TCPServer.h
        nio/Subscriber.cpp nio/Subscriber.h
        nio/KillReceiver.cpp nio/KillReceiver.h)
target_link_libraries(Nio Err)

add_library(Dto Dto.h Dto.cpp)
target_link_libraries(Dto Err)

add_library(Dir SharedDirectory.h SharedDirectory.cpp)
target_link_libraries(Dir Err)

add_executable(server
        server.cpp
        UDPServer.cpp UDPServer.h
        FileSendSession.cpp FileSendSession.h
        FileReceiveSession.cpp FileReceiveSession.h)
target_link_libraries(server Nio Dir ${Boost_LIBRARIES})

add_executable(client
        client.cpp
        CLIListener.cpp CLIListener.h
        MultiQuery.cpp MultiQuery.h
        FileReceiveSession.cpp FileReceiveSession.h)
target_link_libraries(client Nio ${Boost_LIBRARIES})

add_executable(tcpserver tcpserver.cpp FileSendSession.cpp FileSendSession.h FileReceiveSession.cpp FileReceiveSession.h)
target_link_libraries(tcpserver Nio ${Boost_LIBRARIES})

add_executable(packetgen packetgen.cpp)
target_link_libraries(packetgen Dto)


#TODO add compiler flags
