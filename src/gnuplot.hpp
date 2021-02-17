#pragma once

#include <vector>
#include <sstream>
#include "vec2.hpp"

namespace gnuplot {
    enum plot_kind {
        polygon,
        points,
    };

    template<typename T>
    struct plot_descriptor {
        plot_kind kind;
        char const *title;
        std::vector<T> dataset;
    };

    class polygons_and_points {
    public:
        polygons_and_points(std::initializer_list<plot_descriptor<vec2>> args) {
            std::ostringstream buf;

            buf << "set style increment user\n";
            buf << "plot ";

            for (auto it = args.begin(); it != args.end(); ++it) {
                auto &arg = *it;
                buf << "'-' title '" << arg.title << "' ";

                switch (arg.kind) {
                case polygon:
                    buf << "with linespoints";
                    break;
                case points:
                    buf << "with points";
                    break;
                }

                if (it + 1 != args.end()) {
                    buf << ",\\";
                }
                buf << "\n";
            }

            for (auto &arg : args) {
                switch (arg.kind) {
                case polygon:
                    for (auto &v : arg.dataset) {
                        buf << v.x << ' ' << v.y << '\n';
                    }
                    buf << arg.dataset[0].x << ' ' << arg.dataset[0].y << '\n';
                    break;
                case points:
                    for (auto &v : arg.dataset) {
                        buf << v.x << ' ' << v.y << '\n';
                    }
                    break;
                }
                buf << "e\n";
            }

            _file = buf.str();
        }

        std::string const &str() const {
            return _file;
        }

    private:
        std::string _file;
    };
}