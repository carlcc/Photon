cmake_minimum_required(VERSION 3.2)

project(Photon)

set(CMAKE_CXX_STANDARD 14)

include(common.cmake)

PhotonFindLibrary(AVCODEC_LIB "avcodec" True)
PhotonFindPath(AVCODEC_INCLUDE "libavcodec/avcodec.h" True)
PhotonFindLibrary(AVFORMAT_LIB "avformat" True)
PhotonFindPath(AVFORMAT_INCLUDE "libavformat/avformat.h" True)
PhotonFindLibrary(AVDEVICE_LIB "avdevice" True)
PhotonFindPath(AVDEVICE_INCLUDE "libavdevice/avdevice.h" True)
PhotonFindLibrary(AVUTIL_LIB "avutil" True)
PhotonFindPath(AVUTIL_INCLUDE "libavutil/avutil.h" True)
PhotonFindLibrary(SWSCALE_LIB "swscale" True)
PhotonFindPath(SWSCALE_INCLUDE "libswscale/swscale.h" True)

PhotonFindLibrary(SDL2_LIB "SDL2" True)
PhotonFindPath(SDL2_INCLUDE "SDL2/SDL.h" True)

set(PHOTON_PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(GLAD_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/thirdparty/glad/include)
set(IMGUI_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/thirdparty/imgui)
set(SSBASE_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/thirdparty/SSBase/src)

add_subdirectory(thirdparty)
add_subdirectory(source)

