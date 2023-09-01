//
// Created by espen on 20.12.2021.
//

#ifndef NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_NODERESOURCEMANAGER_H
#define NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_NODERESOURCEMANAGER_H

#include <vector>
#include "ns3/CpuCore.h"
#include "ns3/Query.h"
#include "ns3/object.h"
#include "ns3/stream-processing-common.h"

using namespace ns3;
namespace ns3 {
    class CpuCore;
    class NodeResourceManager : public Object {
    public:
        static TypeId GetTypeId();

        NodeResourceManager(Ptr<Node> node);

        Ptr<CpuCore> AssignCore(Ptr<Operator> op);
        bool IsSinkNode(Ptr<StreamQuery> query);

        int number_cores;
        int node_id;
        std::vector <Ptr<CpuCore>> cores;
    };
}


#endif //NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_NODERESOURCEMANAGER_H
