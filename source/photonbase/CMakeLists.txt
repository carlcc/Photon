project(photonbase)

file(GLOB_RECURSE SRC_FILES src/* public/*)

add_library(photonbase STATIC ${SRC_FILES})

add_dependencies(photonbase SSNet SSIO SSBase)
target_link_libraries(photonbase
    SSNet SSIO SSBase
    ${UV_LIB}
    ${ZIP_LIB}
    ${Z_LIB} 
)
target_include_directories(photonbase PRIVATE
        public
        ${SSBASE_INCLUDE_DIR}
)

option(PHOTONBASE_ENABLE_TESTS "Build photonebase tests" ON)
if (PHOTONBASE_ENABLE_TESTS)
    # tests
    file(GLOB_RECURSE TEST_SRC_FILES test/*)
    add_executable(photonbase_test ${TEST_SRC_FILES})
    add_dependencies(photonbase_test photonbase)
    target_link_libraries(photonbase_test
        photonbase
        SSNet SSIO SSBase
        ${UV_LIB}
        ${ZIP_LIB}
        ${Z_LIB} 
    )
    target_include_directories(photonbase_test PRIVATE
            public
            src
            ${SSBASE_INCLUDE_DIR}
    )
endif()