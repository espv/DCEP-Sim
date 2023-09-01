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
#include "ns3/dataset-manipulator.h"
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

auto dataset_manipulator = CreateObject<DatasetManipulator>();
void execute_simulation(Ptr<ExposeCoordinator> coordinator, ExperimentUtils utils) {
    auto datasets = coordinator->yaml_configuration["datasets"].as<std::vector<YAML::Node>>();
    auto wrapper = CreateObject<ExposeWrapper>();
    for (YAML::Node ds: datasets) {
        int dataset_id = ds["id"].as<int>();
        if (dataset_id == 94 || dataset_id == 45) {
            wrapper->LoadDataset(ds);
        }
    }
    dataset_manipulator->ModifyDatasets(coordinator->numeric_variables["tuples_between_join"], coordinator->numeric_variables["joins_per_tuple"],
                                        coordinator->numeric_variables["percentage_A_tuples_join"], Seconds(10), MilliSeconds(coordinator->numeric_variables["time_between_B_tuples"]),
                                        ExposeWrapper::dataset_to_tuples[45], ExposeWrapper::dataset_to_tuples[94], "id", "auction",
                                        coordinator->numeric_variables["join_range"]);
   // std::cout << "Running experiment " << eid << " from yaml config " << configFilePath << std::endl;
    utils.running = false;
    //t1.join();
    //std::cout << "Sent a total packet count of " << Communication::total_sent << std::endl;
}

class ExperimentConfiguration {
public:
    int tuples_between_join;
    int joins_per_tuple;
    int percentage_A_tuples_join;
    int time_between_B_tuples = 10;

    ExperimentConfiguration(int tuples_between_join, int joins_per_tuple, int percentage_A_tuples_join, int time_between_B_tuples) {
        this->tuples_between_join = tuples_between_join;
        this->joins_per_tuple = joins_per_tuple;
        this->percentage_A_tuples_join = percentage_A_tuples_join;
        this->time_between_B_tuples = time_between_B_tuples;
    }
};

void RunExperiment(std::string configFilePath, std::string traceFileFolder, int eid, ExperimentUtils utils, std::string migration_mode, std::map<std::string, double> numeric_variables) {
    std::map<std::string, std::string> string_variables;
    string_variables["migration_mechanism"] = migration_mode;
    Ptr<ExposeCoordinator> coordinator = CreateObject<ExposeCoordinator>(configFilePath, traceFileFolder, string_variables, numeric_variables);
    Time after_setup = coordinator->InterpretEvents(eid);
    Simulator::Schedule(after_setup, &execute_simulation, coordinator, utils);
    coordinator->StartSimulation();
}

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
    LogComponentEnable ("JoinOperator", LOG_LEVEL_INFO);
    //LogComponentEnable ("InputOperator", LOG_LEVEL_INFO);
    //LogComponentEnable ("WindowOperator", LOG_LEVEL_INFO);
    LogComponentEnable ("CEPEngine", LOG_LEVEL_INFO);
    //LogComponentEnable ("GroupOperator", LOG_LEVEL_INFO);
    //LogComponentEnable ("Operator", LOG_LEVEL_INFO);
    LogComponentEnable ("Dcep", LOG_LEVEL_INFO);
    //LogComponentEnable ("CEPEngine", LOG_LEVEL_INFO);
    LogComponentEnable ("CpuCore", LOG_LEVEL_INFO);
    //LogComponentEnable ("SelectOperator", LOG_LEVEL_INFO);
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

    std::string x_param_name = "";
    int x_param_value = 0;
    if (utils.cmdOptionExists(argv, argv + argc, "--x-param-name")) {
        x_param_name = utils.getCmdOption(argv, argv + argc, "--x-param-name");
        x_param_value = std::stoi((std::string)utils.getCmdOption(argv, argv + argc, "--x-param-value"));
    }

    int joins_per_tuple = 1;
    int tuples_between_join = 1;
    double percentage_A_tuples_join = 100;
    int join_range = 1;
    int time_between_B_tuples = 10;
    int window_jump = 1000;
    int number_window_extents = 100;
    int disconnection_time = 1000;

    if (utils.cmdOptionExists(argv, argv + argc, "--joins-per-tuple")) {
        joins_per_tuple = std::stoi((std::string)utils.getCmdOption(argv, argv + argc, "--joins-per-tuple"));
    }
    if (utils.cmdOptionExists(argv, argv + argc, "--tuples-between-join")) {
        tuples_between_join = std::stoi((std::string)utils.getCmdOption(argv, argv + argc, "--tuples-between-join"));
    }
    if (utils.cmdOptionExists(argv, argv + argc, "--percentage-A-tuples-join")) {
        percentage_A_tuples_join = std::stod((std::string)utils.getCmdOption(argv, argv + argc, "--percentage-A-tuples-join"));
    }
    if (utils.cmdOptionExists(argv, argv + argc, "--join-range")) {
        join_range = std::stoi((std::string)utils.getCmdOption(argv, argv + argc, "--join-range"));
    }
    if (utils.cmdOptionExists(argv, argv + argc, "--window-jump")) {
        window_jump = std::stoi((std::string)utils.getCmdOption(argv, argv + argc, "--window-jump"));
    }
    if (utils.cmdOptionExists(argv, argv + argc, "--time-between-B-tuples")) {
        time_between_B_tuples = std::stoi((std::string)utils.getCmdOption(argv, argv + argc, "--time-between-B-tuples"));
    }
    if (utils.cmdOptionExists(argv, argv + argc, "--number-window-extents")) {
        number_window_extents = std::stoi((std::string)utils.getCmdOption(argv, argv + argc, "--number-window-extents"));
    }
    if (utils.cmdOptionExists(argv, argv + argc, "--disconnection-time")) {
        disconnection_time = std::stoi((std::string)utils.getCmdOption(argv, argv + argc, "--disconnection-time"));
    }
    ExposeCoordinator::BATCH_SIZE = std::stoi((std::string)utils.getCmdOption(argv, argv + argc, "--batch-size"));
    std::string run_id = utils.getCmdOption(argv, argv + argc, "--run-id");

    //int experiment_configuration_id = -1;//std::stoi(utils.getCmdOption(argv, argv + argc, "--experiment-configuration-id"));
    auto migration_mechanism = utils.getCmdOption(argv, argv + argc, "--migration-mechanism");

    //std::thread t1(ExperimentUtils::PlotThread);

    std::map<std::string, double> numeric_variables;
    numeric_variables["percentage_A_tuples_join"] = percentage_A_tuples_join;
    numeric_variables["time_between_B_tuples"] = time_between_B_tuples;
    numeric_variables["tuples_between_join"] = tuples_between_join;
    numeric_variables["joins_per_tuple"] = joins_per_tuple;
    numeric_variables["join_range"] = join_range;
    numeric_variables["disconnection_time"] = disconnection_time;
    numeric_variables["emit_size"] = window_jump;
    numeric_variables["number_window_extents"] = number_window_extents;
    RunExperiment(configFilePath, traceFileFolder, atoi(eid.c_str()), utils, migration_mechanism, numeric_variables);
    Metrics::WriteToFile(run_id, migration_mechanism);


    std::cout << "Finished with all experiments" << std::endl;
    Metrics::PrintMetrics();
    return 0;
}
