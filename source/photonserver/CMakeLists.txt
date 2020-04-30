project(photonserver)

file(GLOB_RECURSE SRC_FILES src/*)

add_executable(photonserver ${SRC_FILES})

if (WIN32)
    set(PHOTONSERVER_PLATFORM_LIBS ws2_32 Iphlpapi psapi userenv)
else()
    set(PHOTONSERVER_PLATFORM_LIBS pthread)
endif()
target_link_libraries(photonserver
        photonbase
        SSNet SSIO SSBase
        ${UV_LIB}
        ${ZIP_LIB}
        ${Z_LIB}
        ${PHOTONSERVER_PLATFORM_LIBS}
        )
target_include_directories(photonserver PRIVATE
        src
        ../photonbase/public
        ${SSBASE_INCLUDE_DIR}
        ${UV_INCLUDE}
        )