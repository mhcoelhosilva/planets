set(FIND_GLM_PATHS c:/repo/glm-0.9.9.5)

find_path(GLM_INCLUDE_DIR glm.hpp
        PATH_SUFFIXES glm
        PATHS ${FIND_GLM_PATHS})
