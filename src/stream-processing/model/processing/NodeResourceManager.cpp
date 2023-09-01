//
// Created by espen on 20.12.2021.
//

#include "NodeResourceManager.h"

namespace ns3 {
    TypeId
    NodeResourceManager::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::NodeResourceManager")
                .SetParent<Object> ()
        ;

        return tid;
    }

    NodeResourceManager::NodeResourceManager(Ptr<Node> node) {
        AggregateObject(node);
        this->number_cores = MAXIMUM_CORES;
        for (int i = 0; i < number_cores; i++) {
            auto cpuCore = CreateObject<CpuCore>(nullptr, this);
            cores.push_back(cpuCore);
        }
        this->node_id = std::stoi(StreamQuery::ns3NodeIdToExposeNodeId[node->GetId()]);
    }

    bool NodeResourceManager::IsSinkNode(Ptr<StreamQuery> query) {
        for (auto sink_node : query->sink_nodes) {
            if (this->node_id == sink_node) {
                return true;
            }
        }

        return false;
    }

    Ptr<CpuCore> NodeResourceManager::AssignCore(Ptr<Operator> op) {
        int min_operators = 2000000000;
        Ptr<CpuCore> min_cpu_core;
        for (auto core : cores) {
            if (core->operatorIdToTupleQueue.empty()) {
                min_cpu_core = core;
                break;
            }
            if ((int)core->operatorIdToTupleQueue.size() < min_operators) {
                min_cpu_core = core;
                min_operators = core->operatorIdToTupleQueue.size();
            }
        }
        return min_cpu_core;
    }
}
