add_subdirectory(init)

# Object library type is used to prevent compiler from removing unit test functions from the binary.
add_library(apputils-tests OBJECT EXCLUDE_FROM_ALL
    appMain.cpp
    bits.cpp
    EventStats.cpp
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

conan_get(REQUIRES catch2/2.13.7 fmt/8.0.1 OPTIONS fmt:header_only=True)
find_package(Catch2 REQUIRED)
find_package(fmt REQUIRED)

target_compile_definitions(apputils-tests
    # This is required to selectively run Catch2 tests using tags on platforms, where cmd line arguments are not available.
    PRIVATE $<$<BOOL:${TEST_TAGS}>:TEST_TAGS="${TEST_TAGS}">
)

target_compile_definitions(Catch2::Catch2
    INTERFACE CATCH_CONFIG_DISABLE_EXCEPTIONS $<$<STREQUAL:${PLATFORM},freertos-arm>:CATCH_CONFIG_NO_POSIX_SIGNALS>
)

target_link_libraries(apputils-tests
    PUBLIC platform-init
    PRIVATE Catch2::Catch2 fmt::fmt osal::cpp utils::registry utils::logger utils::types utils::bits utils::fsm utils::functional
            utils::watchdog utils::debug
)

if (OSAL_PLATFORM STREQUAL linux)
    add_subdirectory(network)
endif ()
