add_library(component_voltage_sense STATIC
    voltage_sense.c
)

target_include_directories(component_voltage_sense PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(component_voltage_sense PUBLIC
    c_std_11
)

target_link_libraries(project_components INTERFACE
    component_voltage_sense
)

if(BUILD_TESTING)
    add_executable(test_voltage_sense
        Tests/test_voltage_sense.c
    )

    target_link_libraries(test_voltage_sense PRIVATE
        component_voltage_sense
        unity
    )

    target_compile_options(test_voltage_sense PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    add_test(
        NAME voltage_sense
        COMMAND test_voltage_sense
    )
endif()
