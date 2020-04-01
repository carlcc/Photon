cmake_minimum_required(VERSION 3.2)
project(imgui)

add_library(imgui STATIC
        cimgui.cpp
        cimgui.h
        CMakeLists.txt
        imconfig.h
        imgui.cpp
        imgui_demo.cpp
        imgui_draw.cpp
        imgui.h
        imgui_impl_opengl3.cpp
        imgui_impl_opengl3.h
        imgui_impl_sdl.cpp
        imgui_impl_sdl.h
        imgui_internal.h
        imgui_widgets.cpp
        imstb_rectpack.h
        imstb_textedit.h
        imstb_truetype.h
        )
target_include_directories(imgui PRIVATE ../glad/include)