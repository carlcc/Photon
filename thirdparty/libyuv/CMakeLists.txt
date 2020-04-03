cmake_minimum_required(VERSION 3.2)

project(libyuv)

file(GLOB YUV_SRC_FILES source/* include/*)

add_library(yuv STATIC ${YUV_SRC_FILES})
target_include_directories(yuv PRIVATE include)