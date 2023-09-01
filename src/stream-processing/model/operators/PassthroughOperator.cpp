//
// Created by espen on 03.11.2021.
//
#include "ns3/Operator.h"
#include "ns3/log.h"


using namespace ns3;
namespace ns3 {
    NS_OBJECT_ENSURE_REGISTERED(PassthroughOperator);
    NS_LOG_COMPONENT_DEFINE ("PassthroughOperator");

    TypeId
    PassthroughOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::PassthroughOperator")
                .SetParent<Operator> ()
        ;

        return tid;
    }

    PassthroughOperator::PassthroughOperator() : Operator(passthroughType) {}

    //int passthrough_cnt = 0;
    void PassthroughOperator::process(Ptr<TupleWrapper> tupleWrapper) {
        //std::cout << "PassthroughOperator number tuples: " << ++passthrough_cnt << ", next node Id: " << stream->next_stream->operators[0]->node_id << std::endl;
        emit(tupleWrapper);
    }

    int PassthroughOperator::calculateMigrationSize(migration_types migration_type) {
        return 1;
    }

    void PassthroughOperator::ExtractState(Ptr<MigrationPlan> mp) {

    }

    void PassthroughOperator::ImportState(Ptr<PartialStateBatch> batch) {

    }
}
