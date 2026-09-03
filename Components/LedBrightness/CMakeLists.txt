add_library(component_led_brightness STATIC
    led_brightness.c
)

target_include_directories(component_led_brightness PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(component_led_brightness PUBLIC
    c_std_11
)

target_link_libraries(project_components INTERFACE
    component_led_brightness
)

if(BUILD_TESTING)
    add_executable(test_led_brightness
        Tests/test_led_brightness.c
    )

    target_link_libraries(test_led_brightness PRIVATE
        component_led_brightness
        unity
    )

    target_compile_options(test_led_brightness PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    add_test(
        NAME led_brightness
        COMMAND test_led_brightness
    )
endif()
