project(nasdc)
cmake_minimum_required(VERSION 2.6)
# ######### General setup ##########
include_directories(${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/common ${PROJECT_SOURCE_DIR}/gnu ${PROJECT_SOURCE_DIR}/.. )
aux_source_directory(${PROJECT_SOURCE_DIR} nasdc_SRCS)

if (XMLRPC_DAEMON)
    if (NOT NO_UI_DAEMON)
        set (NO_UI_DAEMON 1 CACHE BOOL "Daemon without anyone ui" FORCE)
    endif (NOT NO_UI_DAEMON)
    find_package (XMLRPC REQUIRED c++2 abyss-server)
    include_directories(${XMLRPC_INCLUDE_DIRS})
    set (XMLRPCLIBS ${XMLRPC_LIBRARIES})
    add_definitions (-DXMLRPC_DAEMON )
endif (XMLRPC_DAEMON)

if (WITH_DHT)
  add_definitions ( -DWITH_DHT )
endif (WITH_DHT)

if (HAVE_DAEMON)
    add_definitions(-DHAVE_DAEMON)
endif()

if (WIN32)
  list (APPEND nasdc_SRCS ../win32/applink.c)
endif (WIN32)

if (LUA_SCRIPT)
  include_directories (${LUA_INCLUDE_DIR})
endif (LUA_SCRIPT)

if (ENABLE_STACKTRACE)
    set_property(SOURCE ${PROJECT_SOURCE_DIR}/src/nasdc.cpp PROPERTY COMPILE_DEFINITIONS ENABLE_STACKTRACE)
endif (ENABLE_STACKTRACE)

# actual target:
add_executable (nasdc MACOSX_BUNDLE WIN32
                ${nasdc_SRCS}
                )

target_link_libraries (nasdc dcpp ${XMLRPCLIBS})

if (UNIX)
 set (BINDIR bin)
 if (APPLE)
    set (BUNDLEDIR .)
 endif ()
elseif (WIN32)
  set (BINDIR .)
endif (UNIX)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-D_DEBUG)
endif()

set_property (TARGET nasdc PROPERTY OUTPUT_NAME "${PROJECT_NAME_GLOBAL}-daemon")

install (TARGETS nasdc
    RUNTIME DESTINATION ${BINDIR}
    BUNDLE DESTINATION ${BUNDLEDIR})

if (UNIX)
  execute_process (
                OUTPUT_FILE ${PROJECT_BINARY_DIR}/eiskaltdcpp-daemon.1.gz
                COMMAND "${GZIP_CMD}" -9
                INPUT_FILE eiskaltdcpp-daemon.1
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
endif (UNIX)

if (APPLE)
  install (FILES ${PROJECT_BINARY_DIR}/eiskaltdcpp-daemon.1.gz DESTINATION ${PROJECT_NAME_GLOBAL}.app/man/man1)
elseif (UNIX AND NOT APPLE)
  install (FILES ${PROJECT_BINARY_DIR}/eiskaltdcpp-daemon.1.gz DESTINATION ${SHARE_DIR}/man/man1)
endif (APPLE)
