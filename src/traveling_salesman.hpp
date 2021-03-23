#pragma once

#include <cassert>
#include <utility>
#include <vector>
#include <algorithm>
#include <numeric>
#include <unordered_set>
#include <random>
#include <functional>
#include <iterator>

template<typename City>
class traveling_salesman {
public:
    using path = std::vector<size_t>;
    using solution = path;
    using population = std::vector<path>;
    using solution_with_fitness = std::pair<path, float>;
    using evaluated_population = std::vector<solution_with_fitness>;

    traveling_salesman(std::vector<City> cities, size_t start_idx)
        : _cities(std::move(cities)), _start_idx(start_idx) {
    }

    population init_population() {
        population ret;

        auto n_cities = _cities.size();

        // Sablon utvonal, ahol a kiindulasi ponttol kezdve sorban
        // bejarjuk a varosokat
        path p_template;
        p_template.push_back(_start_idx);
        for (size_t city_idx = 0; city_idx < n_cities; city_idx++) {
            if (city_idx != _start_idx) {
                p_template.push_back(city_idx);
            }
        }

        for (int i = 0; i < 64; i++) {
            // Hanyadik permutaciojat szeretnenk a sablon utvonalnak
            auto n_permutations = rand() % 100;
            auto p = p_template;

            while (n_permutations > 0) {
                std::next_permutation(p.begin() + 1, p.end());
                n_permutations--;
            }

            ret.push_back(std::move(p));
        }

        return ret;
    }

    float total_distance(path const &p) {
        float ret = 0;
        for (size_t i = 1; i < p.size(); i++) {
            ret += distance(
                _cities[p[i - 1]],
                _cities[p[i - 0]]
            );
        }
        return ret;
    }

    evaluated_population evaluate(population const &pop) {
        evaluated_population ret;
        std::transform(
            pop.begin(), pop.end(),
            std::back_inserter(ret),
            [&](solution const &s) {
                return std::make_pair(s, total_distance(s));
            }
        );
        std::sort(ret.begin(), ret.end(), [](auto &lhs, auto &rhs) { return lhs.second < rhs.second; });
        return ret;
    }

    float average_fitness(evaluated_population const &pop) {
        float sum = 0;
        int n = 0;

        for (auto &sf : pop) {
            sum += sf.second;
            n++;
        }

        return sum / n;
    }

    std::pair<population, population>
        select_next_gen(evaluated_population const &pop) {
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

    population select_parents(evaluated_population &pop) {
        auto k = 16;
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

    path crossover(population const &pop) {
        path ret;
        auto &p0 = pop[0];
        auto &p1 = pop[1];

        auto N = p0.size();

        std::uniform_int_distribution<size_t> dist_index(0, N - 1);
        auto generate_random_index = std::bind(dist_index, _rand);

        // p0-bol atmasoljuk a [first, last] indexu alszekvenciat (p0');
        // p1-bol atmasoljuk azokat az elemeket, amelyek nincsenek benne
        // a fenti alszekvenciaban.

        auto first = generate_random_index();
        std::uniform_int_distribution<size_t> dist_sequence_size(0, N - first - 1);
        auto last = first + dist_sequence_size(_rand);

        assert(first >= 0 && first < N);
        assert(last >= 0 && last < N);
        assert(last - first >= 0);
        assert(last - first <= N);

        auto p0_first = p0.begin() + first;
        auto p0_end = p0.begin() + last + 1;

        // Minden varis indexre egy flag, amely azt jelzi, hogy benne van-e a
        // p0-bol kivagott reszben.
        std::vector<bool> p0_elements(N);
        for (auto it = p0_first; it != p0_end; ++it) {
            p0_elements[*it] = true;
        }
        // Minden varos indexre egy flag, amely azt jelzi, hogy benne van-e az
        // eddig felepitett utvonalban (`ret`).
        std::vector<bool> ret_elements(N);

        for (size_t i = 0; i < N; i++) {
            if (i < first || last < i) {
                size_t p1_it = 0;
                // Az elem p1-bol amit be szeretnenk masolni
                auto g = p1[p1_it];
                // Megkeressuk az elso olyan elemet p1-ben ami nincs benne
                // se p0'-ban, se az eddig felepitett utvonalaban
                while (p0_elements[g] || ret_elements[g]) {
                    p1_it++;
                    g = p1[p1_it];
                }
                ret.push_back(g);
                ret_elements[g] = true;
            } else {
                // Szinplan bemasoljuk a p0'-t
                ret.push_back(p0[i]);
                ret_elements[p0[i]] = true;
            }
        }

#define CROSSOVER_SANITY_CHECK 0
#if CROSSOVER_SANITY_CHECK
        std::unordered_set<size_t> sanchk;
        for (auto idx : ret) {
            if (sanchk.count(idx)) {
                __debugbreak();
                std::abort();
            }
            sanchk.insert(idx);
        }
#endif

        return ret;
    }

    void mutate(path &p, float mutation_rate) {
        std::uniform_real_distribution<float> dist_dice(0.f, 1.f);
        auto dice = dist_dice(_rand);
        if (dice > mutation_rate) {
            auto N = p.size();
            auto i0 = rand() % N;
            auto i1 = rand() % N;
            std::swap(p[i0], p[i1]);
        }
    }

    path find_best_in(population const &pop) {
        auto pop_fit = evaluate(pop);
        auto best = pop_fit[0];
        printf("top fitness: %f | avg fitness: %f\n", best.second, average_fitness(pop_fit));
        return best.first;
    }

private:
    std::vector<City> _cities;
    size_t _start_idx;
    std::mt19937 _rand;
};
