set(FIND_GLEW_PATHS c:/repo/glew-2.1.0-win32/glew-2.1.0)

find_path(GLEW_DIR glew.h
        PATH_SUFFIXES include/GL
        PATHS ${FIND_GLEW_PATHS})

find_library(GLEW_LIBRARY NAMES glew32
        PATH_SUFFIXES bin/Release/x64
        PATHS ${FIND_GLEW_PATHS})
		