# DPLDs
add_executable(
    dplds nanobench.cpp dplds.cpp
)

target_link_libraries(
    dplds PRIVATE tsl
)

target_link_libraries(
    dplds PRIVATE teddy
)

target_include_directories(
    dplds PRIVATE ${PROJECT_SOURCE_DIR}/lib
)

target_compile_options(
    dplds PRIVATE ${LIBTEDDY_COMPILE_OPTIONS}
)

target_link_options(
    dplds PRIVATE ${LIBTEDDY_LINK_OPTIONS}
)

# time-probs
if(LIBTEDDY_SYMBOLIC_RELIABILITY)
    add_executable(
        time-probs nanobench.cpp time_probs.cpp
    )

    target_link_libraries(
        time-probs PRIVATE tsl
    )

    target_link_libraries(
        time-probs PRIVATE teddy
    )

    target_include_directories(
        time-probs PRIVATE ${PROJECT_SOURCE_DIR}/lib
    )

    target_compile_options(
        time-probs PRIVATE ${LIBTEDDY_COMPILE_OPTIONS}
    )

    target_link_options(
        time-probs PRIVATE ${LIBTEDDY_LINK_OPTIONS}
    )
endif()

# graphrel-scaling — LBL vs BDD scaling benchmark
# Requires both LIBTEDDY_BUILD_EXPERIMENTS and LIBTEDDY_BUILD_GRAPHREL
if(LIBTEDDY_BUILD_GRAPHREL)
    add_executable(
        graphrel-scaling graphrel_scaling.cpp
    )

    target_link_libraries(
        graphrel-scaling PRIVATE teddy::graphrel
    )

    # Internal src/ include so lbl_detail.hpp (lbl_calculator + statistics) is accessible
    target_include_directories(
        graphrel-scaling PRIVATE ${PROJECT_SOURCE_DIR}/libgraphrel/src
    )

    target_compile_options(
        graphrel-scaling PRIVATE ${LIBTEDDY_COMPILE_OPTIONS}
    )

    target_link_options(
        graphrel-scaling PRIVATE ${LIBTEDDY_LINK_OPTIONS}
    )
endif()