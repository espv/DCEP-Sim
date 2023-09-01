/*
 * Copyright (C) 2018, Fabrice S. Bigirimana
 * Copyright (c) 2018, University of Oslo
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * 
 */

#include "dcep.h"
#include "ns3/Tuple.h"
#include "ns3/node.h"
#include "ns3/uinteger.h"
#include "ns3/object.h"
#include "ns3/log.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "placement.h"
#include "communication.h"
#include "cep-engine.h"
#include "common.h"
#include "ns3/simulator.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/object-base.h"
#include <boost/asio/buffer.hpp>
#include <boost/asio/write.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <ctime>
#include <chrono>
#include <iostream>
#include <fstream>
#include <random>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(Dcep);
NS_LOG_COMPONENT_DEFINE ("Dcep");


    TypeId
    Dcep::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::Dcep")
        .SetParent<Application> ()
        .SetGroupName("Applications")
        .AddConstructor<Dcep> ();

        return tid;
    }
    
     
    Dcep::Dcep()
    {
        this->m_cepPort = 60123;
    }
    
    uint16_t
    Dcep::getCepEventCode()
    {
        return this->event_code;
    }
    
    void
    Dcep::StartApplication (void)
    {
        
        
        NS_LOG_FUNCTION (this);
        
        
        
        Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
        uint32_t mrand = x->GetInteger (1,30);
        mrand = mrand;
        
        Ptr<Placement> c_placement = CreateObject<Placement> ();
        
        AggregateObject (c_placement);
       
        Ptr<CEPEngine> c_cepengine = CreateObject<CEPEngine> ();
        AggregateObject (c_cepengine);

        this->c_communication = CreateObject<Communication> ();

        c_placement->configure();
        c_cepengine->Configure();

        c_communication->setNode(GetNode());
        c_communication->setPort(m_cepPort);
        c_communication->SetAttribute("SinkAddress", Ipv4AddressValue (m_sinkAddress));
        c_communication->Configure();
        //c_communication->SetupConnections();
        
        NS_LOG_INFO(Simulator::Now() << " STARTED DCEP APPLICATION AT NODE " << c_communication->GetLocalAddress());


    }

    void
    Dcep::StopApplication (void)
    {
        NS_LOG_FUNCTION (this);
    }

    void
    Dcep::SendPacket(Ptr<Packet> p, InetSocketAddress addr)
    {
        Ptr<Communication> comm = this->c_communication;
        comm->ScheduleSend(p, addr);
    }

    Ptr<Dcep>
    Dcep::GetDcepFromNode(Ptr<Node> node) {
        for (std::size_t i = 0; i < node->GetNApplications(); i++) {
            auto dcep = DynamicCast<Dcep>(node->GetApplication(i));
            if (dcep) {
                return dcep;
            }
        }
        return nullptr;
    }

    void
    Dcep::HandleStandardMovingMigration(Ptr<DcepPackage> dcep_package) {
        // Invoke ImportState in the relevant operator
        auto dcep_header = dcep_package->dcep_header;
        auto node_id = StreamQuery::ns3NodeIdToExposeNodeId[this->m_node->GetId()];
        Ptr<Placement> p = GetObject<Placement>();
        auto cepEngine = GetObject<CEPEngine>();
        auto wrapper = ExposeCoordinator::nodeIdsToWrappers[node_id];
        auto batch_key = dcep_header.arguments["batch-id"];
        auto batch = wrapper->nodeToMigrationPayloads[dcep_header.GetFromNode()][batch_key];
        //NS_ASSERT(!batch->partial_states.empty());
        wrapper->nodeToMigrationPayloads[dcep_header.GetFromNode()].erase(batch_key);
        std::vector<int> partial_states_moved;

        auto unique_query_id = dcep_header.arguments["unique_query_id"];
        auto operator_id = dcep_header.arguments["operator_id"];
        auto old_host = dcep_header.GetFromNode();
        c_communication->in_migration = true;
        c_communication->migration_node = old_host;
        c_communication->migrating_query_id = unique_query_id;

        int new_host = std::stoi(StreamQuery::ns3NodeIdToExposeNodeId[m_node->GetId()]);
        auto old_query = ExposeCoordinator::queryIdToNodeIdToQuery[unique_query_id][old_host];
        auto new_query = ExposeCoordinator::queryIdToNodeIdToQuery[unique_query_id][new_host];

        Ptr<Operator> old_op = batch->op;
        auto new_op = new_query->streamIdToOperatorIdToOperator[old_op->stream->input_stream_ids[0]][old_op->id]->stream->operators[old_op->partition];
        if (dcep_header.arguments["op_final_batch"]) {
            for (auto op : new_op->stream->operators) {
                op->in_migration = false;
            }
            for (auto op : old_op->stream->operators) {
                op->in_migration = false;
            }
        }

        // We don't do this because of Lazy, because lazy has multiple phases for each partial state, so this partial state
        // is not necessarily done with the migration at this point
        /*for (auto ps_wrapper : batch->partial_states) {
            ps_wrapper->object->in_migration = false;
        }*/

        new_op->ImportState(batch);
        if (dcep_header.arguments["active_op"] && !new_op->active) {
            std::cout << Simulator::Now().GetMilliSeconds() << " Operator " << new_op->id << " on Node " << node_id << " is now active!" << std::endl;
            new_op->SetActive();
            //new_op->thread->processQueuedTuple(new_op->thread->operatorIdToMigrationTupleQueue);
            Simulator::ScheduleNow(&CpuCore::processQueuedTuple, new_op->thread);
            //new_op->thread->processQueuedTuple();
        }
        auto last_batch = dcep_header.arguments["last-batch"];
        if (last_batch && batch->mp->migration_mode != "drop-state") {
            std::cout << "Received last batch for operator " << operator_id << " at " << Simulator::Now().GetMilliSeconds() << std::endl;
            std::cout << "New host has " << new_op->thread->operatorIdToTupleQueue[new_op->id].size() << " queued tuples" << std::endl;
            //std::cout << "Old host has " << old_op->thread->operatorIdToTupleQueue[old_op->id].size() << " queued tuples" << std::endl;
            Metrics::trace_enabled = false;
            Metrics::migration_stops.emplace_back(Simulator::Now());
            c_communication->in_migration = false;
            c_communication->migration_node = -1;
            c_communication->migrating_query_id = -1;
        }
    }

    void
    Dcep::HandleWindowRecreationMigration(Ptr<DcepPackage> dcep_package) {

    }

    void
    Dcep::HandleStateRecreationMigration(Ptr<DcepPackage> dcep_package) {

    }

    void
    Dcep::HandleStatelessMigration(Ptr<DcepPackage> dcep_package) {

    }

    void
    Dcep::HandleFetchRequest(Ptr<DcepPackage> dcep_package) {
        auto dcep_header = dcep_package->dcep_header;
        auto node_id = StreamQuery::ns3NodeIdToExposeNodeId[this->m_node->GetId()];
        //NS_LOG_INFO(Simulator::Now() << " Received fetch request");
        Ptr<Placement> p = GetObject<Placement>();
        auto cepEngine = GetObject<CEPEngine>();
        auto wrapper = ExposeCoordinator::nodeIdsToWrappers[node_id];
        NS_ASSERT(!wrapper->nodeToMigrationFetchRequests[dcep_header.GetFromNode()].empty());
        int count = dcep_package->dcep_header.arguments["count"];

        auto unique_query_id = dcep_header.arguments["unique_query_id"];
        auto operator_id = dcep_header.arguments["operator_id"];

        static int cnt_fetched = 0;
        static int total_fetched = 0;
        total_fetched += count;
        NS_LOG_INFO(Simulator::Now().GetMicroSeconds() << " Received fetch request number " << ++cnt_fetched << ", total fetched: " << total_fetched);


        auto old_host = dcep_header.GetFromNode();

        int new_host = std::stoi(StreamQuery::ns3NodeIdToExposeNodeId[m_node->GetId()]);
        auto old_query = ExposeCoordinator::queryIdToNodeIdToQuery[unique_query_id][old_host];
        auto new_query = ExposeCoordinator::queryIdToNodeIdToQuery[unique_query_id][new_host];

        Ptr<Operator> old_op;
        int stream_id;
        for (auto &[sid, operatorIdToOperator]: new_query->streamIdToOperatorIdToOperator) {
            old_op = old_query->streamIdToOperatorIdToOperator[sid][operator_id];
            if (old_op == nullptr) {
                continue;
            }
            stream_id = sid;
            break;
        }
        if (old_op == nullptr) {
            NS_FATAL_ERROR("Invalid operator!");
        }
        auto new_op = new_query->streamIdToOperatorIdToOperator[stream_id][operator_id];

        auto new_batch = CreateObject<PartialStateBatch>(new_op, wrapper->mp);
        if (wrapper->mp->fetchQueue.empty()) {
            wrapper->mp->fetchQueue.emplace_back(CreateObject<PartialStateBatch>(new_op, wrapper->mp));
        }
        for (int i = 0; i < count; i++) {
            auto &request = wrapper->nodeToMigrationFetchRequests[dcep_header.GetFromNode()].front();

            auto &last_fetch_batch = wrapper->mp->fetchQueue[wrapper->mp->fetchQueue.size() - 1];
            bool add_to_last_batch = last_fetch_batch->partial_states.size() + request.size() <= ExposeCoordinator::BATCH_SIZE;
            for (auto it = request.begin(); it != request.end(); it++) {
                auto desired_ps = (Ptr<PartialStateWrapper>)*it;

                bool found = false;
                for (auto batch_it = wrapper->mp->normalEssentialQueue.begin();
                     batch_it !=
                     wrapper->mp->normalEssentialQueue.end(); batch_it++) {//auto batch : wrapper->mp->normalEssentialQueue) {
                    auto batch = *batch_it;
                    for (auto it2 = batch->partial_states.begin(); it2 != batch->partial_states.end(); it2++) {
                        if ((*it2)->object == desired_ps->object) {
                            batch->partial_states.erase(it2);
                            found = true;
                            // Really, this shouldn't happen. But we have to try to handle the case that it does
                            // The problem is that it means that we have tiny batches
                            if (batch->partial_states.empty()) {
                                wrapper->mp->normalEssentialQueue.erase(batch_it);
                                --batch_it;
                                break;
                            }
                            break;
                        }
                    }
                    if (found) {
                        break;
                    }
                }
                if (add_to_last_batch) {
                    last_fetch_batch->partial_states.emplace_back(desired_ps);
                } else {
                    new_batch->partial_states.emplace_back(desired_ps);
                }
            }
            wrapper->nodeToMigrationFetchRequests[dcep_header.GetFromNode()].pop_front();
        }
        if (!new_batch->partial_states.empty()) {
            wrapper->mp->fetchQueue.emplace_back(new_batch);
        }
    }

    void
    Dcep::HandleInterruptedMigration() {
        std::cout << "Migration from Node " << c_communication->migration_node << " is interrupted!" << std::endl;
        // TODO: Traverse the operators in the migrating query and set them to not being in migration
        auto node_id = std::stoi(StreamQuery::ns3NodeIdToExposeNodeId[this->m_node->GetId()]);
        auto new_query = ExposeCoordinator::queryIdToNodeIdToQuery[c_communication->migrating_query_id][node_id];
        std::set<int> interrupted_operators;
        for (auto &[k, v] : new_query->streamIdToOperatorIdToOperator) {
            for (auto &[k2, v2] : v) {
                // We don't want to call MigrationInterrupted() twice on the same operator, which can happen if the
                // operators appear multiple times in the operator graph, e.g., for a join operator
                if (interrupted_operators.contains(v2->id)) {
                    continue;
                }
                v2->in_migration = false;
                v2->MigrationInterrupted();
                interrupted_operators.insert(v2->id);
            }
        }

        c_communication->migration_node = -1;
        c_communication->migrating_query_id = -1;
        c_communication->in_migration = false;
    }

    void
    Dcep::HandleMigration(Ptr<DcepPackage> dcep_package) {
        auto dcep_header = dcep_package->dcep_header;
        auto node_id = StreamQuery::ns3NodeIdToExposeNodeId[this->m_node->GetId()];
        Ptr<Placement> p = GetObject<Placement>();
        auto cepEngine = GetObject<CEPEngine>();
        auto wrapper = ExposeCoordinator::nodeIdsToWrappers[node_id];
        //NS_LOG_INFO("Size of incoming migration payload queue: " << wrapper->nodeToMigrationPayloads[dcep_header.GetFromNode()].size());
        NS_ASSERT(!wrapper->nodeToMigrationPayloads[dcep_header.GetFromNode()].empty());
        auto batch_key = dcep_header.arguments["batch-id"];
        auto batch = wrapper->nodeToMigrationPayloads[dcep_header.GetFromNode()][batch_key];
        std::vector<int> partial_states_moved;

        static int cnt = 0;
        static int sum = 0;
        static int sum_ps = 0;
        //static int last_batch_key = -1;
        //last_batch_key = batch_key;
        sum += batch->GetSerializedSize();
        sum_ps += batch->partial_states.size();
        NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Received batch " << ++cnt << " with operator " << batch->op->id << " (batch-id: " << batch_key << ") with size " << batch->GetSerializedSize() << " bytes, total bytes: " << sum << ", number partial states: " << batch->partial_states.size() << ", total number partial states: " << sum_ps);

        //auto query_id = dcep_header.arguments["query_id"];
        //auto operator_id = dcep_header.arguments["operator_id"];
        //auto old_host = dcep_header.arguments["old_host"];
        auto migration_type = dcep_header.arguments["migration_type"];

        switch (migration_type) {
            case STANDARD_MOVING_STATE:
                HandleStandardMovingMigration(dcep_package);
                break;
            case STATE_RECREATION:
                HandleStateRecreationMigration(dcep_package);
                break;
            case WINDOW_RECREATION:
                HandleWindowRecreationMigration(dcep_package);
                break;
            case STATELESS:
                HandleStatelessMigration(dcep_package);
                break;
            default:
                NS_FATAL_ERROR("Invalid migration type");
        }

        for (auto &ps : batch->partial_states) {
            Metrics::TracePartialStateReceived(ps, std::stoi(node_id), batch->op->query, batch->op);
        }
        wrapper->nodeToMigrationPayloads[dcep_header.GetFromNode()].erase(batch_key);
    }

    int cnt4 = 0;
    void
    Dcep::rcvRemoteMsg(Ptr<DcepPackage> dcep_package) {
        auto dcep_header = dcep_package->dcep_header;
        auto size = (uint32_t) dcep_header.GetContentSize();
        auto msg_type = dcep_header.GetContentType();
        auto data = dcep_package->payload;
        auto node_id = StreamQuery::ns3NodeIdToExposeNodeId[this->m_node->GetId()];

        //auto msg_subtype = dcepHeader.GetContentSubType();
        Ptr<Placement> p = GetObject<Placement>();
        auto cepEngine = GetObject<CEPEngine>();
        auto wrapper = ExposeCoordinator::nodeIdsToWrappers[node_id];

        switch (msg_type) {
            case EVENT: /*handle event*/
            {
                auto deserialized_tuple_wrapper = TupleWrapper::Deserialize(data, size);
                //NS_LOG_INFO(Simulator::Now() << " Received tuple with next operator ID " << deserialized_tuple_wrapper->next_operator_id);
                p->RcvCepEventNew(deserialized_tuple_wrapper);
                break;
            }
            case MIGRATION_FETCH: {
                // TODO: Fetch migration plan and the operator in the message
                // TODO: Look for the partial state that is desired and place it in the front of the queue
                // TODO: This is easiest to do after the priority queue stuff has implemented, because it requires the ordering
                static int cnt_fetch = 0;
                if (++cnt_fetch % 1000 == 0) {
                    NS_LOG_INFO(Simulator::Now().GetMicroSeconds() << "Received fetch request " << cnt_fetch);
                }
                HandleFetchRequest(dcep_package);
                break;
            }
            case MIGRATION: {
                HandleMigration(dcep_package);
                break;
            }
        }
    }
}
