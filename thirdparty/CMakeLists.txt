add_subdirectory(SSBase)
add_subdirectory(glad)
add_subdirectory(libyuv)
add_subdirectory(portaudio)
if (WIN32)
    add_subdirectory(libdshowcapture)
endif()