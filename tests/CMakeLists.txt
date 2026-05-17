add_subdirectory(helpers)
# add_subdirectory(network)

add_executable(utils-tests
    bits.cpp
    ExecAround.cpp
    main.cpp
#     GlobalRegistry.cpp
#     Logger.cpp
#     property.cpp
#     Result.cpp
    ScopedExit.cpp
#     StateMachine.cpp
#     Watchdog.cpp
)

find_package(Catch2)
find_package(platform COMPONENTS main)
find_package(utils COMPONENTS bits functional)

target_link_libraries(utils-tests
    PRIVATE
        Catch2::Catch2
        platform::main
        tests::helpers
        utils::bits
        utils::functional
)

install(TARGETS utils-tests)
