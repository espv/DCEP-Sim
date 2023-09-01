//
// Created by espen on 22.11.2021.
//

#include "ns3/simulator.h"
#include "ns3/CpuCore.h"

namespace ns3 {
    NS_OBJECT_ENSURE_REGISTERED(CpuCore);
    NS_LOG_COMPONENT_DEFINE ("CpuCore");

    TypeId
    CpuCore::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::CpuCore")
                .SetParent<Object> ()
        ;

        return tid;
    }

    CpuCore::CpuCore(const Ptr<CpuCore>& prevThread, Ptr<NodeResourceManager> resourceManager) {
        this->resourceManager = resourceManager;
    }

    struct sort_by_rowtime
    {
        inline bool operator() (const std::tuple<Ptr<Operator>, Time, Ptr<TupleWrapper>>& t1, const std::tuple<Ptr<Operator>, Time, Ptr<TupleWrapper>>& t2)
        {
            return (std::get<1>(t1) < std::get<1>(t2));
        }
    };

    void CpuCore::processTuple(const Ptr<Operator>& op, const Ptr<TupleWrapper>& tupleWrapper) {
        Metrics::TraceReceived(tupleWrapper, this->resourceManager->node_id, op->query, op);
        if (currently_processing || (!op->active && !op->source && !op->sink)) {
            //operatorIdToTupleQueue[op->id].emplace_back(std::tuple(op, tupleWrapper->getTime(), tupleWrapper));
            operatorIdToTupleQueue[op->id].emplace_back(std::tuple(op, Simulator::Now(), tupleWrapper));
            return;
        }
        doProcessTuple(op, tupleWrapper);
    }

    std::tuple<Ptr<Operator>, Time, Ptr<TupleWrapper>> CpuCore::popRandomTupleQueue() {
        Ptr<Operator> nextOp;
        Time received_time;
        Ptr<TupleWrapper> nextTupleWrapper;
        //bool found = false;
        // Why are also choosing inactive operators?
        std::vector<int> operator_ids;
        for (auto &[operator_id, tuple_queue] : this->operatorIdToTupleQueue) {
            operator_ids.emplace_back(operator_id);
        }
        std::sort(operator_ids.begin(), operator_ids.end());
        int i;
        for (i = operator_ids.size()-1; i >= 0; i--) {
            if (!operatorIdToTupleQueue[operator_ids[i]].empty() && std::get<0>(operatorIdToTupleQueue[operator_ids[i]].front())->active) {
                break;
            }
        }
        auto &tuple_queue = operatorIdToTupleQueue[operator_ids[i]];
        auto t = tuple_queue.front();
        nextOp = std::get<0>(t);
        received_time = std::get<1>(t);
        nextTupleWrapper = std::get<2>(t);
        if (!nextOp->source && !nextOp->sink) {
            if (!Metrics::total_input_tuple_latency.count(nextOp->type) || !Metrics::total_input_tuple_latency[nextOp->type].count(nextOp->id)) {
                Metrics::total_input_tuple_latency[nextOp->type][nextOp->id] = Time(0);
            }
            if (Metrics::forwarded_tuple_input_latency.count(nextTupleWrapper->tuples[0]->id) && Metrics::forwarded_tuple_input_latency[nextTupleWrapper->tuples[0]->id].count(nextOp->id)) {
                Metrics::total_input_tuple_latency[nextOp->type][nextOp->id] += Metrics::forwarded_tuple_input_latency[nextTupleWrapper->tuples[0]->id][nextOp->id];
                Metrics::forwarded_tuple_input_latency[nextTupleWrapper->tuples[0]->id].erase(nextOp->id);
            }
            Metrics::total_input_tuple_latency[nextOp->type][nextOp->id] += Simulator::Now() - received_time;
        }
        tuple_queue.pop_front();

        /*while (!found) {
            auto it = this->operatorIdToTupleQueue.begin();
            //std::advance(it, rand() % this->operatorIdToTupleQueue.size());
            while (it != this->operatorIdToTupleQueue.end()) {
                if (!it->second.empty()) {
                    break;
                }
                it++;
            }
            auto &tuple_queue = it->second;
            if (tuple_queue.empty() || !(std::get<0>(tuple_queue[0])->active || std::get<0>(tuple_queue[0])->in_migration)) {
                continue;
            }
            found = true;
            auto t = tuple_queue.front();
            nextOp = std::get<0>(t);
            received_time = std::get<1>(t);
            nextTupleWrapper = std::get<2>(t);
            tuple_queue.pop_front();
        }*/
        /*if (this->operatorIdToTupleQueue == operatorIdToTupleQueue && nextOp->type != passthroughType) {
            //std::cout << "During normal execution, tuple had to wait " << (Simulator::Now() - received_time).GetMilliSeconds() << " milliseconds" << std::endl;
            //Metrics::AddTupleLatency((Simulator::Now() - received_time).GetMilliSeconds());
        } else if (this->operatorIdToTupleQueue == operatorIdToMigrationTupleQueue) {
            Metrics::AddTupleLatency((Simulator::Now() - received_time).GetMilliSeconds());
            std::cout << "During migration, tuple had to wait " << (Simulator::Now() - received_time).GetMilliSeconds() << " milliseconds" << std::endl;
        } else {
            //NS_FATAL_ERROR("this->operatorIdToTupleQueue is not correct!");
        }*/
        return {nextOp, received_time, nextTupleWrapper};
    }

    void CpuCore::processQueuedTuple() {
        bool any_tuples_stored = std::any_of(this->operatorIdToTupleQueue.begin(), this->operatorIdToTupleQueue.end(), [](const auto &p){return std::any_of(p.second.begin(), p.second.end(), [](const auto &tuple){return (std::get<0>(tuple)->active);}); });
        //bool any_tuples_stored = std::any_of(this->operatorIdToTupleQueue.begin(), this->operatorIdToTupleQueue.end(), [](const auto &p){return std::any_of(p.second.begin(), p.second.end(), [](const auto &tuple){return (std::get<0>(tuple)->active || std::get<0>(tuple)->in_migration);}); });
        if (currently_processing || !any_tuples_stored) {
            return;
        }
        // Now we choose random operator to process
        auto t = popRandomTupleQueue();
        Ptr<Operator> nextOp = std::get<0>(t);
        Ptr<TupleWrapper> nextTupleWrapper = std::get<2>(t);

        nextOp->SendToNextOp(nextTupleWrapper, nextOp, false);
    }

    void CpuCore::doneProcessing(Ptr<TupleWrapper> tuple_wrapper) {
        --number_tuples_processing;

        NS_ASSERT(this->currently_processing == true);
        this->currently_processing = false;
        processQueuedTuple();
    }

    void CpuCore::doProcessTuple(const Ptr<Operator>& op, const Ptr<TupleWrapper>& tupleWrapper) {
        this->currently_processing = true;
        op->emit_time_delta = op->get_base_processing_time();
        op->time_for_next_forward = Time(0);
        ++number_tuples_processing;
        ++op->tuples_processed;
        op->query->setTime(tupleWrapper);
        tupleWrapper->query = op->query;
        op->process(tupleWrapper);
        //op->emit_time_delta += MicroSeconds(1);
        Metrics::TraceIngestion(tupleWrapper, this->resourceManager->node_id, op->query, op);
        Simulator::Schedule(op->emit_time_delta, &CpuCore::doneProcessing, this, tupleWrapper);
    }
}
