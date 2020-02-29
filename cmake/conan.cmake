if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake" "${CMAKE_BINARY_DIR}/conan.cmake")
endif ()

include(${CMAKE_BINARY_DIR}/conan.cmake)

macro(conan_get)
    conan_cmake_run(
        REQUIRES            ${ARGN}
        BUILD_TYPE          ${CMAKE_BUILD_TYPE}
        PROFILE             ${CONAN_PROFILE}
        BUILD               missing
        GENERATORS          cmake_find_package cmake_paths
    )

    include(${CMAKE_CURRENT_BINARY_DIR}/conan_paths.cmake)
endmacro ()
