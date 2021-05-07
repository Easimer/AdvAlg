#pragma once

#include <vector>
#include <deque>
#include <random>
#include <iterator>
#include <functional>

template<typename City>
class traveling_salesman_program {
public:
	enum operation {
		OP_RELJMP_NC,
		OP_PREPEND_NEXT,
		OP_APPEND_NEXT,
		OP_ROTATE_LEFT,
		OP_ROTATE_RIGHT,
		OP_SWAP,
		OP_PREVPERM,
		OP_NEXTPERM,
		OP_MAX
	};

	struct instruction {
		operation op;
		size_t x, y;
	};

	using program = std::vector<instruction>;
	using solution = program;
	using population = std::vector<program>;
	using solution_with_fitness = std::pair<program, float>;
	using evaluated_population = std::vector<solution_with_fitness>;

    traveling_salesman_program(std::vector<City> const &cities, size_t start_idx) : _cities(cities), _start_idx(start_idx) {
    }

	population init_population() {
		population ret;

		for (int i = 0; i < num_min_population; i++) {
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

        if (!result.flag_complete) {
            return INFINITY;
        }
        
        auto path = result.path;
        path.push_front(_start_idx);
        float total_distance = 0;
        auto N = path.size();

        if (N > 1) {
            for (size_t i = 0; i < N; i++) {
                total_distance += distance(
                    _cities[path[(i + 0)]],
                    _cities[path[(i + 1) % N]]
                );
            }

            return total_distance;
        } else {
            return INFINITY;
        }
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

        if (mating.size() < num_min_population / 2) {
            for (size_t i = 0; i < num_min_population / 4; i++) {
                mating.emplace_back(random_program());
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
        auto roll = std::uniform_real_distribution(0.f, 1.f)(_rand);
        if (roll < chance) {
            std::uniform_int_distribution<size_t> dist_pidx(0, prog.size() - 1);
            auto i0 = dist_pidx(_rand);
            auto i1 = dist_pidx(_rand);
            std::swap(prog[i0], prog[i1]);
        }
    }

    instruction random_instruction() {
        auto dist_op = std::uniform_int_distribution(int(OP_RELJMP_NC), int(OP_MAX - 1));
        auto op = dist_op(_rand);
        auto dist_param = std::uniform_int_distribution<size_t>(0, last_idx());
        auto param_x = dist_param(_rand);
        auto param_y = dist_param(_rand);

        return { operation(op), param_x, param_y };
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
        std::deque<size_t> path;
        bool flag_complete = false;
        int pc = 0;
        int steps = 0;
    };

    template<typename Callback>
    program_state execute_program(program const &P, Callback const &callback) {
        program_state state;

        size_t next_city_index = 0;

        while (state.steps < 1000 && state.pc >= 0 && state.pc < P.size()) {
            // Fetch
            auto &instr = P[state.pc];
            state.steps++;

            // Execute
            auto next_pc = state.pc + 1;
            switch (instr.op) {
                case OP_RELJMP_NC:
                {
                    if (!state.flag_complete) {
                        auto rel = unsigned(instr.x);
                        next_pc = state.pc - rel;
                        if (next_pc < 0) {
                            next_pc = 0;
                        }
                    }
                    break;
                }
                case OP_PREPEND_NEXT:
                {
                    if (next_city_index == _start_idx) {
                        next_city_index++;
                    }
                    if (next_city_index <= last_idx()) {
                        state.path.push_front(next_city_index);
                        next_city_index++;
                    } else {
                        state.flag_complete = true;
                    }
                    break;
                }
                case OP_APPEND_NEXT:
                {
                    if (next_city_index == _start_idx) {
                        next_city_index++;
                    }
                    if (next_city_index <= last_idx()) {
                        state.path.push_back(next_city_index);
                        next_city_index++;
                    } else {
                        state.flag_complete = true;
                    }
                    break;
                }
                case OP_ROTATE_LEFT:
                {
                    if (state.path.size() > 0) {
                        auto l = state.path.front();
                        state.path.pop_front();
                        state.path.push_back(l);
                    }
                    break;
                }
                case OP_ROTATE_RIGHT:
                {
                    if (state.path.size() > 0) {
                        auto r = state.path.back();
                        state.path.pop_back();
                        state.path.push_front(r);
                    }
                    break;
                }
                case OP_SWAP:
                {
                    if (state.path.size() > 0) {
                        auto x = instr.x;
                        auto y = instr.y;
                        x = x % state.path.size();
                        y = y % state.path.size();
                        std::swap(state.path[x], state.path[y]);
                    }
                    break;
                }
                case OP_PREVPERM:
                    if (state.path.size() > 0) {
                        std::prev_permutation(state.path.begin(), state.path.end());
                    }
                    break;
                case OP_NEXTPERM:
                    if (state.path.size() > 0) {
                        std::next_permutation(state.path.begin(), state.path.end());
                    }
                    break;
            }

            state.pc = next_pc;
        }

        return state;
    }

    void disassemble(instruction const &I, char const *&mnemonic, size_t &param0, size_t &param1) {
        param0 = 0;
        param1 = 0;
        switch (I.op) {
            case OP_RELJMP_NC: mnemonic = "jmp.nc"; param0 = I.x; break;
            case OP_PREPEND_NEXT: mnemonic = "add.front"; break;
            case OP_APPEND_NEXT: mnemonic = "add.back"; break;
            case OP_ROTATE_LEFT: mnemonic = "rot.l"; break;
            case OP_ROTATE_RIGHT: mnemonic = "rot.r"; break;
            case OP_SWAP: mnemonic = "swap"; param0 = I.x; param1 = I.y; break;
            case OP_PREVPERM: mnemonic = "perm.prev"; break;
            case OP_NEXTPERM: mnemonic = "perm.next"; break;
        }
    }

    void print_program(solution const &P) {
        printf("====================\n");
        printf("DISASSEMBLY\n");
        for (auto &instr : P) {
            char const *mnemonic;
            size_t param0, param1;
            disassemble(instr, mnemonic, param0, param1);
            printf("  %s %zu, %zu\n", mnemonic, param0, param1);
        }
        printf("====================\n");
    }
protected:
	size_t last_idx() {
		return _cities.size() - 1;
	}

private:
	size_t _start_idx;
	std::vector<City> _cities;
    std::mt19937 _rand;

    const size_t program_min_length = 32;
    const size_t program_max_length = 128;
    const size_t num_min_population = 100;
};