#include <cstdio>
#include <vector>

#include "vec2.hpp"
#include "gnuplot.hpp"

#include "hc_stochastic.hpp"
#include "hc_steepest_ascent.hpp"
#include "smallest_bound_poly.hpp"

std::vector<vec2> test_data() {
    return { {0, 0}, {2, 3}, {-1, 4}, {-3, -6}, {2, 2}, {-1, 3} };
}

std::vector<vec2> random_data() {
    std::vector<vec2> ret;

    for (int i = 0; i < 50; i++) {
        auto x = ((rand() % 2000) - 1000) / 1000.0f * 40;
        auto y = ((rand() % 2000) - 1000) / 1000.0f * 40;
        ret.push_back({ x, y });
    }

    return ret;
}

int main(int argc, char **argv) {
    auto points = random_data();
    smallest_bounding_polygon problem(7, points);
    auto epsilon = 0.5f;
    auto minimum_change = 0.01f;
    auto max_steps = 100000;

    auto solver0 = hill_climbing::stochastic(problem, epsilon, minimum_change, max_steps);
    auto solution0 = solver0.optimize();

    auto solver1 = hill_climbing::steepest_ascent(problem, epsilon, minimum_change, max_steps);
    auto solution1 = solver1.optimize();

    auto plot = gnuplot::polygons_and_points{
        { gnuplot::polygon, "HC-Stochastic", solution0 },
        { gnuplot::polygon, "HC-Steepst-Ascent", solution1 },
        { gnuplot::points, "Pontok", points }
    };

    printf("%s\n", plot.str().c_str());

    return 0;
}