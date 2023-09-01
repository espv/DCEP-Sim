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

/*
 * Implementation of a CEP engine including models for:
 *  1. CepEvent and Query models
 *  2. Detector and Producer models to process CEP events
 *  3. A forwarder model to forward composite events produced by the CEP engine
 */

#include "cep-engine.h"
#include "ns3/names.h"
#include "ns3/log.h"
#include "ns3/config.h"
#include "placement.h"
#include "message-types.h"
#include "ns3/abort.h"
#include "ns3/inet-socket-address.h"
#include "dcep.h"
#include "ns3/event-impl.h"
#include <yaml-cpp/yaml.h>
#include "ns3/Query.h"

#include <algorithm>
#include "ns3/dcep-header.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(CEPEngine);
NS_LOG_COMPONENT_DEFINE ("CEPEngine");

/**************** CEP CORE *******************
 * *************************************************
 * ***************************************************/
/* ... */
    TypeId
    FogApplicationEngine::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::FogApplicationEngine")
                .SetParent<Object> ()
        ;

        return tid;
    }

    TypeId
    CEPEngine::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::CEPEngine")
        .SetParent<FogApplicationEngine> ()
        .AddConstructor<CEPEngine> ()
        
        ;
        
        return tid;
    }
    
    
    CEPEngine::CEPEngine()
    {
    }

    void CEPEngine::parseSql(std::string query)
    {
    }

    void
    CEPEngine::DeployQuery(const Ptr<StreamQuery>& query) {
      auto new_query = StreamQuery::buildQuery(query->vertices_yaml, query->edges_yaml, query->id, query->unique_query_id);
      NS_LOG_INFO("Deploying query:\n" << query);
      //parseSql(query);
      query->node = GetObject<Dcep>()->GetNode();
      /*for (auto &[stream_id, operatorIdToOperator] : query->streamIdToOperatorIdToOperator) {
          for (auto &[operator_id, op] : operatorIdToOperator) {
              op->SetNodeId(this->node_id);
          }
      }*/
    }

    void
    CEPEngine::AddSchemas(YAML::Node stream_schemas) {
      NS_LOG_INFO(Simulator::Now() << ": AddSchemas");
      for (YAML::iterator it = stream_schemas.begin(); it != stream_schemas.end(); ++it) {
        const YAML::Node& stream_schema = *it;
        std::cout << stream_schema["stream-id"] << " - " << stream_schema["name"] << ", ";
        stream_id_to_schema[stream_schema["stream-id"].as<int>()] = stream_schema;
        int stream_id = stream_schema["stream-id"].as<int>();
        auto stream_name = stream_schema["name"].as<std::string>();
        stream_id_to_name[stream_id] = stream_schema["name"].as<std::string>();
        stream_name_to_id[stream_schema["name"].as<std::string>()] = stream_schema["stream-id"].as<int>();
        // TODO: Add schema to global stream processing schema streamIdToFieldToType and streamNameToId
        Tuple::streamNameToId[stream_name] = stream_id;
        auto &fieldToType = Tuple::streamIdToFieldToType[stream_id];
        int i = 0;
        for (auto attribute : stream_schema["tuple-format"].as<std::vector<YAML::Node>>()) {
            auto field = attribute["name"].as<std::string>();
            auto type = attribute["type"].as<std::string>();
            Tuple::streamIdToIndexToField[stream_id][i++] = field;
            if (type == "int") {
                fieldToType[field] = LONG;
            } else if (type == "long") {
                fieldToType[field] = LONG;
            } else if (type == "long-timestamp") {
                fieldToType[field] = LONG;
            } else if (type == "timestamp") {
                fieldToType[field] = TIMESTAMP;
            } else if (type == "double") {
                fieldToType[field] = DOUBLE;
            } else if (type == "float") {
                fieldToType[field] = DOUBLE;
            } else if (type == "string") {
                fieldToType[field] = STRING;
            }
        }
      }
      std::cout << "so that they can be handled" << std::endl;
    }
    
    void
    CEPEngine::Configure()
    {
        //GetObject<Forwarder>()->TraceConnectWithoutContext("new event",
        //        MakeCallback(&CEPEngine::ForwardProducedCepEvent, this));
    }

    int immediately_forwarded_cnt = 0;
    int cnt = 0;
    void
    CEPEngine::ProcessCepEvent(Ptr<TupleWrapper> tupleWrapper)
    {
        auto dcep = GetObject<Dcep>();
        auto node = dcep->GetNode();
        auto str_node_id = StreamQuery::ns3NodeIdToExposeNodeId[node->GetId()];
        int nid = std::stoi(str_node_id);
        auto wrapper = ExposeCoordinator::nodeIdsToWrappers[str_node_id];
        tupleWrapper->node = node;
        last_received_tuple = Simulator::Now().GetMilliSeconds();
        ++number_tuples_received;

        for (auto &[query_id, nodeIdToQuery] : ExposeCoordinator::queryIdToNodeIdToQuery) {
            auto query = nodeIdToQuery[nid];
            if (!query) {
                continue;
            }
            auto next_op = query->streamIdToOperatorIdToOperator[tupleWrapper->tuples[0]->stream_id][tupleWrapper->next_operator_id];
            if (next_op != nullptr) {
                int part = 0;
                if (tupleWrapper->group != nullptr) {
                    part = (int) tupleWrapper->group->partition;
                }
                next_op = next_op->stream->operators[part%next_op->stream->operators.size()];
                if (Metrics::forwarded_tuples.count(tupleWrapper->tuples[0]->id) != 0) {
                    Metrics::forwarded_tuples_input_latency += Simulator::Now() - Metrics::forwarded_tuples[tupleWrapper->tuples[0]->id];
                    Metrics::forwarded_tuple_input_latency[tupleWrapper->tuples[0]->id][next_op->id] = Simulator::Now() - Metrics::forwarded_tuples[tupleWrapper->tuples[0]->id];
                    Metrics::forwarded_tuples.erase(tupleWrapper->tuples[0]->id);
                }

                if (!next_op->ProcessOnHost(nid, tupleWrapper) && !next_op->source_sinkable) {
                    // We will forward this to next node, and therefore, trace it
                    //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Next op " << next_op->id << " with type " << next_op->type << " is not on this node, forwarding tuple " << tupleWrapper->tuples[0]->id << " to " << next_op->node_id);
                    Metrics::forwarded_tuples[tupleWrapper->tuples[0]->id] = Simulator::Now();
                }
                next_op->EmitToNextOp(next_op, tupleWrapper, true);
            }
        }
    }

    void CEPEngine::SetNodeIdToTuplesAddress(std::map<int, std::pair<Ipv4Address, int>> nodeIdToIpAndPort) {
        this->nodeIdToTuplesIpAndPort = nodeIdToIpAndPort;
    }

    void CEPEngine::SetNodeIdToMigrationAddress(std::map<int, std::pair<Ipv4Address, int>> nodeIdToIpAndPort) {
        this->nodeIdToMigrationIpAndPort = nodeIdToIpAndPort;
    }

    std::map<int, std::pair<Ipv4Address, int>>
    CEPEngine::GetNodeIdToTuplesAddress() {
        return nodeIdToTuplesIpAndPort;
    }

    std::map<int, std::pair<Ipv4Address, int>>
    CEPEngine::GetNodeIdToMigrationAddress() {
        return nodeIdToMigrationIpAndPort;
    }

    void
    CEPEngine::SetNodeId(int nid) {
        this->node_id = nid;
    }

    void
    CEPEngine::AddNextHop(int stream_id, int nid)
    {
        streamIdToNodes[stream_id].emplace_back(nid);
    }


    std::map<int, std::vector<int>>
    CEPEngine::GetStreamIdToNodes() { return streamIdToNodes; }
}
