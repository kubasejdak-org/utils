# Component Dependency Management — Spec

## Problem Statement

The `utils`, `osal`, and `platform` repos each contain multiple micro-library components under `lib/`. Consumers request
specific components via:

```cmake
find_package(utils COMPONENTS bits logger watchdog)
target_link_libraries(myapp PRIVATE utils::bits utils::logger utils::watchdog)
```

The `cmake/components.cmake` mechanism makes components available selectively on demand: it uses `FetchContent` to add
each requested component's `CMakeLists.txt` into the build. Components marked `EXCLUDE_FROM_ALL` are not compiled unless
something links against them, so unused components never touch the build even though their CMake files are parsed.

### Current problem in Yocto

When building apps for an embedded Yocto Linux system, each app independently resolves every internal dependency via
`FetchContent` (pulling from GitHub) and links them statically. This results in:

- **Binary bloat**: every app embeds the full code of every lib it uses.
- **Version fragmentation**: two apps on the same device can silently use different commits of `osal`.
- **No sharing**: there is no `.so` on the device — the linker cannot share code between processes.

External 3rd-party deps (`spdlog`, `Catch2`) already avoid this problem. In development, `conan_provider.cmake`
intercepts `find_package` and invokes Conan to produce pre-built packages. In Yocto and on native systems with packages
installed, the same `find_package` call finds Config/Find cmake files already present in the sysroot — transparently. We
want the same transparency for internal deps.

---

## Design

### Two global modes

A single CMake cache variable `INTERNAL_DEPS_FROM_SYSTEM` controls how ALL internal dependencies are resolved. It is not
set per-package.

| Mode                    | Variable | How deps resolve                                               | Linking |
| ----------------------- | -------- | -------------------------------------------------------------- | ------- |
| **Repo mode** (default) | `OFF`    | `FetchContent` from GitHub, pinned SHA                         | Static  |
| **System mode**         | `ON`     | CMake `CONFIG` — finds installed package in sysroot/SDK/system | Dynamic |

This mirrors the existing `conan` preset pattern exactly:

- `conan` preset → activates `conan_provider.cmake` → 3rd party deps from Conan
- New `system-internal` preset → sets `INTERNAL_DEPS_FROM_SYSTEM=ON` → internal deps from system

### Why a single global switch instead of per-package variables

Per-package cmake variables (e.g., `-DOSAL_GIT_TAG=abc123`) require the developer to remember to pass them on every
configure invocation, cannot be committed to the repo as a lasting state, and are awkward to use from IDEs. A single
global switch is simpler: either you're building against system packages or you're building from source. The only
per-dependency information that matters is the pinned SHA, which lives in the Find module file and is committed as a
normal code change.

### Version pinning and overrides

The `GIT_TAG` in each Find module (`cmake/modules/Findosal.cmake`, etc.) is the authoritative pinned version for repo
mode. To use a different version of an internal dep:

1. Edit the `GIT_TAG` SHA in the relevant `Find<pkg>.cmake` file.
2. Switch to repo mode (use a non-system preset).
3. Rebuild — the custom version is compiled and linked **statically**.

Static linking is intentional for the override case. When debugging a custom osal commit on target hardware, the
developer does not want to copy `.so` files to the device — the custom code is embedded in the binary.

When the custom version is accepted, the SHA change is committed as the new pin. There is no separate "version override"
mechanism.

### Find module pattern

Each Find module for an internal package (Findosal.cmake, Findplatform.cmake, and Findutils.cmake in consumer repos)
implements:

```cmake
if (INTERNAL_DEPS_FROM_SYSTEM)
    # System mode: find installed package via CMake's CONFIG mechanism.
    # CONFIG mode skips Find modules — no recursion risk.
    # In Yocto, CMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY (set by the Yocto
    # toolchain file) automatically scopes this to CMAKE_SYSROOT.
    find_package(osal CONFIG REQUIRED)
    return()
endif ()

# Repo mode: FetchContent from GitHub with pinned SHA.
# To use a different version: edit GIT_TAG and commit.
FetchContent_Declare(osal
    GIT_REPOSITORY  https://github.com/kubasejdak-org/osal.git
    GIT_TAG         dfe8bff5bbe6962372972d23809f4f6a8ca7fe05
    SOURCE_SUBDIR   lib
    SYSTEM
)
FetchContent_MakeAvailable(osal)
include(${osal_SOURCE_DIR}/cmake/components.cmake)
```

The `Findutils.cmake` inside the `utils` repo itself (used for self-referencing when utils components depend on each
other) does NOT implement this pattern — it always points to the repo's own source tree and is only used during
standalone builds of the repo.

### Why CONFIG mode does not recurse

`find_package(osal CONFIG ...)` explicitly requests Config-file mode, which searches for `osalConfig.cmake` /
`osal-config.cmake` in standard CMake locations and the sysroot. It does NOT trigger another search for
`Findosal.cmake`. There is no recursion risk.

---

## CMake Presets

### New hidden preset: `system-internal`

Added to `cmake/presets/dependencies.json` (alongside the existing `conan` preset):

```json
{
    "name": "system-internal",
    "hidden": true,
    "cacheVariables": {
        "INTERNAL_DEPS_FROM_SYSTEM": "ON"
    }
}
```

### Which combinations are practical

The concern with adding a new axis to presets is combinatorial explosion. The key observation is that most
combinations are either nonsensical or not encountered in practice:

| Platform | External deps | Internal deps | Preset pattern | Used? |
| -------- | ------------- | ------------- | -------------- | ----- |
| native | Conan | FetchContent | `linux-native-conan-*` | ✓ dev/CI |
| native | system | FetchContent | `linux-native-*` (existing, no conan) | edge: custom internal on native HW |
| native | system | system | `linux-native-system-*` | ✓ native HW with all pkgs installed |
| native | Conan | system | — | nonsensical (why Conan if system has everything?) |
| arm64 cross | Conan | FetchContent | `linux-arm64-conan-*` | ✓ manual cross-compile |
| arm64 cross | system | system | — | not practical (nobody builds natively on the arm64 target) |
| yocto-sdk | SDK sysroot | FetchContent | `yocto-sdk-*` (existing) | ✓ override a specific internal dep version |
| yocto-sdk | SDK sysroot | SDK sysroot | `yocto-sdk-system-*` | ✓ production-equivalent debugging |
| yocto-sdk | Conan | anything | — | SDK already provides all external deps; Conan redundant |
| yocto full | BitBake | BitBake | (no CMake presets at all) | BitBake handles everything |

This leaves **8 new concrete presets**: 4 for `linux-native-system-*` and 4 for `yocto-sdk-system-*`. No sanitizer
variants are added for system presets; sanitizer testing is a dev/CI concern, not a production-equivalent concern.

### Naming convention

The `-system-` infix in the preset name means "internal deps from system packages." The position in the name after
the platform and before the compiler is consistent:

```
linux-native-system-gcc-debug      ← native HW, system for everything
linux-native-conan-gcc-debug       ← dev/CI, Conan + FetchContent
yocto-sdk-system-gcc-debug         ← SDK, everything from sysroot
yocto-sdk-gcc-debug                ← SDK toolchain, FetchContent for internal (override mode)
```

### New concrete presets

Added to `CMakePresets.json`:

```json
{ "name": "linux-native-system-gcc-debug",    "inherits": ["linux-native-gcc",   "debug",   "system-internal"] },
{ "name": "linux-native-system-gcc-release",  "inherits": ["linux-native-gcc",   "release", "system-internal"] },
{ "name": "linux-native-system-clang-debug",  "inherits": ["linux-native-clang", "debug",   "system-internal"] },
{ "name": "linux-native-system-clang-release","inherits": ["linux-native-clang", "release", "system-internal"] },
{ "name": "yocto-sdk-system-gcc-debug",       "inherits": ["yocto-sdk-gcc",      "debug",   "system-internal"] },
{ "name": "yocto-sdk-system-gcc-release",     "inherits": ["yocto-sdk-gcc",      "release", "system-internal"] },
{ "name": "yocto-sdk-system-clang-debug",     "inherits": ["yocto-sdk-clang",    "debug",   "system-internal"] },
{ "name": "yocto-sdk-system-clang-release",   "inherits": ["yocto-sdk-clang",    "release", "system-internal"] }
```

### Role of existing `linux-native-*` (no conan, no system) presets

These are kept unchanged. They represent "external deps from native system packages, internal deps from GitHub" — a
valid edge case when a developer wants to test a specific internal dep version on native hardware where the external
deps (spdlog etc.) are already installed. The developer edits the SHA in the Find module, uses one of these presets,
and gets the exact internal dep they want, statically linked.

---

## Install Rules (required for Yocto recipes)

For the system mode to work, each internal repo must install proper CMake package files alongside its `.so` and headers.
This is what Yocto recipes call via `cmake --install`.

### Per-component: include path generator expressions

All `target_include_directories` calls must use `BUILD_INTERFACE` / `INSTALL_INTERFACE` generator expressions so that
IMPORTED targets (created from the installed Config files) point to the installed headers, not the build tree:

```cmake
# INTERFACE component
target_include_directories(utils-bits
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Compiled component
target_include_directories(utils-watchdog
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
```

### Per-component: install targets and export sets

```cmake
install(TARGETS utils-bits
    EXPORT utils-bits-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(EXPORT utils-bits-targets
    FILE        utils-bits-targets.cmake
    NAMESPACE   utils::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/utils
)
```

For INTERFACE (header-only) components: `LIBRARY DESTINATION` has no effect (there is no `.so`), but is harmless. The
important outputs are the headers and the targets file, which encodes the IMPORTED target's include paths and transitive
link libraries.

### Top-level: Config and Version files

The top-level `CMakeLists.txt` generates and installs two files that make `find_package(utils CONFIG)` work:

```cmake
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(UTILS_VERSION "1.0.0")

configure_package_config_file(
    cmake/utilsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/utilsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/utils
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/utilsConfigVersion.cmake
    VERSION       ${UTILS_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/utilsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/utilsConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/utils
)
```

### Package Config template: `cmake/utilsConfig.cmake.in`

This file is the entry point for `find_package(utils CONFIG COMPONENTS bits logger)`. It iterates over the requested
components and includes each component's individual targets file, which in turn creates the IMPORTED target (e.g.,
`utils::bits`).

```cmake
@PACKAGE_INIT@

set(_utils_supported_components bits types logger watchdog fsm registry functional preprocessor network info)

foreach (_comp IN LISTS utils_FIND_COMPONENTS)
    if (NOT _comp IN_LIST _utils_supported_components)
        set(utils_FOUND False)
        set(utils_NOT_FOUND_MESSAGE "Unknown component: ${_comp}")
        return()
    endif ()

    set(_comp_targets "${CMAKE_CURRENT_LIST_DIR}/utils-${_comp}-targets.cmake")
    if (EXISTS "${_comp_targets}")
        include("${_comp_targets}")
    elseif (${utils_FIND_REQUIRED_${_comp}})
        set(utils_FOUND False)
        set(utils_NOT_FOUND_MESSAGE "Required component ${_comp} not installed (missing ${_comp_targets})")
        return()
    endif ()
endforeach ()

set(utils_FOUND True)
```

### Installed filesystem layout

After `cmake --install build --prefix /path/to/sysroot/usr`:

```
sysroot/usr/
  include/
    utils/
      bits/endianness.hpp  numerics.hpp  ...
      types/Result.hpp  ...
      logger/Logger.hpp  ...
      watchdog/Watchdog.hpp  ...
      ...
  lib/
    libutils-watchdog.so
    libutils-network.so
    libutils-info.so
    cmake/
      utils/
        utilsConfig.cmake
        utilsConfigVersion.cmake
        utils-bits-targets.cmake        # INTERFACE — no .so, encodes include path
        utils-types-targets.cmake
        utils-logger-targets.cmake      # INTERFACE — also records spdlog::spdlog dep
        utils-watchdog-targets.cmake    # SHARED — references libutils-watchdog.so
        utils-network-targets.cmake
        utils-info-targets.cmake
```

### Transitive dependencies in installed targets

The `utils::logger` IMPORTED target has `spdlog::spdlog` as a transitive interface dependency (recorded in
`utils-logger-targets.cmake`). Consumers of the installed `utils::logger` in system mode therefore also need
`spdlog::spdlog` to be available as an IMPORTED target. In Yocto this is satisfied by the `spdlog` recipe's sysroot
packages. In SDK mode, spdlog is part of the SDK sysroot.

The same applies to `utils::watchdog` (depends on `osal::cpp`) and `utils::network`. All cross-repo dependencies must be
installed in the sysroot before the consuming app's CMake configure step.

### The `info` component

The `info` component runs a custom CMake target (`utils-git-info`) at build time that generates `git.h` containing the
commit hash, branch, and tag of the utils repo at build time. For installed packages, this header is pre-generated and
installed as a normal header:

```cmake
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/git.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/utils/info
)
```

Consumers of the installed `utils::info` see the git metadata from when the Yocto recipe was built. No custom target
propagates through the IMPORTED target.

---

## Yocto Recipes

### Recipe for each internal lib (utils, osal, platform)

Each internal library repo becomes a Yocto recipe that builds all installable components and installs them to the
sysroot.

The `EXCLUDE_FROM_ALL` on compiled components (`watchdog`, `network`, `info`) means they are not part of the default
cmake build target. The recipe builds them explicitly:

```bitbake
# utils_1.0.0.bb
SUMMARY = "utils micro-library collection"
inherit cmake

EXTRA_OECMAKE = "-DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release"
# BitBake sets CMAKE_SYSROOT and CMAKE_FIND_ROOT_PATH_MODE_PACKAGE=ONLY
# DEPENDS = "osal platform spdlog" populates sysroot before this recipe builds
# No CMAKE_PROJECT_TOP_LEVEL_INCLUDES — no conan, no system-internal provider needed
# (no Find modules are involved; CMake finds osal/spdlog via CONFIG in the sysroot)

do_compile:append() {
    cmake --build ${B} --target utils-watchdog utils-network utils-info
}

do_install() {
    cmake_do_install
}

FILES:${PN}     = "${libdir}/libutils-*.so*"
FILES:${PN}-dev = "${includedir}/utils/ ${libdir}/cmake/utils/"
```

### Why INTERNAL_DEPS_FROM_SYSTEM is not needed in recipes

In the recipe context, the app's `cmake/modules/` Find files are only on `CMAKE_MODULE_PATH` if the recipe adds the
app's source directory to it. In practice, BitBake's cmake.bbclass configures CMake directly without the app's cmake
module path being relevant to finding pre-installed deps. CMake looks up `utils`, `osal`, `platform` via standard
CONFIG-file resolution, which finds them from the BitBake-populated sysroot. The Find modules and
`INTERNAL_DEPS_FROM_SYSTEM` variable are a developer-side concern, not a recipe concern.

---

## Developer Workflows

### Pure development / CI (no SDK, no system packages)

```bash
cmake --preset linux-native-conan-gcc-debug
cmake --build build
ctest --test-dir build
```

- Conan provides 3rd-party deps (spdlog, Catch2)
- Find modules take FetchContent path for osal, platform
- Everything compiled from source, statically linked

### Yocto SDK, production-equivalent behavior

```bash
source /opt/poky/<version>/environment-setup-aarch64-poky-linux
cmake --preset yocto-sdk-system-gcc-debug
cmake --build build
```

- SDK toolchain + sysroot active
- `INTERNAL_DEPS_FROM_SYSTEM=ON` → utils/osal/platform resolved from SDK sysroot
- Dynamic linking against SDK `.so` files (matches production)
- Conan is NOT used; 3rd-party deps come from SDK sysroot as well

### Yocto SDK, custom dep version

```bash
# 1. Edit cmake/modules/Findosal.cmake — change GIT_TAG to the desired commit
# 2. Commit the change (this is the new pinned version for the repo)
# 3. Build with non-system SDK preset
source /opt/poky/<version>/environment-setup-aarch64-poky-linux
cmake --preset yocto-sdk-gcc-debug
cmake --build build
# osal is compiled from the specified commit, statically linked into the binary
# No need to copy .so files to target for debugging
```

---

## Scope of Changes

All three changes — Find module update, install rules, Config file — must be replicated in `utils`, `osal`, and
`platform` repos. Changes in consumer app repos: their `Find<pkg>.cmake` files for internal packages get the same
`INTERNAL_DEPS_FROM_SYSTEM` branch.

The change is backward compatible: repo mode (the default) is identical to the current behavior.
