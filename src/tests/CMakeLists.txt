# Pinned to the last Catch2 v2 release (not v3): v3 requires C++14 (std::enable_if_t,
# decay_t, etc.), but this project forces -std=gnu++11 globally via CMAKE_CXX_FLAGS,
# which FetchContent'd subprojects inherit too. v2 is C++11-clean and matches the
# project's actual standard.
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v2.13.10
)
FetchContent_MakeAvailable(Catch2)

# afx/resource.cpp (AfxResourceLib, needed transitively by render/gui) is compiled directly
# here rather than pulled in via $<TARGET_OBJECTS:afx> - that object library also contains
# sdlapp.cpp, which defines its own SDL-bootstrap main() and needs an AfxApp factory only
# provided by MACHINES_SOURCES; neither is relevant to, or resolvable by, a unit test binary.
add_executable(machines_tests
    test_mexvec2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../libdev/afx/resource.cpp
)

# Mirrors the `libs` INTERFACE target's object library list (this project's engine is not
# cleanly layered - base/ctl reach into recorder/system for RNG and platform hooks, so a
# hand-picked minimal subset chases undefined references indefinitely), minus `afx` (see above).
target_link_libraries(machines_tests PRIVATE
    Catch2::Catch2
    $<TARGET_OBJECTS:base>
    $<TARGET_OBJECTS:mathex>
    $<TARGET_OBJECTS:utility>
    $<TARGET_OBJECTS:profiler>
    $<TARGET_OBJECTS:system>
    $<TARGET_OBJECTS:device>
    $<TARGET_OBJECTS:world4d>
    $<TARGET_OBJECTS:gxin>
    $<TARGET_OBJECTS:xin>
    $<TARGET_OBJECTS:stdlib>
    $<TARGET_OBJECTS:render>
    $<TARGET_OBJECTS:sound>
    $<TARGET_OBJECTS:network>
    $<TARGET_OBJECTS:ctl>
    $<TARGET_OBJECTS:phys>
    $<TARGET_OBJECTS:sim>
    $<TARGET_OBJECTS:gui>
    $<TARGET_OBJECTS:recorder>
    $<TARGET_OBJECTS:envirnmt>
    $<TARGET_OBJECTS:machphys>
    $<TARGET_OBJECTS:machlog>
    libaudio
    ${LIBS}
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib)
include(Catch)
catch_discover_tests(machines_tests)
