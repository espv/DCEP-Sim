//
// Created by espen on 11.03.2022.
//

#include "utility.h"
#include <iostream>
#include <chrono>
#include "date/date.h"

namespace ns3 {
    bool Utility::replace(std::string &str, const std::string &from, const std::string &to) {
        size_t start_pos = str.find(from);
        if (start_pos == std::string::npos)
            return false;
        str.replace(start_pos, from.length(), to);
        return true;
    }

    int Utility::gcd(int a, int b)
    {
        if (a == 0)
            return b;
        return gcd(b % a, a);
    }

    std::string formats[]{"%Y-%m-%dT%H:%M:%S"};
    long Utility::timestampToLong(std::string datetime) {
//        struct tm tm;
//        long dateInEpoch;
        std::setlocale(LC_ALL, "C");
        std::istringstream stream(datetime);


        date::sys_time<std::chrono::milliseconds> tTimePoint;

        for (auto format : formats) {
            date::from_stream(stream, format.c_str(), tTimePoint);
            auto time_since_epoch = tTimePoint.time_since_epoch();
            return time_since_epoch.count();
        }
 //       return dateInEpoch;
          return 0;
    }

    std::string Utility::longToTimestamp(long timestamp) {
        return "";//strftime();
    }

    // Function to find gcd of array of
    // numbers
    int Utility::findGCD(int arr[], int n)
    {
        int result = arr[0];
        for (int i = 1; i < n; i++)
        {
            result = gcd(arr[i], result);

            if(result == 1)
            {
                return 1;
            }
        }
        return result;
    }
}
