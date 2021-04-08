#pragma once

#include <vector>
#include <list>
#include <random>
#include <numeric>
#include <iterator>
#include <functional>

struct worker {
	float memory_capacity;
	float speed;
	float running_cost_multiplier;
};

inline float running_cost(worker const &w) {
	return w.running_cost_multiplier * (w.memory_capacity + w.speed);
}

inline bool operator==(worker const &lhs, worker const &rhs) {
	return
		(fabs(rhs.memory_capacity - lhs.memory_capacity) < 0.00001f) &&
		(fabs(rhs.speed - lhs.speed) < 0.00001f) &&
		(fabs(rhs.running_cost_multiplier - lhs.running_cost_multiplier) < 0.00001f);
}

class work_allocation {
public:
	using memory_requirement = float;
	using total_time_requirement = float;

	struct task {
		memory_requirement memory_required;
		total_time_requirement total_time;
	};

	using solution = worker;
	using population = std::list<worker>;

	using solution_with_fitness = std::pair<solution, float>;
	using evaluated_population = std::list<solution_with_fitness>;

	work_allocation(task const &task) : _task(task) {
	}

	population init_population() {
		population ret;

		for (int i = 0; i < 500; i++) {
			ret.push_front(random_worker());
		}

		return ret;
	}

	evaluated_population evaluate(population const &pop) {
		evaluated_population ret;

		std::transform(pop.cbegin(), pop.cend(), std::back_inserter(ret), [&](auto const &p) {
			return std::make_pair(p, 0);
		});

		return ret;
	}

	evaluated_population evaluate(evaluated_population const &pop) {
		auto ret = pop;

		for (auto &p : ret) {
			if (p.first.memory_capacity < _task.memory_required) {
				p.second = 0;
			}
		}

		return ret;
	}

	std::pair<population, evaluated_population> select_next_gen(evaluated_population const &pop) {
		std::pair<population, evaluated_population> ret;

		auto P = pop;
		auto &M = ret.second;
		float fitnessBase = 1;
		
		while (P.size() > 0) {
			population F;

			for (auto &p : P) {
				bool non_dominated = std::all_of(P.begin(), P.end(), [&](solution_with_fitness const &q) {
					return !(dominates(_task, q.first, p.first));
				});

				if (non_dominated) {
					F.push_front(p.first);
				}
			}

			for (auto &p : F) {
				P.erase(std::remove_if(P.begin(), P.end(), [&](solution_with_fitness const &sol) {
					if (sol.first == p) {
						return true;
					} else {
						return false;
					}
				}), P.end());
			}

			std::vector<std::pair<worker, float>> F_sh;
			std::transform(F.cbegin(), F.cend(), std::back_inserter(F_sh), [&](worker const &p) {
				float sh_sum = 0;
				for (auto &q : F) {
					sh_sum += sharing(dist(p, q));
				}
				return std::make_pair(p, sh_sum);
			});

			auto S_sh = std::accumulate(F_sh.cbegin(), F_sh.cend(), 0.0f, [&](auto acc, auto const &p) {
				return acc + p.second;
			});

			evaluated_population F_f;
			std::transform(F_sh.cbegin(), F_sh.cend(), std::back_inserter(F_f), [&](auto const &p) {
				auto fitness = fitnessBase * (1 - p.second / S_sh);
				return std::make_pair(p.first, fitness);
			});

			ret.second.insert(ret.second.end(), F_f.cbegin(), F_f.cend());

			float min_fitness = INFINITY;
			for (auto &p : F_f) {
				if (p.second < min_fitness) {
					min_fitness = p.second;
				}
			}

			fitnessBase = min_fitness * fitness_deg;
		}

		return ret;
	}

	population select_parents(evaluated_population const &pop) {
		auto k = 32;
		auto N = pop.size();

		std::uniform_int_distribution<size_t> dist_index(0, N - 1);
		auto generate_random_index = std::bind(dist_index, _rand);

		population ret;

		// Ket szulot keresunk
		while (ret.size() < 2) {
			auto subject_idx = generate_random_index();
			int duels_played = 0;
			int duels_won = 0;
			
			auto it_subject = pop.begin();
			std::advance(it_subject, subject_idx);
			auto &subject = *it_subject;

			// k darab parbajt kell lejatszania
			while (duels_played < k) {
				// keresunk egy ellenfelet (aki nem az alany)
				auto contender_idx = subject_idx;
				while (contender_idx == subject_idx) {
					contender_idx = generate_random_index();
				}

				auto it_contender = pop.begin();
				std::advance(it_contender, contender_idx);
				auto &contender = *it_contender;

				if (subject.second >= contender.second) {
					duels_won++;
				}
				duels_played++;
			}

			// Ha megnyert minden parbajt, akkor szaporodhat
			if (duels_won == k) {
				ret.push_back(subject.first);
			}
		}

		return ret;
	}

	solution crossover(population const &pop) {
		solution ret = {};

		std::uniform_real_distribution dist(0.0f, 1.0f);

		auto it = pop.begin();
		auto p0 = *it;
		++it;
		auto p1 = *it;

		float c = dist(_rand);

		ret.memory_capacity = p0.memory_capacity + c * (p1.memory_capacity - p0.memory_capacity);
		ret.speed = p0.speed + c * (p1.speed - p0.speed);
		ret.running_cost_multiplier = p0.running_cost_multiplier + c * (p1.running_cost_multiplier - p0.running_cost_multiplier);

		return ret;
	}

	solution find_best_in(population const &pop) {
		return *pop.begin();
	}

	solution mutate(solution orig, float chance) {
		std::uniform_real_distribution dist_dice(0.0, 1.0);
		std::normal_distribution dist_mutation(0.0, 1.0);
		std::normal_distribution dist_mutation_mul(0.0, 0.1);

		if (dist_dice(_rand) < chance) {
			orig.memory_capacity += dist_mutation(_rand);
			orig.speed += dist_mutation(_rand);
			orig.running_cost_multiplier += dist_mutation_mul(_rand);

			orig.memory_capacity = std::max(orig.memory_capacity, 1.0f);
			orig.speed = std::max(orig.speed, 0.1f);
			orig.running_cost_multiplier = std::max(orig.running_cost_multiplier, 0.5f);
		}

		return orig;
	}

	worker random_worker() {
		std::uniform_real_distribution dist_mem(32.0, 1024.0);
		std::uniform_real_distribution dist_speed(1.0, 8.0);
		std::normal_distribution dist_cost_multiplier(1.0, 0.25);

		worker w;
		w.memory_capacity = dist_mem(_rand);
		w.speed = dist_speed(_rand);
		w.running_cost_multiplier = dist_cost_multiplier(_rand);
		return w;
	}

	float sharing(float dist) {
		return std::max(0.0f, 1 - (dist / sigma_share) * (dist / sigma_share));
	}

	static float dist(worker const &lhs, worker const &rhs) {
		auto dx = rhs.memory_capacity - lhs.memory_capacity;
		auto dy = rhs.speed - lhs.speed;
		auto dz = rhs.running_cost_multiplier - lhs.running_cost_multiplier;
		return std::sqrt(dx * dx + dy * dy + dz * dz);
	}

	bool dominates(task const &T, worker const &lhs, worker const &rhs) {
		int c = 0;
		int b = 0;

		if (lhs.speed / T.total_time >= rhs.speed / T.total_time) {
			c++;
			if (lhs.speed / T.total_time > rhs.speed / T.total_time) b++;
		}

		if (fabs(T.memory_required - lhs.memory_capacity) <= fabs(T.memory_required - rhs.memory_capacity)) {
			c++;
			if (fabs(T.memory_required - lhs.memory_capacity) < fabs(T.memory_required - rhs.memory_capacity)) b++;
		}

		if (running_cost(lhs) <= running_cost(rhs)) {
			c++;
			if (running_cost(lhs) < running_cost(rhs)) b++;
		}

		return c == 3 && b > 0;
	}
private:
	task _task;
	float fitness_deg = 0.9f;
	float sigma_share = 100;

	std::mt19937 _rand;
};