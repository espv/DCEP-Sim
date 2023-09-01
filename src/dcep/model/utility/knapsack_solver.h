//
// Created by espen on 08.03.2022.
//

#ifndef NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_KNAPSACK_SOLVER_H
#define NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_KNAPSACK_SOLVER_H

#include <vector>
#include <cstdint>

namespace ns3 {
    class KnapsackSolver {
    public:
        std::size_t dynamic_programming(long number, unsigned long capacity,
                                 std::vector<std::tuple<int, int, double, std::size_t, double>> weight_cost, long *result,
                                 uint64_t max_val);
    };
}

#endif //NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_KNAPSACK_SOLVER_H
