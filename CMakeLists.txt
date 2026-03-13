cmake_minimum_required(VERSION 3.14)
project(spaghetti_sort LANGUAGES CXX)

# ── Require C++17 (for std::optional, structured bindings, etc.) ──────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(spaghetti_sort spaghetti_sort.cpp)

# ── Link pthreads (required for std::thread on Linux/macOS) ──────────────────
find_package(Threads REQUIRED)
target_link_libraries(spaghetti_sort PRIVATE Threads::Threads)

# ── Compiler warnings ─────────────────────────────────────────────────────────
target_compile_options(spaghetti_sort PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# ── Optional: Release build optimisations ────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
