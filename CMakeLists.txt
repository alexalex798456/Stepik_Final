cmake_minimum_required (VERSION 2.6)
project (final)


find_package(Threads REQUIRED)

add_executable(final HTTP1_0.cpp HTTPServer.cpp main.cpp Utils.cpp)
if (TARGET Threads::Threads)
  target_link_libraries(final PRIVATE Threads::Threads)
endif()
target_link_libraries(final PRIVATE Threads::Threads)



