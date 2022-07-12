add_subdirectory(init)

add_executable(utils-tests
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

target_compile_definitions(utils-tests
    # This is required to selectively run Catch2 tests using tags on platforms, where cmd line arguments are not available.
    PRIVATE $<$<BOOL:${TEST_TAGS}>:TEST_TAGS="${TEST_TAGS}"> CATCH_CONFIG_DISABLE_EXCEPTIONS $<$<STREQUAL:${PLATFORM},freertos-arm>:CATCH_CONFIG_NO_POSIX_SIGNALS>
)

target_link_libraries(utils-tests
    PRIVATE osal::cpp platform::init platform::main utils::registry utils::logger utils::types utils::bits utils::fsm utils::functional utils::watchdog
)

if (OSAL_PLATFORM STREQUAL linux)
    add_subdirectory(network)
endif ()

if (PLATFORM STREQUAL freertos-arm)
    objcopy_generate_bin(utils-tests)
endif ()
