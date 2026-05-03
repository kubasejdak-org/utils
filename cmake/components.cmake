if (NOT utils_FIND_COMPONENTS)
    file(GLOB utils_FIND_COMPONENTS LIST_DIRECTORIES true RELATIVE ${utils_SOURCE_DIR}/lib ${utils_SOURCE_DIR}/lib/*)
endif ()

include(FetchContent)
foreach (component IN LISTS utils_FIND_COMPONENTS)
    FetchContent_Declare(utils-${component}
        SOURCE_DIR      ${utils_SOURCE_DIR}
        SOURCE_SUBDIR   lib/${component}
        SYSTEM
    )

    FetchContent_MakeAvailable(utils-${component})
endforeach ()
