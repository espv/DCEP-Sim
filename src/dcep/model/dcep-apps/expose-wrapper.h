
#ifndef NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_EXPOSE_WRAPPER_H
#define NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_EXPOSE_WRAPPER_H

#include <iostream>
#include <yaml-cpp/yaml.h>
#include "ns3/type-id.h"
#include "ns3/object.h"
#include "ns3/stream-processing-module.h"
#include "ns3/inet-socket-address.h"
#include "ns3/dcep-module.h"
#include "ns3/dcep.h"
#include "ns3/Tuple.h"
#include "ns3/Operator.h"
#include <yaml-cpp/yaml.h>

namespace ns3 {

    class TaskNode;
    class ExposeCoordinator;
    class Tuple;
    class PartialStateBatch;
    class PartialStateWrapper;
    class PartialState;
    class MigrationPlan;
    class CEPEngine;

    class TupleProcessWrapper : public Object {
    public:
        static TypeId GetTypeId (void);
        void ProcessEvent(Ptr<TupleWrapper> tupleWrapper, Ptr<CEPEngine> cepEngine, Ptr<ExposeCoordinator> coordinator);

    private:
        int number_sent = 0;
    };

    class SendTupleCallback {
    public:
        Ptr<Operator> emitting_operator;
        Ptr<TupleWrapper> emitted_tuple;
        bool being_sent = false;

        SendTupleCallback(const Ptr<Operator>& emitting_operator, const Ptr<TupleWrapper>& emitted_tuple) {
            this->emitting_operator = emitting_operator;
            this->emitted_tuple = emitted_tuple;
        }
    };

    class ExposeWrapper : public Object {
    public:

        static TypeId GetTypeId (void);

        void Configure();
        void SetTupleBatchSize(int size);
        void SetIntervalBetweenTuples(int interval);
        void AddTuples(const YAML::Node &tuple, int quantity);
        void AddDataset(const YAML::Node &ds);
        void LoadDataset(const YAML::Node& ds);
        void LoadDataset(const YAML::Node& ds, int number_rows);
        void doLoadDataset(const YAML::Node& ds, int number_rows);
        void SendDsAsSpecificStream(YAML::Node& ds, int stream_id);
        void SendNRowsDsAsSpecificStream(YAML::Node& ds, int stream_id, int number_rows);
        void SendDsAsStream(YAML::Node &ds);
        void AddSchemas(YAML::Node stream_schema);
        void DeployQuery(const Ptr<StreamQuery>& query, int unique_query_id);
        void RegisterSource(int stream_id, int unique_query_id);
        void RegisterSink(int stream_id, int unique_query_id);
        void AddNextHop(int stream_id, int nodeId);
        void AddNextHopQuery(std::vector<int> nodeIds, Ptr<Operator> op);
        void RemoveNextHop(std::vector<int> nodeIds, Ptr<Operator> op);
        void SetNidToMigrationAddress(std::map<int, std::pair<Ipv4Address, int>>);
        void SetNidToTuplesAddress(std::map<int, std::pair<Ipv4Address, int>>);
        void SetTupleDataRate(long datarate);
        void SetMigrationDataRate(long datarate);
        void SendTupleTo(Ptr<TupleWrapper> tupleWrapper, int node_id);
        void PrepareSendTupleToNextHops();
        void ProcessTuples(int number_tuples, int stream_id);
        Time Migrate(std::map<int, int> operatorIdToNewHost, std::map<int, std::vector<int>> priority_to_operator_ids, int query_id, std::map<int, std::string> operatorIdToAlgorithm, Time deadline, std::string migration_mode);
        void ClearQueries();
        void StartRuntimeEnv();
        void StopRuntimeEnv();
        void RetEndOfStream(int milliseconds);
        void RetSinkEndOfStream(int query_id, int milliseconds);
        void RetWhenReceived(int number_tuples);
        void RetWhenProcessed(int number_tuples, int query_id, std::string operator_type);
        void FetchNextPartialState();
        void AddTpIds(std::vector<int> tracepointIds);
        void EndExperiment();
        void TraceTuple(int tracepointId, std::vector <std::string> traceArguments);
        void FinishTask();
        void FinalizeWindowRecreation(Ptr<Operator> op);
        void DisconnectNode(int ms_before_disconnect);
        void MigrateStateBeforeDisconnect(int ms_before_migration);
        void DoDisconnectNode();
        void MoveNextState();

        Ptr<ExposeCoordinator> coordinator;
        std::map<int, YAML::Node> stream_id_to_schema;
        //std::map<int, std::vector<YAML::Node>> dataset_to_tuples;
        static inline std::map<int, std::vector<Ptr<TupleWrapper>>> dataset_to_tuples;
        std::vector<Ptr<Packet>> packets;
        std::string node_id;
        std::string log_prefix;
        TaskNode *curTask;
        Ptr<Node> node;
        std::deque<SendTupleCallback> sendTupleCallbacks;
        std::map<int, std::map<int, Ptr<PartialStateBatch>>> nodeToMigrationPayloads;
        std::map<int, std::deque<std::vector<Ptr<PartialStateWrapper>>>> nodeToMigrationFetchRequests;
        Ptr<MigrationPlan> mp;
        std::deque<Ptr<PartialStateWrapper>> ps_being_fetched;
        std::deque<std::pair<int, std::deque<Ptr<PartialStateWrapper>>>> ps_fetch_queue;
    };

}

#endif //NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_EXPOSE_WRAPPER_H
