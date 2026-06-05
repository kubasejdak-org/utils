add_subdirectory(helpers)
# add_subdirectory(network)

add_executable(utils-tests
    bits.cpp
    ExecAround.cpp
    GlobalRegistry.cpp
    Logger.cpp
    main.cpp
#     property.cpp
    ScopedExit.cpp
    StateMachine.cpp
#     Watchdog.cpp
)

find_package(Catch2)
find_package(osal)
find_package(platform COMPONENTS main)
find_package(utils COMPONENTS bits functional logger registry sm)

target_link_libraries(utils-tests
    PRIVATE
        Catch2::Catch2
        platform::main
        tests::helpers
        osal::cpp
        utils::bits
        utils::functional
        utils::logger
        utils::registry
        utils::sm
)

install(TARGETS utils-tests)
