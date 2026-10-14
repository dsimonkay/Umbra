# umbra: the GUI application.
add_executable(umbra
    main.cpp
    command_line_options.cpp
    render_job.cpp
    settings.cpp
    texture.cpp
    ui_scale.cpp
    panels/about_dialog.cpp
    panels/dockspace.cpp
    panels/main_menu.cpp
    panels/render_panel.cpp
    panels/status_bar.cpp
    panels/viewport_panel.cpp
)
# Everything this target #includes directly is listed here, even what would also arrive through
# another target (imgui and raylib come with umbra_ui, too): Bazel's layering_check, by hand.
target_link_libraries(umbra PRIVATE
    umbra_core umbra_io umbra_ui
    imgui raylib rlimgui
    umbra_warnings)
target_compile_definitions(umbra PRIVATE UMBRA_VERSION="${PROJECT_VERSION}")

# The UI font lives next to the executable (assets/), see src/ui/theme.cpp.
add_custom_command(TARGET umbra POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:umbra>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${imgui_SOURCE_DIR}/misc/fonts/Roboto-Medium.ttf"
            "$<TARGET_FILE_DIR:umbra>/assets/Roboto-Medium.ttf"
    VERBATIM)
