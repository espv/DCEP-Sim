//
// Created by espen on 20.11.2022.
//

//
// Created by espen on 24.04.2020.
//

#include "ns3/expose-coordinator.h"

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
#include "ns3/Metrics.h"
#include "ExperimentUtils.h"

using namespace ns3;

/*
void CreateGnuplots2() {
    std::string fileNameWithNoExtension = "Run3";
    std::string graphicsFileName        = fileNameWithNoExtension + ".png";
    std::string plotFileName            = fileNameWithNoExtension + ".plt";
    std::string plotTitle               = "Number sequences in state";
    std::string dataTitle               = "State size";
    Gnuplot plot (graphicsFileName);
    plot.SetTitle (plotTitle);
    plot.SetTerminal ("png");
    Gnuplot2dDataset dataset;
    dataset.SetTitle (dataTitle);
    dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
    for (auto pair : Metrics::cep_sequences_time) {
        dataset.Add (pair.first.GetSeconds(), pair.second);
    }
    plot.AddDataset (dataset);
    std::ofstream plotFile (plotFileName.c_str());
    plot.GenerateOutput (plotFile);
    plotFile.close ();
}*/

int main(int argc, char **argv) {
    ExperimentUtils utils;
    LogComponentDisableAll(LOG_LEVEL_ALL);
    LogComponentEnable ("ExposeCoordinator", LOG_LEVEL_INFO);
    LogComponentEnable ("ExposeWrapper", LOG_LEVEL_INFO);
    LogComponentEnable ("PrintOperator", LOG_LEVEL_INFO);
    LogComponentEnable ("CepPatternOperator", LOG_LEVEL_INFO);
    LogComponentEnable ("WindowOperator", LOG_LEVEL_INFO);
    //LogComponentEnable ("TcpSocketBase", LOG_LEVEL_INFO);
    //LogComponentEnable ("FilterOperator", LOG_LEVEL_INFO);
    //LogComponentEnable ("JoinOperator", LOG_LEVEL_INFO);
    //LogComponentEnable ("WindowOperator", LOG_LEVEL_INFO);
    //LogComponentEnable ("CEPEngine", LOG_LEVEL_INFO);
    //LogComponentEnable ("GroupOperator", LOG_LEVEL_INFO);
    //LogComponentEnable ("Operator", LOG_LEVEL_INFO);
    LogComponentEnable ("Dcep", LOG_LEVEL_INFO);
    //LogComponentEnable ("CEPEngine", LOG_LEVEL_INFO);
    //LogComponentEnable ("CpuCore", LOG_LEVEL_INFO);
    //LogComponentEnable ("Communication", LOG_LEVEL_INFO);
    LogComponentEnable ("Communication", LOG_LEVEL_WARN);
    //LogComponentEnable ("PointToPointChannel", LOG_LEVEL_WARN);
    //LogComponentEnable ("PointToPointNetDevice", LOG_LEVEL_WARN);

    Random random(0,100);
    Operator::random = &random;

    std::string eid = "";
    std::string no_migration_eid = "";
    std::string configFilePath = "";
    std::string traceFileFolder = "";
    if (utils.cmdOptionExists(argv, argv + argc, "--experiment-id")) {
        eid = utils.getCmdOption(argv, argv + argc, "--experiment-id");
    }

    if (utils.cmdOptionExists(argv, argv + argc, "--yaml-config")) {
        configFilePath = utils.getCmdOption(argv, argv + argc, "--yaml-config");
    }
    if (!configFilePath.starts_with('/')) {
        configFilePath = "expose/experiment-configurations/" + configFilePath;
    }

    Ptr<ExposeCoordinator> coordinator = CreateObject<ExposeCoordinator>(configFilePath, traceFileFolder);
    std::thread t1(ExperimentUtils::PlotThread);
    std::cout << "Running experiment " << eid << " from yaml config " << configFilePath << std::endl;
    coordinator->InterpretEvents(std::atoi(eid.c_str()));
    utils.running = false;
    t1.join();
    std::cout << "Sent a total packet count of " << Communication::total_sent << std::endl;

    sleep(5);
    std::cout << "Number tuples in extents emitted: " << Metrics::tuples_in_extents_emitted << std::endl;

    Metrics::WriteToFile("generic", "Lazy migration");
}
