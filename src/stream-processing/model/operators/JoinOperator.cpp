//
// Created by espen on 27.10.2021.
//

#include <algorithm>
#include "ns3/Operator.h"


using namespace ns3;
using namespace std;
namespace ns3 {
    TypeId
    JoinOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::JoinOperator")
                .SetParent<Operator> ()
        ;

        return tid;
    }

    NS_OBJECT_ENSURE_REGISTERED(JoinOperator);
    NS_LOG_COMPONENT_DEFINE ("JoinOperator");

    JoinOperator::JoinOperator(Ptr<Stream> joining_stream) : Operator(joinType) {
        this->stream2 = joining_stream;
    }

    // Join operator can't be copied like other operators because pointers are used by other operators
    // If we create a new operator object, those pointers will be void
    Ptr<Operator> JoinOperator::copy(Ptr<Stream> s) {
        /*auto joinOperator = CreateObject<JoinOperator>(this->stream2);
        joinOperator->stream = s;
        joinOperator->partition = this->partition;
        joinOperator->windowOperator = this->windowOperator;
        joinOperator->previous_operator = this->previous_operator;
        joinOperator->SetNodeId(this->node_id);
        joinOperator->sink = this->sink;
        joinOperator->source = this->source;
        joinOperator->source_sinkable = this->source_sinkable;
        joinOperator->stateful = this->stateful;
        joinOperator->id = this->id;
        joinOperator->name = this->name;

        // Join-specific things
        joinOperator->stream2 = this->stream2;
        joinOperator->windowOperator2 = this->windowOperator2;
        joinOperator->filterOperators = this->filterOperators;
        joinOperator->pending_matches = this->pending_matches;
        joinOperator->join_depth = this->join_depth;
        return joinOperator;*/
        return this;
    }

    static int number_imported = 0;
    static int number_extracted = 0;
    void JoinOperator::ExtractEssentialState(Ptr<MigrationPlan> mp) {

    }

    void JoinOperator::ExtractUnessentialState(Ptr<MigrationPlan> mp) {

    }

    void JoinOperator::updateWindows(long external_timestamp) const {
        windowOperator1->update_external_time_window(external_timestamp);
        DynamicCast<StorageWindowOperator>(windowOperator1)->updateWindow();
        if (windowOperator1->size_type == INTERMEDIARY_JOIN) {
            auto otherJoin = DynamicCast<JoinOperator>(windowOperator1->stream->previous_stream->operators[0]);
            otherJoin->updateWindows(external_timestamp);
        }
        windowOperator2->update_external_time_window(external_timestamp);
        DynamicCast<StorageWindowOperator>(windowOperator2)->updateWindow();
        if (windowOperator2->size_type == INTERMEDIARY_JOIN) {
            auto otherJoin = DynamicCast<JoinOperator>(windowOperator2->stream->previous_stream->operators[0]);
            otherJoin->updateWindows(external_timestamp);
        }
    }

    void JoinOperator::updateWindow1(long external_timestamp) const {
        windowOperator1->update_external_time_window(external_timestamp);
        DynamicCast<StorageWindowOperator>(windowOperator1)->updateWindow();
        if (windowOperator1->size_type == INTERMEDIARY_JOIN) {
            auto otherJoin = DynamicCast<JoinOperator>(windowOperator1->stream->previous_stream->operators[0]);
            otherJoin->updateWindows(external_timestamp);
        }
    }

    void JoinOperator::updateWindow2(long external_timestamp) const {
        windowOperator2->update_external_time_window(external_timestamp);
        DynamicCast<StorageWindowOperator>(windowOperator2)->updateWindow();
        if (windowOperator2->size_type == INTERMEDIARY_JOIN) {
            auto otherJoin = DynamicCast<JoinOperator>(windowOperator2->stream->previous_stream->operators[0]);
            otherJoin->updateWindows(external_timestamp);
        }
    }

    void JoinOperator::InsertPendingMatch(std::pair<Ptr<TupleWrapper>, Ptr<TupleWrapper>> pending_match) {
        pending_matches.emplace_back(pending_match);
    }

    void JoinOperator::process(Ptr<TupleWrapper> tupleWrapper) {
        static int number_tuples = 0;
        ++number_tuples;
        if (number_tuples % 1000 == 0) {
           NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id
                      << " Join receiving tuple " << number_tuples << " with id " << tupleWrapper->tuples[0]->id
                      << " and stream id " << tupleWrapper->tuples[0]->stream_id << ", tuples in window 1: "
                      << windowOperator1->tuples.size() << ", tuples in window 2: " << windowOperator2->tuples.size());
        }
        std::vector < Ptr<TupleWrapper>  > matching_tuples;
        if (tupleWrapper->cmp(this->stream->input_stream_ids)) {
            //std::cout << "Received stream 1 tuple " << ++stream1_tuple_cnt << std::endl;
            if (windowOperator2->current_external_time < windowOperator1->current_external_time) {
                updateWindow2(windowOperator1->current_external_time);
            }
            std::vector<std::vector<Ptr<TupleWrapper>>> matches;
            std::vector<std::vector<Ptr<TupleWrapper>>> tmp_matches(filterOperators.size());
            for (int i = 0; i < (int) filterOperators.size(); i++) {
                auto fo = filterOperators[i];
                auto arg1 = DynamicCast<AttributeFilterArgument>(fo->arg1);
                arg1->cache(tupleWrapper);
                matches.push_back(fo->getArg2Matches(tupleWrapper));
            }

            if (matches.empty()) {
                // We need a vector to work with below
                matches.emplace_back();
            }
            NS_ASSERT(!matches.empty());
            auto arg_match = matches[0];
            for (auto tw: arg_match) {
                bool match = true;
                for (int j = 1; j < (int)matches.size(); j++) {
                    auto &match_vector = matches[j];
                    if (std::find(match_vector.begin(), match_vector.end(), tw) == match_vector.end()) {
                        match = false;
                        break;
                    }
                }
                if (!match || tw->evicted) {
                    continue;
                }
                // Match found
                matching_tuples.push_back(tw);
            }
        } else if (tupleWrapper->cmp(this->stream2->input_stream_ids)) {
            //if (rand()%100 > selectivity) {
            //    for (int i = 0; i < 7; i++) {
            //        tupleWrapper->tuples[0]->longValues[i] = 9999999;
            //    }
            //}
            //std::cout << "Received stream 2 tuple " << ++stream2_tuple_cnt << std::endl;
            if (windowOperator1->current_external_time < windowOperator2->current_external_time) {
                updateWindow1(windowOperator2->current_external_time);
            }

            std::vector<std::vector<Ptr<TupleWrapper>>> matches;
            std::vector<std::vector<Ptr<TupleWrapper>>> tmp_matches(filterOperators.size());
            for (int i = 0; i < (int) filterOperators.size(); i++) {
                auto fo = filterOperators[i];
                auto arg2 = DynamicCast<AttributeFilterArgument>(fo->arg2);
                arg2->cache(tupleWrapper);
                matches.push_back(fo->getArg1Matches(tupleWrapper));
            }

            if (matches.empty()) {
                // We need a vector to work with below
                matches.emplace_back();
            }
            NS_ASSERT(!matches.empty());
            auto arg_match = matches[0];
            for (auto tw: arg_match) {
                bool match = true;
                for (int j = 1; j < (int)matches.size(); j++) {
                    auto &match_vector = matches[j];
                    if (std::find(match_vector.begin(), match_vector.end(), tw) == match_vector.end()) {
                        match = false;
                        break;
                    }
                }
                if (!match || tw->evicted) {
                    continue;
                }
                // Match found
                matching_tuples.push_back(tw);
            }
        } else {
            // Invalid join
            exit(5);
        }
        auto output_stream_ids = this->stream->output_stream_ids;
        for (Ptr<TupleWrapper> t: matching_tuples) {
            std::vector<Ptr<Tuple>> output_tuples(output_stream_ids.size());
            for (auto tuple: tupleWrapper->tuples) {
                int stream_id = tuple->stream_id;
                ++tuple->join_depth;
                output_tuples[stream->output_stream_id_to_tuple_position[stream_id]] = tuple;
            }
            for (auto tuple: t->tuples) {
                int stream_id = tuple->stream_id;
                ++tuple->join_depth;
                output_tuples[stream->output_stream_id_to_tuple_position[stream_id]] = tuple;
            }

            auto joinedTupleWrapper = CreateObject<TupleWrapper>(output_tuples);
            joinedTupleWrapper->node = tupleWrapper->node;
            // || true is used for testing fetch requests
            // It was a mistake to include Meces here, as Meces does not support the same kind of fetching that Lazy does
            if (!t->imported_essential_attributes && this->thread->resourceManager->node_id != this->old_node_id && t->in_migration && /*(this->getExposeWrapper()->mp->migration_mode == "meces" || */this->getExposeWrapper()->mp->migration_mode == "lazy"/*)*/) {
                // Sending fetch request for the joined tuple
                // TODO: Place t in a queue and then call FetchNextPartialState, similar to MoveNextState()
                auto ps_wrapper = CreateObject<PartialStateWrapper>(t, t->GetSerializedSize(true), this);
                getExposeWrapper()->ps_fetch_queue.emplace_back(std::make_pair(this->old_node_id, (std::deque<Ptr<PartialStateWrapper>>){ps_wrapper}));
                //static int cnt_fetched = 0;
                //std::cout << Simulator::Now().GetMicroSeconds() << " Fetch number " << ++cnt_fetched << std::endl;
                Simulator::Schedule(MilliSeconds(10), &ExposeWrapper::FetchNextPartialState, getExposeWrapper());
                //getExposeWrapper()->FetchNextPartialState();
                InsertPendingMatch(std::make_pair(tupleWrapper, t));
            } else if (t->imported_essential_attributes || this->thread->resourceManager->node_id == this->old_node_id) {
                static int emitted_tuples = 0;
                static int total_size = 0;
                total_size += joinedTupleWrapper->GetSerializedSize(true);
                if (++emitted_tuples % 1000 == 0) {
                    NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id
                                                                   << " Join emitting tuple " << emitted_tuples
                                                                   << ", Total size: " << total_size
                                                                   << ", Source tuple " << tupleWrapper->tuples[0]->id
                                                                   << ", Joined tuple: "
                                                                   << joinedTupleWrapper->tuples[1]->id);
                }
                Metrics::latency_data.emplace_back(TraceOutputLatencyEvent(joinedTupleWrapper, joinedTupleWrapper->tuples[0]->id));
                /*auto text = std::to_string(Simulator::Now().GetMilliSeconds()) + " Node " + std::to_string(this->thread->resourceManager->node_id) + " Join emitting (1) tuple " + std::to_string(++this->number_emitted) + ", Source tuple ";
                //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " Join emitting (1) tuple " << ++this->number_emitted << ", Source tuple " << tupleWrapper->tuples[0]->id);
                if (tupleWrapper->tuples[0] == joinedTupleWrapper->tuples[0]) {
                    text += std::to_string(joinedTupleWrapper->tuples[1]->id) + ", Joined tuple: " + std::to_string(joinedTupleWrapper->tuples[0]->id);
                } else {
                    text += std::to_string(joinedTupleWrapper->tuples[0]->id) + ", Joined tuple: " + std::to_string(joinedTupleWrapper->tuples[1]->id);
                }
                NS_LOG_INFO(text);*/
                emit(joinedTupleWrapper);
            } else {
                NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " NODE " << this->thread->resourceManager->node_id << " THIS TUPLE HAS BEEN LOST DUE TO INTERRUPTED MIGRATION");
            }
        }
    }

    void JoinOperator::MigrationInterrupted() {
        std::cout << Simulator::Now().GetMilliSeconds() << " Migration is now interrupted, and the goal is to emit pending matches that have all essential attributes" << std::endl;
        for (auto t : windowOperator1->getPartialStates()) {
            if (t->in_migration && t->imported_essential_attributes) {
                t->in_migration = false;
                DynamicCast<TupleWrapper>(t)->shed_unessential();
            }
        }
        for (auto t : windowOperator2->getPartialStates()) {
            if (t->in_migration && t->imported_essential_attributes) {
                t->in_migration = false;
                DynamicCast<TupleWrapper>(t)->shed_unessential();
            }
        }

        // Set in_migration to false for all tuples in migration that have all essential attributes, whether they are pending matches or not
        for (auto p : pending_matches) {
            auto t1 = DynamicCast<TupleWrapper>(p.first);
            auto t2 = DynamicCast<TupleWrapper>(p.second);
            if (t1->imported_essential_attributes && t2->imported_essential_attributes) {
                //std::cout << "Emitting pending match" << std::endl;
                auto joinedTupleWrapper = CreateObject<TupleWrapper>(std::vector<Ptr<Tuple>>({t1->tuples[0], t2->tuples[0]}));
                joinedTupleWrapper->node = t1->node;
                Metrics::latency_data.emplace_back(TraceOutputLatencyEvent(joinedTupleWrapper, joinedTupleWrapper->tuples[0]->id));
                //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " Join emitting tuple " << ++this->number_emitted << ", Source tuple " << joinedTupleWrapper->tuples[0]->id << ", Joined tuple: " << joinedTupleWrapper->tuples[1]->id);
                emit(joinedTupleWrapper);
            }
        }

        SetActive();
    }

    void JoinOperator::notifyEviction() {
        DynamicCast<StorageWindowOperator>(this->windowOperator)->evicted_tuples = true;
        auto nextOp = this->stream->next_stream->operators[0];
        if (nextOp->type == joinType) {
            DynamicCast<JoinOperator>(nextOp)->notifyEviction();
        }
    }

    int JoinOperator::calculateMigrationSize(migration_types migration_type) {
        int payload_size = 0;
        switch(migration_type) {
            case STANDARD_MOVING_STATE: {
                for (auto tuple : this->windowOperator1->tuples) {
                    payload_size += tuple->GetSerializedSize(true);
                }
                for (auto tuple : this->windowOperator2->tuples) {
                    payload_size += tuple->GetSerializedSize(true);
                }
                break;
            } case WINDOW_RECREATION: {
                break;
            } case STATE_RECREATION: {
                for (auto tuple : this->windowOperator1->tuples) {
                    payload_size += tuple->GetSerializedSize(true);
                }
                for (auto tuple : this->windowOperator2->tuples) {
                    payload_size += tuple->GetSerializedSize(true);
                }
                break;
            }
          case STATELESS:
            break;
          }
        return payload_size;
    }

    void JoinOperator::ExtractLazyState(Ptr<MigrationPlan> mp) {
        std::vector<Ptr<WindowOperator>> window_operators;
        window_operators.emplace_back(windowOperator1);
        window_operators.emplace_back(windowOperator2);
        auto batch_critical = CreateObject<PartialStateBatch>(this, mp);
        for (auto &wo: window_operators) {
            auto batch_fetch = CreateObject<PartialStateBatch>(this, mp);
            // TODO: Should this be filterOperators[0] or should we iterate through all filterOperators?
            auto jf = filterOperators[0];
            for (auto ps: wo->getPartialStates()) {
                ps->in_migration = true;
                auto ps_wrapper_critical = CreateObject<PartialStateWrapper>(ps, 0);
                auto ps_wrapper_fetch = CreateObject<PartialStateWrapper>(ps);
                ps_wrapper_fetch->disable_state = false;
                auto t = DynamicCast<TupleWrapper>(ps);
                ps_wrapper_fetch->size = t->GetSerializedSize(true);

                switch (Tuple::streamIdToFieldToType[jf->arg1->stream_id][jf->arg1->field]) {
                    case NOTYPE:
                        break;
                    case LONG:
                        ps_wrapper_critical->size += sizeof(long);
                        break;
                    case DOUBLE:
                        ps_wrapper_critical->size += sizeof(double);
                        break;
                    case STRING:
                        ps_wrapper_critical->size += jf->arg1->getStringValue(t).size();
                        break;
                    case TIMESTAMP:
                        ps_wrapper_critical->size += sizeof(long);
                        break;
                }
                // timestamp of tuple
                ps_wrapper_critical->size += sizeof(long);
                ps_wrapper_fetch->size -= ps_wrapper_critical->size;
                // Id of tuple is sent in both the critical and fetched state
                ps_wrapper_critical->size += sizeof(long);
                batch_critical->partial_states.emplace_back(ps_wrapper_critical);
                batch_fetch->partial_states.emplace_back(ps_wrapper_fetch);
                if (batch_fetch->partial_states.size() >= ExposeCoordinator::BATCH_SIZE) {
                    mp->normalEssentialQueue.emplace_back(batch_fetch);
                    batch_fetch = CreateObject<PartialStateBatch>(this, mp);
                }
            }

            if (!batch_fetch->partial_states.empty()) {
                number_extracted += batch_fetch->partial_states.size();
                mp->normalEssentialQueue.emplace_back(batch_fetch);
            }
        }

        if (!batch_critical->partial_states.empty()) {
            mp->critQueue.emplace_back(batch_critical);
        }
    }

    void JoinOperator::ExtractUtilityState(Ptr<MigrationPlan> mp) {
        std::vector<Ptr<WindowOperator>> window_operators;
        window_operators.emplace_back(windowOperator1);
        window_operators.emplace_back(windowOperator2);
        for (auto &wo: window_operators) {
            auto batch_essential = CreateObject<PartialStateBatch>(this, mp);
            auto batch_unessential = CreateObject<PartialStateBatch>(this, mp);
            for (auto jf: filterOperators) {
                for (auto ps: wo->getPartialStates()) {
                    ps->in_migration = true;
                    auto ps_wrapper_essential = CreateObject<PartialStateWrapper>(ps, 0);
                    auto ps_wrapper_unessential = CreateObject<PartialStateWrapper>(ps);
                    ps_wrapper_unessential->disable_state = false;
                    auto t = DynamicCast<TupleWrapper>(ps);
                    ps_wrapper_essential->size = t->GetSerializedSize(true);
                    ps_wrapper_unessential->size = t->GetSerializedSize(true);

                    // Here, we deduct the size of essential state for all unessential attributes,
                    //      and deduct the size of the unessential state for all essential attributes
                    for (auto &tuple : t->tuples) {
                      int string_index = 0;
                      for (auto &[index, field] : Tuple::streamIdToIndexToField[tuple->stream_id]) {
                          if (this->query->aliasToEssentialAttributes[tuple->alias].contains (field)) {
                              switch (Tuple::streamIdToFieldToType[tuple->stream_id][field]) {
                                case NOTYPE:
                                  break;
                                case LONG:
                                  ps_wrapper_unessential->size -= sizeof (long);
                                  break;
                                case DOUBLE:
                                  ps_wrapper_unessential->size -= sizeof (double);
                                  break;
                                case STRING:
                                  ps_wrapper_unessential->size -= tuple->getStringValue (string_index++).size ();
                                  break;
                                case TIMESTAMP:
                                  ps_wrapper_unessential->size -= sizeof (long);
                                  break;
                                }
                            } else {
                              switch (Tuple::streamIdToFieldToType[tuple->stream_id][field]) {
                                case NOTYPE:
                                  break;
                                case LONG:
                                  ps_wrapper_essential->size -= sizeof (long);
                                  break;
                                case DOUBLE:
                                  ps_wrapper_essential->size -= sizeof (double);
                                  break;
                                case STRING:
                                  ps_wrapper_essential->size -= tuple->getStringValue (string_index++).size ();
                                  break;
                                case TIMESTAMP:
                                  ps_wrapper_essential->size -= sizeof (long);
                                  break;
                                }
                            }
                        }
                    }

                    // TODO: This should be more refined, since
                    //ps_wrapper_essential->size -= ps_wrapper_unessential->size;

                    batch_essential->partial_states.emplace_back(ps_wrapper_essential);
                    if (batch_essential->partial_states.size() >= 100) {
                        // Why critQueue and not normalEssentialQueue? Because the operator shouldn't restart on the new host
                        // until it has received all the essential state, or, the migration ends
                        mp->critQueue.emplace_back(batch_essential);
                        batch_essential = CreateObject<PartialStateBatch>(this, mp);
                    }
                    batch_unessential->partial_states.emplace_back(ps_wrapper_unessential);
                    if (batch_unessential->partial_states.size() >= 100) {
                        mp->normalUnessentialQueue.emplace_back(batch_unessential);
                        batch_unessential = CreateObject<PartialStateBatch>(this, mp);
                    }
                }
            }
            if (!batch_unessential->partial_states.empty()) {
                mp->normalUnessentialQueue.emplace_back(batch_unessential);
            }
            if (!batch_essential->partial_states.empty()) {
                // Why critQueue and not normalEssentialQueue? Because the operator shouldn't restart on the new host
                // until it has received all the essential state, or, the migration ends
                mp->critQueue.emplace_back(batch_essential);
            }
        }
    }

    void JoinOperator::ExtractDiffState(Ptr<MigrationPlan> mp) {
        auto b = this->first_rhino_batch;
        auto new_b = CreateObject<PartialStateBatch>(this, mp);
        new_b->partial_states.emplace_back(CreateObject<PartialStateWrapper>(this));
        mp->critQueue.emplace_back(new_b);
        new_b = CreateObject<PartialStateBatch>(this, mp);
        for (auto ps: windowOperator1->getPartialStates()) {
            if (std::find_if(b->partial_states.begin(), b->partial_states.end(), [ps](Ptr<PartialStateWrapper> ps_wrapper) {return ps_wrapper->object == ps;}) != std::end(b->partial_states)) {
                continue;
            }
            auto ps_wrapper = CreateObject<PartialStateWrapper>(ps);
            new_b->partial_states.emplace_back(ps_wrapper);
        }

        for (auto ps: windowOperator2->getPartialStates()) {
            if (std::find_if(b->partial_states.begin(), b->partial_states.end(), [ps](Ptr<PartialStateWrapper> ps_wrapper) {return ps_wrapper->object == ps;}) != std::end(b->partial_states)) {
                continue;
            }
            auto ps_wrapper = CreateObject<PartialStateWrapper>(ps);
            new_b->partial_states.emplace_back(ps_wrapper);
        }

        DisableState(new_b);
        mp->critQueue.emplace_back(new_b);
    }

    Ptr<PartialStateBatch> JoinOperator::ExtractPreambleState(Ptr<MigrationPlan> mp) {
        // First, we're setting the imported flags to false
        for (auto wo: {windowOperator1, windowOperator2}) {
            auto batch = CreateObject<PartialStateBatch>(this, mp);
            for (auto ps: wo->getPartialStates()) {
                ps->imported_critical_attributes = false;
                ps->imported_essential_attributes = false;
                ps->imported_fully = false;
            }
        }

        Ptr<PartialStateBatch> static_batch;
        if (mp->migration_mode == "rhino") {
            static_batch = CreateObject<PartialStateBatch> (this, mp);
            // We add a wrapper to represent the static state of the operator itself
            auto op_wrapper = CreateObject<PartialStateWrapper> (this);
            static_batch->partial_states.emplace_back (op_wrapper);
            mp->preambleQueue.emplace_back (static_batch);
            for (auto t : this->windowOperator1->getPartialStates()) {
                auto ps_wrapper = CreateObject<PartialStateWrapper>(t);
                static_batch->partial_states.emplace_back(ps_wrapper);
            }
            for (auto t : this->windowOperator2->getPartialStates()) {
                auto ps_wrapper = CreateObject<PartialStateWrapper>(t);
                static_batch->partial_states.emplace_back(ps_wrapper);
            }
            this->first_rhino_batch = static_batch;
        } else {
            static_batch = Operator::ExtractPreambleState(mp);
        }

        return static_batch;
    }

    void JoinOperator::DisableState(Ptr<PartialStateBatch> batch) {
        windowOperator1->DisableState(batch);
        windowOperator2->DisableState(batch);
        for (auto ps : batch->partial_states) {
            if (ps->disable_state) {
                ps->object->imported_critical_attributes = false;
                ps->object->imported_essential_attributes = false;
                ps->object->imported_fully = false;
            }
        }
    }

    void JoinOperator::ClearState() {
    }

    void JoinOperator::ExtractState(Ptr<MigrationPlan> mp) {
        // If the migration mode includes no state, we just return
        if (mp->migration_mode == "drop-state") {
            return;
        }
        if (mp->migration_mode == "lazy") {
            ExtractLazyState(mp);
        } else if (mp->migration_mode == "utility") {
            ExtractUtilityState(mp);
        } else {
            if (mp->migration_mode == "rhino") {
                /*auto batch = CreateObject<PartialStateBatch>(this, mp);
                for (auto ps: windowOperator1->getPartialStates()) {
                    auto ps_wrapper = CreateObject<PartialStateWrapper>(ps);
                    ps_wrapper->disable_state = false;
                    ps_wrapper->size = ps->GetSerializedSize();
                    batch->partial_states.emplace_back(ps_wrapper);
                }

                for (auto ps: windowOperator2->getPartialStates()) {
                    auto ps_wrapper = CreateObject<PartialStateWrapper>(ps);
                    ps_wrapper->size = ps->GetSerializedSize();
                    ps_wrapper->disable_state = false;
                    batch->partial_states.emplace_back(ps_wrapper);
                }
                if (!batch->partial_states.empty()) {
                    mp->critQueue.emplace_back(batch);
                }*/
            } else if (mp->migration_mode == "all-at-once" || mp->migration_mode == "megaphone" || mp->migration_mode == "meces") {
                auto batch = CreateObject<PartialStateBatch>(this, mp);
                for (auto ps: windowOperator1->getPartialStates()) {
                    auto ps_wrapper = CreateObject<PartialStateWrapper>(ps);
                    auto t = DynamicCast<TupleWrapper>(ps_wrapper->object);
                    ps_wrapper->size = t->GetSerializedSize(true);
                    batch->partial_states.emplace_back(ps_wrapper);
                }

                for (auto ps: windowOperator2->getPartialStates()) {
                    auto ps_wrapper = CreateObject<PartialStateWrapper>(ps);
                    auto t = DynamicCast<TupleWrapper>(ps_wrapper->object);
                    ps_wrapper->size = t->GetSerializedSize(true);
                    batch->partial_states.emplace_back(ps_wrapper);
                }
                mp->critQueue.emplace_back(batch);
            }/* else if (mp->migration_mode == "megaphone" || mp->migration_mode == "meces") {
                // These two migration modes are identical here, because Meces differs in that it takes fetch requests

                auto batch = CreateObject<PartialStateBatch>(this, mp);
                for (auto ps: windowOperator1->getPartialStates()) {
                    auto ps_wrapper = CreateObject<PartialStateWrapper>(ps);
                    ps_wrapper->size = ps->GetSerializedSize();
                    batch->partial_states.emplace_back(ps_wrapper);
                }
                if (!batch->partial_states.empty()) {
                    mp->critQueue.emplace_back(batch);
                }

                batch = CreateObject<PartialStateBatch>(this, mp);
                for (auto ps: windowOperator2->getPartialStates()) {
                    auto ps_wrapper = CreateObject<PartialStateWrapper>(ps);
                    auto t = DynamicCast<TupleWrapper>(ps_wrapper->object);
                    ps_wrapper->size = ps->GetSerializedSize();
                    batch->partial_states.emplace_back(ps_wrapper);
                }
                if (!batch->partial_states.empty()) {
                    mp->critQueue.emplace_back(batch);
                }
            }*/ else if (mp->migration_mode == "state-shedding") {
                auto batch_fetch = CreateObject<PartialStateBatch>(this, mp);
                for (auto ps: windowOperator1->getPartialStates()) {
                    auto ps_wrapper = CreateObject<PartialStateWrapper>(ps);
                    auto t = DynamicCast<TupleWrapper>(ps_wrapper->object);
                    ps_wrapper->size = ps->GetSerializedSize();
                    batch_fetch->partial_states.emplace_back(ps_wrapper);
                }

                for (auto ps: windowOperator2->getPartialStates()) {
                    auto ps_wrapper = CreateObject<PartialStateWrapper>(ps);
                    auto t = DynamicCast<TupleWrapper>(ps_wrapper->object);
                    ps_wrapper->size = ps->GetSerializedSize();
                    batch_fetch->partial_states.emplace_back(ps_wrapper);
                }
                mp->critQueue.emplace_back(batch_fetch);
            } else {
                NS_FATAL_ERROR("Invalid migration mode " << mp->migration_mode);
            }
        }
    }

    std::vector<Ptr<PartialState>> JoinOperator::getPartialStates() {
        std::vector<Ptr<PartialState>> partial_states1 = windowOperator1->getPartialStates();
        std::vector<Ptr<PartialState>> partial_states2 = windowOperator2->getPartialStates();
        std::vector<Ptr<PartialState>> partial_states;
        partial_states.insert(partial_states.begin(), partial_states1.begin(), partial_states1.end());
        partial_states.insert(partial_states.begin(), partial_states2.begin(), partial_states2.end());

        return partial_states;
    }

    // When the current migration phase is more than 1, it means that the same partial state
    // has already been loaded into memory. The subsequent migration phases are not to load
    // the state, but to signal to the operator that the next phase can be initiated
    void JoinOperator::ImportState(Ptr<PartialStateBatch> batch) {
        if (batch->partial_states.empty()) {
            return;
        }
        int offset = 0;
        // One idea here is to add the tuples to the window and then call cache() on the argument
        auto op = DynamicCast<JoinOperator>(batch->partial_states[0]->object);
        if (op != nullptr) {
            // This is the static state batch
            // Import filter operator state. We do this before any processing
            for (int i = 0; i < (int) filterOperators.size(); i++) {
                auto new_fo = filterOperators[i];
                auto old_fo = op->filterOperators[i];
                auto new_arg1 = DynamicCast<AttributeFilterArgument>(new_fo->arg1);
                auto new_arg2 = DynamicCast<AttributeFilterArgument>(new_fo->arg2);
                auto old_arg1 = DynamicCast<AttributeFilterArgument>(old_fo->arg1);
                auto old_arg2 = DynamicCast<AttributeFilterArgument>(old_fo->arg2);

                new_arg1->longToTuples = old_arg1->longToTuples;
                new_arg1->bstLongTuples = old_arg1->bstLongTuples;
                new_arg1->bstDoubleTuples = old_arg1->bstDoubleTuples;
                new_arg1->doubleToTuples = old_arg1->doubleToTuples;
                new_arg1->stringToTuples = old_arg1->stringToTuples;

                new_arg2->longToTuples = old_arg2->longToTuples;
                new_arg2->bstLongTuples = old_arg2->bstLongTuples;
                new_arg2->bstDoubleTuples = old_arg2->bstDoubleTuples;
                new_arg2->doubleToTuples = old_arg2->doubleToTuples;
                new_arg2->stringToTuples = old_arg2->stringToTuples;
            }

            size_t number_tuples = 0;
            for (int i = 0; i < (int) this->filterOperators.size(); i++) {
                for (auto &[k, v]: DynamicCast<AttributeFilterArgument>(
                        DynamicCast<JoinFilterOperator>(this->filterOperators[i])->arg1)->longToTuples) {
                    for (auto t: v) {
                        number_tuples++;
                        //NS_ASSERT(t->imported_critical_attributes);
                    }
                }
            }

            std::cout << "Cache number tuples " << number_tuples << std::endl;
            /*for (auto &t : op->windowOperator1->tuples) {
                this->windowOperator1->tuples.emplace_front(t);
            }
            for (auto &t : op->windowOperator2->tuples) {
                this->windowOperator2->tuples.emplace_front(t);
            }*/
            if (batch->partial_states.size() == 1) {
                return;
            }
            offset = 1;
        }

        //static int sum = 0;
        //sum += batch->partial_states.size();
        //NS_LOG_INFO("Total number of imported partial states: ");
        //S_LOG_INFO("Start of ImportState, pending_matches size: " << pending_matches.size());
        //std::cout << Simulator::Now().GetMilliSeconds() << " Start of ImportState, number tuples to import: " << batch->partial_states.size() << std::endl;
        for (int i = offset; i < (int) batch->partial_states.size(); i++) {
            /*if (DynamicCast<RhinoMigrationMechanism>(batch->mp->migration_mechanism)) {
                break;
            }*/
            auto ps = batch->partial_states[i];
            auto tupleWrapper = DynamicCast<TupleWrapper>(ps->object);
            NS_ASSERT(tupleWrapper != nullptr);
            ++tupleWrapper->current_migration_phase;
            /*if (tupleWrapper->current_migration_phase == 1) {
                // First migration phase, we place tuples in the windows
                bool first_window = true;
                for (int j = 0; j < (int) this->windowOperator1->stream->input_stream_ids.size(); j++) {
                    if (tupleWrapper->tuples[j]->stream_id != this->windowOperator1->stream->input_stream_ids[j]) {
                        first_window = false;
                    }
                }
                if (first_window) {
                    this->windowOperator1->insertTuple(tupleWrapper);
                } else {
                    this->windowOperator2->insertTuple(tupleWrapper);
                }
            }*/
            // Lazy and utility have different ways of dealing with "phases"
            // Lazy does a kind of fake import for Phase 1 where it just enables matching to be done, but we do that when we migrate the operator
            // Now, why don't we have to do anything here to enable that? Well, because the operator won't be running until the first phase is done
            // The second phase, however, might require to output some pending matches
            // For utility, however, the phases are different
            // Phase 1 imports the essential attributes, and Phase 2 imports the rest of the attributes
            // The simples way of implementing this is just to "mark" the tuples with the phases they've been through: critical attributes would be the join predicates,
            // the essential attributes would be the utility phase 1, and all attributes would be every algorithm's final phase
            // Maybe replace this implementation with marking the tuples? That sounds like a good way of keeping things simple, but also add interesting information
            // for debugging purposes
            if (tupleWrapper->current_migration_phase < batch->mp->operatorIdToNumberPhases[this->id] && batch->mp->migration_mode == "lazy") {
                // We have already imported the stuff
                tupleWrapper->imported_critical_attributes = true;
            } else if (tupleWrapper->current_migration_phase < batch->mp->operatorIdToNumberPhases[this->id] && batch->mp->migration_mode == "rhino") {
                NS_LOG_INFO("Rhino receiving tuple " << tupleWrapper->tuples[0]->id << " in JoinOperator");
            } else if (tupleWrapper->current_migration_phase < batch->mp->operatorIdToNumberPhases[this->id] && batch->mp->migration_mode == "utility") {
                NS_LOG_INFO("Utility receiving tuple " << tupleWrapper->tuples[0]->id << " in JoinOperator, meaning, essential state imported, ready to join");
                tupleWrapper->imported_critical_attributes = true;
                tupleWrapper->imported_essential_attributes = true;
            } else {
                ++number_imported;
                //NS_LOG_INFO("JoinOperator::ImportState has fully imported " << number_imported << " tuples");
                // All migration mechanisms go through this part, as first or final phase
                // Next migration phase in JoinOperator is to migrate the rest of tuples
                // That means to iterate through all partial states and check if there are pending matches to be emitted
                tupleWrapper->in_migration = false;
                tupleWrapper->imported_critical_attributes = true;
                tupleWrapper->imported_essential_attributes = true;
                tupleWrapper->imported_fully = true;

                auto match = std::find_if(pending_matches.begin(), pending_matches.end(), [ps](const auto &p){return p.second == ps->object;});
                while (match != pending_matches.end()) {
                    // The already joined tuple is the first tuple in the pair, ready to be emitted
                    auto pending_tuple = DynamicCast<TupleWrapper>((*match).second);
                    auto joined_tuple = DynamicCast<TupleWrapper>((*match).first);
                    auto output_stream_ids = this->stream->output_stream_ids;
                    std::vector<Ptr<Tuple>> output_tuples(output_stream_ids.size());
                    for (auto tuple: joined_tuple->tuples) {
                        int stream_id = tuple->stream_id;
                        ++tuple->join_depth;
                        output_tuples[stream->output_stream_id_to_tuple_position[stream_id]] = tuple;
                    }
                    for (auto tuple: pending_tuple->tuples) {
                        int stream_id = tuple->stream_id;
                        ++tuple->join_depth;
                        output_tuples[stream->output_stream_id_to_tuple_position[stream_id]] = tuple;
                    }

                    auto joinedTupleWrapper = CreateObject<TupleWrapper>(output_tuples);
                    joinedTupleWrapper->node = joined_tuple->node;
                    // || true is used for testing fetch requests
                    static int nr_emitted_pending = 0;
                    ++number_emitted;
                    if (number_emitted % 1000 == 0) {
                        NS_LOG_INFO("Pending emitted: " << ++nr_emitted_pending);
                    }
                    Metrics::latency_data.emplace_back(TraceOutputLatencyEvent(joinedTupleWrapper, joinedTupleWrapper->tuples[0]->id));
                    //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " Join emitting tuple " << this->number_emitted << ", Source tuple " << tupleWrapper->tuples[0]->id << ", Joined tuple: " << joinedTupleWrapper->tuples[1]->id);
                    emit(joinedTupleWrapper);

                    pending_matches.erase(match);

                    match = std::find_if(pending_matches.begin(), pending_matches.end(), [ps](const auto &p){return p.second == ps->object;});
                }
            }
        }
        emit_time_delta = Time(0);
        time_for_next_forward = Time(0);
        /*for (auto t : this->windowOperator1->getPartialStates()) {
            NS_ASSERT(t->imported_critical_attributes);
        }
        for (auto t : this->windowOperator2->getPartialStates()) {
            NS_ASSERT(t->imported_critical_attributes);
        }
        size_t number_tuples = 0;
        for (int i = 0; i < (int) this->filterOperators.size(); i++) {
            for (auto &[k, v]: DynamicCast<AttributeFilterArgument>(
                    DynamicCast<JoinFilterOperator>(this->filterOperators[i])->arg1)->longToTuples) {
                for (auto t: v) {
                    number_tuples++;
                    NS_ASSERT(t->imported_critical_attributes);
                }
            }
        }
        std::cout << "Window 1 has " << this->windowOperator1->getPartialStates().size() << " number tuples" << std::endl;
        std::cout << "Window 2 has " << this->windowOperator2->getPartialStates().size() << " number tuples" << std::endl;
        std::cout << "Cache number tuples " << number_tuples << std::endl;*/
        NS_LOG_INFO(Simulator::Now().GetMicroSeconds() << " End of ImportState, pending_matches size: " << pending_matches.size());
    }

    void JoinOperator::ForwardTuplesTo(int host) {
        if (this->thread == nullptr) {
            return;
        }
        if (!this->thread->operatorIdToTupleQueue[this->id].empty()) {
            std::cout << "Forwarding " << this->thread->operatorIdToTupleQueue[this->id].size()
                      << " tuples from old host to new host" << std::endl;
        }
        for (auto p : this->thread->operatorIdToTupleQueue[this->id]) {
            std::cout << "Forwarding tuple " << std::get<2>(p)->tuples[0]->id << " from old host to new host with group " << std::get<2>(p)->group->id << std::endl;
            getExposeWrapper()->SendTupleTo(std::get<2>(p), host);
        }
        //windowOperator1->ForwardTuplesTo(host);
        //windowOperator2->ForwardTuplesTo(host);
        this->thread->operatorIdToTupleQueue[this->id].clear();
    }
}
