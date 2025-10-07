cmake_minimum_required(VERSION 3.16)
project(NewtonFractal LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

foreach(cfg IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
  string(TOUPPER ${cfg} CFG)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CFG} "${PROJECT_SOURCE_DIR}/bin")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CFG} "${PROJECT_SOURCE_DIR}/bin")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CFG} "${PROJECT_SOURCE_DIR}/bin")
endforeach()

option(BUILD_WITH_VCPKG "Use vcpkg packages" ON)
set(ISPC_EXECUTABLE "C:/ispc/bin/ispc.exe" CACHE FILEPATH "Path to ispc compiler")

add_subdirectory(third_party)  


set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

set(GLFW_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

foreach(flag_var CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
                 CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
                 CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                 CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
  if(${flag_var} MATCHES "/MD")
    string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
  endif()
endforeach()

add_subdirectory(src)
