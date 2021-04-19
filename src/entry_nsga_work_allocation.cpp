#include "gen_selection.hpp"
#include "work_allocation.hpp"

#include "vec2.hpp"
#include "gnuplot.hpp"

static void print_contractor(contractor const &c, char const *label0 = "", char const *label1 = "") {
	fprintf(stderr, "%s%f %s%f\n", label0, c.cost, label1, c.inverse_quality);
}

int main(int argc, char **argv) {
	std::mt19937 _rand(0);

	fprintf(stderr, "Initial pop:\n");
	auto contractors = std::list<contractor>();
	for (int i = 0; i < 512; i++) {
		contractors.emplace_back(work_allocation::random_contractor(_rand));
		print_contractor(contractors.back());
	}
	fprintf(stderr, "=== CUT HERE ===\n");

	work_allocation problem(contractors);
	auto logger = [&](int gen, contractor const &best) {
		fprintf(stderr, "gen %d\n", gen);
		fprintf(stderr, "%f %f\n", best.cost, best.inverse_quality);
		print_contractor(best);
	};
	genetic::algorithm<work_allocation, decltype(logger)> solver(problem, 20, 0.0f, &logger);

	auto pop = solver.optimize();
	for (auto &p : pop) {
		fprintf(stderr, "===\n");
		print_contractor(p, "Cost: ", "\nInverse quality: ");
	}

	std::vector<vec2> pareto_front;
	pareto_front.reserve(pop.size());
	std::transform(
		pop.begin(),
		pop.end(),
		std::back_inserter(pareto_front),
		[&](contractor const &c) {
            return vec2{ c.inverse_quality, c.cost };
        }
	);

	std::vector<vec2> contractor_points;
	contractor_points.reserve(contractors.size());
	std::transform(
		contractors.cbegin(),
		contractors.cend(),
		std::back_inserter(contractor_points),
		[&](contractor const &c) {
            return vec2{ c.inverse_quality, c.cost };
        }
	);

    auto plot = gnuplot::polygons_and_points{
        { gnuplot::polygon, "Pareto-front", pareto_front },
        { gnuplot::points, "Contractors",  contractor_points }
    };

    fprintf(stdout, "%s\n", plot.str().c_str());

	return 0;
}