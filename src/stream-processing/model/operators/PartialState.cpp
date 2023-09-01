//
// Created by espen on 10.10.2022.
//

#include "ns3/Operator.h"

using namespace ns3;
namespace ns3 {
    TypeId
    PartialState::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::PartialState")
                .SetParent<Object>();

        return tid;
    }

    TypeId
    PartialStateWrapper::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::PartialStateWrapper")
                .SetParent<Object>();

        return tid;
    }

    NS_OBJECT_ENSURE_REGISTERED(PartialState);

    NS_LOG_COMPONENT_DEFINE ("PartialState");

    TypeId
    TuplePartialState::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::TuplePartialState")
                .SetParent<PartialState>();

        return tid;
    }

    TypeId
    SequencePartialState::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::SequencePartialState")
                .SetParent<PartialState>();

        return tid;
    }

    TypeId
    WindowExtent::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::WindowExtent")
                .SetParent<PartialState> ()
        ;

        return tid;
    }

    TypeId
    PartialStateBatch::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::PartialStateBatch")
                .SetParent<Object>();

        return tid;
    }

    TypeId
    DeadlinePartialStateBatch::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::DeadlinePartialStateBatch")
                .SetParent<PartialStateBatch>();

        return tid;
    }

    size_t
    PartialStateBatch::GetSerializedSize() {
        size_t size = sizeof(PartialStateBatch);
        for (auto ps : partial_states) {
            size += ps->size;
        }
        NS_ASSERT(size > 0);
        return size;
    }

    void PartialStateBatch::merge(Ptr<PartialStateBatch> batch) {
        partial_states.insert(this->partial_states.end(), batch->partial_states.begin(), batch->partial_states.end());
    }

    PartialStateWrapper::PartialStateWrapper(Ptr<PartialState> object, int size, Ptr<Operator> op) {
        this->object = object;
        this->size = size;
        this->op = op;
    }

    PartialStateWrapper::PartialStateWrapper(Ptr<PartialState> object, int size) {
        this->object = object;
        this->size = size;
    }

    PartialStateWrapper::PartialStateWrapper(Ptr<PartialState> object) {
        this->object = object;
        this->size = object->GetSerializedSize();
    }

    PartialStateWrapper::PartialStateWrapper() {
        this->object = nullptr;
        this->size = 0;
    }

    PartialStateBatch::PartialStateBatch(Ptr<Operator> op, Ptr<MigrationPlan> mp) {
        dcepHeader.arguments["query_type"] = op->type;
        dcepHeader.arguments["query_id"] = op->query->id;
        dcepHeader.arguments["unique_query_id"] = op->query->unique_query_id;
        this->op = op;
        this->mp = mp;
        dcepHeader.arguments["operator_id"] = op->id;
        //dcepHeader.arguments["old_host"] = op->node_id;
        dcepHeader.arguments["migration_type"] = STANDARD_MOVING_STATE;
        dcepHeader.SetContentType(MIGRATION);
    }

    DeadlinePartialStateBatch::DeadlinePartialStateBatch(Ptr<Operator> op, Ptr<MigrationPlan> mp) : PartialStateBatch(op, mp) {}

    Time PartialStateBatch::willBeSentBy(Time sentAt) {
        auto size = GetSerializedSize();
        // TODO: Add TCP overhead
        return sentAt + ExposeCoordinator::bw_migration.CalculateBytesTxTime(size);
    }

    bool DeadlinePartialStateBatch::canBeSentAfter(Ptr<PartialStateBatch> batch) {
        return batch->willBeSentBy(Simulator::Now()) < this->must_be_sent_by;
    }
}