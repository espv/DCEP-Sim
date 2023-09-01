//
// Created by espen on 07.12.2021.
//

#ifndef NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_RANDOM_H
#define NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_RANDOM_H

#include <random>

namespace ns3 {
    class Random {
    public:
        std::random_device rd;
        std::mt19937 mt;
        std::uniform_int_distribution<int> dist;

        Random(int min, int max) {
            mt = std::mt19937(rd());
            dist = std::uniform_int_distribution<int>(min, max);
        }

        int randInt() {
            return dist(mt);
        }
    };
}

#endif //NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_RANDOM_H
