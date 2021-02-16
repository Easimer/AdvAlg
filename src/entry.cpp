#include <cstdio>
#include "vec2.hpp"

#include "hc_stochastic.hpp"
#include "hc_steepest_ascent.hpp"
#include "smallest_bound_poly.hpp"

std::vector<vec2>
test_data() {
    return { {0, 0}, {2, 3}, {-1, 4}, {-3, -6}, {2, 2}, {-1, 3} };
}

std::vector<vec2>
random_data() {
    std::vector<vec2> ret;

    for (int i = 0; i < 50; i++) {
        auto x = ((rand() % 2000) - 1000) / 1000.0f * 40;
        auto y = ((rand() % 2000) - 1000) / 1000.0f * 40;
        ret.push_back({ x, y });
    }

    return ret;
}

static void print_gnuplot_header() {
    printf("set style increment user\n");
    printf("plot '-' title 'Pontok' with points ps 2,\\\n\
                 '-' title 'HC-Stochastic' with linespoints ls 3,\\\n\
                 '-' title 'HC-Steepest-Ascent' with linespoints ls 4\n\
    ");
}

static void print_gnuplot_points(
    std::vector<vec2> const &points) {
    printf("# Points\n");
    printf("# X Y\n");
    for (auto &pt : points) {
        printf("%f %f\n\n", pt.x, pt.y);
    }
    printf("e\n");
}

static void print_gnuplot_polygon(
    smallest_bounding_polygon::solution const &polygon) {
    auto const N = polygon.size();

    for (int i = 0; i < N + 1; i++) {
        auto &p = polygon[i % N];
        printf("%f %f\n", p.x, p.y);
    }
    printf("e\n");
}

int main(int argc, char **argv) {
    srand(0);

    auto points = random_data();
    smallest_bounding_polygon problem(7, points);
    auto epsilon = 0.5f;
    auto minimum_change = 0.01f;
    auto max_steps = 100000;

    print_gnuplot_header();
    print_gnuplot_points(points);

    {
        auto solver = hill_climbing::stochastic(problem, epsilon, minimum_change, max_steps);
        auto solution = solver.optimize();
        print_gnuplot_polygon(solution);
    }
    {
        auto solver = hill_climbing::steepest_ascent(problem, epsilon, minimum_change, max_steps);
        auto solution = solver.optimize();
        print_gnuplot_polygon(solution);
    }

    return 0;
}