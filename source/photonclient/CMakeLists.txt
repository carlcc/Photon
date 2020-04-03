project(photonclient)

file(GLOB SRC_FILES src/* src/imgui/* video/*)

set(REQUIRED_PACKAGES
        AVFORMAT AVCODEC AVDEVICE AVUTIL SWSCALE SDL2
        )

if (WIN32)
    file(GLOB PHOTON_PLATFORM_SRC_FILES src/video/impl/win/*)
elseif (APPLE)
    file(GLOB PHOTON_PLATFORM_SRC_FILES src/video/impl/apple/*)
else ()
    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
        file(GLOB PHOTON_PLATFORM_SRC_FILES src/video/impl/linux/*)
    else()
        message(FATAL_ERROR "Not supported OS: ${CMAKE_SYSTEM_NAME}")
    endif ()
endif ()

add_executable(photonclient ${SRC_FILES} ${PHOTON_PLATFORM_SRC_FILES})
PhotonAddLinkLibraryAndIncludePath(photonclient "${REQUIRED_PACKAGES}")
PhotonAddPredefinedMacros(photonclient)
add_dependencies(photonclient SSNet SSIO SSBase yuv glad)
target_link_libraries(photonclient SSNet SSIO SSBase glad yuv ${UV_LIB} ${ZIP_LIB} ${Z_LIB} dl pthread)
target_include_directories(photonclient PRIVATE
        ${SSBASE_INCLUDE_DIR}
        ${GLAD_INCLUDE_DIR}
        ${YUV_INCLUDE_DIR}
)