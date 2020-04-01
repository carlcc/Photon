project(photonclient)

file(GLOB_RECURSE SRC_FILES src/*)

set(REQUIRED_PACKAGES
        AVFORMAT AVCODEC AVDEVICE AVUTIL SWSCALE SDL2
        )

add_executable(photonclient ${SRC_FILES})
PhotonAddLinkLibraryAndIncludePath(photonclient "${REQUIRED_PACKAGES}")
add_dependencies(photonclient SSNet SSIO SSBase)
target_link_libraries(photonclient SSNet SSIO SSBase imgui glad ${UV_LIB} ${ZIP_LIB} ${Z_LIB} dl pthread)
target_include_directories(photonclient PRIVATE ${SSBASE_INCLUDE_DIR} ${GLAD_INCLUDE_DIR} ${IMGUI_INCLUDE_DIR})