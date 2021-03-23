#pragma once

#include <array>

namespace function_approx {
	// Egyutthatokat tarolo tomb
	// 
	// Hasznalhatnank std::array-t is, de akkor az ADL nem talalna az
	// alul levo operator overload-okat.
	template<size_t Length>
	struct coefficients {
		template<typename... T>
		coefficients(T... list) : _values{ static_cast<float>(list)... } {
		}

		float &operator[](size_t idx) { return _values[idx]; }
		float const &operator[](size_t idx) const { return _values[idx]; }
		constexpr size_t size() const { return Length; }
		std::array<float, Length> _values;
	};

	template<size_t Length>
	inline coefficients<Length> operator+(coefficients<Length> const &lhs, coefficients<Length> const &rhs) {
		coefficients<Length> ret;

		for (size_t i = 0; i < Length; i++) {
			ret[i] = lhs[i] + rhs[i];
		}

		return ret;
	}

	template<size_t Length>
	inline coefficients<Length> operator*(float lhs, coefficients<Length> const &rhs) {
		coefficients<Length> ret;

		for (size_t i = 0; i < Length; i++) {
			ret[i] = lhs * rhs[i];
		}

		return ret;
	}

	template<size_t Length>
	inline coefficients<Length> operator-(coefficients<Length> const &lhs, coefficients<Length> const &rhs) {
		return lhs + (-1.f * rhs);
	}

	template<size_t NumCoefficients>
	class problem {
	public:
		using error_t = float;
		using coefficients_t = coefficients<NumCoefficients>;
		using eval_lambda_t = std::function<error_t (coefficients_t const &)>;

		using pso_position = coefficients_t;
		// ujrahasznaljuk ugyanazt a tipust
		using pso_velocity = coefficients_t;

		problem(eval_lambda_t eval) : _eval(std::move(eval)) {
		}

		error_t evaluate_fitness(pso_position const &p) {
			return _eval(p);
		}

		std::vector<pso::particle<pso_position, pso_velocity>>
		generate_swarm(size_t num_particles) {
			std::vector<pso::particle<pso_position, pso_velocity>> ret;
			std::mt19937 rand;
			std::uniform_real_distribution<float> dist_pos(-1000, 1000);
			std::uniform_real_distribution<float> dist_vel(-1, 1);
			auto rnd_pos = std::bind(dist_pos, rand);
			auto rnd_vel = std::bind(dist_vel, rand);

			for (size_t i = 0; i < num_particles; i++) {
				pso_position pos = { };
				pso_velocity vel = { };

				for (size_t j = 0; j < NumCoefficients; j++) {
					pos[j] = rnd_pos();
					vel[j] = rnd_vel();
				}

				ret.push_back({ pos, vel, pos });
			}

			return ret;
		}

	private:
		eval_lambda_t _eval;
	};
}