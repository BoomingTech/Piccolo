# cmake -S . -B build && cmake --build build && build/app

# cmake -S . -B build -G "Ninja" && cmake --build build && build/app
# cmake -S . -B build -G "CodeBlocks - Ninja" && cmake --build build && build/app
# cmake -S . -B build -G "Visual Studio 17 2022" && cmake --build build && build/app

cmake_minimum_required(VERSION 3.18)
project( app VERSION 0.1 )

file(GLOB SOURCE_FILES "*.c*" )
add_executable(app ${SOURCE_FILES})

find_package( OpenGL REQUIRED )
find_package( glfw3 REQUIRED )

set(ADDITIONAL_LIBRARIES "")
if(WIN32)
set(ADDITIONAL_LIBRARIES winmm)
endif()

set(GLEW_LIBRARY "")
if(UNIX)
set(GLEW_LIBRARY GLEW)
else()
find_package( glew REQUIRED )
set(GLEW_LIBRARY GLEW::glew)
endif()

target_link_libraries(${PROJECT_NAME} OpenGL::GL OpenGL::GLU glfw ${ADDITIONAL_LIBRARIES} ${GLEW_LIBRARY} )
