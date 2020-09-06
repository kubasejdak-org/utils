include(FetchContent)
FetchContent_Declare(liballocator
    GIT_REPOSITORY  https://gitlab.com/kubasejdak-libs/liballocator.git
    GIT_TAG         origin/master
)

FetchContent_GetProperties(liballocator)
if (NOT liballocator_POPULATED)
    FetchContent_Populate(liballocator)
endif ()
