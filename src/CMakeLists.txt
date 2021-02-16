set(SRC
    entry.cpp

    hc_stochastic.hpp
    hc_steepest_ascent.hpp

    smallest_bound_poly.hpp

    vec2.hpp
)

add_executable(halal ${SRC})

set_property(TARGET halal PROPERTY CXX_STANDARD 17)
set_property(TARGET halal PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET halal PROPERTY CXX_EXTENSION OFF)