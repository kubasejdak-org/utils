add_subdirectory(init)

# Object library type is used to prevent compiler from removing unit test functions from the binary.
add_library(apputils-tests OBJECT EXCLUDE_FROM_ALL
    appMain.cpp
    bits.cpp
    ExecAround.cpp
    GlobalRegistry.cpp
    Logger.cpp
    property.cpp
    Result.cpp
    ScopedExit.cpp
    StateMachine.cpp
    Watchdog.cpp
)

target_include_directories(apputils-tests
    PRIVATE .
)

target_compile_definitions(apputils-tests
    # This is required to selectively run Catch2 tests using tags on platforms, where cmd line arguments are not available.
    PRIVATE $<$<BOOL:${TEST_TAGS}>:TEST_TAGS="${TEST_TAGS}">
)

target_compile_definitions(apputils-tests
    PRIVATE CATCH_CONFIG_DISABLE_EXCEPTIONS $<$<STREQUAL:${PLATFORM},freertos-arm>:CATCH_CONFIG_NO_POSIX_SIGNALS>
)

target_link_libraries(apputils-tests
    PUBLIC platform-init
    PRIVATE osal::cpp utils::registry utils::logger utils::types utils::bits utils::fsm utils::functional utils::watchdog
)

if (OSAL_PLATFORM STREQUAL linux)
    add_subdirectory(network)
endif ()
