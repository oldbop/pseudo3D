cmake_minimum_required(VERSION 3.22)
project(pseudo3D)

set(CMAKE_DEBUG_POSTFIX d)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

if(SYS_GL_HEADERS)
  find_package(OpenGL REQUIRED)
else()
  add_subdirectory(deps/glad/)
endif()

add_subdirectory(deps/glfw/)
add_subdirectory(deps/stb/)

file(GLOB SOURCES CONFIGURE_DEPENDS
  ${PROJECT_SOURCE_DIR}/src/*.h
  ${PROJECT_SOURCE_DIR}/src/*.c
)

add_executable("${CMAKE_PROJECT_NAME}" ${SOURCES}) 

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES 
  DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
)

target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC
  $<$<COMPILE_LANG_AND_ID:C,Clang>:-std=c99;-Wall>
  $<$<COMPILE_LANG_AND_ID:CXX,Clang>:-std=c++11;-Wall>
)

if(SYS_GL_HEADERS)
  target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC SYS_GL_HEADERS)
  target_link_libraries(${CMAKE_PROJECT_NAME} OpenGL::GL)
else()
  target_link_libraries(${CMAKE_PROJECT_NAME} glad)
endif()

target_link_libraries(${CMAKE_PROJECT_NAME} glfw)
target_link_libraries(${CMAKE_PROJECT_NAME} stb)
