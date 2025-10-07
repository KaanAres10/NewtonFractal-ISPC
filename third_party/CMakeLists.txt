# third_party/CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# --- GLFW ---
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)
add_library(glfw3 ALIAS glfw)

add_library(glad STATIC
  glad/src/glad.c
)
target_include_directories(glad PUBLIC glad/include)

add_library(imgui STATIC
  imgui/imgui.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_widgets.cpp
  imgui/backends/imgui_impl_glfw.cpp
  imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
  imgui
  imgui/backends
)
target_link_libraries(imgui PUBLIC glfw3)  # backend needs GLFW

find_package(OpenGL REQUIRED)
