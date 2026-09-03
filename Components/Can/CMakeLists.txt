add_library(component_can STATIC
    can.c
    Generated/can_generated.c
)

target_include_directories(component_can 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Generated
)

target_compile_features(component_can PUBLIC
    c_std_11
)

target_link_libraries(project_components INTERFACE
    component_can
)

if(BUILD_TESTING)
    add_executable(test_can
        Tests/test_can.c
    )

    target_link_libraries(test_can PRIVATE
        component_can
        unity
    )

    target_compile_options(test_can PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    add_test(
        NAME can
        COMMAND test_can
    )
endif()
