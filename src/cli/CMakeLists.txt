# umbra-cli: renders straight to a PNG file, no window.
# Handy for quick checks, benchmarks and for Claude Code to look at results.
add_executable(umbra-cli
    main.cpp
)
# raylib: main.cpp calls SetTraceLogLevel() to keep raylib quiet.
target_link_libraries(umbra-cli PRIVATE umbra_core umbra_io raylib umbra_warnings)

# Tests (ctest). For now just a smoke test; real unit tests arrive in M2.
add_test(NAME cli_smoke
    COMMAND umbra-cli --width 64 --height 36 --out "${CMAKE_BINARY_DIR}/test-output/smoke.png")
