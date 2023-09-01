//
// Created by espen on 11.03.2022.
//

#ifndef NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_UTILITY_H
#define NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_UTILITY_H

#include <iostream>

namespace ns3 {
    class Utility {
    public:
        static bool replace(std::string &str, const std::string &from, const std::string &to);

        static int gcd(int a, int b);

        static int findGCD(int arr[], int n);

        static long timestampToLong(std::string datetime);

        static std::string longToTimestamp(long timestamp);
    };
}


#endif //NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_UTILITY_H
