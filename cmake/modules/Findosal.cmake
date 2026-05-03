include(FetchContent)
FetchContent_Declare(osal
    GIT_REPOSITORY  https://github.com/kubasejdak-org/osal.git
    GIT_TAG         dfe8bff5bbe6962372972d23809f4f6a8ca7fe05
    SOURCE_SUBDIR   lib
    SYSTEM
)

FetchContent_MakeAvailable(osal)
include(${osal_SOURCE_DIR}/cmake/components.cmake)
