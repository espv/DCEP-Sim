//
// Created by espen on 03.11.2021.
//
#include "ns3/Operator.h"
#include "ns3/Query.h"
#include "ns3/log.h"


using namespace ns3;
namespace ns3 {
    NS_OBJECT_ENSURE_REGISTERED(PrintOperator);
    NS_LOG_COMPONENT_DEFINE ("PrintOperator");

    PrintOperator::PrintOperator() : Operator(printType) {
        NS_LOG_FUNCTION (this);
    }

    TypeId
    PrintOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::PrintOperator")
                .SetParent<Operator> ()
        ;

        return tid;
    }

    void PrintOperator::process(Ptr<TupleWrapper> tupleWrapper) {
        ++PrintOperator::total_cnt;
        ++print_cnt;
        if (print_cnt % 1000 == 0) {
            NS_LOG_INFO(Simulator::Now().GetMicroSeconds() << " " << total_cnt << ", Node "
                                                           << StreamQuery::ns3NodeIdToExposeNodeId[tupleWrapper->node->GetId()]
                                                           << ", " << *tupleWrapper);
        }
        Metrics::total_time_end = std::chrono::steady_clock::now();
        emit(tupleWrapper);
    }

    int PrintOperator::calculateMigrationSize(migration_types migration_type) {
        return 1;
    }

    void PrintOperator::ExtractState(Ptr<MigrationPlan> mp) {

    }

    void PrintOperator::ImportState(Ptr<PartialStateBatch> batch) {

    }
}
