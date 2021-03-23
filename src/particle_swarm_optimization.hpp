#pragma once

#include <vector>
#include <random>

namespace pso {
#if __cplusplus > 201703L
	template<typename P>
	concept indexable = requires(P p, size_t idx) {
		{ p[idx] } -> std::same_as<float&>;
	};

	template<typename P>
	concept has_size = requires(P p) {
		{ p.size() } -> std::convertible_to<size_t>;
	};

	template<typename P, typename V>
	concept search_space = requires(P p0, V v0, P p1, V v1, float s) {
		// A vektorok indexelhetok
		requires indexable<P> && indexable<V>;
		// A vektorok hossza lekerheto
		requires has_size<P> && has_size<V>;
		// Sebesseg beszorozhato skalarral
		{ s * v0 } -> std::same_as<V>;
		// Ket sebesseg osszeadhato
		{ v0 + v1 } -> std::same_as<V>;
		// Pozicio es sebesseg osszege pozicio
		{ p0 + v0 } -> std::same_as<P>;
		// Ket pozicio kulonbsege sebesseg
		{ p1 - p0 } -> std::same_as<V>;
	};

	template<typename Problem, typename Position>
	concept fitness_evaluable = requires(Problem P, Position p) {
		{ P.evaluate_fitness(p) } -> std::same_as<float>;
	};

	template<typename P, typename V>
	struct particle {
		P position;
		V velocity;
		P optima;
	};

	template<typename P, typename V>
	struct swarm {
		std::vector<particle<P, V>> particles;
		P optima;
	};

	template<typename Problem, typename Position, typename Velocity>
	concept can_generate_swarm = requires(Problem p, size_t num_particles) {
		{ p.generate_swarm(num_particles) } -> std::same_as<std::vector<particle<Position, Velocity>>>;
	};

	template<typename P>
	concept particle_swarm_optimizable = requires(P p) {
		typename P::pso_position;
		typename P::pso_velocity;

		requires search_space<typename P::pso_position, typename P::pso_velocity>;
		requires fitness_evaluable<P, typename P::pso_position>;
		requires can_generate_swarm<P, typename P::pso_position, typename P::pso_position>;
	};
#else
#define particle_swarm_optimizable typename
#endif

	struct params {
		size_t max_iterations;

		float omega;
		
		float phi_p;
		float phi_g;
	};

	template<particle_swarm_optimizable Problem>
	class solver {
	public:
		using position_t = typename Problem::pso_position;
		using velocity_t = typename Problem::pso_velocity;
		using swarm_t = swarm<position_t, velocity_t>;
		using particle_t = particle<position_t, velocity_t>;

		class logger {
		public:
			virtual ~logger() = default;
			virtual void on_iteration_done(size_t gen, float global_fitness) = 0;
			virtual void on_next_state(size_t gen, size_t pidx, float fitness, particle_t const &particle) = 0;
		};

		solver(
			Problem &problem,
			params const &params) : _problem(problem), _params(params) {
		}

		position_t solve(size_t num_particles, logger *logger) {
			auto swarm = swarm_t{ _problem.generate_swarm(num_particles) };

			size_t current_iteration = 0;

			while (current_iteration < _params.max_iterations) {
				update_velocities(swarm);
				update_positions(swarm, 1.0f);
				evaluate_all(swarm);

				if (logger != nullptr) {
					log(logger, current_iteration, swarm);
				}

				current_iteration++;
			}

			return swarm.optima;
		}

	protected:
		velocity_t calculate_velocity(swarm_t const &s, particle_t const &p) {
			std::uniform_real_distribution<float> dist(0, 1);
			auto rnd = std::bind(dist, _rand);
			
			velocity_t ret = {};

			auto dirLocalOptima = p.optima - p.position;
			auto dirGlobalOptima = s.optima - p.position;

			for (size_t i = 0; i < p.velocity.size(); i++) {
				auto r_p = rnd();
				auto r_g = rnd();

				ret[i] =
					_params.omega * p.velocity[i] +
					_params.phi_p * r_p * dirLocalOptima[i] +
					_params.phi_g * r_g * dirGlobalOptima[i];
			}

			return ret;
		}

		void update_velocities(swarm_t &swarm) {
			for (auto &particle : swarm.particles) {
				particle.velocity = calculate_velocity(swarm, particle);
			}
		}

		void update_positions(swarm_t &swarm, float step) {
			for (auto &particle : swarm.particles) {
				particle.position = particle.position + step * particle.velocity;
			}
		}

		void log(logger *logger, size_t gen, swarm_t const &swarm) {
			logger->on_iteration_done(gen, _problem.evaluate_fitness(swarm.optima));
			
			for (size_t i = 0; i < swarm.particles.size(); i++) {
				auto &particle = swarm.particles[i];
				auto f = _problem.evaluate_fitness(particle.position);
				logger->on_next_state(gen, i, f, particle);
			}
		}

		void evaluate_all(swarm_t &swarm) {
			float g_opt = _problem.evaluate_fitness(swarm.optima);

			for (auto &particle : swarm.particles) {
				auto f_opt = _problem.evaluate_fitness(particle.optima);
				auto f = _problem.evaluate_fitness(particle.position);

				if (f < f_opt) {
					particle.optima = particle.position;

					if (f_opt < g_opt) {
						g_opt = f_opt;
						swarm.optima = particle.optima;
					}
				}
			}
		}

	private:
		Problem &_problem;
		params const _params;
		std::mt19937 _rand;
	};
}