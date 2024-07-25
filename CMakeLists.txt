cmake_minimum_required(VERSION 3.29)
set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "build type")
project(NetProject
		LANGUAGES C CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR}/src
                    ${PROJECT_SOURCE_DIR}/lib)
link_directories(${PROJECT_SOURCE_DIR}/lib/spdlog 
                 ${PROJECT_SOURCE_DIR}/lib/libevent)

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -fPIC -pipe -std=gnu90")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -pipe -std=c++11")

# —∞’“libeventø‚
find_package(Libevent REQUIRED)
aux_source_directory(${PROJECT_SOURCE_DIR}/src CPPLIST)
add_executable(${PROJECT_NAME} ${CPPLIST})

target_link_libraries(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/lib/spdlog/libspdlog.a 
                                        ${PROJECT_SOURCE_DIR}/lib/libevent/libevent.a
                                        ${PROJECT_SOURCE_DIR}/lib/libevent/libevent_pthreads.a
                                        ${PROJECT_SOURCE_DIR}/lib/libevent/libevent_core.a
                                        ${PROJECT_SOURCE_DIR}/lib/libevent/libevent_extra.a
                                         pthread) 
                                        