//
// Created by espen on 04.12.2021.
//
#include "ns3/object.h"
#include "ns3/Operator.h"
#include "ns3/Tuple.h"

namespace ns3 {
    TypeId
    ForwardOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::ForwardOperator")
                .SetParent<Operator> ()
        ;

        return tid;
    }

    Ptr<Operator> ForwardOperator::copy(Ptr<Stream> s) {
        auto forwardOperator = CreateObject<ForwardOperator>();
        Operator::copy(forwardOperator, s);
        return forwardOperator;
    }

    void ForwardOperator::process(Ptr<TupleWrapper> tupleWrapper) {
        emit(tupleWrapper);
    }

    int ForwardOperator::calculateMigrationSize(migration_types migration_type) {
        return 1;
    }

    void ForwardOperator::ExtractState(Ptr<MigrationPlan> mp) {

    }

    void ForwardOperator::ImportState(Ptr<PartialStateBatch> batch) {

    }
}