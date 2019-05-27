add_library(Nio nio/util.h nio/util.cpp nio/Poll.cpp nio/Poll.h nio/TCPServer.cpp nio/TCPServer.h nio/Subscriber.cpp nio/Subscriber.h nio/KillReceiver.cpp nio/KillReceiver.h nio/TCPSessionFactory.cpp nio/TCPSessionFactory.h)
add_executable(tcpserver tcpserver.cpp FTSession.h FTSession.cpp FTSessionFactory.cpp FTSessionFactory.h)
target_link_libraries(tcpserver Nio)

SET(CMAKE_CXX_FLAGS "-D_GLIBCXX_USE_CXX11_ABI=0")
