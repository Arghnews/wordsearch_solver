# Based on https://github.com/ericniebler/range-v3/blob/master/Version.cmake
if (DEFINED CMAKE_PROJECT_NAME AND
        NOT "${CMAKE_PROJECT_NAME}" STREQUAL "wordsearch_solver")
    message(FATAL_ERROR "Project name changed, update version.cmake")
endif()

# These 3 lines defining the version variables must be uncommented and
# formatted exactly like this

set(WORDSEARCH_SOLVER_MAJOR 0)
set(WORDSEARCH_SOLVER_MINOR 1)
set(WORDSEARCH_SOLVER_PATCH 8)

set(WORDSEARCH_SOLVER_VERSION_STRING
    "${WORDSEARCH_SOLVER_MAJOR}.${WORDSEARCH_SOLVER_MINOR}.${WORDSEARCH_SOLVER_PATCH}")
