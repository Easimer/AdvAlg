#pragma once

#include <cassert>
#include <utility>
#include <vector>
#include <algorithm>

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

        for (int i = 0; i < 12; i++) {
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
        auto n_elite = n_solutions / 2; // 25% elit rata

        population elite;
        population mating;

        auto avg_fit = average_fitness(pop);

        for (size_t i = 0; i < n_solutions; i++) {
            if (i < n_elite) {
                elite.push_back(pop[i].first);
            } else {
                if (pop[i].second >= avg_fit) {
                    mating.push_back(pop[i].first);
                }
            }
        }

        return { std::move(elite), std::move(mating) };
    }

    population select_parents(population const &pop) {
        auto N = pop.size();
        auto p0 = rand() % N;
        // Megbizonyosodunk arrol, hogy a ket szulo nem ugyanaz a peldany
        // Eleg getto megoldas, mert ha szerencsesek vagyunk
        // akkor ez egy vegtelen loop
        auto p1 = p0;
        while (p1 == p0) {
            p1 = rand() % N;
        }

        return { pop[p0], pop[p1] };
    }

    path crossover(population const &pop) {
        path ret;
        auto &p0 = pop[0];
        auto &p1 = pop[1];

        // p0-bol atmasoljuk a [first, last] indexu alszekvenciat (p0');
        // p1-bol atmasoljuk azokat az elemeket, amelyek nincsenek benne
        // a fenti alszekvenciaban.

        auto N = p0.size();
        auto first = rand() % N;
        auto last = first + rand() % (N - first);

        assert(first >= 0 && first < N);
        assert(last >= 0 && last < N);
        assert(last - first >= 0);
        assert(last - first <= N);

        auto p0_first = p0.begin() + first;
        auto p0_end = p0.begin() + last + 1;

        for (size_t i = 0; i < N; i++) {
            if (i < first || last < i) {
                size_t p1_it = 0;
                // Az elem p1-bol amit be szeretnenk masolni
                auto g = p1[p1_it];
                // Megkeressuk az elso olyan elemet p1-ben ami nincs benne
                // se p0'-ban, se az eddig felepitett utvonalaban
                while (
                    std::any_of(p0_first, p0_end, [&](size_t x) {return x == g; }) ||
                    std::any_of(ret.begin(), ret.end(), [&](size_t x) {return x == g; })
                    ) {
                    p1_it++;
                    g = p1[p1_it];
                }
                ret.push_back(g);
            } else {
                // Szinplan bemasoljuk a p0'-t
                ret.push_back(p0[i]);
            }
        }

        return ret;
    }

    void mutate(path &p, float mutation_rate) {
        auto dice = (rand() % 100) / 100.0f;
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
};
