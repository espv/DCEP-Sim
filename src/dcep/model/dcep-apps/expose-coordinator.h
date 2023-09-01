
#ifndef NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_EXPOSE_COORDINATOR_H
#define NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_EXPOSE_COORDINATOR_H

#include "ns3/object.h"
#include "ns3/type-id.h"
#include "ns3/dcep-module.h"
#include "ns3/inet-socket-address.h"
#include "ns3/Query.h"
#include "ns3/data-rate.h"
#include <yaml-cpp/yaml.h>

namespace ns3 {
    class TaskNode {
    public:
        TaskNode() = default;
        TaskNode(int task_id, std::string task);
        virtual ~TaskNode();

        static void SetupTasks(std::vector<YAML::Node> cmds, TaskNode *parent);

        YAML::Node GetYamlNode();

        TaskNode *GetRoot();
        TaskNode *GetParent();
        TaskNode *GetNext();
        TaskNode *GetPrevious();
        void DecrIterations();
        bool IsParallel();
        std::vector<TaskNode *> GetChildren();
        int GetIterations();
        void SetIterations(int iterations);
        void SetRoot(TaskNode *);

    private:
        int iterations;
        int task_id;
        std::string task;
        TaskNode *root;
        TaskNode *parent;
        TaskNode *next;
        TaskNode *previous;
        std::vector<TaskNode *> children;
        YAML::Node yamlNode;
        bool executed = false;
    };

    class ExposeCoordinator : public Object {
    public:
        static TypeId GetTypeId(void);

        ExposeCoordinator(std::string configFilePath, std::string traceFileFolder);
        ExposeCoordinator(std::string configFilePath, std::string traceFileFolder, std::map<std::string, std::string> string_variables, std::map<std::string, double> numeric_variables);
        ExposeCoordinator() = default;
        void StartApplication();
        void doHandleEvent(std::string cmd, YAML::Node args, /*std::vector<Object> args,*/ std::string node_id, bool parallel);
        void handleEvent(YAML::Node event);
        Time InterpretEvents(int eid);
        void Configure();
        void SetupNodes(std::vector<YAML::Node> cmds, std::map<std::string, YAML::Node> nodeIdsToClients);
        void SetupTopology();
        void HandleTask();
        void FinishedTask();
        static void Adapt(int query_id, bool allow_parallel_track, int from_host, int to_host, int migration_deadline, std::string migration_mode);
        void DeployQuery(int query_id, int unique_query_id);
        void AddPotentialHost(const std::vector<int>& potential_hosts, int unique_query_id);
        void AddSinkNode(const std::vector<int>& sink_nodes, int unique_query_id);
        void AddSourceNode(const std::vector<int>& source_nodes, int unique_query_id);
        void Trace(std::string plot_name, std::string event, int stream_id, int query_id, std::string operator_name);
        void EnableTrace();
        void DisableTrace();
        void MigrationCheck();
        void StartSimulation();
        void EndExperiment();

        static void UpdateDeployedQueries();
        static void InitialPlacement(Ptr<StreamQuery>);
        static void UpdateOperatorPlacements();
        static void ResetTupleCount();

        inline static std::map<int, std::map<int, Ptr<StreamQuery>>> queryIdToNodeIdToQuery;
        inline static std::map<int, Ptr<StreamQuery>> queryIdToBaseQuery;
        inline static std::map<std::string, Ptr<ExposeWrapper> > nodeIdsToWrappers;
        inline static std::map<int, std::pair<Ipv4Address, int>> nodeIdToMigrationIpAndPort;
        inline static std::map<int, std::pair<Ipv4Address, int>> nodeIdToFetchIpAndPort;
        inline static std::map<int, std::pair<Ipv4Address, int>> nodeIdToTuplesIpAndPort;
        inline static Ptr<Node> router_node;
        inline static NodeContainer router_container;
        inline static int experiment_id;
        inline static unsigned long BATCH_SIZE = 1000;
        inline static DataRate bw_tuples;
        inline static DataRate bw_migration;
        inline static std::map<std::string, std::string> string_variables;
        inline static std::map<std::string, double> numeric_variables;
        std::map<int, std::string> dataset_manipulation;
        YAML::Node yaml_configuration;
        std::vector<std::string> node_ids;
    private:
        bool available = true;
        //ExperimentAPI experimentAPI;
        std::string trace_output_folder;
        std::map<std::string, bool> nodeIdReady;
        std::map<std::string, YAML::Node> nodeIdsToClients;
        std::vector<YAML::Node> cmds;
        TaskNode *curTask;
        TaskNode *rootTaskNode;
        bool experiment_finished = false;
        //std::map<int, std::map<std::string, std::string>> streamIdToFieldToType;
    };
}

#endif //NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_EXPOSE_COORDINATOR_H
