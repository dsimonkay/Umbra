# umbra_tests: unit tests of umbra_core, with Catch2 v3 (Catch2WithMain provides main()).
# One <name>_test.cpp per core file. catch_discover_tests registers every TEST_CASE as its own ctest test.
# Run them in the debug preset: release builds compile the asserts out (-DNDEBUG).
#
#   ctest --preset debug                      # all tests (and the CLI smoke test)
#   ./build/debug/umbra_tests "[scene]"       # only the tests tagged [scene]; -s shows passing checks too
add_executable(umbra_tests
    scene_test.cpp
)
target_link_libraries(umbra_tests PRIVATE umbra_core Catch2::Catch2WithMain umbra_warnings)

include(Catch)
catch_discover_tests(umbra_tests)
