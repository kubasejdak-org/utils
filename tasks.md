# utils modernization — phased task plan

## Context

Repository: `/home/kuba/projects/kubasejdak/libs/utils` (GitHub: `kubasejdak-org/utils`) Reference:
`/home/kuba/projects/kubasejdak/libs/osal` (GitHub: `kubasejdak-org/osal`)

`osal` is the gold-standard reference. Every task below brings `utils` in line with `osal`. Each phase is designed to be
a self-contained PR. Tasks within a phase should be committed together.

---

## Phase 2 — Code style tooling

**Goal**: Bring all static analysis and formatting configs to the current standard used in `osal`. Each tool config is a
separate task.

## Phase 3 — Repository hygiene

**Goal**: Fix `.gitignore`, license files, add `CONTRIBUTING.md`, and write `README.md`.

### Task 3.1 — Update `.gitignore`

Replace `.gitignore` with:

```gitignore
# Object and executable files
cmake-build-*/
out/
build/

# Build systems
CMakeLists.txt.user
CMakeUserPresets.json

# IDE
.idea/
```

Changes from current:

- Remove: `bin/`, `.vscode/`, `.DS_Store` (not in osal)
- Add: `out/`, `build/`, `CMakeUserPresets.json` (in osal, missing here)
- Rewrite comments to match osal style

### Task 3.3 — Update license headers in all source files

All `.hpp` and `.cpp` files in `lib/` (30 files) and `test/` (19 files, will become `tests/` in Phase 6) currently have
BSD 2-Clause headers with year ranges. They must be updated to MIT with a single creation year.

**Pattern to change** in each file header:

- `@copyright BSD 2-Clause License` → `@copyright MIT License`
- `Copyright (c) YYYY-2023, Kuba Sejdak <kuba.sejdak@gmail.com>` →
  `Copyright (c) YYYY Kuba Sejdak (kuba.sejdak@gmail.com)`
    - Keep `YYYY` as the first year (creation year of that file)
    - Remove the year range end (`, 2023`)
    - Remove the comma after the year
    - Change `<email>` angle-bracket format to `(email)` parenthesis format
    - Remove the `All rights reserved.` line
- Replace the entire BSD 2-Clause license body text with the MIT license body text (see Phase 3.2 for the MIT text)

**Files by creation year** (use the first year from the existing header):

- Files with `2020-2023` → use `2020`
- Files with `2021-2023` → use `2021`

Exact osal header format for reference:

```cpp
/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright MIT License
///
/// Copyright (c) 2020 Kuba Sejdak (kuba.sejdak@gmail.com)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///
/////////////////////////////////////////////////////////////////////////////////////
```

### Task 3.5 — Write `README.md`

Write a full `README.md` following the structure of `osal/README.md`. Adapt content for `utils`:

**Required sections and their content**:

1. **Title** (`# utils`) + one-sentence description of what utils provides
2. **Main features** — bullet list of the components: bits, fsm, functional, info, logger, network, preprocessor,
   registry, types, watchdog
3. **Architecture**
    - **Components** — one paragraph per component with its purpose
    - **Technologies** — table: Language C++23/C17, Build System CMake 3.28, Package Manager Conan (test deps only),
      Static Analysis clang-format/clang-tidy, CI/CD GitHub Actions
    - **Repository Structure** — `bash` code block tree showing the directory layout
4. **Usage**
    - **CMake Integration** — `find_package(utils COMPONENTS ...)` pattern with `FetchContent_Declare` in
      `Findutils.cmake`, `list(APPEND CMAKE_MODULE_PATH ...)` usage
    - **Linking** — example `target_link_libraries` for one or two components
5. **Development**
    - **Commands** — configure, build, run tests, reformat code, run linter (same commands as osal but with
      `utils-tests`)
    - **Available CMake Presets** — Native Linux (system/conan × gcc/clang × debug/release), Cross-compilation (arm64,
      yocto, FreeRTOS baremetal), Sanitizers
    - **Code Quality** — Zero Warning Policy, No Exceptions, Code Formatting, Static Analysis, Sanitizers
    - **Important Notes** — component structure, testing approach, dependencies, error handling conventions

Use GitHub markdown callouts (`> [!NOTE]`, `> [!IMPORTANT]`) for important notices, same as osal.

---

## Phase 4 — CMake structure

**Goal**: Align the CMake structure with `osal`: modern minimum version, clean root, `compilation-flags.cmake`,
component loader.

### Task 4.1 — Bump `cmake_minimum_required` to 3.28

Update in these files:

- `CMakeLists.txt` (root): `cmake_minimum_required(VERSION 3.24)` → `cmake_minimum_required(VERSION 3.28)`
- `lib/CMakeLists.txt`: `cmake_minimum_required(VERSION 3.24)` → remove entirely (the root is authoritative)

### Task 4.2 — Create `cmake/compilation-flags.cmake`

Create `cmake/compilation-flags.cmake`:

```cmake
add_compile_options(-Wall -Wextra -Wpedantic -Werror $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
```

Notes:

- C++ standard upgrades from 20 to 23 to match `osal`.
- The `-fno-exceptions` flag is NOT C-standard-gated (same as osal — applies to both C and CXX via the CXX generator
  expression is removed intentionally; `-fno-exceptions` is only meaningful for C++ but using `-fno-exceptions` without
  gating is still fine).
- This replaces: the `add_compile_options` line in `lib/CMakeLists.txt`, the `-fno-exceptions` handling in
  `test/settings.cmake`, and any equivalent lines in root `CMakeLists.txt`.

### Task 4.3 — Create `cmake/components.cmake`

Create `cmake/components.cmake`:

```cmake
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
```

This mirrors `osal/cmake/components.cmake` exactly, substituting `osal` with `utils`.

### Task 4.4 — Create `cmake/modules/Findutils.cmake`

Create the `cmake/modules/` directory and `cmake/modules/Findutils.cmake`:

```cmake
set(utils_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
include(${CMAKE_CURRENT_LIST_DIR}/../components.cmake)
```

This mirrors `osal/cmake/modules/Findosal.cmake`.

### Task 4.5 — Rewrite root `CMakeLists.txt`

Replace the entire root `CMakeLists.txt` with:

```cmake
cmake_minimum_required(VERSION 3.28)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

find_package(platform COMPONENTS toolchain)

project(utils ASM C CXX)

include(cmake/compilation-flags.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_subdirectory(lib)
add_subdirectory(tests)
```

This removes:

- `include(test/settings.cmake)` — toolchain paths move to preset files (Phase 5)
- All `include(cmake/conan.cmake)`, `include(cmake/coverage.cmake)`, etc. — these are replaced by the conan provider
  preset approach (Phase 6) and external actions (Phase 8)
- `project(utils VERSION 1.0.0 LANGUAGES ...)` → `project(utils ASM C CXX)` (no version, no LANGUAGES keyword, same as
  osal)
- Manual `add_subdirectory(${platform_SOURCE_DIR}/lib ...)` and `add_subdirectory(${osal_SOURCE_DIR}/lib ...)` —
  dependencies are handled by `find_package` inside `lib/` CMakeLists
- Inline `add_compile_options` — now in `cmake/compilation-flags.cmake`
- Sanitizer and coverage conditional blocks — handled by platform/CI externally
- `add_subdirectory(test)` → `add_subdirectory(tests)` (Phase 6 renames the directory)

### Task 4.6 — Update `lib/CMakeLists.txt`

Remove the `cmake_minimum_required` line (now covered by root). Remove the `add_compile_options` line (now in
`cmake/compilation-flags.cmake`).

The resulting `lib/CMakeLists.txt` should contain only `add_subdirectory(...)` calls for each component:

```cmake
add_subdirectory(bits)
add_subdirectory(fsm)
add_subdirectory(functional)
add_subdirectory(info)
add_subdirectory(logger)
add_subdirectory(preprocessor)
add_subdirectory(registry)
add_subdirectory(types)
add_subdirectory(watchdog)

if (OSAL_PLATFORM STREQUAL linux)
    add_subdirectory(network)
endif ()
```

### Task 4.7 — Delete old cmake helper files

Delete these files that are replaced by the new structure:

- `cmake/conan.cmake` — replaced by `cmake/conan_provider.cmake` (Phase 6)
- `cmake/coverage.cmake` — coverage handled by CI action externally
- `cmake/osal.cmake` — osal is now a `find_package` dep via `cmake/modules/`
- `cmake/platform.cmake` — platform is now a `find_package` dep
- `cmake/sanitizers.cmake` — sanitizers handled via preset cache variables and CI
- `test/settings.cmake` — toolchain paths move into cmake preset files (Phase 5)

---

## Phase 5 — CMake presets

**Goal**: Upgrade `CMakePresets.json` from version 3 to version 8, split into modular preset files, rename presets to
match `osal` naming convention, and add sanitizer variants.

### Task 5.1 — Create `cmake/presets/type.json`

```json
{
    "version": 8,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 28,
        "patch": 0
    },
    "configurePresets": [
        {
            "name": "debug",
            "hidden": true,
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug"
            }
        },
        {
            "name": "release",
            "hidden": true,
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Release"
            }
        }
    ]
}
```

### Task 5.2 — Create `cmake/presets/dependencies.json`

```json
{
    "version": 8,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 28,
        "patch": 0
    },
    "configurePresets": [
        {
            "name": "conan",
            "hidden": true,
            "cacheVariables": {
                "CMAKE_PROJECT_TOP_LEVEL_INCLUDES": "${sourceDir}/cmake/conan_provider.cmake"
            }
        }
    ]
}
```

### Task 5.3 — Create `cmake/presets/linux.json`

```json
{
    "version": 8,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 28,
        "patch": 0
    },
    "configurePresets": [
        {
            "name": "linux",
            "hidden": true,
            "cacheVariables": {
                "PLATFORM": "linux"
            }
        },
        {
            "name": "linux-native-gcc",
            "hidden": true,
            "inherits": "linux",
            "cacheVariables": {
                "TOOLCHAIN": "gcc"
            }
        },
        {
            "name": "linux-native-clang",
            "hidden": true,
            "inherits": "linux",
            "cacheVariables": {
                "TOOLCHAIN": "clang"
            }
        },
        {
            "name": "linux-arm64",
            "hidden": true,
            "inherits": "linux",
            "cacheVariables": {
                "LINUX_ARM_TOOLCHAIN_PATH": "/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu"
            }
        },
        {
            "name": "linux-arm64-gcc",
            "hidden": true,
            "inherits": "linux-arm64",
            "cacheVariables": {
                "TOOLCHAIN": "aarch64-none-linux-gnu-gcc"
            }
        },
        {
            "name": "linux-arm64-clang",
            "hidden": true,
            "inherits": "linux-arm64",
            "cacheVariables": {
                "TOOLCHAIN": "aarch64-none-linux-gnu-clang"
            }
        },
        {
            "name": "yocto-sdk-gcc",
            "hidden": true,
            "inherits": "linux"
        },
        {
            "name": "yocto-sdk-clang",
            "hidden": true,
            "inherits": "linux",
            "environment": {
                "CC": "$env{CLANGCC}",
                "CPP": "$env{CLANGCPP}",
                "CXX": "$env{CLANGCXX}"
            }
        }
    ]
}
```

Note: The old `linux-arm-gcc`/`linux-arm-clang` presets used `arm-none-linux-gnueabihf` (32-bit ARM). Replace with
`linux-arm64` variants using `aarch64-none-linux-gnu` (64-bit ARM) to match the `osal` toolchain.

### Task 5.4 — Create `cmake/presets/baremetal.json`

```json
{
    "version": 8,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 28,
        "patch": 0
    },
    "configurePresets": [
        {
            "name": "freertos-armv7-m4",
            "hidden": true,
            "cacheVariables": {
                "PLATFORM": "freertos-arm",
                "BAREMETAL_ARM_TOOLCHAIN_PATH": "/opt/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi",
                "APP_C_FLAGS": "-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb --specs=nano.specs",
                "APP_CXX_FLAGS": "-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -mthumb --specs=nano.specs",
                "FREERTOS_VERSION": "freertos-10.2.1",
                "FREERTOS_PORTABLE": "ARM_CM4F"
            }
        }
    ]
}
```

Note: Toolchain path updated from the old `gcc-arm-11.2-2022.02` path to the `arm-gnu-toolchain-13.3.rel1` path
(matching `osal`).

### Task 5.5 — Create `cmake/presets/sanitizers.json`

```json
{
    "version": 8,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 28,
        "patch": 0
    },
    "configurePresets": [
        {
            "name": "asan",
            "hidden": true,
            "cacheVariables": {
                "USE_ASAN": "ON"
            }
        },
        {
            "name": "lsan",
            "hidden": true,
            "cacheVariables": {
                "USE_LSAN": "ON"
            }
        },
        {
            "name": "tsan",
            "hidden": true,
            "cacheVariables": {
                "USE_TSAN": "ON"
            }
        },
        {
            "name": "ubsan",
            "hidden": true,
            "cacheVariables": {
                "USE_UBSAN": "ON"
            }
        }
    ]
}
```

### Task 5.6 — Rewrite root `CMakePresets.json`

Replace `CMakePresets.json` with version 8 that includes the split files and defines only the concrete non-hidden
presets:

```json
{
    "version": 8,
    "cmakeMinimumRequired": {
        "major": 3,
        "minor": 28,
        "patch": 0
    },
    "include": [
        "cmake/presets/baremetal.json",
        "cmake/presets/dependencies.json",
        "cmake/presets/linux.json",
        "cmake/presets/sanitizers.json",
        "cmake/presets/type.json"
    ],
    "configurePresets": [
        {
            "name": "linux-native-gcc-debug",
            "inherits": ["linux-native-gcc", "debug"]
        },
        {
            "name": "linux-native-gcc-release",
            "inherits": ["linux-native-gcc", "release"]
        },
        {
            "name": "linux-native-clang-debug",
            "inherits": ["linux-native-clang", "debug"]
        },
        {
            "name": "linux-native-clang-release",
            "inherits": ["linux-native-clang", "release"]
        },
        {
            "name": "linux-native-gcc-debug-asan",
            "inherits": ["linux-native-gcc-debug", "asan"]
        },
        {
            "name": "linux-native-gcc-debug-lsan",
            "inherits": ["linux-native-gcc-debug", "lsan"]
        },
        {
            "name": "linux-native-gcc-debug-tsan",
            "inherits": ["linux-native-gcc-debug", "tsan"]
        },
        {
            "name": "linux-native-gcc-debug-ubsan",
            "inherits": ["linux-native-gcc-debug", "ubsan"]
        },
        {
            "name": "linux-native-conan-gcc-debug",
            "inherits": ["linux-native-gcc-debug", "conan"]
        },
        {
            "name": "linux-native-conan-gcc-release",
            "inherits": ["linux-native-gcc-release", "conan"]
        },
        {
            "name": "linux-native-conan-clang-debug",
            "inherits": ["linux-native-clang-debug", "conan"]
        },
        {
            "name": "linux-native-conan-clang-release",
            "inherits": ["linux-native-clang-release", "conan"]
        },
        {
            "name": "linux-native-conan-gcc-debug-asan",
            "inherits": ["linux-native-conan-gcc-debug", "asan"]
        },
        {
            "name": "linux-native-conan-gcc-debug-lsan",
            "inherits": ["linux-native-conan-gcc-debug", "lsan"]
        },
        {
            "name": "linux-native-conan-gcc-debug-tsan",
            "inherits": ["linux-native-conan-gcc-debug", "tsan"]
        },
        {
            "name": "linux-native-conan-gcc-debug-ubsan",
            "inherits": ["linux-native-conan-gcc-debug", "ubsan"]
        },
        {
            "name": "linux-arm64-conan-gcc-debug",
            "inherits": ["linux-arm64-gcc", "debug", "conan"]
        },
        {
            "name": "linux-arm64-conan-gcc-release",
            "inherits": ["linux-arm64-gcc", "release", "conan"]
        },
        {
            "name": "linux-arm64-conan-clang-debug",
            "inherits": ["linux-arm64-clang", "debug", "conan"]
        },
        {
            "name": "linux-arm64-conan-clang-release",
            "inherits": ["linux-arm64-clang", "release", "conan"]
        },
        { "name": "yocto-sdk-gcc-debug", "inherits": ["yocto-sdk-gcc", "debug"] },
        {
            "name": "yocto-sdk-gcc-release",
            "inherits": ["yocto-sdk-gcc", "release"]
        },
        {
            "name": "yocto-sdk-clang-debug",
            "inherits": ["yocto-sdk-clang", "debug"]
        },
        {
            "name": "yocto-sdk-clang-release",
            "inherits": ["yocto-sdk-clang", "release"]
        },
        {
            "name": "freertos-armv7-m4-conan-gcc-debug",
            "inherits": ["freertos-armv7-m4", "debug", "conan"]
        },
        {
            "name": "freertos-armv7-m4-conan-gcc-release",
            "inherits": ["freertos-armv7-m4", "release", "conan"]
        }
    ]
}
```

**Verification**: `cmake --list-presets` should show the concrete presets. All old preset names (`linux-gcc-debug`,
etc.) are removed and replaced with the new naming.

---

## Phase 6 — Conan migration

**Goal**: Migrate from Conan V1 cmake wrapper to Conan V2 CMake-native provider.

### Task 6.1 — Add `cmake/conan_provider.cmake`

Copy `cmake/conan_provider.cmake` verbatim from `osal/cmake/conan_provider.cmake`. This is the JFrog-maintained
MIT-licensed Conan V2 CMake provider (689 lines). Do not modify its content.

### Task 6.2 — Update `conanfile.txt`

Replace `conanfile.txt` with:

```ini
[test_requires]
catch2/3.13.0

[requires]
fmt/9.1.0
spdlog/1.11.0

[generators]
CMakeDeps

[options]
catch2/*:default_reporter=verbose
catch2/*:no_posix_signals=True
fmt:header_only=True
spdlog:no_exceptions=True
spdlog:header_only=True
```

Changes from current:

- `catch2` version: `3.3.0` → `3.13.0` (matches osal)
- `catch2` moves from `[requires]` to `[test_requires]`
- `fmt` and `spdlog` stay in `[requires]` (they are runtime library dependencies)
- Generator: `cmake` → `CMakeDeps`
- Add Catch2 options: `default_reporter=verbose`, `no_posix_signals=True`

**Note on `USE_CONAN` removal**: The old `USE_CONAN` CMake cache variable and `if (USE_CONAN)` blocks in
`CMakeLists.txt` are deleted in Phase 4. Conan is now activated by choosing a `conan` preset, which sets
`CMAKE_PROJECT_TOP_LEVEL_INCLUDES` to the provider path — no CMake-level `if/else` needed.

---

## Phase 7 — Tests directory

**Goal**: Rename `test/` to `tests/`, modernize `CMakeLists.txt`, add `.clang-tidy`, and fix assertion anti-patterns.

### Task 7.1 — Rename `test/` to `tests/`

Rename the directory `test/` to `tests/`. This also affects:

- Root `CMakeLists.txt` reference: `add_subdirectory(test)` → `add_subdirectory(tests)` (already handled in Phase 4)
- Any path references inside the test CMakeLists files (search for `test/` or `${CMAKE_SOURCE_DIR}/test`)

### Task 7.2 — Update `tests/CMakeLists.txt`

Replace the test `CMakeLists.txt` with a modernized version:

**Changes required**:

1. Replace `find_library(Catch2 NAMES Catch2 Catch2d REQUIRED)` with `find_package(Catch2)` (CMakeDeps-based, same as
   osal)
2. Replace `find_package(utils COMPONENTS ...)` — use the new component find mechanism from Phase 4
3. In `target_link_libraries`, replace `optimized Catch2 debug Catch2d` with `Catch2::Catch2`
4. Add `find_package(osal)` and `find_package(platform COMPONENTS init main)` (needed for test dependencies)
5. Remove `TEST_TAGS` compile definition block
6. Remove `include(settings.cmake)` (file deleted in Phase 4)
7. Remove the `if (PLATFORM STREQUAL freertos-arm)` / `objcopy_generate_bin` conditional — check if this is still needed
   or handled by platform externally
8. The `if (OSAL_PLATFORM STREQUAL linux) add_subdirectory(network) endif()` conditional may stay if network tests are
   Linux-only

The modernized version should follow the pattern of `osal/tests/CMakeLists.txt`:

- `find_package(Catch2)`, `find_package(utils COMPONENTS ...)`, `find_package(platform COMPONENTS ...)`
- Clean `target_link_libraries` with `Catch2::Catch2` and `utils::*` aliases
- No `cmake_minimum_required` (the root is authoritative)

### Task 7.3 — Update `tests/init/freertos-arm/CMakeLists.txt`

The file currently references `${CMAKE_SOURCE_DIR}/external/stm32f4xx`. This dependency must be removed since
`external/` is deleted in Phase 9. The STM32 board support should come from the `platform` dependency via
`find_package(platform)`. Update this file to use the platform-provided stm32f4xx target instead of the external
directory.

If platform does not yet provide this, the `freertos-arm` init can be left as a stub or removed until platform supports
it.

### Task 7.4 — Create `tests/.clang-tidy`

Create `tests/.clang-tidy`:

```yaml
---
InheritParentConfig: true
Checks: '
  -clang-analyzer-optin.core.EnumCastOutOfRange,
  -*-magic-numbers
'
...
```

This mirrors `osal/tests/.clang-tidy` exactly.

### Task 7.5 — Fix assertion negation patterns in test files

**Convention**:

- Use `CHECK_FALSE(expr)` instead of `CHECK(!expr)`
- Use `REQUIRE_FALSE(expr)` instead of `REQUIRE(!expr)`
- Never use the negation operator `!` inside a Catch2 assertion macro

**Scope**: Replace all 191 occurrences of `REQUIRE(!...)` across all test `.cpp` files. Also check for `CHECK(!...)`
(currently 0 but verify).

Files to update (search for `REQUIRE(!` in `tests/`):

- `tests/ExecAround.cpp`
- `tests/GlobalRegistry.cpp`
- `tests/Result.cpp`
- `tests/network/TcpClient.cpp`
- `tests/network/TcpConnection.cpp`
- (any other files returned by `grep -rn "REQUIRE(!" tests/`)

For each occurrence, the transformation is mechanical:

- `REQUIRE(!foo)` → `REQUIRE_FALSE(foo)`
- `REQUIRE(!foo.bar())` → `REQUIRE_FALSE(foo.bar())`
- `REQUIRE(!foo.bar)` → `REQUIRE_FALSE(foo.bar)`

**Additionally**: Review `REQUIRE` vs `CHECK` usage. The rule is:

- `REQUIRE` — for preconditions where failure means the rest of the test would crash or produce meaningless results
- `CHECK` — for independent assertions where failure is reported but execution continues

In the network tests, most `REQUIRE(!error)` after connection setup are genuine preconditions (the test can't proceed if
the connection failed) → keep as `REQUIRE_FALSE`. But assertions checking individual data values (e.g., checking IP
strings, port numbers) should use `CHECK`/`CHECK_FALSE` not `REQUIRE`/`REQUIRE_FALSE`.

---

## Phase 8 — CI: migrate from GitLab to GitHub Actions

**Goal**: Remove all GitLab CI configuration and add equivalent GitHub Actions workflows.

### Task 8.1 — Delete GitLab CI files

Delete:

- `.gitlab-ci.yml` (root file)
- `.gitlab/` directory (entire directory with all files inside: `ci/build-freertos.yml`, `ci/build-linux.yml`,
  `ci/coverage-linux.yml`, `ci/deploy.yml`, `ci/quality.yml`, `ci/sanitizers-linux.yml`, `ci/test-linux.yml`,
  `ci/valgrind-linux.yml`, and `issue_templates/`)

### Task 8.2 — Create `.github/workflows/build-test-linux.yml`

Create `.github/workflows/build-test-linux.yml` modeled after `osal/.github/workflows/build-test-linux.yml`:

**Jobs**:

1. `build-x64` — matrix of: `linux-native-conan-gcc-{debug,release}`, `linux-native-conan-clang-{debug,release}`,
   `linux-native-conan-gcc-debug-{asan,lsan,tsan,ubsan}`. Uses `kubasejdak-org/cmake-build-preset-action@main`.
2. `build-arm64` — matrix of: `linux-arm64-conan-{gcc,clang}-{debug,release}` (corresponding Docker images:
   `kubasejdak/aarch64-none-linux-gnu-gcc:13-24.04`, `kubasejdak/aarch64-none-linux-gnu-clang:18-24.04`)
3. `tests-x64` — needs `build-x64`, runs `utils-tests` binary from `linux-native-conan-gcc-debug` artifact. Uses
   `kubasejdak-org/binary-run-action@main`.
4. `valgrind-x64` — needs `build-x64`, runs `utils-tests` under valgrind from `linux-native-conan-gcc-debug` artifact.
   Uses `kubasejdak-org/valgrind-run-action@main`.
5. `sanitizers-x64` — needs `build-x64`, runs `utils-tests` for each sanitizer preset. Uses
   `kubasejdak-org/binary-run-action@main`.
6. `check-all-linux` — gate job that needs all above jobs.

Triggers: `push`, `schedule: "0 12 * * SAT"`, `workflow_dispatch`.

Docker image mapping: gcc presets → `kubasejdak/gcc:13-24.04`, clang presets → `kubasejdak/clang:18-24.04`.

### Task 8.3 — Create `.github/workflows/build-test-baremetal.yml`

Create `.github/workflows/build-test-baremetal.yml` modeled after `osal/.github/workflows/build-test-baremetal.yml`:

**Jobs**:

1. `build-stm32f4` — matrix of: `freertos-armv7-m4-conan-gcc-{debug,release}`. Docker image:
   `kubasejdak/arm-none-eabi-gcc:13-24.04`. Uses `kubasejdak-org/cmake-build-preset-action@main`.
2. `check-all-baremetal` — gate job.

Triggers: same as above.

### Task 8.4 — Create `.github/workflows/static-analysis.yml`

Create `.github/workflows/static-analysis.yml` modeled after `osal/.github/workflows/static-analysis.yml`:

**Jobs**:

1. `formatting` (name: `clang-format`) — Docker: `kubasejdak/clang:18-24.04`. Uses
   `kubasejdak-org/clang-format-run-action@main`.
2. `linting` (name: `clang-tidy`) — preset `linux-native-conan-clang-debug`, Docker: `kubasejdak/clang:18-24.04`. Uses
   `kubasejdak-org/clang-tidy-run-action@main`.
3. `check-all-static` — gate job.

### Task 8.5 — Create `.github/workflows/code-coverage.yml`

Create `.github/workflows/code-coverage.yml` modeled after `osal/.github/workflows/code-coverage.yml`:

**Jobs** (all use `kubasejdak/gcc:13-24.04`, preset `linux-native-conan-gcc-debug`):

1. `build-coverage` — uses `kubasejdak-org/cpp-coverage-check-action/build@main`.
2. `test-coverage` — needs `build-coverage`, matrix with `APP: utils-tests`. Uses
   `kubasejdak-org/cpp-coverage-check-action/collect@main`.
3. `generate-coverage-report` — needs `test-coverage`. Uses `kubasejdak-org/cpp-coverage-check-action/report@main` with
   `line-coverage: 90.0`, `function-coverage: 90.0`.
4. `check-all-coverage` — gate job.

---

## Phase 9 — Cleanup

**Goal**: Remove legacy files and directories that have no place in the modernized repository.

### Task 9.1 — Delete `external/` directory

Delete the entire `external/stm32f4xx/` directory. This contains vendored STM32 HAL/CMSIS sources that were included
directly in the repo. In the modernized setup, board-support comes from the `platform` dependency (accessed via
`find_package(platform)`). The `tests/init/freertos-arm/CMakeLists.txt` reference to this directory was removed in
Phase 7.

### Task 9.2 — Delete `tools/ci/` directory

Delete `tools/ci/` directory (contains `logs-reader.py` and `program-openocd.py` — GitLab CI runner scripts). The
`tools/` directory itself should be kept; only the `ci/` subdirectory is removed.

---

## File change summary

| File/Directory                               | Phase | Action                        |
| -------------------------------------------- | ----- | ----------------------------- |
| `lib/**/*.{hpp,cpp}` (30 files)              | 3     | Update license headers        |
| `test/**/*.{hpp,cpp}` (19 files)             | 3     | Update license headers        |
| `README.md`                                  | 3     | Write                         |
| `CMakeLists.txt`                             | 4     | Rewrite                       |
| `lib/CMakeLists.txt`                         | 4     | Update                        |
| `cmake/compilation-flags.cmake`              | 4     | Create                        |
| `cmake/components.cmake`                     | 4     | Create                        |
| `cmake/modules/Findutils.cmake`              | 4     | Create                        |
| `cmake/conan.cmake`                          | 4     | Delete                        |
| `cmake/coverage.cmake`                       | 4     | Delete                        |
| `cmake/osal.cmake`                           | 4     | Delete                        |
| `cmake/platform.cmake`                       | 4     | Delete                        |
| `cmake/sanitizers.cmake`                     | 4     | Delete                        |
| `test/settings.cmake`                        | 4     | Delete                        |
| `CMakePresets.json`                          | 5     | Rewrite (version 8)           |
| `cmake/presets/type.json`                    | 5     | Create                        |
| `cmake/presets/dependencies.json`            | 5     | Create                        |
| `cmake/presets/linux.json`                   | 5     | Create                        |
| `cmake/presets/baremetal.json`               | 5     | Create                        |
| `cmake/presets/sanitizers.json`              | 5     | Create                        |
| `cmake/conan_provider.cmake`                 | 6     | Create (copy from osal)       |
| `conanfile.txt`                              | 6     | Update                        |
| `test/` directory                            | 7     | Rename to `tests/`            |
| `tests/CMakeLists.txt`                       | 7     | Update                        |
| `tests/init/freertos-arm/CMakeLists.txt`     | 7     | Update (remove external/ ref) |
| `tests/.clang-tidy`                          | 7     | Create                        |
| `tests/**/*.cpp` (assertion fixes)           | 7     | 191 REQUIRE(!→REQUIRE_FALSE   |
| `.gitlab-ci.yml`                             | 8     | Delete                        |
| `.gitlab/`                                   | 8     | Delete                        |
| `.github/workflows/build-test-linux.yml`     | 8     | Create                        |
| `.github/workflows/build-test-baremetal.yml` | 8     | Create                        |
| `.github/workflows/static-analysis.yml`      | 8     | Create                        |
| `.github/workflows/code-coverage.yml`        | 8     | Create                        |
| `external/`                                  | 9     | Delete                        |
| `tools/ci/`                                  | 9     | Delete                        |
