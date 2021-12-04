add_library(utils-logger INTERFACE)
add_library(utils::logger ALIAS utils-logger)

target_include_directories(utils-logger
    INTERFACE include
)

if (NOT UNIX)
    target_link_libraries(utils-logger
        INTERFACE osal::cpp
    )

    set(COMPATIBILITY_PATH      ${CMAKE_CURRENT_SOURCE_DIR}/compatibility)
    target_compile_options(utils-logger
        INTERFACE -include${COMPATIBILITY_PATH}/mutex -include${COMPATIBILITY_PATH}/thread -include${COMPATIBILITY_PATH}/condition_variable -include${COMPATIBILITY_PATH}/syscalls.hpp
    )
endif ()
