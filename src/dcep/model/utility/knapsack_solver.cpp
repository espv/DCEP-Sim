//
// Created by espen on 08.03.2022.
//

#include "ns3/knapsack_solver.h"
#include <vector>
#include <tuple>
#include <ns3/log.h>
#include <algorithm>
#include "ns3/type-id.h"
#include "ns3/object.h"

namespace ns3 {
    std::size_t KnapsackSolver::dynamic_programming(long number, unsigned long capacity,
                                             std::vector<std::tuple<int, int, double, std::size_t, double>> weight_cost,
                                             long *result, uint64_t max_val) {
        std::size_t number_eligible = 0;
        for (long i = 0; i < number; i++) {
            auto wt = std::get<3>(weight_cost[i]);
            if (wt < capacity) {
                number_eligible++;
            }
        }
        int eligible_index[number_eligible];
        int weight[number_eligible];
        int cost[number_eligible];
        std::size_t number_cnt = 0;
        for (long i = 0; i < number; i++) {
            auto c = std::get<2>(weight_cost[i]);
            auto wt = std::get<3>(weight_cost[i]);
            if (wt < capacity) {
                cost[number_cnt] = c;
                weight[number_cnt] = wt;
                eligible_index[number_cnt] = i;
                ++number_cnt;
            }
        }
        NS_ASSERT(number_cnt == number_eligible);
        uint32_t *table[number_eligible + 1];
        for (std::size_t i = 0; i < number_eligible + 1; i++) {
            table[i] = new uint32_t[max_val + 1];
            //for (int j = 0; j < max_val; j++) {
            table[i][0] = 0;
            //}
        }
        for (std::size_t i = 0; i < number_eligible + 1; i++) {
            for (long j = max_val; j >= 1; j--) {
                table[i][j] = 2000000000;
            }
        }
        table[0][0] = 0;
        std::size_t max_utility = 0;
        /*
        auto largest_utility = 0;
        auto min_weight = 2000000000;
        for (auto wt : weight) {
            if (min_weight > wt) {
                min_weight = wt;
            }
        }
        for (auto c : cost) {
            if (largest_utility < c) {
                largest_utility = c;
            }
        }*/

        for (std::size_t i = 1; i < number_eligible + 1; i++) {
	    std::size_t cost_i_min_1 = cost[i-1];
            auto weight_i_min_1 = weight[i-1];
            for (std::size_t j = 1; j < max_val + 1; j++) {
                if (cost_i_min_1 <= j) {
                    std::size_t s1 = weight_i_min_1 + table[i - 1][j - cost_i_min_1];
                    std::size_t s2 = table[i - 1][j];
                    if (s1 < s2 && s1 <= capacity) {
                        table[i][j] = s1;
                        if (max_utility < j) {
                            max_utility = j;
                        }
                    } else {
                        table[i][j] = s2;
                    }
                } else {
                    table[i][j] = table[i - 1][j];
                }
            }
        }

        long cur_utility = max_utility;

        //int eligible_cnt = 0;
        int total_weight = 0;
        int number_states_to_migrate = 0;
        for (long i = number_eligible; i > 0; i--) {
            if (table[i][cur_utility] != table[i - 1][cur_utility]) {
                result[eligible_index[i - 1]] = 1;
                cur_utility -= cost[i-1];
                total_weight += weight[i-1];
                ++number_states_to_migrate;
            }
        }
        //std::cout << "cur_utility: " << cur_utility << ", total weight: " << total_weight << ", total number states: " << number_states_to_migrate << std::endl;
        for (std::size_t i = 0; i < number_eligible + 1; i++) {
            delete table[i];
        }
        return max_utility;
    }
}
