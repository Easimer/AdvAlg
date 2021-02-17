set(SRC
    entry.cpp

    hc_stochastic.hpp
    hc_steepest_ascent.hpp

    smallest_bound_poly.hpp

    vec2.hpp
)

add_executable(halal ${SRC})

set_property(TARGET halal PROPERTY CXX_STANDARD 20)
set_property(TARGET halal PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET halal PROPERTY CXX_EXTENSION OFF)

if ((MSVC) AND (MSVC_VERSION GREATER_EQUAL 1914))
    target_compile_options(halal PUBLIC "/Zc:__cplusplus")
endif()