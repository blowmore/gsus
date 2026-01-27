cmake_minimum_required(VERSION 3.13)
project(gsus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "Install path" FORCE)
endif()

find_library(SDBUS_LIB NAMES systemd libsystemd)
if(NOT SDBUS_LIB)
  message(FATAL_ERROR "libsystemd (sd-bus) not found. Install libsystemd-dev / systemd-devel.")
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(gsusd src/gsusd.cpp)
target_link_libraries(gsusd PRIVATE ${SDBUS_LIB})

add_executable(gsus src/gsus.cpp)
target_link_libraries(gsus PRIVATE ${SDBUS_LIB})

install(TARGETS gsusd gsus RUNTIME DESTINATION bin)

install(FILES packaging/org.gsus.service DESTINATION "share/dbus-1/services")
install(FILES packaging/gsus.service DESTINATION "share/systemd/user")

message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
