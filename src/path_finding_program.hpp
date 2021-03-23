#pragma once

#include <cassert>
#include <random>
#include <vector>

class path_finding_program {
public:
    enum level_tile {
        TILE_WALL,
        TILE_EMPTY,
        TILE_START,
        TILE_EXIT,
    };

    struct level {
        level_tile const *tiles;
        int width, height;
    };

    enum operation {
        OP_RELJMP = 0,
        OP_MOVFWD,
        OP_LTURN,
        OP_RTURN,
        OP_SKIPWALL,
        OP_MAX
    };

    struct instruction {
        operation op;
        int param;
    };

    using program = std::vector<instruction>;
    using solution = program;
    using population = std::vector<program>;
    using solution_with_fitness = std::pair<program, float>;
    using evaluated_population = std::vector<solution_with_fitness>;

    path_finding_program(level const *level) : _level(level) {
        _exit_x = _exit_y = _start_x = _start_y = -1;

        for (int y = 0; y < _level->height; y++) {
            for (int x = 0; x < _level->width; x++) {
                auto t = _level->tiles[y * _level->width + x];
                if (t == TILE_EXIT) {
                    _exit_x = x;
                    _exit_y = _level->height - 1 - y;
                }
                if (t == TILE_START) {
                    _start_x = x;
                    _start_y = _level->height - 1 - y;
                }
            }
        }

        assert(_exit_x > 0 && _exit_y > 0 && _start_x > 0 && _start_y > 0);
    }

    population init_population() {
        population ret;

        for (int i = 0; i < 50; i++) {
            ret.emplace_back(random_program());
        }

        return ret;
    }

    evaluated_population evaluate(population const &pop) {
        evaluated_population ret;

        for (auto &solution : pop) {
            ret.emplace_back(std::make_pair(solution, fitness(solution)));
        }

        std::sort(ret.begin(), ret.end(), [](auto &lhs, auto &rhs) { return lhs.second < rhs.second; });

        return ret;
    }

    float fitness(program const &program) {
        // exec program
        auto callback = [](int x, int y) {};
        auto result = execute_program(program, callback);
        auto dx = _exit_x - result.x;
        auto dy = _exit_y - result.y;
        float dist = sqrtf(dx * dx + dy * dy);

        return dist;
    }

    float average_fitness(evaluated_population const &pop) {
        // TODO: ugyanaz, mint a traveling_salesman-ben
        float sum = 0;
        int n = 0;

        for (auto &sf : pop) {
            sum += sf.second;
            n++;
        }

        return sum / n;
    }

    std::pair<population, population> select_next_gen(evaluated_population const &pop) {
        // TODO: ugyanaz, mint a traveling_salesman-ben
        auto n_solutions = pop.size();
        auto n_elite = n_solutions / 8;

        population elite;
        population mating;

        auto avg_fit = average_fitness(pop);

        for (size_t i = 0; i < n_solutions; i++) {
            if (i < n_elite) {
                elite.push_back(pop[i].first);
                mating.push_back(pop[i].first);
            } else {
                if (pop[i].second >= avg_fit) {
                    mating.push_back(pop[i].first);
                }
            }
        }

        return { std::move(elite), std::move(mating) };
    }

    population select_parents(evaluated_population const &pop) {
        // TODO: ugyanaz, mint a traveling_salesman-ben
        auto k = 8;
        auto N = pop.size();
        std::vector<size_t> parent_indices;

        std::uniform_int_distribution<size_t> dist_index(0, N - 1);
        auto generate_random_index = std::bind(dist_index, _rand);

        // Ket szulot keresunk
        while (parent_indices.size() < 2) {
            auto subject_idx = generate_random_index();
            int duels_played = 0;
            int duels_won = 0;

            // k darab parbajt kell lejatszania
            while (duels_played < k) {
                // keresunk egy ellenfelet (aki nem az alany)
                auto contender = subject_idx;
                while (contender == subject_idx) {
                    contender = generate_random_index();
                }

                if (pop[subject_idx].second < pop[contender].second) {
                    duels_won++;
                }
                duels_played++;
            }

            // Ha megnyert minden parbajt, akkor szaporodhat
            if (duels_won == k) {
                parent_indices.push_back(subject_idx);
            }
        }

        population ret;
        std::transform(
            parent_indices.begin(), parent_indices.end(),
            std::back_inserter(ret),
            [&](auto i) { return pop[i].first; }
        );
        return ret;
    }

    program crossover(population const &parents) {
        program ret;
        auto &p0 = parents[0];
        auto &p1 = parents[1];

        auto dist_p0_idx = std::uniform_int_distribution(size_t(0), p0.size() - 1);
        auto dist_p1_idx = std::uniform_int_distribution(size_t(0), p1.size() - 1);

        auto idx_p0 = dist_p0_idx(_rand);
        auto idx_p1 = dist_p1_idx(_rand);

        ret.insert(ret.end(), p0.begin(), p0.begin() + idx_p0);
        ret.insert(ret.end(), p1.begin() + idx_p1, p1.end());

        return ret;
    }

    solution find_best_in(population const &pop) {
        auto pop_fit = evaluate(pop);
        auto best = pop_fit[0];
        printf("top fitness: %f | avg fitness: %f\n", best.second, average_fitness(pop_fit));
        return best.first;
    }

    void mutate(solution &prog, float chance) {
    }

    instruction random_instruction() {
        auto dist_op = std::uniform_int_distribution(int(OP_RELJMP), int(OP_MAX - 1));
        auto op = dist_op(_rand);
        auto dist_param = std::uniform_int_distribution(-99, 99);
        auto param = dist_param(_rand);

        return { operation(op), param };
    }

    program random_program() {
        program ret;

        auto dist_prog_len = std::uniform_int_distribution(program_min_length, program_max_length);
        ret.resize(dist_prog_len(_rand));

        for (auto &instr : ret) {
            instr = random_instruction();
        }

        return ret;
    }

    struct program_state {
        int x = 0;
        int y = 0;
        int dx = 1;
        int dy = 0;
        bool flag_exit = false;
        bool flag_bump = false;
        int pc = 0;
        int steps = 0;
    };

    template<typename Callback>
    program_state execute_program(program const &P, Callback const& callback) {
        program_state state;

        state.x = _start_x;
        state.y = _start_y;

        /*
        printf("====================\n");
        printf("DISASSEMBLY\n");
        for (auto &instr : P) {
            char const *mnemonic;
            int param;
            disassemble(instr, mnemonic, param);
            printf("  %s %d\n", mnemonic, param);
        }
        printf("====================\n");
        */

        while (state.steps < 1000 && state.pc >= 0 && state.pc < P.size()) {
            // Fetch
            auto &instr = P[state.pc];
            state.steps++;

            // Execute
            auto next_pc = state.pc + 1;
            switch (instr.op) {
            case OP_RELJMP:
                next_pc = state.pc + instr.param;
                break;
            case OP_MOVFWD:
                if (!state.flag_bump) {
                    state.x += state.dx;
                    state.y += state.dy;
                }
                break;
            case OP_LTURN:
                rotate_left(state);
                break;
            case OP_RTURN:
                rotate_right(state);
                break;
            case OP_SKIPWALL:
                if (state.flag_bump) {
                    next_pc = state.pc + 2;
                }
                break;
            }

            if (sample_level(state.x, state.y) == TILE_EXIT) {
                state.flag_exit = true;
            }

            if (sample_level(state.x + state.dx, state.y + state.dy) == TILE_WALL) {
                state.flag_bump = true;
            } else {
                state.flag_bump = false;
            }

            callback(state.x, state.y);

            state.pc = next_pc;

            if (state.flag_exit) {
                break;
            }
        }

        if (state.steps > 1000) {
            // Couldn't finish maze within 1k steps
            // Set position to a real far away coordinate so this program gets
            // assigned a really high fitness value
            state.x = -10000;
            state.y = -10000;
        }

        return state;
    }

    void rotate_left(program_state &state) {
        int dx1 = -state.dy;
        int dy1 = state.dx;
        state.dx = dx1;
        state.dy = dy1;
    }

    void rotate_right(program_state &state) {
        int dx1 = state.dy;
        int dy1 = -state.dx;
        state.dx = dx1;
        state.dy = dy1;
    }

    level_tile sample_level(int x, int y) {
        if (x < 0 || x >= _level->width || y < 0 || y >= _level->height) {
            return TILE_WALL;
        }

        auto row = _level->height - 1 - y;
        auto col = x;

        return _level->tiles[row * _level->width + col];
    }

    void disassemble(instruction const &I, char const *&mnemonic, int &param) {
        param = 0;
        switch (I.op) {
        case OP_RELJMP  : mnemonic = "reljmp"; param = I.param; break;
        case OP_MOVFWD  : mnemonic = "movfwd"; break;
        case OP_LTURN   : mnemonic = "lturn"; break;
        case OP_RTURN   : mnemonic = "rturn"; break;
        case OP_SKIPWALL: mnemonic = "skipwall"; break;
        }
    }

private:
    const size_t program_min_length = 4;
    const size_t program_max_length = 20;

    level const *_level;
    int _start_x;
    int _start_y;
    int _exit_x;
    int _exit_y;

    std::mt19937 _rand;
};