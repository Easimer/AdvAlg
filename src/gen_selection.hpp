#pragma once

#include <vector>

namespace genetic {
#if __cplusplus > 201703L
    // A populaciot tarolo container kovetelmenyei
    template<typename P, typename Solution>
    concept population_container = requires(P a, Solution s) {
        // A populacio tarolo mozgathato
        requires std::movable<P>;
        // A populacio tarolo merete lekerheto
        size(a);
        // A populacio vegere be lehet szurni
        { a.insert(a.end(), s) };
    };

    // Kepes-e a problema egy megoldast mutalni?
    template<typename P, typename Solution>
    concept can_mutate = requires(P a, Solution sol) {
        { a.mutate(sol, 1.0f) };
    };

    // Kepes-e a problema populacioval kapcsolatos muveleteket elvegezni?
    template<typename P, typename Population, typename EvaluatedPopulation>
    concept can_manipulate_populations = requires(P a, Population pop, EvaluatedPopulation eval_pop) {
        { a.init_population() } -> std::convertible_to<Population>;
        { a.evaluate(pop) } -> std::convertible_to<EvaluatedPopulation>;
        { a.select_next_gen(eval_pop) } -> std::convertible_to<std::pair<Population, Population>>;
        { a.select_parents(eval_pop) } -> std::convertible_to<Population>;

        { a.crossover(pop) } -> std::convertible_to<typename P::solution>;

        { a.find_best_in(pop) } -> std::convertible_to<typename P::solution>;

    };

    // Megoldhato-e a P problema genetikus algoritmussal?
    template<typename P>
    concept genetic_solveable = requires(P a) {
        typename P::solution;
        typename P::population;
        requires population_container<typename P::population, typename P::solution>;

        typename P::solution_with_fitness;
        typename P::evaluated_population;
        requires population_container<typename P::evaluated_population, typename P::solution_with_fitness>;

        requires can_manipulate_populations<P, typename P::population, typename P::evaluated_population>;
        requires can_mutate<P, typename P::solution>;
    };
#else
#define genetic_solveable typename
#endif

    template<typename T>
    struct dummy_logger {
        void operator()(int gen, T const &) {}
    };

    template<
        genetic_solveable Problem,
        typename Logger = dummy_logger<typename Problem::solution>>
    class algorithm {
    public:
        algorithm(
            Problem &problem,
            int max_generation,
            float mutation_rate,
            Logger *logger = nullptr
        ) : _problem(problem), _max_generation(max_generation), _mutation_rate(mutation_rate), _logger(logger) {
        }

        typename Problem::population
            optimize() {
            auto pop = _problem.init_population();
            auto pop_fitness = _problem.evaluate(pop);
            state state;

            while (!should_stop(state)) {
                auto [next_gen, mating] = _problem.select_next_gen(pop_fitness);
                auto mating_eval = _problem.evaluate(mating);
                while (size(next_gen) < size(pop)) {
                    auto selected_parents = _problem.select_parents(mating_eval);
                    auto c = _problem.crossover(selected_parents);
                    _problem.mutate(c, _mutation_rate);
                    next_gen.insert(next_gen.end(), std::move(c));
                }
                pop = std::move(next_gen);
                pop_fitness = _problem.evaluate(pop);
                state.generation++;

                if (_logger != nullptr) {
                    auto p_best = _problem.find_best_in(pop);
                    (*_logger)(state.generation, p_best);
                }
            }

            return pop;
        }

        typename Problem::solution
            optimize_best() {
            auto best_pop = optimize();
            auto best_pop_fitness = _problem.evaluate(best_pop);
            return _problem.find_best_in_population(best_pop_fitness);
        }

    private:
        struct state {
            int generation = 0;
        };

    private:
        bool should_stop(state const &state) {
            return state.generation > _max_generation;
        }

    private:
        Problem &_problem;
        int _max_generation;
        float _mutation_rate;
        Logger *_logger;
    };
}