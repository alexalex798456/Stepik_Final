cmake_minimum_required (VERSION 2.6)
project (final)


macro(use_cxx14)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")
    endif ()
  else ()
    set (CMAKE_CXX_STANDARD 14)
  endif ()
endmacro(use_cxx14)


use_cxx14()

find_package(Threads REQUIRED)

add_executable(final HTTP1_0.cpp HTTPServer.cpp main.cpp Utils.cpp)
if (TARGET Threads::Threads)
  target_link_libraries(final PRIVATE Threads::Threads)
endif()
target_link_libraries(final PRIVATE Threads::Threads)
