
#include "ns3/dcep-module.h"

#include <iostream>
#include <unistd.h>

#include <yaml-cpp/yaml.h>
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/dcep-app-helper.h"
#include "ns3/netanim-module.h"
#include "ns3/stats-module.h"
#include "ns3/data-collector.h"
#include "ns3/time-data-calculators.h"
#include "ns3/log.h"
#include "ns3/communication.h"
#include "ns3/NodeResourceManager.h"
#include "ns3/stream-processing-module.h"
#include "ns3/inet-socket-address.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"


#include "ns3/type-id.h"


using namespace ns3;


NS_OBJECT_ENSURE_REGISTERED(ExposeCoordinator);

NS_LOG_COMPONENT_DEFINE ("ExposeCoordinator");

YAML::Node Loadyaml(std::string yaml_file) {
    return YAML::LoadFile(yaml_file);
}

ExposeCoordinator::ExposeCoordinator(std::string yaml_file, std::string trace_output_folder) {
    this->yaml_configuration = Loadyaml(yaml_file);
    this->trace_output_folder = trace_output_folder;
    ExposeCoordinator::queryIdToNodeIdToQuery.clear();
    ExposeCoordinator::queryIdToBaseQuery.clear();
    ExposeCoordinator::nodeIdsToWrappers.clear();
    ExposeCoordinator::nodeIdToTuplesIpAndPort.clear();
    ExposeCoordinator::nodeIdToMigrationIpAndPort.clear();
    ExposeCoordinator::router_node = nullptr;
    Communication::ipToNodeId.clear();
    Communication::nodeSetup.clear();
    CEPEngine::stream_id_to_name.clear();
    CEPEngine::stream_name_to_id.clear();
    StreamQuery::ns3NodeIdToExposeNodeId.clear();
    StreamQuery::nodeIdToNode.clear();
    //this->tf = new TracingFramework();
    //Simulator::Schedule(Seconds(60000.1), &ExposeCoordinator::MigrationCheck, this);
}

ExposeCoordinator::ExposeCoordinator(std::string yaml_file, std::string trace_output_folder, std::map<std::string, std::string> string_variables, std::map<std::string, double> numeric_variables) : ExposeCoordinator(yaml_file, trace_output_folder) {
    this->string_variables = string_variables;
    this->numeric_variables = numeric_variables;
}

TypeId
ExposeCoordinator::GetTypeId(void) {
    static TypeId tid = TypeId("ExposeCoordinator")
            .SetParent<Object>()
            .SetGroupName("Applications")
            .AddConstructor<ExposeCoordinator>();

    return tid;
}

void
ExposeCoordinator::StartApplication(void) {
    NS_LOG_FUNCTION (this);

    //Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable>();
    //uint32_t mrand = x->GetInteger(1, 30);

}

TaskNode::TaskNode(int task_id, std::string task) { this->task_id = task_id; this->task = task; }

TaskNode::~TaskNode() {}

YAML::Node TaskNode::GetYamlNode() { return yamlNode; }

TaskNode *TaskNode::GetRoot() { return root; }

TaskNode *TaskNode::GetParent() { return parent; }

TaskNode *TaskNode::GetNext() { return next; }

TaskNode *TaskNode::GetPrevious() { return previous; }

void TaskNode::DecrIterations() { --iterations; }

void TaskNode::SetIterations(int iterations) { this->iterations = iterations; }

void TaskNode::SetRoot(TaskNode *root) { this->root = root; }

bool TaskNode::IsParallel() {
    bool parallel = false;
    if (YAML::Node isParallel = yamlNode["parallel"]) {
        parallel = isParallel.as<bool>();
    }
    return parallel;
}

std::vector<TaskNode *> TaskNode::GetChildren() { return children; }

int TaskNode::GetIterations() { return iterations; }

void ExposeCoordinator::FinishedTask() {
    curTask = curTask->GetNext();
    HandleTask();
}

void ExposeCoordinator::MigrationCheck() {
    if (!experiment_finished) {
        Simulator::Schedule(Seconds(1000000), &ExposeCoordinator::MigrationCheck, this);
    }
    for (auto &[node_id, wrapper] : nodeIdsToWrappers) {
        auto dcep = Dcep::GetDcepFromNode(wrapper->node);
        auto comm = dcep->c_communication;
        if (comm->currently_migrating) {
            return;
        }
    }

    for (auto &[query_id, base_query] : queryIdToBaseQuery) {
        Adapt(query_id, false, -1, -1, -1, "lazy");
    }

    UpdateDeployedQueries();
    UpdateOperatorPlacements();
}

void ExposeCoordinator::HandleTask() {
    if (curTask == nullptr) {
        return;
    }
    while (!curTask->GetChildren().empty()) {
        if (curTask->GetIterations() > 0) {
            curTask->DecrIterations();
            curTask = curTask->GetChildren().at(0);
            HandleTask();
            return;
        } else {
            curTask = curTask->GetNext();
            if (curTask == nullptr) {
                return;
            }
        }
    }

    auto nextEvent = curTask->GetYamlNode();
    if (nextEvent["task"]) {
        handleEvent(nextEvent);
    } else {
        // We're done
        return;
    }
}

void
ExposeCoordinator::UpdateDeployedQueries() {
    // Update all deployed queries with the correct information about which node runs the operator
    for (auto &[query_id, baseQuery] : queryIdToBaseQuery) {
        for (auto &[node_id, deployedQuery] : queryIdToNodeIdToQuery[query_id]) {
            deployedQuery->source_nodes = baseQuery->source_nodes;
            deployedQuery->sink_nodes = baseQuery->sink_nodes;
            deployedQuery->potential_hosts = baseQuery->potential_hosts;
            deployedQuery->streamIdToFieldToEssential = baseQuery->streamIdToFieldToEssential;
            for (auto &[stream_id, operatorIdToOperator] : baseQuery->streamIdToOperatorIdToOperator) {
                for (auto &[operator_id, base_op] : operatorIdToOperator) {
                    if (base_op == nullptr) {
                        continue;
                    }
                    auto deployed_base_op = deployedQuery->streamIdToOperatorIdToOperator[stream_id][operator_id];
                    for (auto deployed_op : deployed_base_op->stream->operators) {
                        deployed_op->parallel_old_node_id = base_op->parallel_old_node_id;
                        deployed_op->SetNodeId(base_op->node_id);
                        auto resourceManager = StreamQuery::nodeIdToNode[node_id]->GetObject<NodeResourceManager>();
                        if (!deployed_op->thread || deployed_op->thread->resourceManager->node_id != resourceManager->node_id) {
                            deployed_op->SetCpuCore (resourceManager->AssignCore (deployed_op));
                        }
                    }
                }
            }
        }
    }
}

void
ExposeCoordinator::InitialPlacement(Ptr<StreamQuery> query) {
    auto query_id = query->unique_query_id;
    auto baseQuery = queryIdToBaseQuery[query_id];
    for (auto &[stream_id, operatorIdToOperator]: baseQuery->streamIdToOperatorIdToOperator) {
        for (auto &[operator_id, base_op]: operatorIdToOperator) {
            // A source_sinkable operator doesn't get a specific placement
            if (base_op == nullptr || base_op->source_sinkable) {
                continue;
            }
            base_op->SetNodeId(baseQuery->potential_hosts[rand() % baseQuery->potential_hosts.size()]);
            for (auto &[node_id, deployedQuery]: queryIdToNodeIdToQuery[query_id]) {
                auto deployed_query = queryIdToNodeIdToQuery[query_id][node_id];
                auto deployed_op = deployed_query->streamIdToOperatorIdToOperator[stream_id][operator_id];
                deployed_op->parallel_old_node_id = base_op->parallel_old_node_id;
                deployed_op->SetNodeId(base_op->node_id);
                auto resourceManager = StreamQuery::nodeIdToNode[node_id]->GetObject<NodeResourceManager>();
                if (!deployed_op->thread || deployed_op->thread->resourceManager->node_id != resourceManager->node_id) {
                    deployed_op->SetCpuCore(resourceManager->AssignCore(deployed_op));
                }
            }
        }
    }
}

void
ExposeCoordinator::UpdateOperatorPlacements() {
    for (auto &[query_id, baseQuery] : queryIdToBaseQuery) {
        for (auto &[node_id, deployedQuery] : queryIdToNodeIdToQuery[query_id]) {
            for (auto &[stream_id, operatorIdToOperator] : baseQuery->streamIdToOperatorIdToOperator) {
                for (auto &[operator_id, base_op] : operatorIdToOperator) {
                    if (base_op == nullptr) {
                        continue;
                    }
                    auto deployed_base_op = deployedQuery->streamIdToOperatorIdToOperator[stream_id][operator_id];
                    for (auto deployed_op : deployed_base_op->stream->operators) {
                        deployed_op->parallel_old_node_id = base_op->parallel_old_node_id;
                        deployed_op->SetNodeId (base_op->node_id);
                        auto resourceManager = StreamQuery::nodeIdToNode[node_id]->GetObject<NodeResourceManager>();
                        if (!deployed_op->thread || deployed_op->thread->resourceManager->node_id != resourceManager->node_id) {
                            deployed_op->SetCpuCore (resourceManager->AssignCore (deployed_op));
                        }
                    }
                }
            }
        }
    }

    for (auto &[query_id, baseQuery] : queryIdToBaseQuery) {
        for (auto &[stream_id, operatorIdToOperator] : baseQuery->streamIdToOperatorIdToOperator) {
            for (auto &[operator_id, base_op] : operatorIdToOperator) {
                if (base_op == nullptr) {
                    continue;
                }
                auto deployed_op = ExposeCoordinator::queryIdToNodeIdToQuery[query_id][base_op->node_id]->streamIdToOperatorIdToOperator[stream_id][operator_id];
                auto node_id = base_op->node_id;
                auto resourceManager = StreamQuery::nodeIdToNode[node_id]->GetObject<NodeResourceManager>();
                if (!deployed_op->thread || deployed_op->thread->resourceManager->node_id != resourceManager->node_id) {
                    deployed_op->SetCpuCore(resourceManager->AssignCore(deployed_op));
                }
            }
        }
    }
}

void
ExposeCoordinator::AddSourceNode(const std::vector<int>& source_nodes, int unique_query_id) {
    auto query = queryIdToBaseQuery[unique_query_id];
    for (int node_id : source_nodes) {
        auto str_node_id = std::to_string(node_id);
        auto wrapper = nodeIdsToWrappers[str_node_id];
        wrapper->DeployQuery(query, unique_query_id);
        queryIdToBaseQuery[unique_query_id]->source_nodes.push_back(node_id);
    }
    UpdateDeployedQueries();
}

void
ExposeCoordinator::AddSinkNode(const std::vector<int>& sink_nodes, int unique_query_id) {
    auto query = queryIdToBaseQuery[unique_query_id];
    for (int node_id : sink_nodes) {
        auto str_node_id = std::to_string(node_id);
        auto wrapper = nodeIdsToWrappers[str_node_id];
        wrapper->DeployQuery(query, unique_query_id);
        queryIdToBaseQuery[unique_query_id]->sink_nodes.push_back(node_id);
    }
    UpdateDeployedQueries();
}

void
ExposeCoordinator::EndExperiment() {
    NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Ending experiment");
    this->experiment_finished = true;
}

void
ExposeCoordinator::AddPotentialHost(const std::vector<int>& potential_hosts, int unique_query_id) {
    auto query = queryIdToBaseQuery[unique_query_id];
    for (int node_id : potential_hosts) {
        ns3::ExposeCoordinator::queryIdToBaseQuery[unique_query_id]->potential_hosts.push_back(node_id);
        auto str_node_id = std::to_string(node_id);
        auto wrapper = nodeIdsToWrappers[str_node_id];
        wrapper->DeployQuery(query, unique_query_id);
    }
    UpdateDeployedQueries();
    InitialPlacement(query);
}

void
ExposeCoordinator::EnableTrace() {
    Metrics::trace_enabled = true;
}

void
ExposeCoordinator::DisableTrace() {
    Metrics::trace_enabled = false;
}

void
ExposeCoordinator::Trace(std::string plot_name, std::string trace_event, int stream_id, int query_id, std::string operator_name) {
    TraceEvent te;
    if (trace_event == "migration") {
        te = TRACEMIGRATION;
    } else if (trace_event == "ingestion") {
        te = TRACEINGESTION;
    } else if (trace_event == "received") {
        te = TRACERECEIVED;
    } else if (trace_event == "partialstatesent") {
        te = TRACEPARTIALSTATESENT;
    } else if (trace_event == "partialstatereceived") {
        te = TRACEPARTIALSTATERECEIVED;
    } else {
        NS_FATAL_ERROR("Invalid trace event " << trace_event);
    }

    // Search for operator within the query with operator_name as name
    // It's not very efficient, but we only do it once for each trace event
    // There's no need to develop new infrastructure to make this more efficient
    Metrics::traceEventToStreamIdToQueryIdToOperatorNameToFilename[te][stream_id][query_id][operator_name] = plot_name;
    Metrics::traceEventToStreamIdToQueryIdToOperatorNameToTraced[te][stream_id][query_id][operator_name] = true;
    //NS_FATAL_ERROR("Failed to find " << operator_name << " within query " << query_id);
}

void
ExposeCoordinator::DeployQuery(int query_id, int unique_query_id) {
    // TODO: Perform operator placement, both sources and sinks
    // TODO: pass along vertices and edges to the function call
    auto queries = yaml_configuration["spequeries"].as<YAML::Node>()["queries"].as<std::vector<YAML::Node>>();
    YAML::Node query_yaml;
    bool found = false;
    for (int i = 0; i < (int)queries.size(); i++) {
        if (queries[i]["id"].as<int>() == query_id) {
            query_yaml = queries[i];
            found = true;
            break;
        }
    }
    if (!found) {
        NS_FATAL_ERROR("Couldn't find query " << query_id);
    }
    auto query = StreamQuery::buildQuery(query_yaml["operators"].as<std::vector<YAML::Node>>(), query_yaml["edges"].as<std::vector<YAML::Node>>(), query_id, unique_query_id);
    std::set<int> node_list;
    for (auto &[stream_id, operatorIdToOperator] : query->streamIdToOperatorIdToOperator) {
        for (auto &[operatorId, op] : operatorIdToOperator) {
            for (auto stream = op->stream;
                 stream != nullptr && stream->next_stream != nullptr; stream = stream->next_stream) {
                auto next_stream = stream->next_stream;
                for (int i = 0; i < (int)next_stream->operators.size(); i++) {
                    auto cur_op = stream->operators[i % stream->operators.size()];
                    auto next_node_id = next_stream->operators[i]->node_id;
                    if (cur_op->node_id != next_node_id) {
                        node_list.insert(next_node_id);
                    }
                }
            }
            break;
       }
    }
    /*for (int query_host : node_list) {
        nodeIdsToWrappers.at(std::to_string(query_host))->DeployQuery(query, unique_query_id);
    }*/
    ExposeCoordinator::queryIdToBaseQuery[unique_query_id] = query;
}

void
InsertOperatorIntoMigrationPlan(Ptr<Operator> op, int *inserted_operators, std::map<int, Ptr<Operator>> *operators_to_migrate, std::vector<std::pair<int, std::pair<int, int>>> *migration_plan, int old_host, int new_host) {
    //auto op_size = (int)op->GetSerializedSize();
    //bool inserted = false;
    /*for (int i = 0; i < *inserted_operators; i++) {
        auto p = (*migration_plan)[i];
        if ((*operators_to_migrate)[p.first]->GetSerializedSize() < op_size) {
            continue;
        }
        inserted = true;
        migration_plan->emplace_back((*migration_plan)[(*migration_plan).size()-1]);
        // We start at *inserted_operators - 1 because we already add the operator above
        for (int j = *inserted_operators-1; j >= i; j--) {
            (*migration_plan)[j+1] = (*migration_plan)[j];
        }
        (*migration_plan)[i] = std::make_pair(op->id, std::make_pair(old_host, new_host));
    }*/
    //if (!inserted) {
        migration_plan->emplace_back(std::make_pair(op->id, std::make_pair(old_host, new_host)));
    //    *inserted_operators = *inserted_operators + 1;
    //}
}

void
ExposeCoordinator::ResetTupleCount() {
    for (auto &[nid, wrapper] : ExposeCoordinator::nodeIdsToWrappers) {
        auto d = Dcep::GetDcepFromNode(StreamQuery::nodeIdToNode[std::stoi(nid)]);
        auto c = d->GetObject<CEPEngine>();
        c->last_received_tuple = -1;
        c->number_tuples_received = 0;
    }
}

void
ExposeCoordinator::Adapt(int query_id, bool allow_parallel_track, int from_host, int to_host, int migration_deadline, std::string migration_mode) {
    Metrics::PrintMetrics();
    Metrics::trace_enabled = true;
    if (migration_mode == "no-migration") {
        return;
    }
    std::cout << Simulator::Now().GetMilliSeconds() << " Adaptation started with migration mode " << migration_mode << std::endl;
    if (from_host != -1) {
        NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << ": Adapting operators of Query " << query_id << " that reside on node "
                                     << from_host);
    } else {
        NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << ": Adapting operators of Query " << query_id);
    }
    if (to_host != -1) {
        NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << ": Moving operators of Query " << query_id << " to node " << to_host);
    }
    if (migration_deadline != -1) {
        NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " The adaptation has a deadline of " << migration_deadline << " ms");
    }

    NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << ": A Task: {task: migrate, node: 2, arguments: [4, SMPA-, 114]}");
    auto query = ExposeCoordinator::queryIdToBaseQuery[query_id];
    NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << ": A Potential hosts for Query " << query_id << ": ");
    for (int potential_host : query->potential_hosts) {
        NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << ": A Potential host: " << potential_host);
    }
    int inserted_operators = 0;
    std::vector<std::pair<int, std::pair<int, int>>> migration_plan;
    std::map<int, std::vector<int>> priority_to_operator_ids;
    std::map<int, Ptr<Operator>> operators_to_migrate;
    int priority;
    // Migrate in reverse order for utility-driven migration
    if (migration_mode == "utility") {
        priority = INT_MAX;
    } else {
        priority = 0;
    }
    for (auto &[stream_id, operatorIdToOperator] : query->streamIdToOperatorIdToOperator) {
        for (auto &[operator_id, op] : operatorIdToOperator) {
            if (!op || op->source_sinkable || op->in_migration) {
                continue;
            }
            bool already_migrating = false;
            for (auto &[inner_operator_id, inner_op] : operators_to_migrate) {
                if (inner_op == op) {
                    already_migrating = true;
                    break;
                }
            }
            if (already_migrating) {
                continue;
            }
            if (migration_mode == "utility") {

                priority_to_operator_ids[--priority].emplace_back (op->id);
                auto aggregationWindowOperator = DynamicCast<AggregationWindowOperator>(op);
                if (aggregationWindowOperator != nullptr && aggregationWindowOperator->groupOperator != nullptr) {
                    for (int i = priority+1; i < INT_MAX; i++) {
                        for (auto op_id : priority_to_operator_ids[i]) {
                            if (op_id == aggregationWindowOperator->groupOperator->id) {
                                priority_to_operator_ids[i].erase(std::remove(priority_to_operator_ids[i].begin(), priority_to_operator_ids[i].end(), op_id), priority_to_operator_ids[i].end());
                                priority_to_operator_ids[--priority].emplace_back (aggregationWindowOperator->groupOperator->id);
                                break;
                            }
                        }
                    }
                }
            } else {
                priority_to_operator_ids[priority++].emplace_back (op->id);
            }

            int old_host = op->node_id;
            if (old_host == to_host) {
                // Skip operator if it is already placed on the to_host
                continue;
            }
            int new_host = old_host;
            if (to_host == -1) {
                while (new_host == old_host) {
                    new_host = query->potential_hosts[rand() % query->potential_hosts.size()];
                }
            } else {
                new_host = to_host;
                bool found_ph = false;
                for (int ph : query->potential_hosts) {
                    if (to_host == ph) {
                        found_ph = true;
                        break;
                    }
                }
                NS_ASSERT(found_ph);
            }
            if (from_host != -1) {
                if (from_host != old_host) {
                    NS_LOG_INFO("Skipping operator " << op->id << " because it's not placed on node " << from_host);
                    continue;
                } else {
                    NS_LOG_INFO("Migrating operator " << op->id << " to node " << new_host << " because it's placed on node " << from_host);
                }
            }

            //op->node_id = new_host;
            InsertOperatorIntoMigrationPlan(op, &inserted_operators, &operators_to_migrate, &migration_plan, old_host, new_host);
            operators_to_migrate[operator_id] = op;
        }
    }

    //bool first = true;
    Time deadline = MilliSeconds(migration_deadline);
    // Sort by serialized size in descending order
    std::map<int, int> operatorIdToNewHost;
    std::map<int, int> operatorIdToOldHost;
    std::map<int, std::vector<int>> oldHostToOperatorIds;
    std::map<int, std::string> operatorIdToAlgorithm;
    for (auto p : migration_plan) {
        auto operator_id = p.first;
        auto oh_and_nh = p.second;
        /*if (first) {
            first = false;
            continue;
        }*/
        int old_host = oh_and_nh.first;
        oldHostToOperatorIds[old_host].emplace_back(operator_id);
        int new_host = oh_and_nh.second;
        auto str_old_host = std::to_string(old_host);
        Ptr<Operator> op = operators_to_migrate[operator_id];
        //op->SetNodeId(new_host);
        //auto resource_manager = StreamQuery::nodeIdToNode[op->node_id]->GetObject<NodeResourceManager>();
        //op->thread = resource_manager->AssignCore(op);
        auto algorithm = op->algorithms[rand()%op->algorithms.size()];
        while (!allow_parallel_track && algorithm[2] == 'P') {
            algorithm = op->algorithms[rand()%op->algorithms.size()];
        }
        operatorIdToNewHost[operator_id] = new_host;
        operatorIdToAlgorithm[operator_id] = algorithm;
    }
    for (auto &[old_host, operator_ids] : oldHostToOperatorIds) {
        std::map<int, std::vector<int>> priority_to_operator_ids_to_new_host;
        std::map<int, int> opIdToNewHost;
        for (auto op_id : operator_ids) {
            opIdToNewHost[op_id] = operatorIdToNewHost[op_id];
            for (auto &[priority, inner_operator_ids] : priority_to_operator_ids) {
                for (auto inner_op_id : inner_operator_ids) {
                    if (op_id == inner_op_id) {
                        priority_to_operator_ids_to_new_host[priority].emplace_back(op_id);
                        break;
                    }
                }
            }
        }
        ExposeCoordinator::nodeIdsToWrappers[std::to_string(old_host)]->Migrate(opIdToNewHost, priority_to_operator_ids_to_new_host, query_id, operatorIdToAlgorithm, deadline, migration_mode);
    }

    //ExposeCoordinator::UpdateOperatorPlacements();
    //ExposeCoordinator::UpdateDeployedQueries();
}

void
ExposeCoordinator::doHandleEvent(std::string cmd, YAML::Node args, std::string node_id,
                                 bool parallel) {
    Ptr<ExposeWrapper> wrapper;
    NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Running command " << cmd << " on node " << node_id);
    if (node_id != "coordinator") {
        wrapper = nodeIdsToWrappers.at(node_id);
    }
    if (cmd == "startRuntimeEnv") {
        NS_ASSERT(args.size() == 0);
        wrapper->StartRuntimeEnv();
        wrapper->FinishTask();
    } else if (cmd == "stopRuntimeEnv") {
        NS_ASSERT(args.size() == 0);
        wrapper->StopRuntimeEnv();
        wrapper->FinishTask();
    } else if (cmd == "deployQueries") {
        NS_ASSERT(args.size() == 2);
        int query_id = args[0].as<int>();
        int unique_query_id = args[1].as<int>();
        DeployQuery(query_id, unique_query_id);
        FinishedTask();
    } else if (cmd == "enableTrace") {
        NS_ASSERT(args.size() == 0);
        EnableTrace();
        FinishedTask();
    } else if (cmd == "disableTrace") {
        NS_ASSERT(args.size() == 0);
        DisableTrace();
        FinishedTask();
    } else if (cmd == "addSourceNode") {
        NS_ASSERT(args.size() == 2);
        auto source_nodes = args[0].as<std::vector<int>>();
        auto unique_query_id = args[1].as<int>();
        AddSourceNode(source_nodes, unique_query_id);
        FinishedTask();
    } else if (cmd == "addSinkNode") {
        NS_ASSERT(args.size() == 2);
        std::vector<int> sink_nodes;
        if (this->numeric_variables.contains("sink_node")) {
            sink_nodes.emplace_back(this->numeric_variables["sink_node"]);
        } else {
            sink_nodes = args[0].as<std::vector<int>>();
        }
        auto unique_query_id = args[1].as<int>();
        AddSinkNode(sink_nodes, unique_query_id);
        FinishedTask();
    } else if (cmd == "addPotentialHost") {
        NS_ASSERT(args.size() == 2);
        auto potential_hosts = args[0].as<std::vector<int>>();
        auto unique_query_id = args[1].as<int>();
        AddPotentialHost(potential_hosts, unique_query_id);
        FinishedTask();
    } else if (cmd == "loopTasks") {
        //HandleTask();
        FinishedTask();
    } else if (cmd == "addNextHop") {
        NS_ASSERT(args.size() == 1);
        std::vector<YAML::Node> streamDefinitions = yaml_configuration["stream-definitions"].as<std::vector<YAML::Node>>();
        for (YAML::Node stream_definition: streamDefinitions) {
            if (stream_definition["id"].as<int>() == args[0].as<int>()) {
                wrapper->AddNextHop(stream_definition["stream-id"].as<int>(), args[1].as<int>());
                break;
            }
        }
        wrapper->FinishTask();
    } else if (cmd == "sendDsAsStream") {
        NS_ASSERT(args.size() == 1);
        int dataset_id = args[0].as<int>();
        auto datasets = yaml_configuration["datasets"].as<std::vector<YAML::Node>>();
        for (YAML::Node ds: datasets) {
            if (ds["id"].as<int>() == dataset_id) {
                wrapper->SendDsAsStream(ds);
                break;
            }
        }
        wrapper->FinishTask();
    } else if (cmd == "sendDsAsSpecificStream") {
        NS_ASSERT(args.size() == 2);
        int dataset_id = args[0].as<int>();
        int stream_id = args[1].as<int>();
        auto datasets = yaml_configuration["datasets"].as<std::vector<YAML::Node>>();
        for (YAML::Node ds: datasets) {
            if (ds["id"].as<int>() == dataset_id) {
                wrapper->SendDsAsSpecificStream(ds, stream_id);
                break;
              }
          }
        wrapper->FinishTask();
    } else if (cmd == "sendNRowsDsAsSpecificStream") {
        NS_ASSERT(args.size() == 3);
        int dataset_id = args[0].as<int>();
        int stream_id = args[1].as<int>();
        int number_rows = args[2].as<int>();
        auto datasets = yaml_configuration["datasets"].as<std::vector<YAML::Node>>();
        for (YAML::Node ds: datasets) {
            if (ds["id"].as<int>() == dataset_id) {
                wrapper->SendNRowsDsAsSpecificStream(ds, stream_id, number_rows);
                break;
            }
        }
        wrapper->FinishTask();
    } else if (cmd == "clearQueries") {
        NS_ASSERT(args.size() == 0);
        //tf.traceEvent(222);
        wrapper->ClearQueries();
        wrapper->FinishTask();
    } else if (cmd == "setNidToAddress") {
        NS_ASSERT(args.size() == 0);
        //wrapper->SetNidToAddress(nodeIdToIpAndPort);
        wrapper->FinishTask();
    } else if (cmd == "endExperiment") {
        NS_ASSERT(args.size() == 0);
        EndExperiment();
        FinishedTask();
    } else if (cmd == "addTpIds") {
        NS_ASSERT(args.size() == 1);
        wrapper->AddTpIds(args.as<std::vector<int>>());
        wrapper->FinishTask();
    } else if (cmd == "retEndOfStream") {
        NS_ASSERT(args.size() == 1);
        wrapper->RetEndOfStream(args[0].as<int>());
    } else if (cmd == "retWhenReceived") {
        NS_ASSERT(args.size() == 1);
        wrapper->RetWhenReceived(args[0].as<int>());
    } else if (cmd == "retWhenProcessed") {
        NS_ASSERT(args.size() == 3);
        wrapper->RetWhenProcessed(args[0].as<int>(), args[1].as<int>(), args[2].as<std::string>());
    } else if (cmd == "retSinkEndOfStream") {
        NS_ASSERT(args.size() == 2);
        wrapper->RetSinkEndOfStream(args[0].as<int>(), args[1].as<int>());
    } else if (cmd == "traceTuple") {
        NS_ASSERT(args.size() == 1);
        wrapper->TraceTuple(args[0].as<int>(), args[1].as<std::vector<std::string>>());
        wrapper->FinishTask();
    } else if (cmd == "configure") {
        NS_ASSERT(args.size() == 0);
        wrapper->Configure();
        wrapper->FinishTask();
    } else if (cmd == "migrate") {
        NS_ASSERT(args.size() == 4);
        int new_host = args[0].as<int>();
        int query_id = args[1].as<int>();
        auto algorithm = args[2].as<std::string>();
        int operator_id = args[3].as<int>();
        std::map<int, int> operatorIdToNewHost;
        std::map<int, std::string> operatorIdToAlgorithm;
        operatorIdToNewHost[operator_id] = new_host;
        operatorIdToAlgorithm[operator_id] = algorithm;
        std::map<int, std::vector<int>> priority_to_operator_ids;
        int priority = 0;
        priority_to_operator_ids[priority].emplace_back(operator_id);
        wrapper->Migrate(operatorIdToNewHost, priority_to_operator_ids, query_id, operatorIdToAlgorithm, Years(1), "lazy");
        wrapper->FinishTask();
    } else if (cmd == "adapt") {
        NS_ASSERT(args.size() == 3);
        int query_id = args[0].as<int>();
        int to_host = args[1].as<int>();
        auto migration_mode = args[2].as<std::string>();
        if (this->string_variables.contains(migration_mode)) {
            migration_mode = this->string_variables[migration_mode];
        }
        Adapt(query_id, false, -1, to_host, -1, migration_mode);
        FinishedTask();
    } else if (cmd == "wait") {
        NS_ASSERT(args.size() == 1);
        long wait_time_microseconds = args[0].as<long>();
        Simulator::Schedule(MicroSeconds(wait_time_microseconds), &ExposeCoordinator::FinishedTask, this);
       // Wait(wait_time_ms);
    } else if (cmd == "disconnectNode") {
        NS_ASSERT(args.size() == 1);
        int ms_before_disconnect = args[0].as<int>();
        if (this->numeric_variables.contains("disconnection_time")) {
            ms_before_disconnect = this->numeric_variables["disconnection_time"];
        }
        wrapper->DisconnectNode(ms_before_disconnect);
        wrapper->FinishTask();
    } else {
        NS_ABORT_MSG("Invalid task issued: " << cmd);
    }
}

void ExposeCoordinator::handleEvent(YAML::Node event) {
    auto cmd = event["task"].as<std::string>();
    auto args = event["arguments"].as<YAML::Node>();
    auto node_id = event["node"].as<std::string>();
    //System.out.println("Node " + node_id + ": " + event);
    bool parallel = false;
    if (event["parallel"]) {
        parallel = event["parallel"].as<bool>();
    }
    if (node_id != "coordinator") {
        nodeIdsToWrappers.at(node_id)->curTask = this->curTask;
    }
    if (this->curTask == nullptr) {
        std::cout << "nullptr curTask" << std::endl;
    }
    doHandleEvent(cmd, args, node_id, parallel);
}


void ExposeCoordinator::Configure() {
    for (uint32_t i = 0; i < node_ids.size(); i++) {
        auto nid = node_ids.at(i);
        this->nodeIdsToWrappers[nid] = CreateObject<ExposeWrapper>();
        auto wrapper = this->nodeIdsToWrappers[nid];
        wrapper->node_id = nid;
        wrapper->coordinator = this;
        auto node = StreamQuery::nodeIdToNode[std::stoi(nid)];
        wrapper->node = node;
    }

    auto plots = yaml_configuration["plots"].as<std::vector<YAML::Node>>();
    for (auto plot : plots) {
        auto plot_name = plot["name"].as<std::string>();
        // TODO: Call ExposeWrapper::EnableTrace for the traces we need
        auto plot_type = plot["type"].as<std::string>();
        auto control_experiment = plot["control-experiment-id"].as<int>();
        if (plot_type == "Migration-time") {
            if (this->experiment_id == control_experiment) {
                // We skip the control experiment here
                continue;
            }
            Metrics::trace_migration = true;
            Metrics::migration_time_plot_name = plot_name;
        } else if (plot_type == "Input-latency") {
            auto stream_id = plot["stream-id"].as<int>();
            auto query_id = plot["query-id"].as<int>();
            auto operator_name = plot["operator-name"].as<std::string>();
            ExposeCoordinator::Trace(plot_name, "ingestion", stream_id, query_id, operator_name);
            ExposeCoordinator::Trace(plot_name, "received", stream_id, query_id, operator_name);
            //ExposeCoordinator::Trace(plot_name, "partialstatesent", stream_id, query_id, operator_name);
            //ExposeCoordinator::Trace(plot_name, "partialstatereceived", stream_id, query_id, operator_name);
        } else if (plot_type == "Output-latency") {
            auto stream_id = plot["stream-id"].as<int>();
            auto query_id = plot["query-id"].as<int>();
            auto operator_name = plot["operator-name"].as<std::string>();
            ExposeCoordinator::Trace(plot_name, "ingestion", stream_id, query_id, operator_name);
            ExposeCoordinator::Trace(plot_name, "received", stream_id, query_id, operator_name);
            //ExposeCoordinator::Trace(plot_name, "partialstatesent", stream_id, query_id, operator_name);
            //ExposeCoordinator::Trace(plot_name, "partialstatereceived", stream_id, query_id, operator_name);
        } else {
            NS_FATAL_ERROR("Invalid plot type!");
        }
    }

    for (int i = 0; i < (int) node_ids.size(); i++) {
        auto node_id = node_ids[i];
        //int cur_node_id = std::stoi(node_id);
        auto tracepoints = yaml_configuration["tracepoints"].as<std::vector<YAML::Node>>();
        std::vector<int> activeTracepointIds;
        for (YAML::Node tracepoint: tracepoints) {
            if (!tracepoint["active"].as<bool>()) {
                continue;
            }
            int tracepoint_id = tracepoint["id"].as<int>();
            //tf.addTracepoint(tracepoint_id);
            activeTracepointIds.push_back(tracepoint_id);
        }

        this->nodeIdsToWrappers.at(node_id)->Configure();
        //System.out.println("Node " + node_id + ": AddTpIds " + activeTracepointIds);
        this->nodeIdsToWrappers.at(node_id)->AddTpIds(activeTracepointIds);

        YAML::Node streamDefinitions = yaml_configuration["stream-definitions"];
        this->nodeIdsToWrappers.at(node_id)->AddSchemas(streamDefinitions);
        //System.out.println("Node " + node_id + ": AddSchemas " + streamDefinitions);
        // Hub
        Simulator::Schedule(Seconds(1 + i), &ExposeWrapper::SetNidToTuplesAddress, this->nodeIdsToWrappers[node_ids.at(0)],
                            nodeIdToTuplesIpAndPort);
        Simulator::Schedule(Seconds(1 + i), &ExposeWrapper::SetNidToMigrationAddress, this->nodeIdsToWrappers[node_ids.at(0)],
                            nodeIdToMigrationIpAndPort);
        Simulator::Schedule(Seconds(1 + i), &ExposeWrapper::SetNidToMigrationAddress, this->nodeIdsToWrappers[node_ids.at(0)],
                            nodeIdToFetchIpAndPort);
        for (int j = 1; j < (int) node_ids.size(); j++) {
            //this->nodeIdsToWrappers[node_ids.at(i)]->SetNidToAddress(nodeIdToIpAndPort);
            Simulator::Schedule(Seconds(1 + j), &ExposeWrapper::SetNidToTuplesAddress, this->nodeIdsToWrappers[node_ids.at(j)],
                                nodeIdToTuplesIpAndPort);
            Simulator::Schedule(Seconds(1 + j), &ExposeWrapper::SetNidToMigrationAddress, this->nodeIdsToWrappers[node_ids.at(j)],
                                nodeIdToMigrationIpAndPort);
            Simulator::Schedule(Seconds(1 + j), &ExposeWrapper::SetNidToMigrationAddress, this->nodeIdsToWrappers[node_ids.at(j)],
                                nodeIdToFetchIpAndPort);
        }
    }
}

void ExposeCoordinator::SetupNodes(std::vector<YAML::Node> cmds, std::map<std::string, YAML::Node> nodeIdsToClients) {
    for (YAML::Node event: cmds) {
        bool isCoordinator = event["node"].as<std::string>() == "coordinator";

        auto task = event["task"].as<std::string>();
        if (task == "loopTasks") {
            auto args = event["arguments"].as<std::vector<YAML::Node>>();
            auto loopCmds = args[1].as<std::vector<YAML::Node>>();
            SetupNodes(loopCmds, nodeIdsToClients);
        } else if (task == "addSinkNode" || task == "addSourceNode" || task == "addPotentialHost") {
            auto args = event["arguments"].as<std::vector<YAML::Node>>();
            std::vector<int> nodes;
            if (task == "addSinkNode" && this->numeric_variables.contains("sink_node")) {
                nodes.emplace_back(this->numeric_variables["sink_node"]);
            } else {
                nodes = args[0].as<std::vector<int>>();
            }
            for (int node : nodes) {
                auto string_node = std::to_string(node);
                if (std::find(this->node_ids.begin(), this->node_ids.end(), string_node) == this->node_ids.end()) {
                    this->node_ids.emplace_back(string_node);
                }
            }
        }
        if (isCoordinator) {
            continue;
        }

        std::string node_id = event["node"].as<std::string>();
        // Create node if it doesn't exist
        if (std::find(this->node_ids.begin(), this->node_ids.end(), node_id) == this->node_ids.end()) {
            this->node_ids.emplace_back(node_id);
        }
    }
    std::sort(this->node_ids.begin(), this->node_ids.end());
}

void ExposeCoordinator::SetupTopology() {
    std::string placementPolicy("centralized");
    // TODO: Use P2P links instead, but where all nodes are connected to a router, and that router node forwards to the right node

    //CsmaHelper csma;
    //csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (10000000)));
    //csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds(10)));
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", DataRateValue (DataRate (1000000)));
    //p2p.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (0)));

    // TODO: Find out why the delay causes awful performance, because it's not supposed to
    // TODO: It adds a bit delay, but it's not supposed to increase migration time by a factor of five
    // TODO: I am making sure to use a tx buffer callback to place new data in buffer when there's space,
    // TODO: but it's not fixing the problem
    //csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds(1)));

    NS_LOG_INFO ("Install internet stack on all nodes.");
    InternetStackHelper internet;

    NS_LOG_INFO ("Assign IP Addresses.");

    NS_LOG_INFO ("Create applications.");

    // Network performance
    auto bw_tuples = yaml_configuration["network"]["bandwidth"]["tuples"].as<std::string>();
    //auto lat_tuples = std::stoi(yaml_configuration["network"]["latency"]["tuples"].as<std::string>());
    auto bw_migration = yaml_configuration["network"]["bandwidth"]["migration"].as<std::string>();
    //auto lat_migration = std::stoi(yaml_configuration["network"]["latency"]["migration"].as<std::string>());

    ExposeCoordinator::bw_tuples = DataRate(bw_tuples);
    ExposeCoordinator::bw_migration = DataRate(bw_migration);

    std::vector<NetDeviceContainer> dev_containers;
    NodeContainer n;
    NodeContainer router_container;
    n.Create(node_ids.size());
    router_container.Create(3);
    ExposeCoordinator::router_container = router_container;

    //int MIGRATION_INDEX = 0;
    //int TUPLES_INDEX = 1;
    for (int i = 0; i < (int) node_ids.size(); i++) {
        NodeContainer p2p_container;
        p2p_container.Add(n.Get(i));
        p2p_container.Add(router_container.Get(0));

        NetDeviceContainer dev = p2p.Install(p2p_container);
        dev_containers.emplace_back(dev);
        for (int j = 0; j < (int) dev.GetN(); j++) {
            /*\Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
            em->SetAttribute ("ErrorRate", DoubleValue (0));
            em->SetAttribute ("ErrorUnit", EnumValue(RateErrorModel::ERROR_UNIT_PACKET));
            dev.Get(j)->SetAttribute ("ReceiveErrorModel", PointerValue (em));*/
            auto p2pNetDevice = DynamicCast<PointToPointNetDevice>(dev.Get(j));
            p2pNetDevice->SetMtu(1500);
            p2pNetDevice->SetAttribute("InterframeGap", TimeValue(MicroSeconds(50)));
            p2pNetDevice->SetAttribute("DataRate", DataRateValue(DataRate(bw_tuples)));
        }
    }

    //NetDeviceContainer ethInterfaces = csma.Install (n);
    //for (int i = 0; i < ethInterfaces.GetN(); i++) {
    //    ethInterfaces.Get(i)->SetMtu(1500);
    //}
    InternetStackHelper internetStack;
    internetStack.InstallAll ();

    Ipv4AddressHelper ipv4;
    Ipv4AddressHelper ipv4_2;
    //Ipv4InterfaceContainer ipv4Interfaces = ipv4.Assign (ethInterfaces);

    DcepAppHelper dcepApphelper;
    ApplicationContainer dcepApps = dcepApphelper.Install(n);

    StreamQuery::nodeIdToNode.clear();
    //uint32_t eventCode = 1;
    for (uint32_t i = 0; i < node_ids.size(); i++) {
        auto network = std::string("10.0.") + std::to_string(i) + std::string(".0");
        auto dev1 = dev_containers.at(i);
        ipv4.SetBase (network.c_str(), "255.255.255.0");
        NetDeviceContainer nc;
        nc.Add(dev1);
        auto ipv4Interfaces = ipv4.Assign (nc);
        auto ip1 = ipv4Interfaces.Get(0).first->GetAddress(1,0).GetAddress();
        nodeIdToMigrationIpAndPort[std::stoi(node_ids.at(i))] = std::make_pair(ip1 , 60123);
        //auto network2 = std::string("10.0.") + std::to_string(i + node_ids.size()) + std::string(".0");
        //ipv4.SetBase (network2.c_str(), "255.255.255.0");
        //auto ipv4Interfaces2 = ipv4.Assign (dev2);
        auto node = n.Get(i);
        int ns3_node_id = node->GetId();
        auto ip2 = ipv4Interfaces.Get(0).first->GetAddress(1,0).GetAddress();
        nodeIdToTuplesIpAndPort[std::stoi(node_ids.at(i))] = std::make_pair(ip2 , 60123);

        // First argument to GetAddress is the same as the one used by the migration socket
        auto ip3 = ipv4Interfaces.Get(0).first->GetAddress(1,0).GetAddress();
        nodeIdToFetchIpAndPort[std::stoi(node_ids.at(i))] = std::make_pair(ip3 , 60123);

        std::cout << "Node " << node_ids[i] << " has assigned " << ip1 << " (" << ip1.Get() << ") for migration and " << ip2 << " (" << ip2.Get() << ") for query processing" << std::endl;

        StreamQuery::ns3NodeIdToExposeNodeId[ns3_node_id] = node_ids[i];
        auto node_resource_manager = CreateObject<NodeResourceManager>(node);
        StreamQuery::nodeIdToNode[std::stoi(node_ids.at(i))] = node;
    }
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    dcepApps.Start(Seconds(0.0));
    dcepApps.Stop(Seconds(2000000000.0));

    NS_LOG_INFO ("Enable static global routing.");
    //
    // Turn on global static routing so we can actually be routed across the star.
    //
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    NS_LOG_INFO ("Enable pcap tracing.");

}

void TaskNode::SetupTasks(std::vector<YAML::Node> cmds, TaskNode *parent) {
    for (size_t i = 0; i < cmds.size(); i++) {
        YAML::Node n = cmds.at(i);
        auto task = n["task"].as<std::string>();
        auto taskNode = new TaskNode(i, task);
        if (i == 0) {
            taskNode->previous = parent;
        } else {
            taskNode->previous = parent->GetChildren().at(i - 1);
            taskNode->previous->next = taskNode;
        }
        if (i == cmds.size() - 1) {
            taskNode->next = parent;
        }
        taskNode->parent = parent;
        parent->children.push_back(taskNode);
        taskNode->yamlNode = n;
        taskNode->iterations = 1;
        taskNode->root = parent->root;

        if (task == "loopTasks") {
            auto args = n["arguments"].as<YAML::Node>();
            taskNode->iterations = args[0].as<int>();
            TaskNode::SetupTasks(args[1].as<std::vector<YAML::Node>>(), taskNode);
        }
    }
}

void ExposeCoordinator::StartSimulation() {
    Simulator::Schedule(Seconds(5+this->node_ids.size()), &ExposeCoordinator::HandleTask, this);

    Simulator::Stop(Days(900000000.0));
    //Simulator::Stop(MicroSeconds(1025362254.0L));
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_INFO("Time to send datasets: " << std::chrono::duration_cast<std::chrono::milliseconds>(Metrics::total_time_end-Metrics::total_time_begin).count() << " ms");
}

Time ExposeCoordinator::InterpretEvents(int eid) {
    //System.out.println("Running from yaml config");
    auto experiments = yaml_configuration["experiments"].as<std::vector<YAML::Node>>();
    for (YAML::Node experiment: experiments) {
        auto yaml_eid = experiment["id"].as<int>();
        if (yaml_eid == eid) {
            auto cmds = experiment["flow"].as<std::vector<YAML::Node>>();
            this->experiment_id = eid;
            this->rootTaskNode = new TaskNode();
            this->rootTaskNode->SetIterations(1);
            this->rootTaskNode->SetRoot(this->rootTaskNode);
            this->curTask = this->rootTaskNode;
            TaskNode::SetupTasks(cmds, this->rootTaskNode);
            //tf.traceEvent(0, new Object[]{eid});
            // Pre-processing to ensure that all the necessary mediators are registered
            SetupNodes(cmds, nodeIdsToClients);

            // Create nodes
            Simulator::Schedule(Seconds(3), &ExposeCoordinator::SetupTopology, this);
            // All agents have registered, and now they can set up connections
            //for (int i = 0; i < (int)this->node_ids.size(); i++) {
                Simulator::Schedule(Seconds(4), &ExposeCoordinator::Configure, this);
            //}
            //this->SendCoordinatorTask(cmd);

            this->cmds = cmds;
            /*for (YAML::Node event : cmds) {
              handleEvent(event);
            }*/
            break;
        }
    }

    // 4 seconds plus 1 nanoseconds
    return NanoSeconds(4000000001);
}
