#include <cstdio>
#include "vec2.hpp"

#include "gen_selection.hpp"
#include "path_finding_program.hpp"
#include "particle_swarm_optimization.hpp"
#include "function_approximation.hpp"

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

int main(int argc, char **argv) {
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

    return 0;
}
