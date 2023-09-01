//
// Created by espen on 16.11.2021.
//

#ifndef STREAMPROCESSOR_QUERY_H
#define STREAMPROCESSOR_QUERY_H

#include <map>
#include <mutex>
#include <condition_variable>
#include "ns3/object.h"
#include "ns3/Operator.h"
#include "ns3/node.h"

namespace ns3 {
    class Operator;
    class Stream;
    class TupleWrapper;
    class PartialStateWrapper;

    class StreamQuery : public Object {
    public:
        std::mutex mtx;
        std::condition_variable cv;
        bool started = false;
        Ptr<Node> node;
        Callback<void, Ptr<TupleWrapper>> outputCallback;
        inline static std::map<int, std::string> ns3NodeIdToExposeNodeId;
        inline static std::map<int, Ptr<Node>> nodeIdToNode;
        std::map<int, std::map<std::string, bool>> streamIdToFieldToEssential;
        std::map<int, std::map<int, Ptr<Operator>>> streamIdToOperatorIdToOperator;
        std::vector<int> sink_nodes;
        std::vector<int> source_nodes;
        std::vector<int> potential_hosts;
        int id;
        int unique_query_id;
        int operator_id_cnt = 1;
        Time current_time = Time(0);
        bool shed = false;
        TimeType time_type = PROCESSING_TIME_TYPE;
        std::vector<YAML::Node> vertices_yaml;
        std::vector<YAML::Node> edges_yaml;
        std::map<std::string, int> aliasToStreamId;
        std::map<std::string, std::set<std::string>> aliasToEssentialAttributes;
        std::map<std::string, std::set<std::string>> aliasToUnssentialAttributes;
        std::map<std::string, std::map<int, std::string>> aliasToIndexToField;

        StreamQuery(int id, int unique_query_id) {this->id = id; this->unique_query_id = unique_query_id;}
        StreamQuery() {}

        void setTime(Ptr<TupleWrapper> tupleWrapper);

        void setFieldIndexes();

        Ptr<StreamQuery> copy();

        void addStream(Ptr<Stream> stream);

        std::vector<Ptr<PartialStateWrapper>> GetEssentialStates(std::string alias);

        void AddEssentialParameter(std::string alias, std::string field);

        void AddUnessentialParameter(std::string alias, std::string field);

        static Ptr<StreamQuery> buildQuery(std::vector<YAML::Node> vertices_yaml, std::vector<YAML::Node> edges_yaml, int query_index, int unique_query_id);

        static TypeId GetTypeId(void);
    };
}

#endif //STREAMPROCESSOR_QUERY_H
