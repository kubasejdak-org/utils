add_subdirectory(helpers)
add_subdirectory(init)

add_executable(utils-tests
    bits.cpp
    main.cpp
#     ExecAround.cpp
#     GlobalRegistry.cpp
#     Logger.cpp
#     property.cpp
#     Result.cpp
#     ScopedExit.cpp
#     StateMachine.cpp
#     Watchdog.cpp
)

find_package(Catch2)
find_package(platform COMPONENTS main)
find_package(utils COMPONENTS bits)

target_link_libraries(utils-tests
    PRIVATE
        Catch2::Catch2
        platform::init
        platform::main
        tests::helpers
        utils::bits
)

# if (OSAL_PLATFORM STREQUAL linux)
#     add_subdirectory(network)
# endif ()

if (NOT UNIX)
    objcopy_generate_bin(utils-tests)
endif ()

install(TARGETS utils-tests)
