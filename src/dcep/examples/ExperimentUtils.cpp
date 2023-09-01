//
// Created by espen on 20.11.2022.
//

#include "ns3/expose-coordinator.h"
#include "ExperimentUtils.h"
#include <iostream>
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/stats-module.h"
#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/type-id.h"
#include "ns3/random.h"
#include "ns3/Operator.h"
#include "ExperimentUtils.h"
#include <yaml-cpp/yaml.h>

namespace ns3 {
    std::string ExperimentUtils::getCmdOption(char **begin, char **end, const std::string &option) {
        char **itr = std::find(begin, end, option);
        if (itr != end && ++itr != end) {
            return std::string(*itr);
        }
        return 0;
    }

    bool ExperimentUtils::cmdOptionExists(char **begin, char **end, const std::string &option) {
        return std::find(begin, end, option) != end;
    }

    void
    ExperimentUtils::CreateGnuplot(std::string fileNameWithNoExtension, std::string plotTitle, std::string dataTitle,
                                   std::vector <std::pair<Time, long>> &data) {
        //std::string fileNameWithNoExtension = "Run2";
        std::string graphicsFileName = fileNameWithNoExtension + ".png";
        std::string plotFileName = fileNameWithNoExtension + ".plt";
        //std::string plotTitle               = "Number tuples in state";
        //std::string dataTitle               = "State size";
        Gnuplot plot(graphicsFileName);
        plot.SetTitle(plotTitle);
        plot.SetTerminal("png");
        Gnuplot2dDataset dataset;
        dataset.SetTitle(dataTitle);
        dataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);
        for (auto i = 0; i < (int) data.size(); i++) {
            auto pair = data[i];
            dataset.Add(pair.first.GetSeconds(), pair.second);
        }
        plot.AddDataset(dataset);
        std::ofstream plotFile(plotFileName.c_str());
        plot.GenerateOutput(plotFile);
        plotFile.close();
    }

    void ExperimentUtils::PlotThread() {
        while (running) {
            // Tuple latencies
            auto &tuple_latencies = Metrics::GetTupleLatencies();
            ExperimentUtils::CreateGnuplot("TupleLatency", "Tuple latency", "ms", tuple_latencies);

            sleep(1);
        }
    }
}
