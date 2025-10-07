cmake_minimum_required(VERSION 3.16)

# ISPC: compile to one .obj + header
set(ISPC_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(ISPC_OUT_OBJ "${ISPC_OUT_DIR}/newton_ispc${CMAKE_C_OUTPUT_EXTENSION}")
set(ISPC_OUT_HDR "${ISPC_OUT_DIR}/newton_ispc.h")

set(ISPC_TARGET "avx2-i32x8")

add_custom_command(
  OUTPUT  ${ISPC_OUT_OBJ} ${ISPC_OUT_HDR}
  COMMAND ${ISPC_EXECUTABLE}
          ${CMAKE_CURRENT_SOURCE_DIR}/newton.ispc
          -O2
          --target=${ISPC_TARGET}
          -h ${ISPC_OUT_HDR}
          -o ${ISPC_OUT_OBJ}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/newton.ispc
  COMMENT "Compiling ISPC kernel (single target ${ISPC_TARGET})"
  VERBATIM
)

add_custom_target(newton_ispc_gen DEPENDS ${ISPC_OUT_OBJ} ${ISPC_OUT_HDR})

add_executable(newton
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${ISPC_OUT_OBJ}
)

target_include_directories(newton PRIVATE ${ISPC_OUT_DIR})
add_dependencies(newton newton_ispc_gen)

target_compile_definitions(newton PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

find_package(OpenGL)
if (OpenGL_FOUND AND TARGET OpenGL::GL)
  target_link_libraries(newton PRIVATE OpenGL::GL)
else()
  target_link_libraries(newton PRIVATE opengl32)
endif()

# ---- Local deps ----
target_link_libraries(newton PRIVATE
  imgui
  glad
  glfw3
)

install(TARGETS newton RUNTIME DESTINATION .)

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_FILE_NAME "NewtonFractal")
include(CPack)