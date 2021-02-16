#pragma once

#include <cmath>
#include <vector>
#include "vec2.hpp"

class smallest_bounding_polygon {
public:
    smallest_bounding_polygon(int vertices, std::vector<vec2> const &points)
        : _vertices(vertices), _points(points) {
    }

    using solution = std::vector<vec2>;

    float objective(solution const &solution) {
        return perimeter(solution);
    }

    float constraint(solution const &solution) {
        // Minden pont-egyenes parra megnezzuk, hogy a pont milyen messze es melyik
        // oldalan van az egyenesnek.
        // Egyenesenkent a legkisebb tavolsagot hozzaadjuk az akkumulatorhoz.

        float sum_min_dist = 0;

        for (auto &point : _points) {
            float min_dist = 0;
            for (int cur = 0; cur < _vertices; cur++) {
                auto next = (cur + 1) % _vertices;
                auto sdist = signed_distance_from_line(solution[cur], solution[next], point);
                if (sdist < min_dist) {
                    min_dist = sdist;
                }
            }

            sum_min_dist += min_dist;
        }

        return sum_min_dist;
    }

    solution initial_guess() {
        // Kiszamitjuk a tomegkozeppontot; minden suly w=1.0
        vec2 cm{ 0, 0 };
        for (auto &p : _points) {
            cm = cm + p;
        }
        cm = (1 / (float)_points.size()) * cm;

        // Megkeressuk a tomegkozepponttol valo legnagyobb tavolsagot
        float max_dist = 0;
        for (auto &p : _points) {
            auto dist = length(cm - p);
            if (max_dist < dist) {
                max_dist = dist;
            }
        }

        // Iteralunk a `max_dist` sugaru kor kerulete menten, ang_step szoggel
        // lepkedve
        solution ret;
        float ang_step = 2 * PI / _vertices;
        for (float theta = 0; theta < 2 * PI; theta += ang_step) {
            auto x = max_dist * std::cos(theta);
            auto y = max_dist * std::sin(theta);
            ret.push_back({ x, y });
        }
        return ret;
    }

    float perimeter(solution const &solution) {
        float ret = 0;
        for (int cur = 0; cur < _vertices; cur++) {
            auto next = (cur + 1) % _vertices;
            ret += distance(solution[cur], solution[next]);
        }
        return ret;
    }

    solution random_neighbor(solution const &s, float epsilon) {
        // Lemasoljuk a bemeneti megoldast
        auto ret = s;
        // Melyik csucsot manipulaljuk
        int idx = rand() % _vertices;
        // Mekkora mertekben toljuk el azt a csucsot
        // Az RNG egy -1..1 erteket general, ezt skalazzuk epszilonnal
        auto xoff = epsilon * ((rand() % 200) - 100) / 100.0f;
        auto yoff = epsilon * ((rand() % 200) - 100) / 100.0f;

        ret[idx] = ret[idx] + vec2{ xoff, yoff };
        return ret;
    }

    solution best_neighbor(solution const &s, float epsilon) {
        auto ret = s;
        auto ret_fitness = fitness(s);

        vec2 dirs[] = { { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } };

        // Minden pont
        for (int v = 0; v < _vertices; v++) {
            // Mind a negy iranyba torteno manipulaciojat megnezzuk
            for (int d = 0; d < 4; d++) {
                auto tmp = s;
                auto off = epsilon * dirs[d];
                tmp[v] = tmp[v] + off;

                // Jobb-e ennek a fitnessze?
                auto tmp_fitness = fitness(tmp);
                if (tmp_fitness < ret_fitness) {
                    // Ha igen, berakjuk a visszateresi valtozoba
                    ret = std::move(tmp);
                    ret_fitness = tmp_fitness;
                }
            }
        }

        return ret;
    }

    float fitness(solution const &s) {
        auto ret = objective(s);
        auto c = constraint(s);
        if (c < 0) {
            ret += -100 * c;
        }
        return ret;
    }
private:
    int _vertices;
    std::vector<vec2> const &_points;

    // faszom C++
    const float PI = 3.14159265358979323846;
};
