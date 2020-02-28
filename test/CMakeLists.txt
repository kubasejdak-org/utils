add_subdirectory(init)

# Object library type is used to prevent compiler from removing unit test functions from the binary.
add_library(apputils-tests OBJECT EXCLUDE_FROM_ALL
    appMain.cpp
    GlobalRegistry.cpp
)

target_include_directories(apputils-tests
    PRIVATE .
)

target_compile_features(apputils-tests
    PRIVATE cxx_std_17
)

find_package(Catch2 REQUIRED)

if (PLATFORM STREQUAL freertos-arm)
    # This is required, because FreeRTOS doesn't support signals or exceptions.
    target_compile_definitions(Catch2::Catch2 INTERFACE CATCH_CONFIG_NO_POSIX_SIGNALS CATCH_CONFIG_DISABLE_EXCEPTIONS)
endif ()

target_link_libraries(apputils-tests
    PUBLIC platform-init
    PRIVATE Catch2::Catch2 osal::cpp utils
)

# This is required to selectively run Catch2 tests using tags on platforms, where cmd line arguments are not available.
if (DEFINED TEST_TAGS)
    target_compile_definitions(apputils-tests
        PRIVATE TEST_TAGS="${TEST_TAGS}"
    )
endif ()
