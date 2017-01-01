cmake_minimum_required(VERSION 3.6)
project(SimpleWebServer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c")

set(SERVER_FILES
        src/cse.h
        src/cse.c
        src/queue.c
        src/queue.h
        src/request.h
        src/request.c
        src/server.c
        src/server.h
        )
set(CLIENT_FILES
        src/cse.h
        src/cse.c
        src/client.h
        src/client.c
#        src/pthread_barrier.c
#        src/pthread_barrier.h
        )
add_executable(SERVER ${SERVER_FILES})
add_executable(CLIENT ${CLIENT_FILES})