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

#ifndef CEP_ENGINE_H
#define CEP_ENGINE_H

#include "ns3/object.h"
#include "ns3/traced-callback.h"
#include "ns3/ipv4-address.h"
#include "ns3/event-impl.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/Tuple.h"
#include "ns3/Query.h"
#include <map>
#include <yaml-cpp/yaml.h>

namespace ns3 {

    class CepEvent;
    class CepEventPattern;
    class CepOperator;
    class SerializedCepEvent;
    class SerializedQuery;
    class Producer;
    class CEPEngine;
    class FogApplicationComponent;
    class Window;
    class StreamQuery;

    class FogApplicationEngine : public Object
    {
    public:
        static TypeId GetTypeId ();
    };

    class CEPEngine : public FogApplicationEngine
        {
        public:
            static TypeId GetTypeId (void);
            CEPEngine();
            void Configure();
            void ProcessCepEvent(Ptr<TupleWrapper> e);
            void AddSchemas(YAML::Node stream_schemas);
            void DeployQuery(const Ptr<StreamQuery>& query);
            void parseSql(std::string query);
            void AddNextHop(int stream_id, int node_id);
            std::map<int, std::vector<int>> GetStreamIdToNodes();
            std::map<int, std::pair<Ipv4Address, int>> GetNodeIdToTuplesAddress();
            void SetNodeIdToTuplesAddress(std::map<int, std::pair<Ipv4Address, int>> nodeIdToTuplesIpAndPort);
            std::map<int, std::pair<Ipv4Address, int>> GetNodeIdToMigrationAddress();
            void SetNodeIdToMigrationAddress(std::map<int, std::pair<Ipv4Address, int>> nodeIdToMigrationIpAndPort);
            void SetNodeId(int node_id);

            long last_received_tuple = -1;
            long number_tuples_received = 0;
            inline static std::map<int, std::string> stream_id_to_name = {};
            inline static std::map<std::string, int> stream_name_to_id = {};


    private:
        std::map<int, YAML::Node> stream_id_to_schema;
        //std::map<int, std::vector<InsertIntoClause *>> stream_id_to_queries;
        std::map<int, std::vector<int>> streamIdToNodes;
        std::map<int, std::pair<Ipv4Address, int>> nodeIdToTuplesIpAndPort;
        std::map<int, std::pair<Ipv4Address, int>> nodeIdToMigrationIpAndPort;
        int node_id;

        };
}

#endif /* CEP_ENGINE_H */

