project(photonclient)

file(GLOB SRC_FILES src/* src/imgui/* src/video/* src/audio/*)

set(REQUIRED_PACKAGES
        SDL2
        )

if (WIN32)
    file(GLOB PHOTON_PLATFORM_SRC_FILES src/video/impl/win32/*)
    set(PHOTON_PLATFORM_LIBS ws2_32 strmiids)
elseif (APPLE)
    file(GLOB PHOTON_PLATFORM_SRC_FILES src/video/impl/mac/*)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(AVFOUNDATION_LIBRARY AvFoundation)
    find_library(COREMEDIA_LIBRARY CoreMedia)
    find_library(COREVIDEO_LIBRARY CoreVideo)
    find_library(FOUNDATION_LIBRARY Foundation)
    set(PHOTON_PLATFORM_LIBS "${COREMEDIA_LIBRARY}" "${COREFOUNDATION_LIBRARY}" "${AVFOUNDATION_LIBRARY}" "${COREVIDEO_LIBRARY}" "${FOUNDATION_LIBRARY}")
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

if (APPLE)
    # build macox bundle
    set_target_properties(photonclient PROPERTIES
        MACOSX_BUNDLE true
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/extra/mac/Info.plist.in
    )
endif()

if (WIN32)
    set(LIBDSHOWCAPTURE_TARGET libdshowcapture)
else()
    set(LIBDSHOWCAPTURE_TARGET "")
endif ()

add_dependencies(photonclient SSNet SSIO SSBase yuv glad portaudio ${LIBDSHOWCAPTURE_TARGET})
target_link_libraries(photonclient
    portaudio SSNet SSIO SSBase glad yuv
    ${LIBDSHOWCAPTURE_TARGET}
    ${SDL2_LIB} 
    ${UV_LIB}
    ${ZIP_LIB}
    ${Z_LIB} 
    ${PHOTON_PLATFORM_LIBS}
)
target_include_directories(photonclient PRIVATE
        ${SSBASE_INCLUDE_DIR}
        ${GLAD_INCLUDE_DIR}
        ${YUV_INCLUDE_DIR}
		${PORTAUDIO_INCLUDE_DIR}
        ${LIBDSHOWCAPTURE_INCLUDE_DIR}
)