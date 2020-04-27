project(photonserver)

file(GLOB_RECURSE SRC_FILES src/*)

add_executable(photonserver ${SRC_FILES})

target_link_libraries(photonserver
        photonbase
        SSNet SSIO SSBase
        ${UV_LIB}
        ${ZIP_LIB}
        ${Z_LIB}
        )
target_include_directories(photonserver PRIVATE
        src
        ../photonbase/public
        ${SSBASE_INCLUDE_DIR}
        ${UV_INCLUDE}
        )