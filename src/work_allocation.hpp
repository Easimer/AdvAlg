#pragma once

#include <vector>
#include <list>
#include <random>
#include <numeric>
#include <iterator>
#include <functional>
#include <algorithm>

struct contractor {
	float cost;
	float inverse_quality;
};

inline bool operator==(contractor const &lhs, contractor const &rhs) {
	return
		(fabs(rhs.cost - lhs.cost) < 0.00001f) &&
		(fabs(rhs.inverse_quality - lhs.inverse_quality) < 0.00001f);
}

class work_allocation {
public:
	using solution = contractor;
	using population = std::list<contractor>;

	using solution_with_fitness = std::pair<solution, float>;
	using evaluated_population = std::list<solution_with_fitness>;

	work_allocation(population contractors) : _contractors(std::move(contractors)) {
	}

	population init_population() {
		return _contractors;
	}

	evaluated_population evaluate(population const &pop) {
		evaluated_population ret;

		std::transform(pop.cbegin(), pop.cend(), std::back_inserter(ret), [&](auto const &p) {
			return std::make_pair(p, 0);
		});

		return ret;
	}

	evaluated_population evaluate(evaluated_population const &pop) {
		return pop;
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
					return !(dominates(q.first, p.first));
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

			std::vector<std::pair<contractor, float>> F_sh;
			std::transform(F.cbegin(), F.cend(), std::back_inserter(F_sh), [&](contractor const &p) {
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

		ret.cost = p0.cost + c * (p1.cost - p0.cost);
		ret.inverse_quality = p0.inverse_quality + c * (p1.inverse_quality - p0.inverse_quality);

		return ret;
	}

	solution find_best_in(population const &pop) {
		return *pop.begin();
	}

	solution mutate(solution orig, float chance) {
		return orig;
	}

	template<typename T>
	static contractor random_contractor(T &rand) {
		std::exponential_distribution dist_cost(0.25f);
		std::exponential_distribution dist_inverse_quality(0.25f);

		contractor w;
		w.cost = 1 + std::max(0.f, dist_cost(rand));
		w.inverse_quality = 1 + std::max(0.f, dist_inverse_quality(rand));

		return w;
	}

	float sharing(float dist) {
		return std::max(0.0f, 1 - (dist / sigma_share) * (dist / sigma_share));
	}

	static float dist(contractor const &lhs, contractor const &rhs) {
		auto dx = rhs.cost - lhs.cost;
		auto dy = rhs.inverse_quality - lhs.inverse_quality;
		return std::sqrt(dx * dx + dy * dy);
	}

	bool dominates(contractor const &lhs, contractor const &rhs) {
		int c = 0;
		int b = 0;

		if (lhs.cost <= rhs.cost) {
			c++;
			if (lhs.cost < rhs.cost) b++;
		}

		if (lhs.inverse_quality <= rhs.inverse_quality) {
			c++;
			if (lhs.inverse_quality < rhs.inverse_quality) b++;
		}

		return c == 2 && b > 0;
	}
private:
	population _contractors;
	float fitness_deg = 0.9f;
	float sigma_share = 100;

	std::mt19937 _rand;
};