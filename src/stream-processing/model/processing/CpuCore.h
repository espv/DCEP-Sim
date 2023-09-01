//
// Created by espen on 22.11.2021.
//

#ifndef NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_CPUCORE_H
#define NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_CPUCORE_H

#include <deque>
#include "ns3/object.h"
#include "ns3/Operator.h"
#include "ns3/Tuple.h"
#include "ns3/NodeResourceManager.h"

namespace ns3 {
    class Operator;
    class TupleWrapper;
    class NodeResourceManager;
    class CpuCore : public Object {
    public:
        static TypeId GetTypeId();

        CpuCore(const Ptr<CpuCore>& prevThread, Ptr<NodeResourceManager> resourceManager);

        std::map<int, std::deque<std::tuple<Ptr<Operator>, Time, Ptr<TupleWrapper>>>, std::greater<int>> operatorIdToTupleQueue;
        std::map<int, std::deque<std::tuple<Ptr<Operator>, Time, Ptr<TupleWrapper>>>, std::greater<int>> operatorIdToMigrationTupleQueue;
        bool currently_processing = false;
        int number_tuples_processing = 0;
        Ptr<NodeResourceManager> resourceManager;

        void processTuple(const Ptr<Operator>& op, const Ptr<TupleWrapper>& tupleWrapper);

        void doProcessTuple(const Ptr<Operator>& op, const Ptr<TupleWrapper>& tupleWrapper);

        void doneProcessing(Ptr<TupleWrapper> tuple_wrapper);

        std::tuple<Ptr<Operator>, Time, Ptr<TupleWrapper>> popRandomTupleQueue();

        void processQueuedTuple();
    };
}

#endif //NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_CPUCORE_H
