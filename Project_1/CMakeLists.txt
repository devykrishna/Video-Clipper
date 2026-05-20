cmake_minimum_required(VERSION 3.5.0)
project(video_system VERSION 0.1.0)

find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

add_executable(video_processor video_processor.cpp common/sharedFrame.cpp)
add_executable(video_clipper video_clipper.cpp common/sharedFrame.cpp)
add_executable(event_notifier event_notifier.cpp )
add_executable(CPU_watcher CPU_watcher.cpp )


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX__STANDARD_REQUIRED ON)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    common
)
target_link_libraries(video_processor ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(video_clipper ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT})

