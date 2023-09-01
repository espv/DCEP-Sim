//
// Created by espen on 18.11.2021.
//

#include "ns3/Operator.h"


using namespace ns3;
namespace ns3 {
    TypeId
    SinkOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::SinkOperator")
                .SetParent<Operator> ()
        ;

        return tid;
    }

    Ptr<Operator> SinkOperator::copy(Ptr<Stream> s) {
        auto sinkOperator = CreateObject<SinkOperator>();
        Operator::copy(sinkOperator, s);
        return sinkOperator;
    }

    void SinkOperator::process(Ptr<TupleWrapper> tupleWrapper) {
        ++number_tuples;
        ++total_number_tuples;
        time_last_received = Simulator::Now().GetMilliSeconds();
        if (tupleWrapper->tuples[0]->stream_id == 21) {
            // We know we have count, and can use it to trace
            //static int count_accumulator = 0;
            //NS_ASSERT(tupleWrapper->tuples[0]->longValues[1] == 2);
            Metrics::count_accumulator += tupleWrapper->tuples[0]->longValues[1];
            Metrics::sink_received_tuples[tupleWrapper->group->id].emplace_back(tupleWrapper);
            //for (int i = 0; i < tupleWrapper->tuples[0]->longValues[1]; i++) {
            //    Metrics::sink_received_tuples[tupleWrapper->group->id].emplace_back(tupleWrapper);
            //}
            /*if (number_tuples % 1000 == 0 || number_tuples > 8000 || number_tuples == 1) {
                std::cout << Simulator::Now().GetMilliSeconds() << " Sink operator receiving tuple " << number_tuples << ", and total count of tuples in window extents: " << count_accumulator << std::endl;
            }*/
        } else {
            Metrics::count_accumulator++;
            Metrics::sink_received_tuples[tupleWrapper->group->id].emplace_back(tupleWrapper);
        }
        //std::cout << "Sink receives group " << tupleWrapper->group->id << std::endl;
    }

    int SinkOperator::calculateMigrationSize(migration_types migration_type) {
        return 1;
    }

    void SinkOperator::ExtractState(Ptr<MigrationPlan> mp) {

    }

    void SinkOperator::ImportState(Ptr<PartialStateBatch> batch) {

    }
}
