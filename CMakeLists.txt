cmake_minimum_required(VERSION 3.2)

project(Photon)

set(CMAKE_CXX_STANDARD 14)

include(common.cmake)

PhotonFindLibrary(PORTAUDIO_LIB "portaudio" True)
PhotonFindPath(PORTAUDIO_INCLUDE "portaudio.h" True)

PhotonFindLibrary(SDL2_LIB "SDL2" True)
PhotonFindPath(SDL2_INCLUDE "SDL2/SDL.h" True)

set(PHOTON_PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(GLAD_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/thirdparty/glad/include)
set(YUV_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/thirdparty/libyuv/include)
set(SSBASE_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/thirdparty/SSBase/src)

add_subdirectory(thirdparty)
add_subdirectory(source)

