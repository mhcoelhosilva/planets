cmake_minimum_required(VERSION 2.8.4)
project(sample)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -static-libgcc -static-libstdc++")

set(CMAKE_MODULE_PATH /usr/local/lib/cmake /usr/local/lib/x86_64-linux-gnu/cmake)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

#find_package (PkgConfig REQUIRED)
find_package (OpenGL REQUIRED)
find_package (glfw3 3.3 REQUIRED)
find_package (glm REQUIRED)

#set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/../libs/glew-2.1.0-win32/glew-2.1.0")
#set(CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/../libs/glew-2.1.0-win32/glew-2.1.0/bin/Release/Win32")
#find_package (glew REQUIRED STATIC)

set(SOURCE_FILES main.cpp)
add_executable(planets ${SOURCE_FILES})

include_directories(${OPENGL_INCLUDE_DIR} ${GLEW_DIR} ${GLFW_INCLUDE_DIR} ${GLM_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/soil/src ${CMAKE_SOURCE_DIR}/../libs/bullet3/src)

add_library(glewLib STATIC ${CMAKE_SOURCE_DIR}/../libs/glew-2.1.0/src/glew.c)

file(GLOB_RECURSE bulletSrc RELATIVE ${CMAKE_SOURCE_DIR} "../libs/bullet3/src/Bullet3Collision/*.cpp"
							 "../libs/bullet3/src/Bullet3Common/*.cpp"
							 "../libs/bullet3/src/Bullet3Dynamics/*.cpp"
							 "../libs/bullet3/src/Bullet3Geometry/*.cpp"
							 "../libs/bullet3/src/Bullet3Serialize/*.cpp"
							 "../libs/bullet3/src/BulletCollision/*.cpp"
							 "../libs/bullet3/src/BulletDynamics/*.cpp"
							 "../libs/bullet3/src/BulletInverseDynamics/*.cpp"
							 "../libs/bullet3/src/BulletSoftBody/*.cpp"
							 "../libs/bullet3/src/LinearMath/*.cpp")

add_library(bulletLib STATIC ${bulletSrc})

set(SOIL_LIBRARY ${CMAKE_SOURCE_DIR}/../libs/soil/lib/libSOIL.a)

add_library(planetLib STATIC loadOBJ.hpp loadTex.hpp loadShaders.hpp text2D.hpp planet.hpp physics.hpp myCustomMotionState.hpp GLDebugDrawer.hpp tangentspace.hpp VBOindexer.hpp)
set_target_properties(planetLib PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries (planets 
	glewLib
	${SOIL_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${OPENGL_gl_LIBRARY} 
	${GLFW_LIBRARY}
	bulletLib
	planetLib
)