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
#include "ExperimentUtils.h"
#include <yaml-cpp/yaml.h>

using namespace ns3;

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
  std::string configFilePath = "";
  std::string traceFileFolder = "";
  if (utils.cmdOptionExists(argv, argv + argc, "--yaml-config")) {
    configFilePath = utils.getCmdOption(argv, argv + argc, "--yaml-config");
  }
  if (utils.cmdOptionExists(argv, argv + argc, "--trace-folder")) {
    traceFileFolder = utils.getCmdOption(argv, argv + argc, "--trace-folder");
  }
  if (utils.cmdOptionExists(argv, argv + argc, "--experiment-id")) {
    eid = utils.getCmdOption(argv, argv + argc, "--experiment-id");
  }

  if (!configFilePath.starts_with('/')) {
      configFilePath = "expose/experiment-configurations/" + configFilePath;
  }

  Ptr<ExposeCoordinator> coordinator = CreateObject<ExposeCoordinator>(configFilePath, traceFileFolder);
  std::thread t1(ExperimentUtils::PlotThread);
  coordinator->InterpretEvents(std::atoi(eid.c_str()));
  utils.running = false;
  t1.join();
  std::cout << "Sent a total packet count of " << Communication::total_sent << std::endl;
}
