set(SRC
    entry.cpp

    hc_stochastic.hpp
    hc_steepest_ascent.hpp
	gen_selection.hpp

    smallest_bound_poly.hpp
    traveling_salesman.hpp
    path_finding_program.hpp

    vec2.hpp
 )

add_executable(halal ${SRC})

set_property(TARGET halal PROPERTY CXX_STANDARD 20)
set_property(TARGET halal PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET halal PROPERTY CXX_EXTENSION OFF)

if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
    target_compile_options(halal PUBLIC "/Zc:__cplusplus")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/labyrinth0.txt ${CMAKE_CURRENT_BINARY_DIR}/labyrinth0.txt COPYONLY)