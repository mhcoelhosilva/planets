set(FIND_GLFW_PATHS c:/repo/glfw-3.3.bin.WIN32/glfw-3.3.bin.WIN32)

find_path(GLFW_INCLUDE_DIR glfw3.h
        PATH_SUFFIXES include/GLFW
        PATHS ${FIND_GLFW_PATHS})

find_library(GLFW_LIBRARY NAMES glfw3
        PATH_SUFFIXES lib-mingw-w64
        PATHS ${FIND_GLFW_PATHS})
		