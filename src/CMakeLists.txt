set(SRC_HEADERS
    hc_stochastic.hpp
    hc_steepest_ascent.hpp
	gen_selection.hpp
    particle_swarm_optimization.hpp

    smallest_bound_poly.hpp
    traveling_salesman.hpp
    path_finding_program.hpp
    function_approximation.hpp

    vec2.hpp
 )

macro(add_solution TARGET ENTRY_FILE)
    add_executable(${TARGET} ${SRC_HEADERS} ${ENTRY_FILE})
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
    set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD_REQUIRED ON)
    set_property(TARGET ${TARGET} PROPERTY CXX_EXTENSION OFF)

    if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
        target_compile_options(${TARGET} PUBLIC "/Zc:__cplusplus")
    endif()
endmacro()

add_solution(genprog_pathfind entry_genprog_pathfind.cpp)
add_solution(pso_funcapprox entry_pso_funcapprox.cpp)
add_solution(hillclimb_polygon entry_hillclimb_polygon.cpp)
add_solution(genetic_travelingsalesman entry_genetic_travelingsalesman.cpp)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/labyrinth0.txt ${CMAKE_CURRENT_BINARY_DIR}/labyrinth0.txt COPYONLY)