#include <cstdio>
#include "vec2.hpp"

#include "gnuplot.hpp"
#include "hc_stochastic.hpp"
#include "hc_steepest_ascent.hpp"
#include "gen_selection.hpp"
#include "smallest_bound_poly.hpp"
#include "traveling_salesman.hpp"
#include "path_finding_program.hpp"

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

static void hill_climbing_demo() {
    auto points = random_data();
    smallest_bounding_polygon problem(7, points);
    auto epsilon = 0.5f;
    auto minimum_change = 0.01f;
    auto max_steps = 100000;

    auto solver0 = hill_climbing::stochastic(problem, epsilon, minimum_change, max_steps);
    auto solution0 = solver0.optimize();

    auto solver1 = hill_climbing::steepest_ascent(problem, epsilon, minimum_change, max_steps);
    auto solution1 = solver1.optimize();

    auto plot = gnuplot::polygons_and_points {
        { gnuplot::polygon, "HC-Stochastic", solution0 },
        { gnuplot::polygon, "HC-Steepst-Ascent", solution1 },
        { gnuplot::points, "Pontok", points }
    };

    printf("%s\n", plot.str().c_str());
}

struct city {
    float x, y;
};

static float distance(city const &lhs, city const &rhs) {
    auto dx = rhs.x - lhs.x;
    auto dy = rhs.y - lhs.y;
    return std::sqrt(dx * dx + dy * dy);
}

static void print_graph(FILE *f, std::vector<city> const &cities, traveling_salesman<city>::path const &path) {
     fprintf(f, "digraph cities {\n");
     auto N = path.size();
     fprintf(f, "C%zu -> C%zu;\n", path[N - 1], path[0]);
     for (size_t i = 1; i < N; i++) {
         fprintf(f, "C%zu -> C%zu;\n", path[i - 1], path[i - 0]);
     }
     fprintf(f, "\n");
     for (auto idx : path) {
         auto &city = cities[idx];
         fprintf(f, "C%zu [ pos = \"%f,%f!\"];\n", idx, city.x, city.y);
     }
     fprintf(f, "}\n\n");
}

static void genetic_demo() {
    std::vector<city> cities = {
        { 63, 71 },
        { 94, 71 },
        { 142, 370 },
        { 173, 1276 },
        { 205, 1213 },
        { 213, 69 },
        { 244, 69 },
        { 276, 630 },
        { 283, 732 },
        { 362, 69 },
        { 394, 69 },
        { 449, 370 },
        { 480, 1276 },
        { 512, 1213 },
        { 528, 157 },
        { 583, 630 },
        { 591, 732 },
        { 638, 654 },
        { 638, 496 },
        { 638, 314 },
        { 638, 142 },
        { 669, 142 },
        { 677, 315 },
        { 677, 496 },
        { 677, 654 },
        { 709, 654 },
        { 709, 496 },
        { 709, 315 },
        { 701, 142 },
        { 764, 220 },
        { 811, 189 },
        { 843, 173 },
        { 858, 370 },
        { 890, 1276 },
        { 921, 1213 },
        { 992, 630 },
        { 1000, 732 },
        { 1197, 1276 },
        { 1228, 1213 },
        { 1276, 205 },
        { 1299, 630 },
        { 1307, 732 },
        { 1362, 654 },
        { 1362, 496 },
        { 1362, 291 },
        { 1425, 654 },
        { 1425, 496 },
        { 1425, 291 },
        { 1417, 173 },
        { 1488, 291 },
        { 1488, 496 },
        { 1488, 654 },
        { 1551, 654 },
        { 1551, 496 },
        { 1551, 291 },
        { 1614, 291 },
        { 1614, 496 },
        { 1614, 654 },
        { 1732, 189 },
        { 1811, 1276 },
        { 1843, 1213 },
        { 1913, 630 },
        { 1921, 732 },
        { 2087, 370 },
        { 2118, 1276 },
        { 2150, 1213 },
        { 2189, 205 },
        { 2220, 189 },
        { 2220, 630 },
        { 2228, 732 },
        { 2244, 142 },
        { 2276, 315 },
        { 2276, 496 },
        { 2276, 654 },
        { 2315, 654 },
        { 2315, 496 },
        { 2315, 315 },
        { 2331, 142 },
        { 2346, 315 },
        { 2346, 496 },
        { 2346, 654 },
        { 2362, 142 },
        { 2402, 157 },
        { 2402, 220 },
        { 2480, 142 },
        { 2496, 370 },
        { 2528, 1276 },
        { 2559, 1213 },
        { 2630, 630 },
        { 2638, 732 },
        { 2756, 69 },
        { 2787, 69 },
        { 2803, 370 },
        { 2835, 1276 },
        { 2866, 1213 },
        { 2906, 69 },
        { 2937, 69 },
        { 2937, 630 },
        { 2945, 732 },
        { 3016, 1276 },
        { 3055, 69 },
        { 3087, 69 },
        { 606, 220 },
        { 1165, 370 },
        { 1780, 370 },
        { 63, 1402 },
        { 94, 1402 },
        { 142, 1701 },
        { 173, 2607 },
        { 205, 2544 },
        { 213, 1400 },
        { 244, 1400 },
        { 276, 1961 },
        { 283, 2063 },
        { 362, 1400 },
        { 394, 1400 },
        { 449, 1701 },
        { 480, 2607 },
        { 512, 2544 },
        { 528, 1488 },
        { 583, 1961 },
        { 591, 2063 },
        { 638, 1985 },
        { 638, 1827 },
        { 638, 1645 },
        { 638, 1473 },
        { 669, 1473 },
        { 677, 1646 },
        { 677, 1827 },
        { 677, 1985 },
        { 709, 1985 },
        { 709, 1827 },
        { 709, 1646 },
        { 701, 1473 },
        { 764, 1551 },
        { 811, 1520 },
        { 843, 1504 },
        { 858, 1701 },
        { 890, 2607 },
        { 921, 2544 },
        { 992, 1961 },
        { 1000, 2063 },
        { 1197, 2607 },
        { 1228, 2544 },
        { 1276, 1536 },
        { 1299, 1961 },
        { 1307, 2063 },
        { 1362, 1985 },
        { 1362, 1827 },
        { 1362, 1622 },
        { 1425, 1985 },
        { 1425, 1827 },
        { 1425, 1622 },
        { 1417, 1504 },
        { 1488, 1622 },
        { 1488, 1827 },
        { 1488, 1985 },
        { 1551, 1985 },
        { 1551, 1827 },
        { 1551, 1622 },
        { 1614, 1622 },
        { 1614, 1827 },
        { 1614, 1985 },
        { 1732, 1520 },
        { 1811, 2607 },
        { 1843, 2544 },
        { 1913, 1961 },
        { 1921, 2063 },
        { 2087, 1701 },
        { 2118, 2607 },
        { 2150, 2544 },
        { 2189, 1536 },
        { 2220, 1520 },
        { 2220, 1961 },
        { 2228, 2063 },
        { 2244, 1473 },
        { 2276, 1646 },
        { 2276, 1827 },
        { 2276, 1985 },
        { 2315, 1985 },
        { 2315, 1827 },
        { 2315, 1646 },
        { 2331, 1473 },
        { 2346, 1646 },
        { 2346, 1827 },
        { 2346, 1985 },
        { 2362, 1473 },
        { 2402, 1488 },
        { 2402, 1551 },
        { 2480, 1473 },
        { 2496, 1701 },
        { 2528, 2607 },
        { 2559, 2544 },
        { 2630, 1961 },
        { 2638, 2063 },
        { 2756, 1400 },
        { 2787, 1400 },
        { 2803, 1701 },
        { 2835, 2607 },
        { 2866, 2544 },
        { 2906, 1400 },
        { 2937, 1400 },
        { 2937, 1961 },
        { 2945, 2063 },
        { 3016, 2607 },
        { 3055, 1400 },
        { 3087, 1400 },
        { 606, 1551 },
        { 1165, 1701 },
        { 1780, 1701 },
        { 63, 2733 },
        { 94, 2733 },
        { 142, 3032 },
        { 173, 3938 },
        { 205, 3875 },
        { 213, 2731 },
        { 244, 2731 },
        { 276, 3292 },
        { 283, 3394 },
        { 362, 2731 },
        { 394, 2731 },
        { 449, 3032 },
        { 480, 3938 },
        { 512, 3875 },
        { 528, 2819 },
        { 583, 3292 },
        { 591, 3394 },
        { 638, 3316 },
        { 638, 3158 },
        { 638, 2976 },
        { 638, 2804 },
        { 669, 2804 },
        { 677, 2977 },
        { 677, 3158 },
        { 677, 3316 },
        { 709, 3316 },
        { 709, 3158 },
        { 709, 2977 },
        { 701, 2804 },
        { 764, 2882 },
        { 811, 2851 },
        { 843, 2835 },
        { 858, 3032 },
        { 890, 3938 },
        { 921, 3875 },
        { 992, 3292 },
        { 1000, 3394 },
        { 1197, 3938 },
        { 1228, 3875 },
        { 1276, 2867 },
        { 1299, 3292 },
        { 1307, 3394 },
        { 1362, 3316 },
        { 1362, 3158 },
        { 1362, 2953 },
        { 1425, 3316 },
        { 1425, 3158 },
        { 1425, 2953 },
        { 1417, 2835 },
        { 1488, 2953 },
        { 1488, 3158 },
        { 1488, 3316 },
        { 1551, 3316 },
        { 1551, 3158 },
        { 1551, 2953 },
        { 1614, 2953 },
        { 1614, 3158 },
        { 1614, 3316 },
        { 1732, 2851 },
        { 1811, 3938 },
        { 1843, 3875 },
        { 1913, 3292 },
        { 1921, 3394 },
        { 2087, 3032 },
        { 2118, 3938 },
        { 2150, 3875 },
        { 2189, 2867 },
        { 2220, 2851 },
        { 2220, 3292 },
        { 2228, 3394 },
        { 2244, 2804 },
        { 2276, 2977 },
        { 2276, 3158 },
        { 2276, 3316 },
        { 2315, 3316 },
        { 2315, 3158 },
        { 2315, 2977 },
        { 2331, 2804 },
        { 2346, 2977 },
        { 2346, 3158 },
        { 2346, 3316 },
        { 2362, 2804 },
        { 2402, 2819 },
        { 2402, 2882 },
        { 2480, 2804 },
        { 2496, 3032 },
        { 2528, 3938 },
        { 2559, 3875 },
        { 2630, 3292 },
        { 2638, 3394 },
        { 2756, 2731 },
        { 2787, 2731 },
        { 2803, 3032 },
        { 2835, 3938 },
        { 2866, 3875 },
        { 2906, 2731 },
        { 2937, 2731 },
        { 2937, 3292 },
        { 2945, 3394 },
        { 3016, 3938 },
        { 3055, 2731 },
        { 3087, 2731 },
        { 606, 2882 },
        { 1165, 3032 },
        { 1780, 3032 },
        { 1417, -79 },
        { 1496, -79 },
        { 1693, 4055 },
    };

    size_t start_idx = 0;

    char path_buf[64];

    auto logger = [&](int gen, std::vector<size_t> const &best) {
#if __linux__
        if (gen % 5 != 0) {
            return;
        }
        snprintf(path_buf, 63, "path%09d.dot", gen);
        FILE *f = fopen(path_buf, "wb");
        print_graph(f, cities, best);
        fclose(f);
#endif
    };

    auto problem = traveling_salesman(cities, start_idx);
    auto solver = genetic::algorithm<
        decltype(problem),
        decltype(logger)
    >(problem, 100000, 0.001f, &logger);

    auto solutions = solver.optimize();
}

std::vector<path_finding_program::level_tile> load_level(char const *path, int *out_width, int *out_height) {
    FILE *f;

    f = fopen(path, "r");
    if (f == nullptr) {
        fprintf(stderr, "load_level: failed to open '%s' for reading\n", path);
        std::abort();
    }

    std::vector<path_finding_program::level_tile> ret;
    int width = 0;
    int height = 0;

    while (!feof(f)) {
        char ch;
        path_finding_program::level_tile tile;

        fread(&ch, 1, 1, f);

        if (ch == '\n') {
            if (width == 0) {
                width = ret.size();
            }

            height++;
            continue;
        }

        switch (ch) {
        case '#': tile = path_finding_program::TILE_WALL; break;
        case ' ': tile = path_finding_program::TILE_EMPTY; break;
        case 'S': tile = path_finding_program::TILE_START; break;
        case 'X': tile = path_finding_program::TILE_EXIT; break;
        default: fprintf(stderr, "load_level: unknown tile '%c'\n", ch);  std::abort(); break;
        }

        ret.push_back(tile);
    }

    *out_width = width;
    *out_height = height - 1; // subtract last empty line
    fclose(f);

    return ret;
}

static void genetic_programming_demo() {
    path_finding_program::level L;

    auto tiles = load_level("labyrinth0.txt", &L.width, &L.height);
    L.tiles = tiles.data();

    path_finding_program problem(&L);

    auto logger = [&](int gen, path_finding_program::solution const &best) {
    };

    auto solver = genetic::algorithm<
        decltype(problem),
        decltype(logger)
    >(problem, 100000, 0.001f, &logger);

    auto results = solver.optimize(0.0f);

    auto best = problem.find_best_in(results);

    int prev_x = -1;
    int prev_y = -1;
    auto callback = [&](int x, int y) {
        if (prev_x != x || prev_y != y) {
            printf("%d %d\n", x, y);
            prev_x = x;
            prev_y = y;
        }
    };

    printf("Exec'ing best program:\n");
    problem.execute_program(best, callback);
    printf("STOP\n");
}

int main(int argc, char **argv) {
    srand(0);

    // hill_climbing_demo();
    // genetic_demo();
    genetic_programming_demo();

    return 0;
}