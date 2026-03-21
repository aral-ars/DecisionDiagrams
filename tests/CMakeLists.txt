find_package(
    Boost
    COMPONENTS unit_test_framework REQUIRED
)

find_package(
    fmt REQUIRED
)

add_executable(
    libteddy-test
        main.cpp
        core.test.cpp
        reliability.test.cpp
)

target_link_libraries(
    libteddy-test
    PRIVATE teddy
    PRIVATE tsl
    PRIVATE fmt::fmt
    PRIVATE ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
)

target_compile_options(
    libteddy-test
    PRIVATE ${LIBTEDDY_COMPILE_OPTIONS}
)

target_link_options(
    libteddy-test
    PRIVATE ${LIBTEDDY_LINK_OPTIONS}
)

add_test(
    NAME    teddy-test-core
    COMMAND libteddy-test --run_test=core_test
)

add_test(
    NAME    teddy-test-reliability
    COMMAND libteddy-test --run_test=reliability_test
)

if(LIBTEDDY_BUILD_GRAPHREL)
    add_executable(
        graphrel-test
            graphrel.test.cpp
    )

    target_link_libraries(
        graphrel-test
        PRIVATE teddy::graphrel
        PRIVATE ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
    )

    target_include_directories(
        graphrel-test
        PRIVATE ${CMAKE_SOURCE_DIR}/libgraphrel/src
    )

    target_compile_options(
        graphrel-test
        PRIVATE ${LIBTEDDY_COMPILE_OPTIONS}
    )

    target_link_options(
        graphrel-test
        PRIVATE ${LIBTEDDY_LINK_OPTIONS}
    )

    add_test(
        NAME    teddy-test-graphrel
        COMMAND graphrel-test --run_test=graphrel_test
    )

    add_test(
        NAME    teddy-test-graphrel-wusun
        COMMAND graphrel-test --run_test=wusun_test
    )

    add_test(
        NAME    teddy-test-graphrel-unit
        COMMAND graphrel-test --run_test=graphrel_unit_test
    )

    add_test(
        NAME    teddy-test-graphrel-integration
        COMMAND graphrel-test --run_test=graphrel_integration_test
    )
endif()