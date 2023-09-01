//
// Created by espen on 20.11.2022.
//

#ifndef NS3_EXPERIMENTUTILS_H
#define NS3_EXPERIMENTUTILS_H

#include "ns3/nstime.h"

namespace ns3 {
    class ExperimentUtils {
    public:
        static inline bool running = false;

        static std::string getCmdOption(char **begin, char **end, const std::string &option);

        static bool cmdOptionExists(char **begin, char **end, const std::string &option);

        static void CreateGnuplot(std::string fileNameWithNoExtension, std::string plotTitle, std::string dataTitle,
                           std::vector <std::pair<Time, long>> &data);

        static void PlotThread();
    };
}


#endif //NS3_EXPERIMENTUTILS_H
