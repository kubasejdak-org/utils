include(FetchContent)
FetchContent_Declare(osal
    GIT_REPOSITORY  https://github.com/kubasejdak-org/osal.git
    GIT_TAG         764f5ea48e964a41c5168464b5e1a3b23ed91bd2
    SOURCE_SUBDIR   lib
    SYSTEM
)

FetchContent_MakeAvailable(osal)
include(${osal_SOURCE_DIR}/cmake/components.cmake)
