add_library(utils-info EXCLUDE_FROM_ALL
    git.cpp
)
add_library(utils::info ALIAS utils-info)

target_include_directories(utils-info
    PUBLIC include
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

find_package(Git)
add_custom_target(utils-git-info
    ${CMAKE_COMMAND} -D VERSION_TARGET=utils
                     -D SRC=${CMAKE_CURRENT_SOURCE_DIR}/git.h.in
                     -D DST=${CMAKE_CURRENT_BINARY_DIR}/git.h
                     -D GIT_EXECUTABLE=${GIT_EXECUTABLE}
                     -P ${CMAKE_CURRENT_SOURCE_DIR}/getGitInfo.cmake
)
add_dependencies(utils-info utils-git-info)
