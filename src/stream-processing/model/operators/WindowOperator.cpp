//
// Created by espen on 27.10.2021.
//

#include "ns3/Operator.h"
#include <deque>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <fstream>


namespace ns3 {
    NS_OBJECT_ENSURE_REGISTERED(WindowOperator);
    NS_LOG_COMPONENT_DEFINE ("WindowOperator");

    static std::ofstream myfile;

    TypeId
    WindowOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::WindowOperator")
                .SetParent<Operator> ()
        ;

        return tid;
    }

    TypeId
    AggregationWindowOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::AggregationWindowOperator")
                .SetParent<WindowOperator> ()
        ;

        return tid;
    }

    TypeId
    CepWindowOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::CepWindowOperator")
                .SetParent<AggregationWindowOperator> ()
        ;

        return tid;
    }

    TypeId
    StorageWindowOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::StorageWindowOperator")
                .SetParent<WindowOperator> ()
        ;

        return tid;
    }

    TypeId
    WindowExtentContainer::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::WindowExtentContainer")
                .SetParent<Object>()
        ;

        return tid;
    }

    int window_tuple_cnt = 0;
    void StorageWindowOperator::process(Ptr<TupleWrapper> tupleWrapper) {
        ++total_received;
        //static int cnt = 0;
        //NS_LOG_INFO("Node " << this->thread->resourceManager->node_id << " Window " << this->id << " received stream " << tupleWrapper->tuples[0]->stream_id << ", tuple " << ++window_tuple_cnt << ", total cnt: " << ++cnt);
        insertTuple(tupleWrapper);
        updateWindow();
        emit(tupleWrapper);
    }

    Ptr<Operator> AggregationWindowOperator::copy(Ptr<Stream> s) {
        auto new_op = CreateObject<AggregationWindowOperator>();
        new_op->groupOperator = this->groupOperator;
        for (auto asf: aggregation_arguments) {
            new_op->aggregation_arguments.emplace_back(DynamicCast<AggregateSelectField>(asf->copy()));
        }
        new_op->external_timestamp_tuple_index = this->external_timestamp_tuple_index;
        new_op->init(size, jump, size_type, emit_size, emit_type, external_timestamp_key);
        Operator::copy(new_op, s);
        return new_op;
    }

    void AggregationWindowOperator::emit_point(bool recurring) {
        // If it's a non-recurring invocation, we can do this call
        if (!this->active && recurring) {
            return;
        }
        if (recurring && (Simulator::Now().GetMilliSeconds() - first_extent_time.GetMilliSeconds()) % emit_size != 0) {
            // Assume mistake was made in scheduling
            Simulator::Schedule(MilliSeconds(emit_size - (Simulator::Now().GetMilliSeconds() - first_extent_time.GetMilliSeconds()) % emit_size), &AggregationWindowOperator::emit_point, this, true);
            return;
        }
        last_emit_time = Simulator::Now();

        int number_emitted = 0;
        for (auto &[group_id, window_extents] : this->groupIdToWindowExtents) {
            /*for (int i = 0; i < (int) window_extents.size(); i++) {
                auto extent = DynamicCast<TimeWindowExtent>(window_extents[i]);
                if (extent->expireTime <= Simulator::Now()) {*/
           while (true) {
               auto before = number_emitted;
               number_emitted += emitExtents(group_id);
               if (number_emitted == before) {
                   break;
               }
           }
                //}
            //}
            //AddWindowExtent(window_extents[0]->group);
        }
        /*static int emitted = 0;
        emitted += number_emitted;
        NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " running emit_point with operator " << this);
        if (number_emitted > 0) {
            NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id
                                                           << " emit_point emitted " << number_emitted
                                                           << " tuples, total: " << emitted);
        }*/

        //std::cout << Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " emit_point produced " << emitted_extents << " tuples" << std::endl;

        if (!recurring) {
            return;
        }
        if (this->active && (this->emit_type == PROCESSING_TIME || this->emit_type == EXTERNAL_TIME) &&
            (std::any_of(groupIdToWindowExtents.begin(), groupIdToWindowExtents.end(), [](const auto &p){return !p.second.empty();}) || this->in_migration)) {
            //std::cout << Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " Scheduling next emit_point at " << (Simulator::Now()+ MilliSeconds(this->emit_size)).GetMilliSeconds() << " with operator " << this << std::endl;
            NS_ASSERT((Simulator::Now().GetMilliSeconds() - first_extent_time.GetMilliSeconds()) % emit_size == 0);
            Simulator::Schedule(MilliSeconds(this->emit_size), &AggregationWindowOperator::emit_point, this, true);
        } else {
            activated_time_emission = false;
        }
    }

    bool AggregationWindowOperator::firstExtentExpired(int group_id) {
        auto &extents = this->groupIdToWindowExtents[group_id];
        if (size_type == PROCESSING_TIME) {
            if (!extents.empty() && extents.front()->expireTime <= Simulator::Now()) {
                return true;
            }
        } else if (size_type == EXTERNAL_TIME) {
            if (!extents.empty() && extents.front()->expireTime <= Simulator::Now()) {
                return true;
            }
        } else if (size_type == TUPLE_COUNT) {
            if ((int) extents.size() > size) {
                return true;
            }
        } else {
            NS_FATAL_ERROR("Invalid size type");
        }
        return false;
    }

    Time AggregationWindowOperator::get_base_processing_time() {
        return Operator::get_base_processing_time();
        //return MilliSeconds(0);
    }

    Time AggregationWindowOperator::get_emit_time_added() {
        return Operator::get_emit_time_added()/10;
        //return MilliSeconds(0);
    }

    Time AggregationWindowOperator::get_forward_time_added() {
        return Operator::get_forward_time_added()/10;
        //return MilliSeconds(0);
    }

    bool AggregationWindowOperator::evictExtents(int group_id) {
        auto &extents = this->groupIdToWindowExtents[group_id];
        /*std::sort(extents.begin(), extents.end(), [](const Ptr<WindowExtent> a, const Ptr<WindowExtent> b) {
            return DynamicCast<TimeWindowExtent>(a)->upper_bound < DynamicCast<TimeWindowExtent>(b)->upper_bound;
        });*/
        if (size_type == PROCESSING_TIME) {
            if (!extents.empty() && extents.front()->expireTime <= Simulator::Now()) {
                //static int evicted = 0;
                //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " evicting extent " << ++evicted << ": " << extents.front() << " with expire time " << extents.front()->expireTime.GetMilliSeconds() << " at " << Simulator::Now().GetMilliSeconds() << " with " << extents.front()->aggregationWindowExtents[0].count << " tuples");
                extents.pop_front();
                return true;
            }
        } else if (size_type == EXTERNAL_TIME) {
            if (!extents.empty() && extents.front()->expireTime <= Simulator::Now()) {
                extents.pop_front();
                return true;
            }
        } else if (size_type == TUPLE_COUNT) {
            if ((int) extents.size() > size) {
                extents.pop_front();
                return true;
            }
        } else {
            NS_FATAL_ERROR("Invalid size type");
        }
        return false;
    }

    void AggregationWindowOperator::jumpExtents(Ptr<TupleWrapper> tupleWrapper) {
        if (shutting_down) {
            return;
        }
        // Espen: I don't think this will ever be relevant, because it's a bit of a weird emission
        if (emit_type == TUPLE_COUNT) {
            if (jump + last_jump < total_received) {
                // Jump
                AddWindowExtent(tupleWrapper->group, size);
                last_jump = total_received;
            }
        }
    }

    WindowExtent::WindowExtent(int created_by) {
        this->created_by = created_by;
    }

    WindowExtent::WindowExtent(Ptr<WindowExtent> extent, int created_by, bool only_skeleton) : WindowExtent(created_by) {
        this->groupToPartitions = extent->groupToPartitions;
        this->group = extent->group;
        this->expired = extent->expired;
        this->in_migration = extent->in_migration;
        this->needs_shedding = extent->needs_shedding;
        this->group_id = extent->group_id;
        this->current_migration_phase = extent->current_migration_phase;
        this->start = extent->start;
        this->expireTime = extent->expireTime;
        this->upper_bound = extent->upper_bound;
        this->lower_bound = extent->lower_bound;

        if (only_skeleton) {
            for (auto &aggregation_window_extent : extent->aggregationWindowExtents) {
                AggregationWindowExtent copy(aggregation_window_extent.aggregate_type, aggregation_window_extent.inputFieldIndex);
                copy.present = aggregation_window_extent.present;
                copy.prev_aggregate = aggregation_window_extent.prev_aggregate;
                this->aggregationWindowExtents.emplace_back(copy);
            }
        } else {
            for (auto awe : extent->aggregationWindowExtents) {
                this->aggregationWindowExtents.emplace_back(AggregationWindowExtent(awe));
            }
        }
    }

    void AggregationWindowOperator::MigrationInterrupted() {
        std::cout << Simulator::Now().GetMilliSeconds() << " Migration is now interrupted, but AggregationWindowOperator won't do anything special for now" << std::endl;
        SetActive();
    }

    int emitted = 0;
    int AggregationWindowOperator::doEmitExtent(Ptr<WindowExtent> extent) {
        int ret = 0;
        for (const auto &[group_id, partition] : extent->groupToPartitions) {
            //std::cout << "Partition " << partition << std::endl;
            auto output_tuple = CreateObject<GroupedTuple>();
            output_tuple->stream_id = this->stream->getOutputStreamId()[0];
            auto outputTupleWrapper = CreateObject<TupleWrapper>((std::vector<Ptr<Tuple>>){output_tuple});
            auto &group_asf_extents = extent->aggregationWindowExtents;
            for (int i = 0; i < (int)aggregation_arguments.size(); i++) {
                auto &agg_extent = group_asf_extents[i];
                output_tuple->aggregated_long_values.push_back(agg_extent.getAggregate());
            }
            // So this is where the magic happens with partition by.
            // Aside from this very line, all other details are identical for outputted tuples.
            // This line makes sure that the foundation for selecting ungrouped attributes
            // is done independently of how attributes are aggregated.
            outputTupleWrapper->group = extent->group;
            outputTupleWrapper->partition_group = partition;
            outputTupleWrapper->node = StreamQuery::nodeIdToNode[this->thread->resourceManager->node_id];
            //std::cout << "Time " << Simulator::Now().GetMilliSeconds() << ", Emitting tuple " << ++emitted << " from node " << this->thread->resourceManager->node_id << " and key " << partition->id << ", deadline: " << DynamicCast<TimeWindowExtent>(extent)->upper_bound.GetMilliSeconds() << std::endl;
            if (!myfile.is_open()) {
                myfile.open("/home/espen/ns-3/ns3-wip/output-text");
            }
            if (emit_type != TUPLE_COUNT && WindowExtent::keyToLowerboundToEmitted[group_id].count(extent->upper_bound.GetMilliSeconds())) {
                //NS_FATAL_ERROR("INVALID");
                auto &p = WindowExtent::keyToLowerboundToEmitted[group_id][extent->upper_bound.GetMilliSeconds()];
                auto t = p.first;
                auto n = p.second;
                std::cout << Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " Sending extent of group " << group_id << " and lower_bound " << extent->lower_bound.GetMilliSeconds() << " and upper_bound " << extent->upper_bound.GetMilliSeconds() << " more than once, last time by Node " << n << " at " << t.GetMilliSeconds() << " and expire time " << extent->upper_bound.GetMilliSeconds() << "!" << std::endl;
            } else {
                //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " Sending extent of group " << group_id << " and lower_bound " << DynamicCast<TimeWindowExtent>(extent)->lower_bound.GetMilliSeconds() << " and upper_bound " << DynamicCast<TimeWindowExtent>(extent)->upper_bound.GetMilliSeconds() << " for the first time!");
                //std::cout << Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " Sending extent of group " << group_id << " and lower_bound " << extent->lower_bound.GetMilliSeconds() << " and upper_bound " << extent->upper_bound.GetMilliSeconds() << " for the first time!" << std::endl;
                myfile << Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " Sending extent of group " << group_id << " and lower_bound " << extent->lower_bound.GetMilliSeconds() << " for the first time!\n";
            }
            WindowExtent::keyToLowerboundToEmitted[group_id][extent->upper_bound.GetMilliSeconds()] = std::make_pair(Simulator::Now(), this->thread->resourceManager->node_id);
            extent->emitted = true;
            //static int number_emitted_extents = 0;
            /*static bool migration_started = false;
            if (migration_started && node_id == 3) {
                NS_FATAL_ERROR("The new host has started producing extents, yet the old host is still emitting extents?");
            }
            if (node_id == 4 && !migration_started) {
                migration_started = true;
            }*/
            Metrics::extents_outputted++;
            Metrics::tuples_in_extents_emitted += extent->aggregationWindowExtents[0].count;
            Metrics::deviation_data.emplace_back(TraceOutputDeviationEvent(outputTupleWrapper, extent->aggregationWindowExtents[0].count, extent->aggregationWindowExtents[0].aggregate));
            //static int emitted = 0;
            static int number_tuples = 0;
            number_tuples += extent->aggregationWindowExtents[0].count;
            //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Extent emitted with group " << group_id << " deadline " << extent->upper_bound.GetMilliSeconds() << " and count " << extent->aggregationWindowExtents[0].count << ", emit number: " << ++emitted << ", tuple number: " << number_tuples);
            if (Simulator::Now() > extent->upper_bound) {
                Metrics::window_emission_delay += (Simulator::Now() - extent->upper_bound).GetMilliSeconds();
                Metrics::window_emission_delays.emplace_back(TraceOutputLatencyEvent(outputTupleWrapper, outputTupleWrapper->tuples[0]->id));
                NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Window extent emitted after deadline " << extent->upper_bound.GetMilliSeconds() << " by " << (Simulator::Now()-extent->upper_bound).GetMilliSeconds() << " ms");
                //NS_ASSERT((Simulator::Now() - extent->upper_bound).GetMilliSeconds() <= (extent->migration_stop-extent->migration_start).GetMilliSeconds());
            }
            emit(outputTupleWrapper);
            ++ret;
        }

        return ret;
    }

    //static int window_extent_cnt = 0;

    int AggregationWindowOperator::emitExtents(int group_id) {
        auto &extents = groupIdToWindowExtents[group_id];

        int emitted_extents = 0;
        if (extents.empty()) {
            return emitted_extents;
        }
        auto &extent = extents.front();
        if (emit_type != TUPLE_COUNT && !firstExtentExpired(group_id)) {
            return emitted_extents;
        }
        //extents.pop_front();
        if (extent->in_migration) {
            // Handle migration

            auto wrapper = ExposeCoordinator::nodeIdsToWrappers[std::to_string(this->thread->resourceManager->node_id)];
            auto migration_mode = wrapper->mp->migration_mode;
            // Currently, there's no fetching going on for the queries in lazy migration simulations, because
            if (emit_type == TUPLE_COUNT) {
                // The diff between Meces and Lazy is that Meces fetches the entire group, but Lazy fetches only the necessary values
                if (migration_mode == "meces") {
                    std::deque<Ptr<PartialStateWrapper>> partialStateWrappers;
                    for (const auto &e : extents) {
                        auto ps_wrapper = CreateObject<PartialStateWrapper>(e, e->GetSerializedSize(), this);
                        partialStateWrappers.emplace_back(ps_wrapper);
                    }
                    getExposeWrapper()->ps_fetch_queue.emplace_back(std::make_pair(this->old_node_id, partialStateWrappers));
                    Simulator::Schedule(MilliSeconds(10), &ExposeWrapper::FetchNextPartialState, getExposeWrapper());
                    //getExposeWrapper()->FetchNextPartialState();
                } else if (migration_mode == "lazy") {
                    for (const auto &e : extents) {
                        auto ps_wrapper = CreateObject<PartialStateWrapper>(e, e->GetSerializedSize(), this);
                        getExposeWrapper()->ps_fetch_queue.emplace_back(std::make_pair(this->old_node_id, (std::deque<Ptr<PartialStateWrapper>>){ps_wrapper}));
                    }
                    Simulator::Schedule(MilliSeconds(10), &ExposeWrapper::FetchNextPartialState, getExposeWrapper());
                    //getExposeWrapper()->FetchNextPartialState();
                } else {
                    NS_FATAL_ERROR("Invalid migration mode trying to fetch states: " << migration_mode);
                }
            }

            // TODO: Create copy of extent and place it in a pending queue
            auto pending_extent = CreateObject<WindowExtent>(extent, this->thread->resourceManager->node_id, false);
            //++window_extent_cnt;
            pending_extents[group_id].emplace_back(pending_extent);
            //static int cnt = 0;
            //static int pending_tuples = 0;
            //pending_tuples += extent->aggregationWindowExtents[0].count;
            //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Pending extents created: " << ++cnt << ", group: " << pending_extent->group->id << " expireTime: " << pending_extent->expireTime.GetMilliSeconds() << ", pending tuples: " << pending_tuples);
            evictExtents(group_id);
            return 1;
        }

        emitted_extents = doEmitExtent(extent);
        NS_ASSERT(!evictExtents(group_id) || emitted_extents > 0);
        return emitted_extents;
    }

    bool check = false;
    void AggregationWindowOperator::AddWindowExtent(Ptr<Group> group, int ms_ahead) {
        if (this->size_type == PROCESSING_TIME) {
            if (first_extent_time.IsZero()) {
                //std::cout << "first_extent_time: " << first_extent_time << std::endl;
                //first_extent_time = Simulator::Now() - MilliSeconds(emit_size+1);
                first_extent_time = Simulator::Now() - MilliSeconds(emit_size+1);
            }
            auto t_received_corrected = Simulator::Now().GetMilliSeconds();
            while ((t_received_corrected - first_extent_time.GetMilliSeconds()) % emit_size != 0) {
                --t_received_corrected;
            }
            // Create an offset
            ++t_received_corrected;
            //++window_extent_cnt;
            Ptr<WindowExtent> new_window_extent = CreateObject<WindowExtent>(this->thread->resourceManager->node_id);
            auto c = std::floor((ms_ahead + t_received_corrected - first_extent_time.GetMilliSeconds()/* + std::abs(Simulator::Now().GetMilliSeconds() % emit_size - first_extent_time.GetMilliSeconds() % emit_size)*/) / emit_size);
            //NS_ASSERT(c > 0);
            new_window_extent->expireTime = MilliSeconds(first_extent_time.GetMilliSeconds() + c * emit_size); //Simulator::Now() + MilliSeconds(deadline);
            ++cnt_extents;
            new_window_extent->upper_bound = new_window_extent->expireTime;
            new_window_extent->lower_bound = new_window_extent->expireTime - MilliSeconds(size);
            //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " creating window extent " << cnt_extents << " with deadline " << DynamicCast<TimeWindowExtent>(new_window_extent)->expireTime.GetMilliSeconds() << ", c: " << c << ", first_extent_time: " << first_extent_time.GetMilliSeconds() << " and key " << group->id << ", ms_ahead: " << ms_ahead);
            NS_ASSERT(!new_window_extent->upper_bound.IsZero() && !new_window_extent->lower_bound.IsZero());
            if (!myfile.is_open()) {
                myfile.open("/home/espen/ns-3/ns3-wip/output-text");
            }
            //myfile << Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " creating window extent " << cnt_extents << " with deadline " << DynamicCast<TimeWindowExtent>(new_window_extent)->expireTime.GetMilliSeconds() << ", c: " << c << ", first_extent_time: " << first_extent_time.GetMilliSeconds() << " and key " << group->id << std::endl;
            auto &extents = this->groupIdToWindowExtents[group->id];
            extents.emplace_back(new_window_extent);

            for (auto &extent_in_migration : groupIdToExtentsInMigration[group->id]) {
                if (extent_in_migration->upper_bound == new_window_extent->upper_bound &&
                    extent_in_migration->lower_bound == new_window_extent->lower_bound) {
                    new_window_extent->in_migration = true;
                }
            }
            new_window_extent->group = group;
            if (WindowExtent::keyToLowerboundToCreated[new_window_extent->group->id].count(new_window_extent->upper_bound.GetMilliSeconds()) && WindowExtent::keyToLowerboundToCreated[new_window_extent->group->id][new_window_extent->upper_bound.GetMilliSeconds()].second == this->thread->resourceManager->node_id) {
                //NS_FATAL_ERROR("INVALID");
                auto &p = WindowExtent::keyToLowerboundToCreated[new_window_extent->group->id][new_window_extent->upper_bound.GetMilliSeconds()];
                auto t = p.first;
                auto n = p.second;
                std::cout << Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " Creating extent of group " << new_window_extent->group->id << " and lower_bound " << new_window_extent->lower_bound.GetMilliSeconds() << " and upper_bound " << new_window_extent->upper_bound.GetMilliSeconds() << " more than once, last time by Node " << n << " at " << t.GetMilliSeconds() << " and expire time " << new_window_extent->upper_bound.GetMilliSeconds() << "!" << std::endl;
            } else {
                //static int number_extents = 0;
                //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " Sending extent of group " << new_window_extent->group->id << " and lower_bound " << new_window_extent->lower_bound.GetMilliSeconds() << " and upper_bound " << new_window_extent->upper_bound.GetMilliSeconds() << " for the first time, extent #" << ++number_extents << "!");
                //myfile << Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " Creating extent of group " << group->id << " and lower_bound " << new_window_extent->lower_bound.GetMilliSeconds() << " for the first time!\n";
            }
            WindowExtent::keyToLowerboundToCreated[new_window_extent->group->id][new_window_extent->upper_bound.GetMilliSeconds()] = std::make_pair(Simulator::Now(), this->thread->resourceManager->node_id);;
        } else if (this->size_type == EXTERNAL_TIME) {
            if (first_extent_time.IsZero()) {
                //...;
            }
            Ptr<WindowExtent> new_window_extent = CreateObject<WindowExtent>(this->thread->resourceManager->node_id);
            new_window_extent->expireTime = Simulator::Now() + MilliSeconds(ms_ahead);
            auto &extents = this->groupIdToWindowExtents[group->id];
            if (!extents.empty()) {
                NS_ASSERT(extents[extents.size() - 1]->upper_bound ==
                          new_window_extent->upper_bound - MilliSeconds(emit_size));
            }
            extents.emplace_back(new_window_extent);
            new_window_extent->group = group;
        } else if (this->size_type == TUPLE_COUNT) {
            Ptr<WindowExtent> new_window_extent = CreateObject<WindowExtent>(this->thread->resourceManager->node_id);
            this->groupIdToWindowExtents[group->id].emplace_back(new_window_extent);
            new_window_extent->group = group;
        } else {
            NS_FATAL_ERROR("Invalid size type!");
        }
    }

    int total_cnt = 0;
    //static inline int number_processed = 0;
    static inline std::map<int, bool> tuple_ids;
    void AggregationWindowOperator::process(Ptr<TupleWrapper> tupleWrapper) {
        Metrics::aggregation_latency_data.emplace_back(TraceOutputLatencyEvent(tupleWrapper, tupleWrapper->tuples[0]->id));
        ++global_total_received;
        ++total_received;
        NS_ASSERT(groupIdToWindowExtents[tupleWrapper->group->id].empty() || groupIdToWindowExtents[tupleWrapper->group->id].front()->expireTime > Simulator::Now());
        tuple_ids[tupleWrapper->tuples[0]->id] = true;

        if (global_total_received % 100 == 0) {
          int total_window_extents = 0;
          int total_tuple_count = 0;
          for (auto &[g, extents] : this->groupIdToWindowExtents) {
              total_window_extents += extents.size();
              for (auto e : extents) {
                  total_tuple_count += e->aggregationWindowExtents[0].count;
              }
          }
        NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Node " << this->thread->resourceManager->node_id << " Processing global tuple "
                                                       << global_total_received << " (local on node " << this->node_id << " " << total_received << " tuples)" " with ID "
                                                       << tupleWrapper->tuples[0]->id
                                                       << " and group " << tupleWrapper->group->id << ", total window extents: " << total_window_extents << ", total tuple count: " << total_tuple_count);
        }
        if ((int) this->groupIdToWindowExtents[tupleWrapper->group->id].size() < size / emit_size) {
            if (emit_type == PROCESSING_TIME || emit_type == EXTERNAL_TIME || emit_type == TUPLE_COUNT) {
                for (int i = this->groupIdToWindowExtents[tupleWrapper->group->id].size(); i < size / emit_size; i++) {
                    AddWindowExtent(tupleWrapper->group, (i+1) * emit_size);
                }
            }
        }
        insertTuple(tupleWrapper);
        if (emit_type == TUPLE_COUNT) {
            // We only emit tuples when receiving them if emission type is tuple count
            // Otherwise, we use a time-based approach
            emitExtents(tupleWrapper->group->id);
        } else if (emit_type == PROCESSING_TIME || emit_type == EXTERNAL_TIME) {
            if (!activated_time_emission) {
                activated_time_emission = true;
                auto next_emit = MilliSeconds(first_extent_time.GetMilliSeconds() % emit_size + Simulator::Now().GetMilliSeconds() - Simulator::Now().GetMilliSeconds() % emit_size);
                while (next_emit < Simulator::Now()) {
                    next_emit += MilliSeconds(emit_size);
                }
                Simulator::Schedule(next_emit-Simulator::Now(), &AggregationWindowOperator::emit_point, this, true);
            }
        } else {
            NS_FATAL_ERROR("Invalid emission type!");
        }
        jumpExtents(tupleWrapper);
    }

    bool StorageWindowOperator::insertTuple(Ptr<TupleWrapper> tupleWrapper) {
        tuples.push_front(tupleWrapper);
        return true;
    }

    void StorageWindowOperator::updateWindow() {
        bool doJump = false;
        // Remove outdated tuples
        if (size_type == TUPLE_COUNT) {
            if ((int)tuples.size() > size) {
                doJump = true;
            }
            // Here we jump and add a window extent
            if (doJump) {
                // Jump
                //int tuple_index = tuples.size();
                for (int i = 0; i < jump; i++) {
                    ++number_evicted;
                    tuples.back()->evicted = true;
                    tuples.pop_back();
                    //NS_LOG_INFO(this << ", " << tuples.size());
                }
            }
        } else if (size_type == PROCESSING_TIME) {

        } else if (size_type == EXTERNAL_TIME) {
            // We always do this after receiving a tuple, so tuples will never be empty
            long current_time = current_external_time;
            if (!tuples.empty()) {
                current_time = std::max(std::any_cast<long>(
                        tuples.front()->tuples[external_timestamp_tuple_index]->getLongValue(
                                this->external_timestamp_key_index)), this->current_external_time);
            }
            this->current_external_time = current_time;
            int index = tuples.size() - 1;
            long new_time = current_time;
            if (index >= 0) {
                if (tuples[index]->tuples[external_timestamp_tuple_index]->getLongValue(
                        this->external_timestamp_key_index) + size - 1 < current_time) {
                    doJump = true;
                    while (last_jump + size < current_time) {
                        last_jump += jump;
                        new_time = last_jump + size;
                    }
                }

                while (index >= 0 && tuples[index]->tuples[external_timestamp_tuple_index]->getLongValue(
                        this->external_timestamp_key_index) + size - 1 < new_time) {
                    // Some redundancy since we're also doing this even if the evicted flag is true
                    auto tw = tuples[index--];
                    Ptr<Tuple> t = tw->tuples[external_timestamp_tuple_index];
                    tw->evicted = true;
                    // Notify next joins that a tuple has been evicted
                    if (stream->next_stream->operators[0]->type == joinType) {
                        bool need_to_notify = false;
                        for (Ptr<Tuple> tuple: tw->tuples) {
                            if (this->joinOperator != nullptr && tuple->join_depth >= this->joinOperator->join_depth) {
                                need_to_notify = true;
                                break;
                            }
                        }
                        if (need_to_notify) {
                            auto join = DynamicCast<JoinOperator>(stream->next_stream->operators[0]);
                            join->notifyEviction();
                        }
                    }
                    ++number_evicted;
                    tuples.pop_back();
                }
            }
        } else if (size_type == INTERMEDIARY_JOIN) {
            if (this->evicted_tuples) {
                tuples.erase(std::remove_if(tuples.begin(), tuples.end(), [](Ptr<TupleWrapper> tw) { return tw->evicted; }),
                             tuples.end());
                this->evicted_tuples = false;
            }
        } else {
            // Error, invalid type is used
        }
    }

    void AggregationWindowOperator::init(int size, int jump, WindowType size_type, int emit_size, WindowType emit_type,
                                         const std::string &external_timestamp_key) {
        this->size = size;
        this->jump = jump;
        this->size_type = size_type;
        this->emit_size = emit_size;
        this->emit_type = emit_type;
        this->external_timestamp_key = external_timestamp_key;
        if (!external_timestamp_key.empty()) {
            int stream_id;
            std::string alias;
            NS_ASSERT(external_timestamp_key.find('.') != std::string::npos);
            std::istringstream iss(external_timestamp_key);
            std::getline(iss, alias, '.');
            stream_id = query->aliasToStreamId[alias];
            std::getline(iss, this->external_timestamp_key, '.');
            query->AddEssentialParameter(alias, this->external_timestamp_key);

            auto input_stream_ids = this->stream->input_stream_ids;
            for (int i = 0; i < (int)input_stream_ids.size(); i++) {
                if (stream_id == input_stream_ids[i]) {
                    this->external_timestamp_tuple_index = i;
                    break;
                }
            }

            this->external_timestamp_key_index = SelectField::getFieldIndex(alias, this->external_timestamp_key, query);
        }
    }

    void StorageWindowOperator::init(int size, int jump, WindowType size_type, WindowType emit_type,
                                     const std::string &external_timestamp_key) {
        this->size = size;
        this->size_type = size_type;
        this->emit_type = emit_type;
        this->jump = jump;
        this->external_timestamp_key = external_timestamp_key;
        if (!external_timestamp_key.empty()) {
            int stream_id;
            std::string alias;
            NS_ASSERT(external_timestamp_key.find('.') != std::string::npos);
            std::string stream_name;
            std::istringstream iss(external_timestamp_key);
            std::getline(iss, stream_name, '.');
            stream_id = Tuple::streamNameToId[stream_name];
            NS_ASSERT_MSG(stream_id != 0, Simulator::Now() << ": Stream " << stream_name << " is not defined");
            std::getline(iss, this->external_timestamp_key, '.');

            auto input_stream_ids = this->stream->getInputStreamId();
            for (int i = 0; i < (int)input_stream_ids.size(); i++) {
                if (stream_id == input_stream_ids[i]) {
                    this->external_timestamp_tuple_index = i;
                    break;
                }
            }

            this->external_timestamp_key_index = SelectField::getFieldIndex(alias, this->external_timestamp_key, this->query);
        }
    }

    std::vector<AggregationWindowExtent> AggregationWindowOperator::getAsfExtentCopies() {
        if (this->asf_extent_copies.size() == this->aggregation_arguments.size()) {
            return this->asf_extent_copies;
        }

        std::vector<AggregationWindowExtent> extent_copies;
        for (auto asf : this->aggregation_arguments) {
            switch (asf->aggregate) {
                case AVG:
                    extent_copies.emplace_back(AggregationWindowExtent(AVG, asf->inputFieldIndex));
                    break;
                case MIN:
                    extent_copies.emplace_back(AggregationWindowExtent(MIN, asf->inputFieldIndex));
                    break;
                case MAX:
                    extent_copies.emplace_back(AggregationWindowExtent(MAX, asf->inputFieldIndex));
                    break;
                case SUM:
                    extent_copies.emplace_back(AggregationWindowExtent(SUM, asf->inputFieldIndex));
                    break;
                case COUNT:
                    extent_copies.emplace_back(AggregationWindowExtent(COUNT, asf->inputFieldIndex));
                    break;
                case NONE:
                default:
                    NS_FATAL_ERROR("No aggregate defined for AggregateSelectField");
            }
        }
        this->asf_extent_copies = extent_copies;
        return this->asf_extent_copies;
    }

    bool AggregationWindowOperator::insertTuple(Ptr<TupleWrapper> tupleWrapper) {
        // We assume there is an underlying group, whether it's from GroupOperator or Group::no_group.
        int group_id = tupleWrapper->group->id;
        //static int number_inserted = 0;
        for (auto &extent : this->groupIdToWindowExtents[group_id]) {
            if (extent->aggregationWindowExtents.empty()) {
                extent->aggregationWindowExtents = getAsfExtentCopies();
                extent->group = tupleWrapper->group;
            }
            if (tupleWrapper->partition_group == Group::no_group) {
                extent->groupToPartitions[tupleWrapper->group->id] = tupleWrapper->group;
            } else if (!extent->groupToPartitions.count(tupleWrapper->partition_group->id)) {
                extent->groupToPartitions[tupleWrapper->partition_group->id] = tupleWrapper->partition_group;
            }
            //++number_inserted;
            for (int i = 0; i < (int)this->aggregation_arguments.size(); i++) {
                auto &agg_extent = extent->aggregationWindowExtents[i];
                if (agg_extent.present) {
                    agg_extent.insertTuple(tupleWrapper->tuples[this->aggregation_arguments[i]->tuple_index]);
                } else {
                    // Add tuple to pending
                    NS_FATAL_ERROR("Not implemented");
                }
            }
        }
        //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Inserted " << number_inserted << " tuples");

        return true;
    }

    void CepWindowOperator::insertTuple(Ptr<SequencePartialState> sequence, Ptr<TupleWrapper> tupleWrapper) {
        auto &extent = sequence->windowExtent;
        if (extent->aggregationWindowExtents.empty()) {
            extent->aggregationWindowExtents = getAsfExtentCopies();
        }
        for (int i = 0; i < (int)this->aggregation_arguments.size(); i++) {
            auto &agg_extent = extent->aggregationWindowExtents[i];
            agg_extent.insertTuple(tupleWrapper->tuples[this->aggregation_arguments[i]->tuple_index]);
        }
    }

    void CepWindowOperator::undoInsertTuple(Ptr<SequencePartialState> sequence, Ptr<TupleWrapper> tupleWrapper) {
        auto extent = sequence->windowExtent;
        for (int i = 0; i < (int)this->aggregation_arguments.size(); i++) {
            auto &agg_extent = extent->aggregationWindowExtents[i];
            agg_extent.undoInsertTuple(tupleWrapper->tuples[this->aggregation_arguments[i]->tuple_index]);
        }
    }

    double AggregationWindowExtent::getAggregate() {
        switch (aggregate_type) {
            case AVG: {
                return (double)aggregate/count;
            } case MIN:
              case MAX:
              case SUM: {
                return aggregate;
            } case COUNT: {
                return count;
            }
            case NONE:
                break;
        }
        return 0;
    }

    bool AggregationWindowExtent::insertTuple(Ptr<Tuple> tuple) {
        // We increment count for all tuples
        ++this->count;
        switch (aggregate_type) {
            case AVG: {
                this->aggregate += tuple->getLongValue(inputFieldIndex);
                break;
            } case MIN: {
                long value = tuple->getLongValue(inputFieldIndex);
                if (value < this->aggregate) {
                    this->aggregate = value;
                }
                break;
            } case MAX: {
                long value = tuple->getLongValue(inputFieldIndex);
                if (value > this->aggregate) {
                    this->aggregate = value;
                }
                break;
            } case SUM: {
                this->aggregate += tuple->getLongValue(inputFieldIndex);
                break;
            } case COUNT: {
                break;
            }
          case NONE:
            break;
        }

        return true;
    }

    void CepWindowOperator::init(int size, int jump, WindowType size_type, int emit_size, WindowType emit_type, const std::string &external_timestamp_key) {
        AggregationWindowOperator::init(size, jump, size_type, emit_size, emit_type, external_timestamp_key);
    }

    SequencePartialState::SequencePartialState(Ptr<CepPatternOperator> cepPatternOperator) {
        this->cepPatternOperator = cepPatternOperator;
        windowExtent = CreateObject<WindowExtent>(cepPatternOperator->thread->resourceManager->node_id);
    }

    bool SequencePartialState::addToSequence(const std::string& alias, Ptr<TupleWrapper> tupleWrapper) {
        this->cepPatternOperator->windowOperator->current_external_time = std::max(this->cepPatternOperator->windowOperator->current_external_time, tupleWrapper->tuples[cepPatternOperator->withinOperator->tuple_index]->getLongValue(cepPatternOperator->withinOperator->index));
        if (cepPatternOperator->withinOperator) {
            auto ts = this->cepPatternOperator->windowOperator->current_external_time;
            if (this->aliasToTuples.empty()) {
                this->first_ts = ts;
            }
            this->last_ts = ts;
            if (first_ts + cepPatternOperator->withinOperator->ms < last_ts) {
                return false;
            }
        }
        this->aliasToTuples[alias].push_back(tupleWrapper);
        return true;
    }

    void CepWindowOperator::addSequence(Ptr<SequencePartialState> sequence) {
        this->sequences.push_back(sequence);
    }

    void CepWindowOperator::addSequence(Ptr<SequencePartialState> sequence, int group_id) {
        this->groupIdToSequences[group_id].push_back(sequence);
        //this->sequences.push_back(sequence);
    }

    void AggregationWindowExtent::undoInsertTuple(Ptr<Tuple> tuple) {
        switch (aggregate_type) {
            case AVG: {
                this->aggregate -= tuple->getLongValue(inputFieldIndex);
                --this->count;
                break;
            } case MIN: {
                this->aggregate = this->prev_aggregate;
                break;
            } case MAX: {
                this->aggregate = this->prev_aggregate;
                break;
            } case SUM: {
                this->aggregate -= tuple->getLongValue(inputFieldIndex);
                break;
            } case COUNT: {
                --this->count;
                break;
            }
          case NONE:
            break;
          }
    }

    int AggregationWindowOperator::calculateMigrationSize(migration_types migration_type) {
        int payload_size = 0;
        switch(migration_type) {
            case STANDARD_MOVING_STATE: {
                for (auto &[group_id, extents] : groupIdToWindowExtents) {
                    for (auto &extent : extents) {
                        payload_size += extent->GetSerializedSize();
                    }
                }
                break;
            } case WINDOW_RECREATION: {
                break;
            } case STATE_RECREATION: {
                for (auto &[group_id, extents] : groupIdToWindowExtents) {
                    for (auto &extent : extents) {
                        payload_size += extent->GetSerializedSize();
                    }
                }
                break;
            }
          case STATELESS:
            break;
          }
        return payload_size;
    }

    int StorageWindowOperator::calculateMigrationSize(migration_types migration_type) {
        int payload_size = 0;
        switch(migration_type) {
            case STANDARD_MOVING_STATE: {
                for (auto tuple : this->tuples) {
                    payload_size += tuple->GetSerializedSize(true);
                }
                break;
            } case WINDOW_RECREATION: {
                payload_size += 1;
                break;
            } case STATE_RECREATION: {
                for (auto tuple : this->tuples) {
                    payload_size += tuple->GetSerializedSize(true);
                }
                break;
            }
          case STATELESS:
            break;
          }
        return payload_size;
    }

    int CepWindowOperator::calculateMigrationSize(migration_types migration_type) {
        return 1;
    }

    void AggregationWindowOperator::ExtractLazyState(Ptr<MigrationPlan> mp) {
        //int total_extents = 0;
        std::size_t max_number_extents = 0;
        for (auto &[group_id, extents] : this->groupIdToWindowExtents) {
            max_number_extents = std::max(max_number_extents, extents.size());
            //total_extents += extents.size();
        }
        //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Extracting " << total_extents << " extents!");

        auto batch = CreateObject<DeadlinePartialStateBatch>(this, mp);
        for (int i = 0; i < (int) max_number_extents; i++) {
            for (auto &[group_id, extents]: this->groupIdToWindowExtents) {
                if (i >= (int) extents.size()) {
                    continue;
                }
                batch->partial_states.emplace_back(CreateObject<PartialStateWrapper>(extents[i]));
                if (batch->deadline.IsZero() || extents[i]->upper_bound < batch->deadline) {
                    batch->deadline = extents[i]->upper_bound;
                }
            }
            //std::cout << "Extracting " << batch->partial_states.size() << " extents!" << std::endl;
            batch->must_be_sent_by = batch->deadline - ExposeCoordinator::bw_migration.CalculateBytesTxTime(batch->GetSerializedSize());
            if ((i+1) % 1 == 0) {
                mp->deadlineQueue.emplace_back(batch);
                batch = CreateObject<DeadlinePartialStateBatch>(this, mp);
            }
        }

        if (!batch->partial_states.empty()) {
            mp->deadlineQueue.emplace_back(batch);
        }
    }

    void StorageWindowOperator::DisableState(Ptr<PartialStateBatch> batch) {
        for (auto it = this->tuples.begin(); it != this->tuples.end(); it++) {
            auto t1 = *it;
            for (auto it2 = batch->partial_states.begin(); it2 != batch->partial_states.end(); it2++) {
                auto t2 = (*it2)->object;
                if (t1 == t2) {
                    //(*it)->evicted = true;
                    it = this->tuples.erase(it);
                    it--;
                    break;
                }
            }
        }
    }

    void StorageWindowOperator::ClearState() {
        for (auto t : this->tuples) {
            t->imported_critical_attributes = false;
            t->imported_fully = false;
            t->imported_essential_attributes = false;
            t->evicted = true;
        }
        this->tuples.clear();
    }

    void AggregationWindowOperator::DisableState(Ptr<PartialStateBatch> batch) {
        //std::cout << Simulator::Now().GetMilliSeconds() << " DisableState" << std::endl;
        std::set<int> group_ids_to_delete;
        for (auto it = batch->partial_states.begin(); it != batch->partial_states.end();) {
            auto &ps = *it;
            int group_id = -1;
            bool been_emitted = false;
            auto window_extent = DynamicCast<WindowExtent>(ps->object);
            if (window_extent != nullptr) {
                //std::cout << "Deleting state with group id " << DynamicCast<TimeWindowExtent>(ps->object)->group->id
                //          << std::endl;
                // Delete the window extent from groupIdToWindowExtents
                group_id = window_extent->group->id;
                for (auto it2 = groupIdToWindowExtents[group_id].begin(); it2 != groupIdToWindowExtents[group_id].end(); it2++) {
                    auto e = *it2;
                    if (e == window_extent) {
                        groupIdToWindowExtents[group_id].erase(it2);
                        break;
                    }
                }
                auto new_query = ExposeCoordinator::queryIdToNodeIdToQuery[this->query->unique_query_id][this->node_id];
                auto new_op = new_query->streamIdToOperatorIdToOperator[this->stream->input_stream_ids[0]][this->id]->stream->operators[this->partition];
                auto &extents_in_migration = DynamicCast<AggregationWindowOperator>(new_op)->groupIdToExtentsInMigration[group_id];

                been_emitted = window_extent->emitted;
                window_extent->migration_start = Simulator::Now();
                // Did doing this only in the else statement below cause the issue of missing tuples?? I'm not very fully convinced this is the solution
                // Is the problem that Megaphone needs a better way of telling which groups are to be redirected? Because currently, I actually don't understand how it's doing correct
                group_ids_to_delete.insert(group_id);
                if (been_emitted) {
                    //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Deleting extent with group ID " << group_id << " and lower_bound " << window_extent->lower_bound.GetMilliSeconds() << " and upper bound " << window_extent->upper_bound.GetMilliSeconds() << " from batch, because it's no longer in the extents of the operator");
                    it = batch->partial_states.erase(it);
                    // Checking to see if this extent is not part of the new host's extents in migration vector, and remove it if it is
                    for (auto it2 = extents_in_migration.begin(); it2 != extents_in_migration.end(); ++it2) {
                        auto e = *it2;
                        if (e == window_extent) {
                            extents_in_migration.erase(it2);
                            break;
                        }
                    }
                    continue;
                } else {
                    bool found = false;
                    // Checking to see if this extent is not part of the new host's extents in migration vector, and add it if it's not
                    for (auto it2 = extents_in_migration.begin(); it2 != extents_in_migration.end(); ++it2) {
                        auto e = *it2;
                        if (e == window_extent) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        extents_in_migration.emplace_back(window_extent);
                    }
                }
            }
            ++it;
        }

        // Megaphone and Meces must
        if (batch->mp->migration_mode == "megaphone" || batch->mp->migration_mode == "meces") {
            for (auto group_id_to_delete: group_ids_to_delete) {
                // For Megaphone, add all extents that haven't been added
                if (batch->mp->migration_mode == "megaphone") {
                    // Looking for extents that we haven't migrated
                    for (auto it = this->groupIdToWindowExtents[group_id_to_delete].begin(); it != this->groupIdToWindowExtents[group_id_to_delete].end();) {
                        auto extent = *it;
                        if (std::find_if(batch->partial_states.begin(), batch->partial_states.end(),
                                         [extent](Ptr<PartialStateWrapper> ps_wrapper) {
                                             return ps_wrapper->object == extent;
                                         }) == std::end(batch->partial_states)) {
                            auto ps_wrapper = CreateObject<PartialStateWrapper>(extent);
                            batch->partial_states.emplace_back(ps_wrapper);
                            it = this->groupIdToWindowExtents[group_id_to_delete].erase(it);
                            // Add this to the new operators groupIdToExtentsInMigration
                            auto new_query = ExposeCoordinator::queryIdToNodeIdToQuery[this->query->unique_query_id][this->node_id];
                            Ptr<Operator> old_op = batch->op;
                            auto new_op = new_query->streamIdToOperatorIdToOperator[old_op->stream->input_stream_ids[0]][old_op->id]->stream->operators[old_op->partition];
                            DynamicCast<AggregationWindowOperator>(
                                    new_op)->groupIdToExtentsInMigration[group_id_to_delete].emplace_back(extent);
                        } else {
                            ++it;
                        }
                    }
                }
                if (this->groupIdToWindowExtents.count(group_id_to_delete)) {
                    // Meces and Megaphone are supposed to migrate all extents
                    NS_ASSERT(this->groupIdToWindowExtents[group_id_to_delete].empty());
                    this->groupIdToWindowExtents.erase(this->groupIdToWindowExtents.find(group_id_to_delete));
                }
            }
        }
    }

    void AggregationWindowOperator::ClearState() {
        groupIdToWindowExtents.clear();
    }

    void AggregationWindowOperator::ExtractDiffState(Ptr<MigrationPlan> mp) {
        // Extract the diff of the state
        // How? Easy, do a diff between the critical partial state batch that is in mp now, and the
        // state that is currently in state. Ignore states to be deleted, since they don't need to
        // be added
        auto b = this->first_rhino_batch;
        auto new_b = CreateObject<PartialStateBatch>(this, mp);
        for (auto &[group_id, extents] : this->groupIdToWindowExtents) {
            // TODO: Look in opposite order of the extents, and just break at the first present extent, because then we know that all earlier ones are there
            for (int i = extents.size()-1; i >= 0; i--) {
                auto &extent = extents[i];
                if (std::find_if(b->partial_states.begin(), b->partial_states.end(), [extent](Ptr<PartialStateWrapper> ps_wrapper) {return ps_wrapper->object == extent;}) != std::end(b->partial_states)) {
                    break;
                }
                auto ps_wrapper = CreateObject<PartialStateWrapper>(extent);
                new_b->partial_states.emplace_back(ps_wrapper);
            }
        }

        mp->critQueue.emplace_back(new_b);
    }

    void StorageWindowOperator::ExtractDiffState(Ptr<MigrationPlan> mp) {
        auto b = this->first_rhino_batch;
        auto new_b = CreateObject<PartialStateBatch>(this, mp);
        for (auto tuple : this->tuples) {
            if (std::find_if(b->partial_states.begin(), b->partial_states.end(), [tuple](Ptr<PartialStateWrapper> ps_wrapper) {return ps_wrapper->object == tuple;}) != std::end(b->partial_states)) {
                continue;
            }
            auto ps_wrapper = CreateObject<PartialStateWrapper>(tuple);
            new_b->partial_states.emplace_back(ps_wrapper);
        }

        mp->critQueue.emplace_back(new_b);
    }

    void CepWindowOperator::ExtractDiffState(Ptr<MigrationPlan> mp) {

    }

    Ptr<PartialStateBatch> StorageWindowOperator::ExtractPreambleState(Ptr<MigrationPlan> mp) {
        // TODO: Extract the size of the entire "all at once" state
        Ptr<PartialStateBatch> static_batch;
        if (mp->migration_mode == "rhino") {
            static_batch = CreateObject<PartialStateBatch> (this, mp);
            // We add a wrapper to represent the static state of the operator itself
            auto op_wrapper = CreateObject<PartialStateWrapper> (this);
            static_batch->partial_states.emplace_back(op_wrapper);
            mp->preambleQueue.emplace_back(static_batch);

            for (auto tuple : this->tuples) {
                auto ps_wrapper = CreateObject<PartialStateWrapper>(tuple);
                static_batch->partial_states.emplace_back(ps_wrapper);
            }
            this->first_rhino_batch = static_batch;
        } else {
            static_batch = Operator::ExtractPreambleState(mp);
        }

        return static_batch;
    }

    Ptr<PartialStateBatch> AggregationWindowOperator::ExtractPreambleState(Ptr<MigrationPlan> mp) {
        // TODO: Extract the size of the entire "all at once" state
        Ptr<PartialStateBatch> static_batch;
        if (mp->migration_mode == "rhino") {
            static_batch = CreateObject<PartialStateBatch> (this, mp);
            // We add a wrapper to represent the static state of the operator itself
            auto op_wrapper = CreateObject<PartialStateWrapper> (this);
            static_batch->partial_states.emplace_back(op_wrapper);
            mp->preambleQueue.emplace_back(static_batch);

            for (auto &[group_id, extents] : this->groupIdToWindowExtents) {
                for (auto &extent : extents) {
                    auto ps_wrapper = CreateObject<PartialStateWrapper>(extent);
                    static_batch->partial_states.emplace_back(ps_wrapper);
                }
            }
            this->first_rhino_batch = static_batch;
        } else {
            static_batch = Operator::ExtractPreambleState(mp);
            if (mp->migration_mode == "megaphone") {
                // First, tell all operators to automatically forward to the new host
                // TODO: This won't work with multiple aggregation operators in the same query, but should work for replication
                for (auto op : mp->operators) {
                    for (auto inner_op : op->stream->operators) {
                        for (auto &[group_id, extents] : this->groupIdToWindowExtents) {
                            Operator::queryIdToOperatorIdToGroupIdToNewHost[this->query->id][this->id][group_id] = true;
                        }
                    }
                }

                // Then, tell all nodes to send to the old host of the aggregation window operators if they have extents
                for (auto inner_op : this->stream->operators) {
                    for (auto &[group_id, extents] : this->groupIdToWindowExtents) {
                        Operator::queryIdToOperatorIdToGroupIdToNewHost[this->query->id][this->id][group_id] = extents.empty();
                    }
                }
            }
        }

        return static_batch;
    }

    void AggregationWindowOperator::ExtractState(Ptr<MigrationPlan> mp) {
        // If the operator has no extents or the migration mode includes no state, we just return
        if (this->groupIdToWindowExtents.empty() || mp->migration_mode == "drop-state") {
            return;
        }
        this->groupIdToExtentsInMigration = this->groupIdToWindowExtents;

        if (mp->migration_mode == "lazy") {
            ExtractLazyState(mp);
        } else if (mp->migration_mode == "rhino") {
            /*auto batch = CreateObject<PartialStateBatch>(this, mp);
            int i = 0;
            for (auto &[group_id, extents] : this->groupIdToWindowExtents) {
                // TODO: Only add the size of the new partial states, i.e., the ones that were added after calling ExtractPreambleState()
                for (auto &extent : extents) {
                    auto ps_wrapper = CreateObject<PartialStateWrapper>(extent);
                    batch->partial_states.emplace_back(ps_wrapper);
                }
            }

            if (!batch->partial_states.empty()) {
                mp->critQueue.emplace_back(batch);
            }*/
        } else {
            if (mp->migration_mode == "all-at-once") {
                auto batch = CreateObject<PartialStateBatch>(this, mp);
                for (auto &[group_id, extents] : this->groupIdToWindowExtents) {
                    for (auto &extent : extents) {
                        auto ps_wrapper = CreateObject<PartialStateWrapper>(extent);
                        batch->partial_states.emplace_back(ps_wrapper);
                    }
                }

                mp->critQueue.emplace_back(batch);
            } else if (mp->migration_mode == "utility") {
                auto batch = CreateObject<PartialStateBatch>(this, mp);
                for (auto &[group_id, extents] : this->groupIdToWindowExtents) {
                    for (auto &extent : extents) {
                        auto ps_wrapper = CreateObject<PartialStateWrapper>(extent);
                        batch->partial_states.emplace_back(ps_wrapper);
                    }
                    if (batch->partial_states.size() > ExposeCoordinator::BATCH_SIZE) {
                        mp->critQueue.emplace_back(batch);
                        batch = CreateObject<PartialStateBatch>(this, mp);
                    }
                }

                if (!batch->partial_states.empty()) {
                    mp->critQueue.emplace_back(batch);
                }
            } else if (mp->migration_mode == "megaphone" || mp->migration_mode == "meces") {
                auto batch = CreateObject<PartialStateBatch>(this, mp);
                for (auto &[group_id, extents] : this->groupIdToWindowExtents) {
                    // TODO: Add the migration of groups here, unless they're already being migrated
                    if (groupOperator->group_map[group_id] != Group::no_group) {
                        auto ps_wrapper = CreateObject<PartialStateWrapper>(groupOperator->group_map[group_id]);
                        batch->partial_states.emplace_back(ps_wrapper);
                    }
                    for (auto &extent : extents) {
                        auto ps_wrapper = CreateObject<PartialStateWrapper>(extent);
                        // Here, we're setting the routing of keys such that by default, tuples are still sent to the
                        // old host. As these batches get dispatched, ExposeWrapper:MoveNextState sets these values
                        // to true, and then the tuples of these keys get forwarded to the new host.
                        batch->partial_states.emplace_back(ps_wrapper);
                    }
                    //if (batch->partial_states.size() > 5000) {
                    if (batch->partial_states.size() > ExposeCoordinator::BATCH_SIZE) {
                        mp->normalEssentialQueue.emplace_back(batch);
                        batch = CreateObject<PartialStateBatch>(this, mp);
                    }
                }

                if (!batch->partial_states.empty()) {
                    mp->normalEssentialQueue.emplace_back(batch);
                }

                std::cout << "Extracted " << mp->normalEssentialQueue.size() << " partial states" << std::endl;
                //mp->normalEssentialQueue.emplace_back(batch);
            } else if (mp->migration_mode == "state-shedding") {
                auto batch = CreateObject<PartialStateBatch>(this, mp);
                for (auto &[group_id, extents] : this->groupIdToWindowExtents) {
                    for (auto &extent : extents) {
                        auto ps_wrapper = CreateObject<PartialStateWrapper>(extent);
                        batch->partial_states.emplace_back(ps_wrapper);
                    }
                }

                mp->critQueue.emplace_back(batch);
            } else {
                NS_FATAL_ERROR("Invalid migration mode " << mp->migration_mode);
            }
        }
    }

    void StorageWindowOperator::ExtractState(Ptr<MigrationPlan> mp) {
        auto batch = CreateObject<PartialStateBatch>(this, mp);
        for (const auto &t : this->tuples) {
            auto ps_wrapper = CreateObject<PartialStateWrapper>(t);
            // This is because the JoinOperator takes the burden of the state size here
            if (this->joinOperator != nullptr) {
                ps_wrapper->size = 0;
            }
            batch->partial_states.emplace_back(ps_wrapper);
        }
        mp->critQueue.emplace_back(batch);
    }

    void CepWindowOperator::ExtractState(Ptr<MigrationPlan> mp) {

    }

    std::vector<Ptr<PartialState>> AggregationWindowOperator::getPartialStates() {
        std::vector<Ptr<PartialState>> partial_states;
        for (auto &[group_id, extents] : groupIdToWindowExtents) {
            for (auto &extent : extents) {
                partial_states.emplace_back(extent);
            }
        }

        return partial_states;
    }

    std::vector<Ptr<PartialState>> StorageWindowOperator::getPartialStates() {
        std::vector<Ptr<PartialState>> partial_states;
        for (const auto& t : tuples) {
            partial_states.emplace_back(t);
        }

        return partial_states;
    }

    size_t WindowExtent::GetSerializedSize() {
        std::size_t size = 0;
        for (auto agg : aggregationWindowExtents) {
            size += sizeof(agg.aggregate);
            size += sizeof(agg.count);
        }
        return size;
    }
    size_t WindowExtent::GetUtility() {return 1;}

    void AggregationWindowOperator::SetActive() {
        if (active) {
            return;
        }
        this->active = true;
        if (emit_type == PROCESSING_TIME || emit_type == EXTERNAL_TIME) {
            //auto next_emit = std::max(MilliSeconds(emit_size) + last_emit_time - Simulator::Now(), MilliSeconds(0));
            if (first_extent_time.IsZero()) {
                //std::cout << "first_extent_time: " << first_extent_time << std::endl;
                //first_extent_time = Simulator::Now() - MilliSeconds(emit_size+1);
                first_extent_time = Simulator::Now() - MilliSeconds(emit_size+1);
            }
            activated_time_emission = true;
            auto next_emit = MilliSeconds(first_extent_time.GetMilliSeconds() % emit_size + Simulator::Now().GetMilliSeconds() - Simulator::Now().GetMilliSeconds() % emit_size);
            while (next_emit < Simulator::Now()) {
                next_emit += MilliSeconds(emit_size);
            }
            //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Setting aggregation window operator " << this->id << " active on Node " << this->thread->resourceManager->node_id << ", on " << next_emit.GetMilliSeconds() << ", first_extent_time: " << first_extent_time.GetMilliSeconds());
            Simulator::Schedule(next_emit-Simulator::Now(), &AggregationWindowOperator::emit_point, this, true);
            //Simulator::Schedule(MilliSeconds(first_extent_time.GetMilliSeconds() % emit_size + Simulator::Now().GetMilliSeconds() - Simulator::Now().GetMilliSeconds() % emit_size), &AggregationWindowOperator::emit_point, this, true);
            //Simulator::Schedule(next_emit, &AggregationWindowOperator::emit_point, this, true);
        }
    }

    void WindowExtent::merge(Ptr<WindowExtent> extent) {
        NS_ASSERT(this->aggregationWindowExtents.size() == extent->aggregationWindowExtents.size());
        this->merged = true;
        for (int i = 0; i < (int) aggregationWindowExtents.size(); i++) {
            auto &new_asf = this->aggregationWindowExtents[i];
            auto &migrated_asf = extent->aggregationWindowExtents[i];
            new_asf.count += migrated_asf.count;
            switch (new_asf.aggregate_type) {
                case AVG: {
                    new_asf.aggregate += migrated_asf.aggregate;
                    break;
                } case MIN: {
                    new_asf.aggregate = std::min(new_asf.aggregate, migrated_asf.aggregate);
                    break;
                } case MAX: {
                    new_asf.aggregate = std::max(new_asf.aggregate, migrated_asf.aggregate);
                    break;
                } case SUM: {
                    new_asf.aggregate += migrated_asf.aggregate;
                    break;
                } case COUNT: {
                    break;
                }
                case NONE:
                    break;
            }
        }
    }

    void WindowExtent::replace(Ptr<WindowExtent> extent) {
        NS_ASSERT(this->aggregationWindowExtents.size() == extent->aggregationWindowExtents.size());
        for (int i = 0; i < (int) aggregationWindowExtents.size(); i++) {
            auto &migrated_asf = extent->aggregationWindowExtents[i];
            auto &new_asf = this->aggregationWindowExtents[i];
            new_asf.count = migrated_asf.count;
            switch (new_asf.aggregate_type) {
                case AVG: {
                    new_asf.aggregate = migrated_asf.aggregate;
                    break;
                } case MIN: {
                    new_asf.aggregate = migrated_asf.aggregate;
                    break;
                } case MAX: {
                    new_asf.aggregate = migrated_asf.aggregate;
                    break;
                } case SUM: {
                    new_asf.aggregate = migrated_asf.aggregate;
                    break;
                } case COUNT: {
                    break;
                }
                case NONE:
                    break;
            }
        }
    }

    bool AggregationWindowOperator::CheckForExistingExtentsAndReplace(Ptr<WindowExtent> extent) {
        auto &extents = this->groupIdToWindowExtents[extent->group->id];
        for (auto it = extents.begin(); it != extents.end(); it++) {
            auto extent_in_migration = *it;
            if (extent->lower_bound == extent_in_migration->lower_bound && extent->upper_bound == extent_in_migration->upper_bound && extent->group->id == extent_in_migration->group->id) {
                extent_in_migration->in_migration = false;
                extent_in_migration->replace(extent);
                return true;
            }
        }
        return false;
    }

    bool AggregationWindowOperator::CheckForExistingExtentsAndMerge(Ptr<WindowExtent> extent) {
        auto &extents = this->groupIdToWindowExtents[extent->group->id];
        for (auto it = extents.begin(); it != extents.end(); it++) {
            auto extent_in_migration = *it;
            if (extent->lower_bound == extent_in_migration->lower_bound && extent->upper_bound == extent_in_migration->upper_bound) {
                //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " i: " << i << " Asserting that extent_in_migration->in_migration is true and then setting it to false, lower_bound: " << extent->lower_bound.GetMilliSeconds() << ", upper_bound: " << extent->upper_bound.GetMilliSeconds() << ", and key: " << extent->group->id);
                NS_ASSERT(extent_in_migration->in_migration);
                extent_in_migration->in_migration = false;
                extent_in_migration->merge(extent);
                return true;
            }
        }
        return false;
    }

    int imported = 0;
    int number_merged = 0;
    void AggregationWindowOperator::ImportState(Ptr<PartialStateBatch> batch) {
        NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " AggregationWindowOperator ImportState");
        if (batch->partial_states.empty()) {
            return;
        }
        static int cnt4 = 0;
        static int cnt5 = 0;

        static int total_cnt_imported_extents = 0;
        static int total_cnt_imported_tuples = 0;
        auto op = DynamicCast<AggregationWindowOperator>(batch->partial_states[0]->object);
        int offset = 0;
        if (op != nullptr) {
            //std::cout << "Old host's operator " << op << " is being migrated to the new host's op " << this << std::endl;
            this->total_received = op->total_received;
            this->groupIdToExtentsInMigration = op->groupIdToExtentsInMigration;
            this->last_emit_time = op->last_emit_time;
            //std::cout << "op->groupIdToWindowExtents.count(2): " << op->groupIdToWindowExtents.count(2) << std::endl;
            if (!op->first_extent_time.IsZero()) {
                this->first_extent_time = op->first_extent_time;
            }
            if (batch->partial_states.size() == 1) {
                return;
            }
            offset = 1;
        }
        //std::cout << "Number keys before: " << this->groupIdToWindowExtents.size() << std::endl;
        for (int i = offset; i < (int) batch->partial_states.size(); i++) {
            auto ps = batch->partial_states[i];
            auto group = DynamicCast<Group>(ps->object);
            if (group != nullptr) {
                auto group_copy = group->copy(groupOperator, false);
                groupOperator->first_grouper->groupTuple(CreateObject<TupleWrapper>(group_copy->first_tuple_wrapper));
                continue;
            }
            auto extent = DynamicCast<WindowExtent>(ps->object);
            ++total_cnt_imported_extents;
            total_cnt_imported_tuples += extent->aggregationWindowExtents[0].count;
            NS_ASSERT(extent != nullptr);
            extent->migration_stop = Simulator::Now();
            auto &extents = this->groupIdToWindowExtents[extent->group->id];
            bool merged = false;
            if (batch->mp->migration_mode == "rhino") {
                if (++extent->current_migration_phase >= batch->mp->operatorIdToNumberPhases[this->id]) {
                    merged = CheckForExistingExtentsAndReplace(extent);
                } else {
                    merged = false;
                }
            } else {
                merged = CheckForExistingExtentsAndMerge(extent);
            }
            if (merged) {
                // DEBUG
                // Assert that the extent is not pending for emission
                for (auto it = pending_extents[extent->group->id].begin(); it != pending_extents[extent->group->id].end(); it++) {
                    auto pending_timed_extent = *it;
                    NS_ASSERT(pending_timed_extent->upper_bound != extent->upper_bound);
                }
            }
            if (!merged) {
                // This means that the extent is missing
                bool inserted = false;

                for (auto it = pending_extents[extent->group->id].begin(); it != pending_extents[extent->group->id].end(); it++) {
                    auto pending_timed_extent = *it;
                    if (pending_timed_extent->upper_bound == extent->upper_bound) {
                        // Emit the pending extent
                        inserted = true;
                        //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Emitting pending extent with " << pending_timed_extent->aggregationWindowExtents[0].count << " tuples!");
                        pending_timed_extent->merge(extent);
                        doEmitExtent(pending_timed_extent);
                        it = pending_extents[extent->group->id].erase(it);
                        break;
                    }
                }

                if (!inserted) {
                    extents.emplace_back (extent);
                }
            }

            // Remove it from groupIdToExtentsInMigration
            // Not sure if this is necessary, but good for consistency's sake for now at least
            groupIdToExtentsInMigration[extent->group->id].erase(std::remove_if(groupIdToExtentsInMigration[extent->group->id].begin(), groupIdToExtentsInMigration[extent->group->id].end(), [extent](Ptr<WindowExtent> e) {
                return e == extent;
            }), groupIdToExtentsInMigration[extent->group->id].end());
        }

        int total_number_extents = 0;
        for (auto &[group, window_extents] : this->groupIdToWindowExtents) {
            for (auto &extent : window_extents) {
                for (auto &[group_id, partition] : extent->groupToPartitions) {
                    extent->groupToPartitions[group_id] = groupOperator->group_map[group_id];
                    // TODO: This sometimes crashes
                    NS_ASSERT(extent->groupToPartitions[group_id] != nullptr);
                }

                if (extent->group == Group::no_group) {
                    continue;
                }

                extent->group = groupOperator->group_map[extent->group->id];
                NS_ASSERT(extent->group != nullptr);
            }
            total_number_extents += window_extents.size();
        }

        total_number_extents = 0;
        for (auto &[group, window_extents] : this->groupIdToWindowExtents) {
            auto &extents = this->groupIdToWindowExtents[group];
            std::sort(extents.begin(), extents.end(), [](const Ptr<WindowExtent> a, const Ptr<WindowExtent> b) {
              return a->upper_bound < b->upper_bound;
            });
            total_number_extents += window_extents.size();
        }
        if (batch->mp->migration_mode == "rhino") {
            // Evict already emitted extents
            // TODO: Make this work in a way that doesn't rely on the old and new host using the same state
            // TODO: It could work by having one batch of deletion requests, but that mechanism is currently missing
            for (auto &[group, window_extents] : this->groupIdToWindowExtents) {
                auto &extents = this->groupIdToWindowExtents[group];
                auto it = extents.begin();
                while (it != extents.end()) {
                    auto e = *it;
                    if (e->emitted) {
                        it = extents.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }
        if (batch->partial_states.size() > 1 && DynamicCast<WindowExtent>(batch->partial_states[offset+1]->object) != nullptr) {
            // True for Meces and Megaphone
            if (batch->mp->migration_mode == "megaphone" || batch->mp->migration_mode == "meces") {
                // This is because I migrate each key completely with Megaphone
                std::vector<int> groups;
                for (auto ps_wrapper: batch->partial_states) {
                    auto group = DynamicCast<Group>(ps_wrapper->object);
                    if (group == nullptr) {
                        continue;
                    }
                    groups.push_back(group->id);
                }
                for (int i = 0; i < (int) groups.size(); i++) {
                    for (auto &pending_extent: pending_extents[groups[i]]) {
                        for (auto extent_in_migration: this->groupIdToExtentsInMigration[groups[i]]) {
                            NS_ASSERT(extent_in_migration->upper_bound != pending_extent->upper_bound);
                        }
                        doEmitExtent(pending_extent);
                    }
                }
            }
            NS_LOG_INFO("Total number window extents: " << total_number_extents << ", Total number imported extents: " << total_cnt_imported_extents << ", total number imported tuples: " << total_cnt_imported_tuples); //", Total merge count (old host's count added to new host's): " << total_merge_cnt2 << ", and total from the new host: " << total_merge_cnt);
            NS_LOG_INFO("Node 4 inserting imported extent " << cnt5 << " with accumulated tuples count " << cnt4);
            Simulator::ScheduleNow(&AggregationWindowOperator::emit_point, this, false);
        }
    }

    void StorageWindowOperator::ImportState(Ptr<PartialStateBatch> batch) {
        if (batch->partial_states.empty()) {
            return;
        }
        int offset = 0;
        auto op = DynamicCast<StorageWindowOperator>(batch->partial_states[0]->object);
        if (op != nullptr) {
            this->evicted_tuples = op->evicted_tuples;
            if (batch->partial_states.size() == 1) {
                return;
            }
            offset = 1;
        }

        // Extract tuples
        for (int i = offset; i < (int) batch->partial_states.size(); i++) {
            auto ps = batch->partial_states[i];
            auto t = DynamicCast<TupleWrapper>(ps->object);
            NS_ASSERT(t != nullptr);
            this->tuples.emplace_back(t);
        }
    }

    void CepWindowOperator::ImportState(Ptr<PartialStateBatch> batch) {

    }
}
