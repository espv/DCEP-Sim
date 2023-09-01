
#include "ns3/Operator.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/communication.h"
#include "ns3/dcep-module.h"
#include "ns3/ptr.h"


namespace ns3 {
    NS_OBJECT_ENSURE_REGISTERED(Operator);
    NS_LOG_COMPONENT_DEFINE ("Operator");

    TypeId
    Operator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::Operator")
                .SetParent<PartialState> ()
        ;

        return tid;
    }

    Operator::Operator(OperatorType type) {
        this->type = type;
        this->id = id_cnt++;
        switch (type) {
            case withinType:
            case windowType:
            case groupType:
            case joinType:
            case selectType:
            case cepType:
                this->source_sinkable = false;
                break;
            case filterType:
            case printType:
            case sinkType:
            case forwardType:
            case defineType:
            case passthroughType:
            case inputType:
                this->source_sinkable = true;
                break;
        }
        switch (type) {
            case windowType:
            case groupType:
            case joinType:
            case cepType:
                this->stateful = true;
                break;
            case filterType:
            case selectType:
            case printType:
            case sinkType:
            case passthroughType:
            case forwardType:
            case defineType:
            case inputType:
            case withinType:
                this->stateful = false;
                algorithms.emplace_back("-----");
                break;
            }
        switch (type) {
            case windowType:
                // Standard moving state
                algorithms.emplace_back("SMSA-");
                // State recreation
                algorithms.emplace_back("SMPA-");
                // Window recreation
                algorithms.emplace_back("S-PA-");
                break;
            case groupType:
            case joinType:
            case cepType:
                // Standard moving state
                algorithms.emplace_back("SMSA-");
                // State recreation
                algorithms.emplace_back("SMPA-");
                break;
            case filterType:
            case selectType:
            case passthroughType:
            case inputType:
            case printType:
            case sinkType:
            case defineType:
            case forwardType:
            case withinType:
              break;
            }
    }

    Operator::~Operator() = default;

    bool Operator::ProcessOnHost(int host, Ptr<TupleWrapper> tuple_wrapper) {
        auto wrapper = getExposeWrapper();
        auto mp = wrapper->mp;
        if (mp != nullptr && mp->migration_mode == "megaphone") {
            // If the group is not to be forwarded to the new host, we compare the host with the old node ID
            // Otherwise, it's the same procedure as in all other cases
            if (Operator::queryIdToOperatorIdToGroupIdToNewHost[this->query->id][this->id].count(tuple_wrapper->group->id)) {
                return (Operator::queryIdToOperatorIdToGroupIdToNewHost[this->query->id][this->id][tuple_wrapper->group->id] && this->node_id == host) ||
                       (!Operator::queryIdToOperatorIdToGroupIdToNewHost[this->query->id][this->id][tuple_wrapper->group->id] && this->old_node_id == host);
            }
        }
        return this->node_id == host;
    }

    static inline int cnt = 0;
    //static inline int cnt2 = 0;
    std::map<int, bool> sent_to_old_host;
    std::map<int, bool> sent_to_new_host;
    void Operator::EmitToNextOp(Ptr<Operator> next_op, Ptr<TupleWrapper> tupleWrapper, bool add_processing_delay) {
        if (this->thread == nullptr) {
            auto resource_manager = StreamQuery::nodeIdToNode[node_id]->GetObject<NodeResourceManager>();
            SetCpuCore(resource_manager->AssignCore(this));
        }

        if (tupleWrapper->getTime().IsZero()) {
            tupleWrapper->setTime(Simulator::Now());
        }
        int cur_node_id = std::stoi(StreamQuery::ns3NodeIdToExposeNodeId[tupleWrapper->node->GetId()]);
        auto &mp = ExposeCoordinator::nodeIdsToWrappers[std::to_string(cur_node_id)]->mp;
        int next_node_id = next_op->node_id;

        /*if (tupleWrapper->tuples[0]->id >= 4999 && next_op->type == groupType) {
            std::cout << std::endl;
        }*/

        if (mp != nullptr && next_op->in_migration && mp->migration_mode == "megaphone") {
            // Check if we forward to new host
            // Three cases:
            // 1: queryIdToOperatorIdToGroupIdToNewHost is filled out for the group and is true
            // 2: queryIdToOperatorIdToGroupIdToNewHost is filled out for the group and is false
            // 3: queryIdToOperatorIdToGroupIdToNewHost is not filled out for the group
            // These result in different things happening:
            // 1 means that the related state to the tuple is migrated or is being migrated. Therefore, the tuples
            // of this key must be forwarded to the new host
            // 2 means that the related state to the tuple is still on the old host. Therefore, the tuples of this key
            // must be forwarded to the old host
            // 3 means that the key has no state at all in the operator. Therefore, the tuples of this key must be
            // forwarded to the new host.
            // Only Megaphone will fill out queryIdToOperatorIdToGroupIdToNewHost, but that's fine, because all other
            // migration mechanisms will then automatically forward to the new host
            if (Operator::queryIdToOperatorIdToGroupIdToNewHost.count(this->query->id) &&
                Operator::queryIdToOperatorIdToGroupIdToNewHost[this->query->id].count(next_op->id) &&
                Operator::queryIdToOperatorIdToGroupIdToNewHost[this->query->id][next_op->id].count(tupleWrapper->group->id)) {
                if (Operator::queryIdToOperatorIdToGroupIdToNewHost[this->query->id][next_op->id][tupleWrapper->group->id]) {
                    next_node_id = mp->operatorIdToNewHost[next_op->id];
                    if (++cnt % 100 == 0) {
                        std::cout << "Sending to new host " << cnt << std::endl;
                    }
                    sent_to_new_host[tupleWrapper->tuples[0]->id] = true;
                    if (sent_to_old_host[tupleWrapper->tuples[0]->id]) {
                        // We send this to the old host, and now we're sending it to the new host?
                        std::cout << "Seems like a tuple was first sent to the old host, before now immediately being forwarded to the new host" << std::endl;
                    }
                } else {
                    //std::cout << Simulator::Now().GetMilliSeconds() << " Sending to old host " << ++cnt2 << std::endl;
                    sent_to_old_host[tupleWrapper->tuples[0]->id] = true;
                    if (sent_to_new_host[tupleWrapper->tuples[0]->id]) {
                        // We send this to the old host, and now we're sending it to the new host?
                        std::cout << "Seems like a tuple was first sent to the new host, before now immediately being forwarded to the old host" << std::endl;
                    }
                    next_node_id = mp->old_host;
                }
            }
        }

        // What to do next with the tuple?
        if (this->sink || next_op->source || (cur_node_id == next_node_id && !next_op->sink) || (next_op->sink && this->thread->resourceManager->IsSinkNode(this->query))) {
            // TODO: THIS SHOULD ACTUALLY TAKE PROCESSING TIME
            if (!next_op->thread) {
                next_op->SetCpuCore(this->thread->resourceManager->AssignCore(next_op));
            }
            /*static int select_cnt = 0;
            if (next_op->type == selectType) {
                //std::cout << std::endl;
            }
            if (next_op->type == selectType && ++select_cnt > 490000) {
                std::cout << Simulator::Now().GetMilliSeconds() << " Forwarding tuple " << select_cnt << " to select9292" << std::endl;
            }*/
            if (!add_processing_delay) {
                next_op->thread->doProcessTuple(next_op, tupleWrapper);
            } else {
                next_op->thread->processTuple(next_op, tupleWrapper);
            }
            //next_op->thread->processTuple(next_op, tupleWrapper);
            //Dcep::GetDcepFromNode(StreamQuery::nodeIdToNode[cur_node_id]->GetObject<ExposeWrapper>()->node)->GetObject<CEPEngine>()->ProcessCepEvent(tupleWrapper);
            //next_op->thread->processTuple(next_op, tupleWrapper);
        } else {
            if (!this->sink && next_op->sink) {
                for (int sink_node_id : this->query->sink_nodes) {
                    Simulator::Schedule(time_for_next_forward, &ExposeWrapper::SendTupleTo, StreamQuery::nodeIdToNode[this->node_id]->GetObject<ExposeWrapper>(), tupleWrapper, sink_node_id);
                }
            } else {
                // If parallel track
                if (next_op->parallel_old_node_id != -1) {
                    Simulator::Schedule(time_for_next_forward, &ExposeWrapper::SendTupleTo, StreamQuery::nodeIdToNode[this->node_id]->GetObject<ExposeWrapper>(), tupleWrapper, next_op->parallel_old_node_id);
                }

                if (next_op->in_migration && next_op->old_node_id == this->node_id) {
                    // We're sending to the new host, but would originally have processed on this host
                    std::cout << Simulator::Now().GetMilliSeconds() << " Operator " << next_op->id << " with type " << next_op->type << " is now forwarded to the new host " << next_node_id << ", but would originally have been processed on this host (old host): " << this->node_id << std::endl;
                }
                Simulator::Schedule(time_for_next_forward, &ExposeWrapper::SendTupleTo, StreamQuery::nodeIdToNode[cur_node_id]->GetObject<ExposeWrapper>(), tupleWrapper, next_node_id);
            }
            time_for_next_forward += get_forward_time_added();
        }
    }

    void Operator::SendToNextOp(Ptr<TupleWrapper> tupleWrapper, Ptr<Operator> next_op, bool add_processing_delay) {
        if (add_processing_delay) {
            Simulator::Schedule(emit_time_delta, &Operator::EmitToNextOp, this, next_op, tupleWrapper, add_processing_delay);
        } else {
            EmitToNextOp(next_op, tupleWrapper, add_processing_delay);
        }
    }

    Time Operator::get_base_processing_time() {return MicroSeconds(10);}
    Time Operator::get_emit_time_added() {return MicroSeconds(0);}
    Time Operator::get_forward_time_added() {return MicroSeconds(0);}

    void Operator::emit(Ptr<TupleWrapper> tupleWrapper) {
        tupleWrapper->query = this->query;
        //NS_LOG_INFO(Simulator::Now() << " Operator::emit, operator type: " << this->type);
        NS_ASSERT(stream->next_stream != nullptr);
        //this->emit_time_delta += get_emit_time_added();

        int part = 0;
        if (tupleWrapper->group != nullptr) {
            part = (int) tupleWrapper->group->partition;
        }

        auto next_op = this->stream->next_stream->operators[part%this->stream->next_stream->operators.size()];
        tupleWrapper->next_operator_id = next_op->id;

        SendToNextOp(tupleWrapper, next_op, true);
    }

    void Operator::SetNodeId(int new_node_id) {
        this->node_id = new_node_id;
        if (thread == nullptr || thread->resourceManager->node_id != new_node_id) {
            auto resource_manager = StreamQuery::nodeIdToNode[new_node_id]->GetObject<NodeResourceManager>();
            //thread = resource_manager->AssignCore(this);
        }
    }

    void Operator::SetCpuCore(Ptr<CpuCore> cpu_core) {
        // Nothing to do when the cpu core is already used
        if (cpu_core == this->thread) {
            return;
        }
        if (thread && thread->operatorIdToTupleQueue.count(id)) {
            auto it = thread->operatorIdToTupleQueue.find(id);
            thread->operatorIdToTupleQueue.erase(it);
        }
        // This removes tuples we want to forward during migration
        cpu_core->operatorIdToTupleQueue[id] = {};
        this->thread = cpu_core;
    }

    void Operator::ForwardTuplesTo(int host) {
        if (this->thread == nullptr) {
            NS_FATAL_ERROR("Operator has nullptr thread!");
        }
        //if (!this->thread->operatorIdToTupleQueue[this->id].empty()) {
        std::cout << "op " << this->id << " forwarding " << this->thread->operatorIdToTupleQueue[this->id].size()
                  << " tuples from old host to new host" << std::endl;
        //}
        for (auto p : this->thread->operatorIdToTupleQueue[this->id]) {
            //std::cout << "Forwarding tuple " << std::get<2>(p)->tuples[0]->id << " from old host to new host with group " << std::get<2>(p)->group->id << std::endl;
            Metrics::forwarded_tuples[std::get<2>(p)->tuples[0]->id] = std::get<1>(p);
            getExposeWrapper()->SendTupleTo(std::get<2>(p), host);
        }
        this->thread->operatorIdToTupleQueue[this->id].clear();
    }

    Ptr<PartialStateBatch> Operator::ExtractPreambleState(Ptr<MigrationPlan> mp) {
        auto static_batch = CreateObject<PartialStateBatch> (this, mp);
        // We add a wrapper to represent the static state of the operator itself
        auto op_wrapper = CreateObject<PartialStateWrapper> (this);
        static_batch->partial_states.emplace_back (op_wrapper);
        mp->preambleQueue.emplace_back (static_batch);
        return static_batch;
    }

    void Operator::UnassignCpuCore() {
        // This removes tuples we want to forward during migration
        /*if (thread && thread->operatorIdToTupleQueue.count(id)) {
            //auto it = thread->operatorIdToTupleQueue.find(id);
            //thread->operatorIdToTupleQueue.erase(it);
        }*/
        thread = nullptr;
    }

    size_t Operator::GetSerializedSize() {
        return 1000;
    }

    size_t Operator::GetUtility() {
        return 1;
    }

    Ptr<Operator> Operator::copy(Ptr<Operator> new_operator, Ptr<Stream> s) {
        NS_ASSERT(this->type == new_operator->type);
        new_operator->stream = s;
        new_operator->query = this->query;
        new_operator->partition = this->partition;
        new_operator->windowOperator = this->windowOperator;
        //new_operator->SetCpuCore(this->thread);
        new_operator->previous_operator = this->previous_operator;
        new_operator->SetNodeId(this->node_id);
        new_operator->sink = this->sink;
        new_operator->source = this->source;
        new_operator->source_sinkable = this->source_sinkable;
        new_operator->stateful = this->stateful;
        new_operator->id = this->id;
        new_operator->name = this->name;
        return new_operator;
    }

    void Operator::SetActive() {
        this->active = true;
    }

    void Operator::SetInactive() {
        this->active = false;
        //this->UnassignCpuCore();
    }

    Ptr<ExposeWrapper> Operator::getExposeWrapper() {
        return ExposeCoordinator::nodeIdsToWrappers[std::to_string(this->thread->resourceManager->node_id)];
    }
}
