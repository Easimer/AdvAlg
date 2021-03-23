#include <array>
#include "particle_swarm_optimization.hpp"
#include "function_approximation.hpp"

int main(int argc, char **argv) {
    using problem = function_approx::problem<4>;
    using coef = typename problem::coefficients_t;

    // Het darab minta az alabbi fuggvenybol:
    // y = 5.75 * x**3 - 4.5 * x**2 + 3
    // Azaz az elvart megoldas: (3, 0, -4.5, 5.75)
    std::array<std::pair<float, float>, 7> known_points{
        std::make_pair(-32.f, -193021),
        std::make_pair(-5.f, -3313 / 4.f),
        std::make_pair(-1.f, -29 / 4.f),
        std::make_pair(0.f, 3.f),
        std::make_pair(1.f, 17 / 4.f),
        std::make_pair(5.f, -2437 / 4.f),
        std::make_pair(18.f, 32079),
    };

    // Fuggveny ami kiertekel egy megoldast
    auto func = [&](coef const &C) -> float {
        auto total_error = 0.0f;

        for (auto &p : known_points) {
            auto x = p.first;
            auto y_expected = p.second;
            auto y = C[0] + C[1] * x + C[2] * x * x + C[3] * x * x * x;
            total_error += std::abs(y_expected - y);
        }

        return total_error;
    };

    problem prob(func);
    pso::params params{};
    params.omega = 0.7f;
    params.phi_g = 0.2f;
    params.phi_p = 0.1f;
    params.max_iterations = 50000;

    using solver_t = pso::solver<problem>;
    solver_t solver(prob, params);

    class logger : public solver_t::logger {
    public:
        ~logger() override = default;
        void on_iteration_done(size_t gen, float global_fitness) {
            printf("fitness [%zu] %f\n", gen, global_fitness);
        }

        void on_next_state(size_t gen, size_t pidx, float fitness, solver_t::particle_t const &particle) override {
        }
    } logger;

    auto solution = solver.solve(1000, &logger);

    printf("Expected:   y = (5.75) * x**3 + (-4.5) * x**2 + (0) * x + (3)\n");
    printf("Calculated: y = (%f) * x**3 + (%f) * x**2 + (%f) * x + (%f)\n", solution[3], solution[2], solution[1], solution[0]);

    return 0;
}