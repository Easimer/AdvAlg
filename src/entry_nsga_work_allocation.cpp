#include "gen_selection.hpp"
#include "work_allocation.hpp"

int main(int argc, char **argv) {
	work_allocation::task task = {
		work_allocation::memory_requirement(512),
		work_allocation::total_time_requirement(90),
	};
	work_allocation problem(task);
	auto logger = [&](int gen, worker const &best) {
		printf("gen %d\n", gen);
		printf("%f %f %f\n", best.memory_capacity, best.speed, best.running_cost_multiplier);
	};
	genetic::algorithm<work_allocation, decltype(logger)> solver(problem, 1000, 0.25f, &logger);

	auto pop = solver.optimize();
	for (auto &p : pop) {
		printf("===\n");
		printf("Mem: %f\nSpeed: %f\nRunning cost: %f\n", p.memory_capacity, p.speed, running_cost(p));
	}

	return 0;
}