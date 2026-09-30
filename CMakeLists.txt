cmake_minimum_required(VERSION 3.16)

project(donstu_cpp_thread LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(donstu_cpp_thread main.cpp
    threadfuncs.cpp
    threadfuncs.h)

include(GNUInstallDirs)
install(TARGETS donstu_cpp_thread
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
