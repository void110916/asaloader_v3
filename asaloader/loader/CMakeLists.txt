# project(asaloaderlib VERSION 3.0.0 LANGUAGES CXX)

add_library(asaloaderlib STATIC
  ihex.cpp
  ihex.h  
  loader.cpp
  loader.h
  protcol.h
  protocol.cpp
  asadevice.h
  prog_arg.h
  endianness.h
)
target_link_libraries(asaloaderlib PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::SerialPort)
target_include_directories(asaloaderlib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
# if(WIN32)
#   target_link_libraries(asaloaderlib PRIVATE wsock32 ws2_32)
# endif()
