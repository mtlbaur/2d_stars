#ifndef ENUMS_H
#define ENUMS_H

namespace Enum {
    namespace Star {
        enum class GenType {
            RNG,
            MIN,
            AVG,
            MAX,
        };

        namespace Shape {
            enum class Core {
                FULL,
                EMPTY,
            };
            enum class Draw {
                FILL,
                LINE,
            };
        } // namespace Shape
    }     // namespace Star

    namespace Config {
        // not enum class because implicit int conversion is convenient for use with ImGui
        enum ColorMode {
            DEFAULT,
            RANDOM,
            UNIFORM,
            CONSISTENT,
        };
    } // namespace Config
} // namespace Enum

#endif