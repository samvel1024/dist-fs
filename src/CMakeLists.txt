set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})


add_library(Nio
        nio/util.h nio/util.cpp
        nio/Poll.cpp nio/Poll.h
        nio/TCPServer.cpp nio/TCPServer.h
        nio/Subscriber.cpp nio/Subscriber.h
        nio/KillReceiver.cpp nio/KillReceiver.h
        nio/TCPSessionFactory.h)

add_library(Dto Dto.h Dto.cpp)

#add_executable(tcpserver tcpserver.cpp FileDownloadSession.h FileDownloadSession.cpp UDPServer.cpp UDPServer.h)
add_executable(mserver mserver.cpp UDPServer.cpp UDPServer.h)
add_executable(packetgen packetgen.cpp)
#add_executable(udpclient err.cpp err.h client.cpp)

#target_link_libraries(tcpserver Nio)
target_link_libraries(mserver Boost::program_options)
target_link_libraries(mserver Nio)
target_link_libraries(packetgen Dto)


#TODO add compiler flags
