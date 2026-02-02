cmake_minimum_required(VERSION 3.13)
project(gsus LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_library(SDBUS_LIB NAMES systemd libsystemd)
if(NOT SDBUS_LIB)
  message(FATAL_ERROR "libsystemd (sd-bus) not found. Install libsystemd-dev / systemd-devel.")
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(gsusd src/daemon/gsusd.cpp)
target_link_libraries(gsusd PRIVATE ${SDBUS_LIB})

add_executable(gsus src/cli/gsus.cpp)
target_link_libraries(gsus PRIVATE ${SDBUS_LIB})

install(TARGETS gsusd gsus RUNTIME DESTINATION bin)


# DBus policy must be in /etc/dbus-1/system.d for the running dbus to read it
install(FILES packaging/org.gsus.conf
        DESTINATION /etc/dbus-1/system.d)

# DBus activation files belong in /usr/share/dbus-1/system-services
install(FILES packaging/org.gsus.service
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dbus-1/system-services) # datarootdir expands to /usr/share when prefix=/usr

# systemd unit: packaged units usually live in /lib/systemd/system or ${CMAKE_INSTALL_LIBDIR}/systemd/system
# For distro packaging prefer ${CMAKE_INSTALL_LIBDIR}/systemd/system (e.g. /usr/lib/systemd/system)
install(FILES packaging/gsus.service
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system)


message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
