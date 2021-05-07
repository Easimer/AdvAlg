#pragma once

#include <cmath>
#include <utility>
#include <concepts>

namespace hill_climbing {
#if __cplusplus > 201703L
    template<typename Problem, typename Solution>
    concept can_calculate_fitness = requires(Problem p, Solution s) {
        { p.fitness(s) } -> std::convertible_to<float>;
    };

    template<typename Problem, typename Solution>
    concept can_find_best_neighbor = requires(Problem p, Solution s, float epsilon) {
        { p.best_neighbor(s, epsilon) } -> std::same_as<typename Problem::solution>;
    };

    template<typename Problem>
    concept steepest_ascent_solvable = requires(Problem a) {
        typename Problem::solution;
        { a.initial_guess() } -> std::same_as<typename Problem::solution>;
        requires can_calculate_fitness<Problem, typename Problem::solution>;
        requires can_find_best_neighbor<Problem, typename Problem::solution>;
    };
#else
#define steepest_ascent_solvable typename
#endif

    template<steepest_ascent_solvable Problem>
    class steepest_ascent {
    public:
        steepest_ascent(
            Problem &problem,
            float epsilon,
            float minimum_change,
            int max_steps
        ) :
            _problem(problem),
            _epsilon(epsilon),
            _minimum_change(minimum_change),
            _max_steps(max_steps) {
        }

        typename Problem::solution
            optimize() {
            state state;
            auto stuck = false;

            auto p = _problem.initial_guess();

            state.p_fitness = _problem.fitness(p);

            while (!stuck && !should_stop(state)) {
                auto q = _problem.best_neighbor(p, _epsilon);
                auto q_fitness = _problem.fitness(q);
                if (q_fitness < state.p_fitness) {
                    state.p_fitness = q_fitness;
                    p = std::move(q);
                } else {
                    stuck = true;
                }
            }

            return p;
        }

    private:
        struct state {
            int stop_counter = 0;
            float last_fitness = INFINITY;
            float p_fitness;
        };

        bool should_stop(state &state) {
            if (state.last_fitness - state.p_fitness < _minimum_change) {
                state.stop_counter++;
            } else {
                state.stop_counter = 0;
            }

            state.last_fitness = state.p_fitness;
            return state.stop_counter > _max_steps;
        }

    private:
        Problem &_problem;
        float _epsilon;
        float _minimum_change;
        int _max_steps;
    };
}
