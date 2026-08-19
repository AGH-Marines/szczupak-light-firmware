add_library(component_current_sense STATIC
    current_sense.c
)

target_include_directories(component_current_sense PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(component_current_sense PUBLIC
    c_std_11
)

target_link_libraries(project_components INTERFACE
    component_current_sense
)

if(BUILD_TESTING)
    add_executable(test_current_sense
        Tests/test_current_sense.c
    )

    target_link_libraries(test_current_sense PRIVATE
        component_current_sense
        unity
    )

    target_compile_options(test_current_sense PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    add_test(
        NAME current_sense
        COMMAND test_current_sense
    )
endif()
