add_subdirectory(helpers)
add_subdirectory(init)

add_executable(utils-tests
    bits.cpp
    ExecAround.cpp
    Logger.cpp
    main.cpp
#     GlobalRegistry.cpp
#     property.cpp
#     Result.cpp
    ScopedExit.cpp
#     StateMachine.cpp
#     Watchdog.cpp
)

find_package(Catch2)
find_package(platform COMPONENTS main)
find_package(utils COMPONENTS bits functional logger)

target_link_libraries(utils-tests
    PRIVATE
        Catch2::Catch2
        platform::init
        platform::main
        tests::helpers
        utils::bits
        utils::functional
        utils::logger
)

# if (OSAL_PLATFORM STREQUAL linux)
#     add_subdirectory(network)
# endif ()

if (NOT UNIX)
    objcopy_generate_bin(utils-tests)
endif ()

install(TARGETS utils-tests)
