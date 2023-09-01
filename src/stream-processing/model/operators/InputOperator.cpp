//
// Created by espen on 03.11.2021.
//
#include "ns3/Operator.h"
#include "ns3/log.h"


using namespace ns3;
namespace ns3 {
    NS_OBJECT_ENSURE_REGISTERED(InputOperator);
    NS_LOG_COMPONENT_DEFINE ("InputOperator");

    TypeId
    InputOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::InputOperator")
                .SetParent<Operator> ()
        ;

        return tid;
    }

    InputOperator::InputOperator() : Operator(inputType) {this->alias = "";}

    InputOperator::InputOperator(std::string alias) : Operator(inputType) {this->alias = alias;}

    void InputOperator::process(Ptr<TupleWrapper> tupleWrapper) {
        ++number_tuples;
        //static int cnt = 0;
        ////if (number_tuples % 1000) {
        //    NS_LOG_INFO("InputOperator " << this->id << " number tuples: " << ++number_tuples << ", next node Id: "
        //              << stream->next_stream->operators[0]->node_id << ", total cnt: " << ++cnt);
        //}
        NS_ASSERT_MSG(tupleWrapper->tuples.size() == 1, "Only one tuple may exist in the InputOperator");

        // We assign the alias before copying the tuple wrapper, because we use the alias to determine the attribute order
        tupleWrapper->tuples[0]->alias = this->alias;
        auto outTupleWrapper = CreateObject<TupleWrapper>(tupleWrapper, this->query);
        emit(outTupleWrapper);
    }

    int InputOperator::calculateMigrationSize(migration_types migration_type) {
        return 1;
    }

    void InputOperator::ExtractState(Ptr<MigrationPlan> mp) {

    }

    void InputOperator::ImportState(Ptr<PartialStateBatch> batch) {

    }
}
