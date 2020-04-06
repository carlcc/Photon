project(photonclient)

file(GLOB SRC_FILES src/* src/imgui/* src/video/* src/audio/*)

set(REQUIRED_PACKAGES
        SDL2
        )

if (WIN32)
    file(GLOB PHOTON_PLATFORM_SRC_FILES src/video/impl/win32/*)
    set(PHOTON_PLATFORM_LIBS ws2_32 strmiids)
elseif (APPLE)
    file(GLOB PHOTON_PLATFORM_SRC_FILES src/video/impl/apple/*)
else ()
    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
        file(GLOB PHOTON_PLATFORM_SRC_FILES src/video/impl/linux/*)
        set(PHOTON_PLATFORM_LIBS dl pthread)
    else()
        message(FATAL_ERROR "Not supported OS: ${CMAKE_SYSTEM_NAME}")
    endif ()
endif ()

add_executable(photonclient ${SRC_FILES} ${PHOTON_PLATFORM_SRC_FILES})
PhotonAddLinkLibraryAndIncludePath(photonclient "${REQUIRED_PACKAGES}")
PhotonAddPredefinedMacros(photonclient)
add_dependencies(photonclient SSNet SSIO SSBase yuv glad portaudio libdshowcapture)
target_link_libraries(photonclient portaudio SSNet SSIO SSBase glad yuv libdshowcapture ${SDL2_LIB} ${UV_LIB} ${ZIP_LIB} ${Z_LIB} ${PHOTON_PLATFORM_LIBS})
target_include_directories(photonclient PRIVATE
        ${SSBASE_INCLUDE_DIR}
        ${GLAD_INCLUDE_DIR}
        ${YUV_INCLUDE_DIR}
		${PORTAUDIO_INCLUDE_DIR}
        ${LIBDSHOWCAPTURE_INCLUDE_DIR}
)