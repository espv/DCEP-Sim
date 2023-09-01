
#ifndef EP_SIM_OPERATOR_H
#define EP_SIM_OPERATOR_H

#include <boost/regex.hpp>
#include "ns3/Tuple.h"
#include <iostream>
#include "ns3/stream-processing-common.h"
#include "ns3/dcep-module.h"
#include "ns3/Metrics.h"
#include "ns3/CpuCore.h"
#include "ns3/communication.h"
#include "ns3/object.h"
#include <chrono>
#include <deque>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <unordered_map>
#include <map>
#include <set>
#include <climits>
#include <queue>
#include "ns3/random.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>


namespace ns3 {
    class Stream;
    class Tuple;
    class TupleWrapper;
    class FilterOperator;
    class FilterArgument;
    class PrintOperator;
    class JoinOperator;
    class GroupOperator;
    class JoinFilterOperator;
    class SelectOperator;
    class SelectField;
    class DefineOperator;
    class WithinOperator;
    class CepPatternOperator;
    class SinkOperator;
    class WindowOperator;
    class AggregationWindowOperator;
    class StorageWindowOperator;
    class Grouper;
    class Group;
    class LongGrouper;
    class DoubleGrouper;
    class StringGrouper;
    class Partitioner;
    class LongPartitioner;
    class DoublePartitioner;
    class StringPartitioner;
    class WindowExtentContainer;
    class AggregateSelectField;
    class AggregationWindowExtent;
    class CpuCore;
    class Random;
    class SendTupleCallback;
    class PartialState;
    class PartialStateBatch;
    class DeadlinePartialStateBatch;
    class PartialStateWrapper;
    class MigrationMechanism;

    class PartialStateWrapper : public Object {
    public:
        static TypeId GetTypeId();

        PartialStateWrapper(Ptr<PartialState> object, int size, Ptr<Operator> op);
        PartialStateWrapper(Ptr<PartialState> object, int size);
        PartialStateWrapper(Ptr<PartialState> object);
        PartialStateWrapper();

        Ptr<PartialState> object;
        Ptr<Operator> op;
        bool disable_state = true;
        // We assign size because depending on the phase of the operator, the size will be different,
        // even though the underlying object is the same. Example: join with crit and fetch state.
        size_t size = 0;
    };

    class PartialState : public Object {
    public:
        static TypeId GetTypeId();

        int group_id = 0;
        bool in_migration = false;
        bool needs_shedding = false;
        int current_migration_phase = 0;

        bool imported_critical_attributes = true;
        bool imported_essential_attributes = true;
        bool imported_fully = true;

        virtual size_t GetSerializedSize() {return 1;}
        virtual size_t GetUtility() {return 1;}
    };

    class Operator : public PartialState {
    public:
        static TypeId GetTypeId(void);

        ~Operator();

        std::vector<Ptr<Operator>> nextOperator;
        std::string name;
        Ptr<StreamQuery> query;
        Ptr<Stream> stream;
        OperatorType type;
        std::thread process_running;
        mutable std::mutex rcv_mtx;
        std::mutex mtx;
        std::condition_variable cv;
        int partition = 0;
        Ptr<WindowOperator> windowOperator;
        bool forked = false;
        Time emit_time_delta;
        Time time_for_next_forward;
        Ptr<CpuCore> thread;
        int parallel_old_node_id = -1;
        int old_node_id = -1;
        int node_id = -1;
        Ptr<Operator> previous_operator;
        inline static Random *random;
        int id;
        static inline int id_cnt = 1;
        bool source_sinkable = false;
        bool stateful = false;
        bool source = false;
        bool sink = false;
        std::vector<std::string> algorithms;
        bool active = true;
        bool shutting_down = false;
        bool in_migration = false;
        int tuples_processed = 0;
        Ptr<PartialStateBatch> first_rhino_batch = nullptr;
        inline static std::map<int, std::map<int, std::map<int, bool>>> queryIdToOperatorIdToGroupIdToNewHost = {};
        inline static std::map<std::string, OperatorType> operatorNameToType = {{"filter", filterType},
                                                                                {"cep", cepType},
                                                                                {"define", defineType},
                                                                                {"groupType", groupType},
                                                                                {"joinType", joinType},
                                                                                {"passthrough", passthroughType},
                                                                                {"input", inputType},
                                                                                {"print", printType},
                                                                                {"select", selectType},
                                                                                {"sink", sinkType},
                                                                                {"window", windowType},
                                                                                {"forward", forwardType},
                                                                                {"within", withinType}
        };

        Operator(OperatorType type);

        Operator(OperatorType type, int node_id) : Operator(type) {
            this->SetNodeId(node_id);
        }

        void SetNodeId(int new_node_id);
        void SetCpuCore(Ptr<CpuCore> cpu_core);

        virtual std::vector<Ptr<PartialState>> getPartialStates() {return {};}

        virtual void process(Ptr<TupleWrapper> tupleWrapper) = 0;

        virtual void emit(Ptr<TupleWrapper> tupleWrapper);

        virtual void EmitToNextOp(Ptr<Operator> nextOp, Ptr<TupleWrapper> tupleWrapper, bool add_processing_delay);

        virtual void MigrationInterrupted() {};

        virtual size_t calculateUtility(int group_id, int partial_state_index) {
            // This generates a "fake" utility
            return std::hash<std::string>{}(std::to_string(partial_state_index)) % 100;
        }

        virtual Time get_base_processing_time();
        virtual Time get_emit_time_added();
        virtual Time get_forward_time_added();

        virtual Ptr<Operator> copy(Ptr<Stream> s) = 0;
        Ptr<Operator> copy(Ptr<Operator> new_operator, Ptr<Stream> s);
        virtual int calculateMigrationSize(migration_types migration_type) = 0;
        virtual void ExtractState(Ptr<MigrationPlan> mp) = 0;
        virtual Ptr<PartialStateBatch> ExtractPreambleState(Ptr<MigrationPlan> mp);
        virtual void ExtractDiffState(Ptr<MigrationPlan> mp) {};
        virtual void DisableState(Ptr<PartialStateBatch> batch) {};
        virtual void ClearState() {};

        virtual void ImportState(Ptr<PartialStateBatch> batch) = 0;
        size_t GetSerializedSize() override;
        size_t GetUtility() override;
        bool ProcessOnHost(int host, Ptr<TupleWrapper> tuple_wrapper);
        void FetchPartialState(Ptr<PartialState> ps);
        Ptr<ExposeWrapper> getExposeWrapper();
        void SendToNextOp(Ptr<TupleWrapper> tupleWrapper, Ptr<Operator> next_op, bool add_processing_delay);

        void UnassignCpuCore();
        virtual void ForwardTuplesTo(int host);

        virtual void SetActive();
        void SetInactive();

        [[noreturn]] void process_tuples();
    };

    class Stream : public Object {
    public:
        Stream() {}

        Stream(std::vector<int> output_stream_ids) {
            this->output_stream_ids = output_stream_ids;
        }

        Stream(std::vector<int> output_stream_ids, int node_id, Ptr<StreamQuery> q) {
            this->output_stream_ids = output_stream_ids;
            this->node_id = node_id;
            this->query = q;
        }

        static TypeId GetTypeId(void);

        std::vector<int> output_stream_ids;
        std::unordered_map<int, int> output_stream_id_to_tuple_position;
        std::vector<int> input_stream_ids;

        Ptr<Stream> next_stream;
        Ptr<Stream> previous_stream;
        std::vector<Ptr<Operator> > operators;

        int node_id = -1;
        Ptr<StreamQuery> query;

        Ptr<Stream> AddOperator(Ptr<Operator> op, const Ptr<Stream> &new_stream);
        Ptr<Stream> filter(Ptr<FilterArgument>fa1, Op op, Ptr<FilterArgument> fa2);
        Ptr<Stream> group(std::vector <std::string> groups);
        Ptr<Stream> group();
        Ptr<Stream> join(Ptr<Stream> joiningStream);
        Ptr<Stream> sendTo(int node_id);
        Ptr<Stream> print();
        Ptr<Stream> passthrough();
        Ptr<Stream> input(std::string alias);
        Ptr<Stream> sink();
        Ptr<Stream> cep(std::string pattern, bool multiple_sequences, bool emit_all_tuples);
        Ptr<Stream> define(std::string alias, Ptr<FilterArgument> fa1, Op op, Ptr<FilterArgument> fa2);
        Ptr<Stream> define(std::string alias);
        Ptr<Stream> within(long ms, std::string timestamp_attribute);
        Ptr<Stream> window(int size, int jump, WindowType size_type, int emit_size, WindowType emit_type,
                           const std::string &external_timestamp_key);
        Ptr<Stream> window(int size, int jump, WindowType size_type, WindowType emit_type,
                           const std::string &external_timestamp_key);
        Ptr<Stream> select(std::string select_stmt, int output_stream_id);
        std::vector<int> getOutputStreamId() { return output_stream_ids; }
        std::vector<int> getInputStreamId() { return input_stream_ids; }
    };

    class ForwardOperator : public Operator {
    public:
        ForwardOperator() : Operator(forwardType) {}

        static TypeId GetTypeId(void);

        void process(Ptr<TupleWrapper> tupleWrapper) override;

        Ptr<Operator> copy(Ptr<Stream> s) override;

        int calculateMigrationSize(migration_types migration_type) override;
        void ExtractState(Ptr<MigrationPlan> mp) override;
        void ImportState(Ptr<PartialStateBatch> batch) override;
    };

    class JoinOperator : public Operator {
    public:
        explicit JoinOperator(Ptr<Stream> joining_stream);

        Ptr<Operator> copy(Ptr<Stream> s) override;

        static TypeId GetTypeId(void);

        void updateWindows(long external_timestamp) const;

        void updateWindow1(long external_timestamp) const;

        void updateWindow2(long external_timestamp) const;

        void InsertPendingMatch(std::pair<Ptr<TupleWrapper>, Ptr<TupleWrapper>> pending_match);

        void MigrationInterrupted() override;

        void notifyEviction();

        void ForwardTuplesTo(int host) override;

        std::vector<Ptr<JoinFilterOperator>> filterOperators;

        Ptr<StorageWindowOperator> windowOperator1{};
        Ptr<StorageWindowOperator> windowOperator2{};
        std::vector<std::pair<Ptr<PartialState>, Ptr<PartialState>>> pending_matches;

        int join_depth = 1;

        inline static int number_emitted = 0;

        Ptr<Stream> stream2;

        void process(Ptr<TupleWrapper> tupleWrapper) override;

        void DisableState(Ptr<PartialStateBatch> batch) override;
        void ClearState() override;

        std::vector<Ptr<PartialState>> getPartialStates() override;

        int calculateMigrationSize(migration_types migration_type) override;
        void ExtractState(Ptr<MigrationPlan> mp) override;
        Ptr<PartialStateBatch> ExtractPreambleState(Ptr<MigrationPlan> mp) override;
        void ExtractDiffState(Ptr<MigrationPlan> mp) override;
        void ExtractLazyState(Ptr<MigrationPlan> mp);
        void ExtractUtilityState(Ptr<MigrationPlan> mp);
        void ExtractEssentialState(Ptr<MigrationPlan> mp);
        void ExtractUnessentialState(Ptr<MigrationPlan> mp);
        void ImportState(Ptr<PartialStateBatch> batch) override;
    };

    class Group : public PartialState {
    public:
        static TypeId GetTypeId();

        Ptr<TupleWrapper> first_tuple_wrapper;

        // This is used as part of the merging of Group by and Partition by
        // internal groups represent the selected attributes
        std::vector<Ptr<Group> > internal_groups;

        int partition;
        int id;
        inline static int group_cnt = 0;

        inline static Ptr<Group> no_group = CreateObject<Group>();

        std::vector <std::string> string_values;
        std::vector<long> long_values;
        std::vector<double> double_values;
        std::string key;
        std::vector<Ptr<TupleWrapper>> tuples;

        Ptr<GroupOperator> groupOperator{};

        Group();

        Group(int id, int partition);

        Ptr<Group> copy(Ptr<GroupOperator> groupOperator, bool shed_unessential_attributes);

        size_t GetSerializedSize() override;
    };

    class GroupOperator : public Operator {

    public:
        std::vector<Ptr<Group>> groups;
        std::vector<Ptr<Group>> partitions;

        Ptr<Grouper> first_grouper;
        Ptr<Partitioner> first_partitioner;

        std::vector<int> depth_indexes;
        std::vector<int> partition_depth_indexes;

        std::vector <std::string> group_keys;
        std::vector <std::string> partition_keys;
        std::vector <Type> group_types;
        std::vector <Type> partition_types;
        std::vector<int> tuple_indexes;
        std::vector<int> partition_tuple_indexes;
        bool single_group = false;
        std::map<int, Ptr<Group>> group_map;

        GroupOperator() : Operator(groupType) {}

        explicit GroupOperator(std::vector <std::string> group_keys);

        Ptr<Operator> copy(Ptr<Stream> s) override {
            auto groupOperator = CreateObject<GroupOperator>(group_keys);
            Operator::copy(groupOperator, s);
            return groupOperator;
        }

        static TypeId GetTypeId(void);

        void init();

        void init_partitions(Ptr<SelectOperator> select_operator);

        Ptr<Group> GetGroup(int group_id);

        void process(Ptr<TupleWrapper> tupleWrapper) override;

        void process(const std::vector<Ptr<Tuple> > &tuple);

        int calculateMigrationSize(migration_types migration_type) override;
        Ptr<PartialStateBatch> ExtractPreambleState(Ptr<MigrationPlan> mp) override;
        void ExtractState(Ptr<MigrationPlan> mp) override;
        void ImportState(Ptr<PartialStateBatch> batch) override;
        void doExtractState(Ptr<MigrationPlan> mp);
        void ExtractDiffState(Ptr<MigrationPlan> mp) override;
        Time get_base_processing_time() override;
        Time get_emit_time_added() override;
        Time get_forward_time_added() override;
    };

    class Grouper : public PartialState {
    public:
        Ptr<GroupOperator> groupOperator = nullptr;
        Ptr<Group> group = Group::no_group;
        int depth = 0;
        int max_depth = 0;
        long long_value;
        double double_value;
        std::string string_value;
        Ptr<Partitioner> first_partitioner;

        Grouper(Ptr<GroupOperator> groupOperator, int depth, Ptr<Grouper> previous_grouper, long long_value);
        Grouper(Ptr<GroupOperator> groupOperator, int depth, Ptr<Grouper> previous_grouper, double double_value);
        Grouper(Ptr<GroupOperator> groupOperator, int depth, Ptr<Grouper> previous_grouper, std::string string_value);

        static TypeId GetTypeId();

        virtual void groupTuple(Ptr<TupleWrapper> t) = 0;
        void partitionTuple(Ptr<TupleWrapper> t);
        virtual Ptr<Grouper> copy(Ptr<GroupOperator> group_operator, bool shed_unessential_attributes) {return this;}
        virtual Ptr<Group> GetGroup(int group_id) {return nullptr;}

        size_t GetSerializedSize() override;
        size_t GetUtility() override;
    };

    class LongGrouper : public Grouper {
        std::unordered_map<long, Ptr<Grouper> > longToGrouper;
    public:
        LongGrouper(Ptr<GroupOperator> groupOperator, int depth, Ptr<Grouper> previous_grouper, long long_value) : Grouper(groupOperator, depth, previous_grouper, long_value) {}
        LongGrouper(Ptr<GroupOperator> groupOperator, int depth, Ptr<Grouper> previous_grouper, double double_value) : Grouper(groupOperator, depth, previous_grouper, double_value) {}
        LongGrouper(Ptr<GroupOperator> groupOperator, int depth, Ptr<Grouper> previous_grouper, std::string string_value) : Grouper(groupOperator, depth, previous_grouper, string_value) {}

        LongGrouper(Ptr<GroupOperator> groupOperator, int depth) : Grouper(groupOperator, depth, nullptr, 0L) {}

        static TypeId GetTypeId();

        void groupTuple(Ptr<TupleWrapper> t) override;
        Ptr<Grouper> copy(Ptr<GroupOperator> group_operator, bool shed_unessential_attributes) override;
        Ptr<Group> GetGroup(int group_id) override;
    };

    class NoGrouper : public Grouper {
    public:
        static TypeId GetTypeId();

        NoGrouper(Ptr<GroupOperator> groupOperator, int depth) : Grouper(groupOperator, depth, nullptr, 0L) {}

        void groupTuple(Ptr<TupleWrapper> t) override;
    };

    class Partitioner : public PartialState {
    public:
        Ptr<GroupOperator> groupOperator = nullptr;
        Ptr<Group> group = Group::no_group;
        int depth = 0;
        int max_depth = 0;
        long long_value;
        double double_value;
        std::string string_value;

        Partitioner(Ptr<GroupOperator> groupOperator, int depth, long long_value);
        Partitioner(Ptr<GroupOperator> groupOperator, int depth, double double_value);
        Partitioner(Ptr<GroupOperator> groupOperator, int depth, std::string string_value);

        static TypeId GetTypeId();

        virtual void partitionTuple(Ptr<TupleWrapper> t) = 0;

        size_t GetSerializedSize() override;
        size_t GetUtility() override;
        virtual Ptr<Partitioner> copy(Ptr<GroupOperator> group_operator, bool shed_unessential_attributes) {return this;}
        virtual Ptr<Group> GetGroup(int group_id) {return nullptr;}
    };

    class LongPartitioner : public Partitioner {
        std::unordered_map<long, Ptr<Partitioner> > longToPartitioner;
    public:
        LongPartitioner(Ptr<Partitioner> partitioner) : Partitioner(partitioner->groupOperator, partitioner->depth, partitioner->long_value) {}
        LongPartitioner(Ptr<GroupOperator> groupOperator, int depth, long long_value) : Partitioner(groupOperator, depth, long_value) {}
        LongPartitioner(Ptr<GroupOperator> groupOperator, int depth, double double_value) : Partitioner(groupOperator, depth, double_value) {}
        LongPartitioner(Ptr<GroupOperator> groupOperator, int depth, std::string string_value) : Partitioner(groupOperator, depth, string_value) {}

        LongPartitioner(Ptr<GroupOperator> groupOperator, int depth) : Partitioner(groupOperator, depth, 0L) {}

        static TypeId GetTypeId();

        void partitionTuple(Ptr<TupleWrapper> t) override;
        Ptr<Partitioner> copy(Ptr<GroupOperator> group_operator, bool shed_unessential_attributes) override;
        Ptr<Group> GetGroup(int group_id) override;
    };

    class DoubleGrouper : public Grouper {
        std::unordered_map<double, Ptr<Grouper> > doubleToGrouper;
    public:
        DoubleGrouper(Ptr<GroupOperator> groupOperator, int depth, Ptr<Grouper> previous_grouper, long long_value) : Grouper(groupOperator, depth, previous_grouper, long_value) {}
        DoubleGrouper(Ptr<GroupOperator> groupOperator, int depth, Ptr<Grouper> previous_grouper, double double_value) : Grouper(groupOperator, depth, previous_grouper, double_value) {}
        DoubleGrouper(Ptr<GroupOperator> groupOperator, int depth, Ptr<Grouper> previous_grouper, std::string string_value) : Grouper(groupOperator, depth, previous_grouper, string_value) {}

        DoubleGrouper(Ptr<GroupOperator> groupOperator, int depth) : Grouper(groupOperator, depth, nullptr, 0L) {}

        static TypeId GetTypeId();

        void groupTuple(Ptr<TupleWrapper> t) override;
        Ptr<Grouper> copy(Ptr<GroupOperator> group_operator, bool shed_unessential_attributes) override;
        Ptr<Group> GetGroup(int group_id) override;
    };

    class DoublePartitioner : public Partitioner {
        std::unordered_map<double, Ptr<Partitioner> > doubleToPartitioner;
    public:
        DoublePartitioner(Ptr<Partitioner> partitioner) : Partitioner(partitioner->groupOperator, partitioner->depth, partitioner->double_value) {}
        DoublePartitioner(Ptr<GroupOperator> groupOperator, int depth, long long_value) : Partitioner(groupOperator, depth, long_value) {}
        DoublePartitioner(Ptr<GroupOperator> groupOperator, int depth, double double_value) : Partitioner(groupOperator, depth, double_value) {}
        DoublePartitioner(Ptr<GroupOperator> groupOperator, int depth, std::string string_value) : Partitioner(groupOperator, depth, string_value) {}

        DoublePartitioner(Ptr<GroupOperator> groupOperator, int depth) : Partitioner(groupOperator, depth, 0L) {}

        static TypeId GetTypeId();

        void partitionTuple(Ptr<TupleWrapper> t) override;
        Ptr<Partitioner> copy(Ptr<GroupOperator> group_operator, bool shed_unessential_attributes) override;
        Ptr<Group> GetGroup(int group_id) override;
    };

    class StringGrouper : public Grouper {
        std::unordered_map<std::string, Ptr<Grouper> > stringToGrouper;
    public:
        StringGrouper(Ptr<GroupOperator> groupOperator, int depth, Ptr<Grouper> previous_grouper, long long_value) : Grouper(groupOperator, depth, previous_grouper, long_value) {}
        StringGrouper(Ptr<GroupOperator> groupOperator, int depth, Ptr<Grouper> previous_grouper, double double_value) : Grouper(groupOperator, depth, previous_grouper, double_value) {}
        StringGrouper(Ptr<GroupOperator> groupOperator, int depth, Ptr<Grouper> previous_grouper, std::string string_value) : Grouper(groupOperator, depth, previous_grouper, string_value) {}

        StringGrouper(Ptr<GroupOperator> groupOperator, int depth) : Grouper(groupOperator, depth, nullptr, 0L) {}

        static TypeId GetTypeId();

        void groupTuple(Ptr<TupleWrapper> t) override;
        Ptr<Grouper> copy(Ptr<GroupOperator> group_operator, bool shed_unessential_attributes) override;
        Ptr<Group> GetGroup(int group_id) override;
    };

    class StringPartitioner : public Partitioner {
        std::unordered_map<std::string, Ptr<Partitioner> > stringToPartitioner;
    public:
        StringPartitioner(Ptr<Partitioner> partitioner) : Partitioner(partitioner->groupOperator, partitioner->depth, partitioner->string_value) {}
        StringPartitioner(Ptr<GroupOperator> groupOperator, int depth, long long_value) : Partitioner(groupOperator, depth, long_value) {}
        StringPartitioner(Ptr<GroupOperator> groupOperator, int depth, double double_value) : Partitioner(groupOperator, depth, double_value) {}
        StringPartitioner(Ptr<GroupOperator> groupOperator, int depth, std::string string_value) : Partitioner(groupOperator, depth, string_value) {}

        StringPartitioner(Ptr<GroupOperator> groupOperator, int depth) : Partitioner(groupOperator, depth, 0L) {}

        static TypeId GetTypeId();

        void partitionTuple(Ptr<TupleWrapper> t) override;
        Ptr<Partitioner> copy(Ptr<GroupOperator> group_operator, bool shed_unessential_attributes) override;
        Ptr<Group> GetGroup(int group_id) override;
    };

    class PrintOperator : public Operator {
    public:
        PrintOperator();

        static TypeId GetTypeId(void);

        Ptr<Operator> copy(Ptr<Stream> s) override {
            auto printOperator = CreateObject<PrintOperator>();
            Operator::copy(printOperator, s);
            return printOperator;
        }

        int print_cnt = 0;
        inline static int total_cnt = 0;

        void process(Ptr<TupleWrapper> tupleWrapper) override;

        int calculateMigrationSize(migration_types migration_type) override;
        void ExtractState(Ptr<MigrationPlan> mp) override;
        void ImportState(Ptr<PartialStateBatch> batch) override;
    };

    class SelectField : public Object {
    public:
        static TypeId GetTypeId();

        /*SelectField(int input_stream_id, const std::string &inputField, Aggregation aggregate,
                    const std::string &outputField, Ptr<StreamQuery> query);

        SelectField(int input_stream_id, const std::string &inputField, const std::string &outputField, Ptr<StreamQuery> query);

        SelectField(int input_stream_id, const std::string &inputField, Ptr<StreamQuery> query);

        SelectField(const std::string &inputField, Aggregation aggregate, const std::string &outputField, Ptr<StreamQuery> query);

        SelectField(const std::string &inputField, const std::string &outputField, Ptr<StreamQuery> query);*/

        SelectField(const std::string &alias, const std::string &inputField, const std::string &outputField, Ptr<StreamQuery> query);

        SelectField(const std::string &alias, const std::string &inputField, Aggregation aggregate, const std::string &outputField, Ptr<StreamQuery> query);

        virtual Ptr<SelectField> copy() {
            auto sf = CreateObject<SelectField>(alias, inputField, aggregate, outputField, query);
            sf->outputFieldIndex = this->outputFieldIndex;
            return sf;
        }

        int input_stream_id = -1;
        std::string alias = "";
        std::string inputField;
        int inputFieldIndex = -1;
        int outputFieldIndex = -1;
        std::string outputField;
        Type type;
        Aggregation aggregate;
        int tuple_index = 0;
        Ptr<StreamQuery> query;

        bool same_as(Ptr<SelectField> sf) const;

        bool isStreamIdSpecified() const { return input_stream_id != -1; }

        static int getFieldIndex(std::string alias, const std::string &field, Ptr<StreamQuery> query);

        static Ptr<SelectField> createSf(std::string &field, Ptr<StreamQuery> query);
    };

    class WindowExtentContainer : public Object {
    public:
        static TypeId GetTypeId();

        int newest_window_extent = 0;
        std::vector <std::deque<AggregationWindowExtent*>> aggregationWindowExtent{1};
    };

    class AggregateSelectField : public SelectField {
    public:
        int aggregation_arguments_index = -1;

        static TypeId GetTypeId();

        AggregateSelectField(std::string alias, const std::string &inputField, Aggregation aggregate,
                             const std::string &outputField, Ptr<StreamQuery> query);

        Ptr<SelectField> copy() {
            auto copy = CreateObject<AggregateSelectField>(alias, inputField, aggregate, outputField, query);
            copy->outputFieldIndex = this->outputFieldIndex;
            return copy;
        }
    };

    class SelectOperator : public Operator {
    public:
        explicit SelectOperator(std::string select_statement) : Operator(selectType) {
            this->select_statement = std::move(select_statement);
        }

        static TypeId GetTypeId(void);

        void parseSelectStatement(std::vector<int> input_stream_ids, int output_stream_id, const std::string &select_statement);

        void init(Ptr<WindowOperator> windowOperator);

        std::vector<Ptr<SelectField>> fields;

        std::string select_statement;

        Ptr<Operator> copy(Ptr<Stream> s) override;

        Ptr<Tuple> selectFields(Ptr<TupleWrapper> tupleWrapper);
        std::vector<Ptr<Tuple>> selectPartitionedFields(Ptr<TupleWrapper> tupleWrapper);

        void process(Ptr<TupleWrapper> tupleWrapper) override;

        Time get_base_processing_time() override;
        Time get_emit_time_added() override;
        Time get_forward_time_added() override;
        int calculateMigrationSize(migration_types migration_type) override;
        void ExtractState(Ptr<MigrationPlan> mp) override;
        void ImportState(Ptr<PartialStateBatch> batch) override;
    };

    class SinkOperator : public Operator {
    public:
        SinkOperator() : Operator(sinkType) {}

        static TypeId GetTypeId(void);

        Ptr<Operator> copy(Ptr<Stream> s) override;

        long number_tuples = 0;
        long time_last_received = -1;
        static inline long total_number_tuples = 0;

        void process(Ptr<TupleWrapper> tupleWrapper) override;

        int calculateMigrationSize(migration_types migration_type) override;
        void ExtractState(Ptr<MigrationPlan> mp) override;
        void ImportState(Ptr<PartialStateBatch> batch) override;
    };

    class WindowOperator : public Operator {
    public:
        WindowOperator() : Operator(windowType) {
            this->windowOperator = this;
        }

        static TypeId GetTypeId(void);

        int size{};
        WindowType size_type;
        WindowType emit_type;
        long last_jump = 0;
        long last_deletion = 0;
        static inline int global_total_received = 0;
        int total_received = 0;
        Ptr<GroupOperator> groupOperator;
        long current_external_time = 0;
        std::string external_timestamp_key;
        int external_timestamp_key_index{};
        int external_timestamp_tuple_index = -1;
        int number_evicted = 0;
        int jump = 1;

        virtual Ptr<Operator> copy(Ptr<Stream> s) = 0;

        virtual void process(Ptr<TupleWrapper> tupleWrapper) = 0;

        virtual bool insertTuple(Ptr<TupleWrapper> tupleWrapper) = 0;

        void update_external_time_window(long external_time) { this->current_external_time = external_time; }

        virtual int calculateMigrationSize(migration_types migration_type) = 0;
        virtual void ExtractState(Ptr<MigrationPlan> mp) = 0;
        virtual void ImportState(Ptr<PartialStateBatch> batch) = 0;
    };

    class StorageWindowOperator : public WindowOperator {
    public:
        std::deque<Ptr<TupleWrapper> > tuples;
        bool evicted_tuples = false;
        Ptr<JoinOperator> joinOperator;

        static TypeId GetTypeId();

        void
        init(int size, int jump, WindowType size_type, WindowType emit_type, const std::string &external_timestamp_key);

        void updateWindow();

        void process(Ptr<TupleWrapper> tupleWrapper) override;

        bool insertTuple(Ptr<TupleWrapper> tupleWrapper);

        std::vector<Ptr<PartialState>> getPartialStates() override;

        void DisableState(Ptr<PartialStateBatch> batch) override;
        void ClearState() override;

        Ptr<Operator> copy(Ptr<Stream> s) override {
            auto new_op = CreateObject<StorageWindowOperator>();
            new_op->external_timestamp_tuple_index = this->external_timestamp_tuple_index;
            Operator::copy(new_op, s);
            new_op->init(size, jump, size_type, emit_type, external_timestamp_key);
            return new_op;
        }

        int calculateMigrationSize(migration_types migration_type) override;
        void ExtractState(Ptr<MigrationPlan> mp) override;
        void ExtractDiffState(Ptr<MigrationPlan> mp) override;
        Ptr<PartialStateBatch> ExtractPreambleState(Ptr<MigrationPlan> mp) override;
        void ImportState(Ptr<PartialStateBatch> batch) override;
    };

    class TuplePartialState : public PartialState {
    public:
        static TypeId GetTypeId();
    };

    class WindowExtent : public PartialState {
    public:
        static TypeId GetTypeId();

        WindowExtent(int created_by);
        WindowExtent(Ptr<WindowExtent> extent, int created_by, bool only_skeleton);

        size_t GetSerializedSize() override;
        size_t GetUtility() override;

        std::vector<AggregationWindowExtent> aggregationWindowExtents;
        Ptr<Group> group = Group::no_group;
        std::map<int, Ptr<Group>> groupToPartitions;
        bool expired = false;
        bool emitted = false;
        bool merged = false;
        inline static int extent_cnt = 0;
        int id = extent_cnt++;
        int created_by;

        Time start;
        Time expireTime;
        Time upper_bound;
        Time lower_bound;
        Time migration_start = Time(0);
        Time migration_stop = Time(0);

        static inline std::map<int, std::map<long, std::pair<Time, int>>> keyToLowerboundToEmitted;
        static inline std::map<int, std::map<long, std::pair<Time, int>>> keyToLowerboundToCreated;

        void merge(Ptr<WindowExtent> extent);
        void replace(Ptr<WindowExtent> extent);
    };

    class SequencePartialState : public PartialState {
    public:
        static TypeId GetTypeId();

        SequencePartialState(Ptr<CepPatternOperator> cepPatternOperator);

        std::string current_state;
        std::unordered_map<std::string, std::vector<Ptr<TupleWrapper>>> aliasToTuples;
        Ptr<WindowExtent> windowExtent;
        Ptr<CepPatternOperator> cepPatternOperator;
        long first_ts;
        long last_ts;

        bool addToSequence(const std::string& alias, Ptr<TupleWrapper> tupleWrapper);
        size_t GetSerializedSize() override;
        size_t GetUtility() override;
    };

    class PartialStateBatch : public Object {
    public:
        static TypeId GetTypeId();

        DcepHeader dcepHeader;
        Ptr<Operator> op;
        Ptr<MigrationPlan> mp;

        inline static int cnt_id = 0;
        int id = cnt_id++;

        bool in_migration = false;

        bool active_op = true;
        bool op_final_batch = true;

        int old_host;
        int next_host;

        PartialStateBatch(Ptr<Operator> op, Ptr<MigrationPlan> mp);

        std::vector<Ptr<PartialStateWrapper>> partial_states;

        size_t GetSerializedSize();
        Ptr<StreamQuery> GetQuery();

        void merge(Ptr<PartialStateBatch>);

        Time willBeSentBy(Time sentAt);
    };

    class DeadlinePartialStateBatch : public PartialStateBatch {
    public:
        static TypeId GetTypeId();

        DeadlinePartialStateBatch(Ptr<Operator> op, Ptr<MigrationPlan> mp);

        bool canBeSentAfter(Ptr<PartialStateBatch> batch);
        
        Time deadline;
        Time must_be_sent_by;
    };

    class AggregationWindowOperator : public WindowOperator {
    public:
        int emit_size = 0;
        Time last_emit_time;
        bool activated_time_emission = false;
        Time first_extent_time = Time(0);
        std::vector<Ptr<AggregateSelectField> > aggregation_arguments;
        std::vector<AggregationWindowExtent> asf_extent_copies;
        std::map<int, std::deque<Ptr<WindowExtent>>> groupIdToWindowExtents;
        std::map<int, std::deque<Ptr<WindowExtent>>> groupIdToExtentsInMigration;
        std::map<int, std::vector<Ptr<WindowExtent>>> pending_extents;
        int cnt_extents = 0;

        static TypeId GetTypeId(void);

        virtual void init(int size, int jump, WindowType size_type, int emit_size, WindowType emit_type,
                  const std::string &external_timestamp_key);

        void process(Ptr<TupleWrapper> tupleWrapper) override;

        void emit_point(bool recurring);

        std::vector<Ptr<PartialState>> getPartialStates() override;

        bool insertTuple(Ptr<TupleWrapper> tupleWrapper);

        void addWindowExtent(int newest_window_extent);

        void AddWindowExtent(Ptr<Group> group, int deadline);

        bool CheckForExistingExtentsAndMerge(Ptr<WindowExtent> extent);

        bool CheckForExistingExtentsAndReplace(Ptr<WindowExtent> extent);

        int doEmitExtent(Ptr<WindowExtent> extent);

        bool evictExtents(int group_id);

        bool firstExtentExpired(int group_id);

        void jumpExtents(Ptr<TupleWrapper> tupleWrapper);

        int emitExtents(int group_id);

        void SetActive() override;

        std::vector<AggregationWindowExtent> getAsfExtentCopies();

        void addExtent();

        Ptr<Operator> copy(Ptr<Stream> s) override;

        int calculateMigrationSize(migration_types migration_type) override;
        void ExtractState(Ptr<MigrationPlan> mp) override;
        Ptr<PartialStateBatch> ExtractPreambleState(Ptr<MigrationPlan> mp) override;
        void ExtractDiffState(Ptr<MigrationPlan> mp) override;
        void ExtractLazyState(Ptr<MigrationPlan> mp);
        void ImportState(Ptr<PartialStateBatch> batch) override;
        void DisableState(Ptr<PartialStateBatch> batch) override;
        void ClearState() override;
        void MigrationInterrupted() override;
        Time get_base_processing_time() override;
        Time get_emit_time_added() override;
        Time get_forward_time_added() override;
    };

    class CepWindowOperator : public AggregationWindowOperator {
    public:
        static TypeId GetTypeId();

        std::vector<Ptr<SequencePartialState>> sequences;
        std::map<int, std::vector<Ptr<SequencePartialState>>> groupIdToSequences;
        void init(int size, int jump, WindowType size_type, int emit_size, WindowType emit_type, const std::string& external_timestamp_key) override;
        void addSequence(Ptr<SequencePartialState> sequence);
        void addSequence(Ptr<SequencePartialState> sequence, int group_id);
        void insertTuple(Ptr<SequencePartialState> sequence, Ptr<TupleWrapper> tupleWrapper);
        void undoInsertTuple(Ptr<SequencePartialState> sequence, Ptr<TupleWrapper> tupleWrapper);
        int calculateMigrationSize(migration_types migration_type) override;
        void ExtractState(Ptr<MigrationPlan> mp) override;
        void ExtractDiffState(Ptr<MigrationPlan> mp) override;
        void ImportState(Ptr<PartialStateBatch> batch) override;
    };

    class WithinOperator : public Operator {
    public:
        static TypeId GetTypeId();
        long ms;
        int tuple_index = 0;
        int index = 0;
        std::string timestamp_attribute;

        WithinOperator(long ms, std::string timestamp_attribute);

        void process(Ptr<TupleWrapper> tupleWrapper) override {}

        Ptr<Operator> copy(Ptr<Stream> s) override;

        int calculateMigrationSize(migration_types migration_type) override;
        void ExtractState(Ptr<MigrationPlan> mp) override;
        void ImportState(Ptr<PartialStateBatch> batch) override;
    };

    class DefineOperator : public Operator {
    public:
        static TypeId GetTypeId();

        std::string alias;
        Ptr<FilterOperator> filterOperator;
        Ptr<FilterArgument> mainFilterArgument = nullptr;
        int stream_id;

        DefineOperator(std::string alias, Ptr<FilterOperator> filterOperator);
        DefineOperator(std::string alias);

        bool match(Ptr<SequencePartialState> sequence, Ptr<TupleWrapper> tupleWrapper) const;

        void process(Ptr<TupleWrapper> tupleWrapper) override {}

        Ptr<Operator> copy(Ptr<Stream> s) override;

        int calculateMigrationSize(migration_types migration_type) override;
        void ExtractState(Ptr<MigrationPlan> mp) override;
        void ImportState(Ptr<PartialStateBatch> batch) override;
    };

    class CepPatternOperator : public Operator {
    public:
        std::string pattern;
        std::vector<Ptr<DefineOperator>> defineOperators;
        boost::regex regex;
        bool multiple_sequences = false;
        bool emit_all_tuples = false;
        Ptr<GroupOperator> groupOperator = nullptr;
        Ptr<WithinOperator> withinOperator = nullptr;

        CepPatternOperator(const std::string& pattern, bool multiple_sequences, bool emit_all_tuples) : Operator(cepType) {
            this->pattern = pattern;
            this->parsePattern(pattern);
            this->multiple_sequences = multiple_sequences;
            this->emit_all_tuples = emit_all_tuples;
            auto cepWindow = CreateObject<CepWindowOperator>();
            this->windowOperator = cepWindow;
        }

        void process(Ptr<TupleWrapper> tupleWrapper) override;

        Ptr<Operator> copy(Ptr<Stream> stream) override;

        size_t calculateUtility(int group_id, int partial_state_index) override;

        void parsePattern(const std::string& pattern);

        size_t GetSerializedSize() override;

        std::vector<Ptr<PartialState>> getPartialStates() override;

        int calculateMigrationSize(migration_types migration_type) override;
        void ExtractState(Ptr<MigrationPlan> mp) override;
        void ImportState(Ptr<PartialStateBatch> batch) override;
    };

    class FilterArgument : public Object {
        int tuple_index = 0;
    public:
        static TypeId GetTypeId();

        FilterArgument(FilterArgumentType filterArgumentType) {
            this->filterArgumentType = filterArgumentType;
        }

        virtual std::string getStringValue(Ptr<TupleWrapper> t) = 0;

        virtual long getLongValue(Ptr<TupleWrapper> t) = 0;

        virtual double getDoubleValue(Ptr<TupleWrapper> t) = 0;

        Type type;
        Ptr<FilterOperator> filterOperator{};
        FilterArgumentType filterArgumentType;
        Aggregation aggregation = NONE;
        SelectorType selector = NOSELECTOR;
        Ptr<AggregateSelectField> asf = nullptr;
        std::string field;
        int index = -1;
        int stream_id = -1;
        double offset = 0;
        std::string alias = "";
        std::string stream_name = "";
        Ptr<StreamQuery> query;

        Type getType() {
            return type;
        }

        int getTupleIndex() { return tuple_index; }

        virtual bool isInitialized() = 0;

        virtual Ptr<FilterArgument> copy() = 0;
    };

    class ConstantFilterArgument : public FilterArgument {
        long long_value{};
        long timestamp_value{};
        std::string string_value;
        double double_value{};
    public:
        static TypeId GetTypeId();

        ConstantFilterArgument() : FilterArgument(CONSTANTFILTERARG) {}

        explicit ConstantFilterArgument(long long_value) : ConstantFilterArgument() {
            this->long_value = long_value;
            type = LONG;
        }

        explicit ConstantFilterArgument(long long_value, double offset) : ConstantFilterArgument(long_value) {
            this->offset = offset;
        }

        explicit ConstantFilterArgument(std::string string_value) : ConstantFilterArgument() {
            this->string_value = std::move(string_value);
            type = STRING;
        }

        explicit ConstantFilterArgument(double double_value) : ConstantFilterArgument() {
            this->double_value = double_value;
            type = DOUBLE;
        }

        explicit ConstantFilterArgument(double double_value, double offset) : ConstantFilterArgument(double_value) {
            this->offset = offset;
        }

        bool isInitialized() override {
            return true;
        }

        std::string getStringValue(Ptr<TupleWrapper> t) override {
            return string_value;
        }

        long getLongValue(Ptr<TupleWrapper> t) override {
            return long_value + offset;
        }

        double getDoubleValue(Ptr<TupleWrapper> t) override {
            return double_value + offset;
        }

        Ptr<FilterArgument> copy();
    };

    class ListConstantFilterArgument : public ConstantFilterArgument {
        std::vector<long> long_values;
        std::vector<std::string> string_values;
        std::vector<double> double_values;
    public:
        static TypeId GetTypeId();

        ListConstantFilterArgument(std::vector<long> long_values) : ConstantFilterArgument() {
            this->long_values = long_values;
            type = LONG;
        }

        ListConstantFilterArgument(std::vector<long> long_values, double offset) : ListConstantFilterArgument(long_values) {
            this->offset = offset;
        }

        ListConstantFilterArgument(std::vector<std::string> string_values) : ConstantFilterArgument() {
            this->string_values = std::move(string_values);
            type = STRING;
        }

        ListConstantFilterArgument(std::vector<double> double_values) : ConstantFilterArgument() {
            this->double_values = double_values;
            type = DOUBLE;
        }

        ListConstantFilterArgument(std::vector<double> double_values, double offset) : ListConstantFilterArgument(double_values) {
            this->offset = offset;
        }

        bool isInitialized() override {
            return true;
        }

        std::vector<std::string> getStringValues() {
            return string_values;
        }

        std::vector<long> getLongValues() {
            return long_values;
        }

        std::vector<double> getDoubleValues() {
            return double_values;
        }

        Ptr<FilterArgument> copy();
    };

    struct LongComparator {
    public:
        bool operator()(const std::pair<Ptr<TupleWrapper> , long> &i, const std::pair<Ptr<TupleWrapper> , long> &j) const {
            return i.second < j.second;
        }
    };

    struct DoubleComparator {
    public:
        bool operator()(const std::pair<Ptr<TupleWrapper> , double> &i, const std::pair<Ptr<TupleWrapper> , double> &j) const {
            return i.second < j.second;
        }
    };

    class AttributeFilterArgument : public FilterArgument {
    public:
        static TypeId GetTypeId();

        bool cacheArgument = false;
        int number_evicted = 0;
        std::string filter = "";
        std::unordered_map<long, std::vector<Ptr<TupleWrapper> >> longToTuples;
        std::unordered_map <std::string, std::vector<Ptr<TupleWrapper> >> stringToTuples;
        std::unordered_map<double, std::vector<Ptr<TupleWrapper> >> doubleToTuples;

        std::multiset <std::pair<Ptr<TupleWrapper> , long>, LongComparator> bstLongTuples;
        std::multiset <std::pair<Ptr<TupleWrapper> , double>, DoubleComparator> bstDoubleTuples;

        inline static std::pair<Ptr<TupleWrapper> , long> lower_bound = std::make_pair(nullptr, LONG_MIN);
        inline static std::pair<Ptr<TupleWrapper> , long> upper_bound = std::make_pair(nullptr, LONG_MAX);
        inline static std::pair<Ptr<TupleWrapper> , long> custom_bound = std::make_pair(nullptr, 0);

        AttributeFilterArgument(const std::string &alias, const std::string &field, bool cacheArgument, Ptr<StreamQuery> query, double offset);
        AttributeFilterArgument(Aggregation aggregation, const std::string &alias, const std::string& field, bool cacheArgument, Ptr<StreamQuery> query, double offset);

        std::string getStringValue(Ptr<TupleWrapper> tupleWrapper) override;

        long getLongValue(Ptr<TupleWrapper> tupleWrapper) override;

        double getDoubleValue(Ptr<TupleWrapper> tupleWrapper) override;

        bool isInitialized() override {
            return index != -1;
        }

        void cache(Ptr<TupleWrapper> tupleWrapper);

        Ptr<FilterArgument> copy();
    };

    class PassthroughOperator : public Operator {
    public:
        PassthroughOperator();

        static TypeId GetTypeId(void);
        void process(Ptr<TupleWrapper> tupleWrapper) override;
        int calculateMigrationSize(migration_types migration_type) override;
        void ExtractState(Ptr<MigrationPlan> mp) override;
        void ImportState(Ptr<PartialStateBatch> batch) override;

        Ptr<Operator> copy(Ptr<Stream> s) override {
            auto passthroughOperator = CreateObject<PassthroughOperator>();
            Operator::copy(passthroughOperator, s);
            return passthroughOperator;
        }
    };

    class InputOperator : public Operator {
    public:
        InputOperator();
        InputOperator(std::string alias);

        static TypeId GetTypeId(void);
        void process(Ptr<TupleWrapper> tupleWrapper) override;
        int calculateMigrationSize(migration_types migration_type) override;
        void ExtractState(Ptr<MigrationPlan> mp) override;
        void ImportState(Ptr<PartialStateBatch> batch) override;

        Ptr<Operator> copy(Ptr<Stream> s) override {
            auto inputOperator = CreateObject<InputOperator>();
            Operator::copy(inputOperator, s);
            return inputOperator;
        }

        std::string alias;
        int number_tuples = 0;
    };

    class FilterOperator : public Operator {
    public:
        static TypeId GetTypeId(void);

        FilterOperator(Ptr<FilterArgument> arg1, Op op, Ptr<FilterArgument> arg2);

        FilterOperator() : Operator(filterType) {}

        bool calculateCondition(Ptr<TupleWrapper> tupleWrapper) const;

        Ptr<Operator> copy(Ptr<Stream> s);

        Ptr<FilterArgument> arg1{};
        Op op;
        Ptr<FilterArgument> arg2{};

        Ptr<SequencePartialState> sequence;

        virtual void init();

        void process(Ptr<TupleWrapper> tupleWrapper) override;

        virtual int calculateMigrationSize(migration_types migration_type) override;
        virtual void ExtractState(Ptr<MigrationPlan> mp) override;
        void ImportState(Ptr<PartialStateBatch> batch) override;
    };

    class JoinFilterOperator : public FilterOperator {
    public:
        static TypeId GetTypeId(void);

        JoinFilterOperator(Ptr<FilterArgument> arg1, Op op, Ptr<FilterArgument> arg2) : FilterOperator(arg1, op, arg2) {}

        Ptr<JoinOperator> joinOperator;

        ContainerType containerType;

        std::vector<Ptr<TupleWrapper> > getArg1Matches(Ptr<TupleWrapper> tupleWrapper) const;

        std::vector<Ptr<TupleWrapper> > getArg2Matches(Ptr<TupleWrapper> tupleWrapper) const;

        void init();

        int calculateMigrationSize(migration_types migration_type) override;
        void ExtractState(Ptr<MigrationPlan> mp) override;
        void ImportState(Ptr<PartialStateBatch> batch) override;
    };

    class AggregationWindowExtent {
    public:
        int inputFieldIndex = -1;
        int count = 0;
        long sum = 0;
        double aggregate = 0;
        double prev_aggregate = 0;
        // Used for migration to detect if the extent has been migrated
        bool present = true;
        Aggregation aggregate_type;

        AggregationWindowExtent(Aggregation aggregate_type, int inputFieldIndex) {
            this->aggregate_type = aggregate_type;
            this->inputFieldIndex = inputFieldIndex;
        }

        bool insertTuple(Ptr<Tuple> tuple);
        void undoInsertTuple(Ptr<Tuple> tuple);
        double getAggregate();
    };

    class Tuple : public Object {
    public:
        inline static int id_cnt = 1;

        Tuple(int stream_id, std::vector<YAML::Node> fieldsToValues) : Tuple() {
            this->stream_id = stream_id;
            setFieldsToValues(fieldsToValues);
        }

        explicit Tuple(int stream_id) : Tuple() {
            this->stream_id = stream_id;
        }

        Tuple(int stream_id, int id) {
            this->stream_id = stream_id;
            this->id = id;
        }

        static int get_next_id() {
            return id_cnt++;
        }

        Tuple() {
            this->id = get_next_id();
        }

        static int get_index(int stream_id, std::string alias, std::string field, Ptr<StreamQuery> query);
        static int get_index(int stream_id, std::string field);

        static TypeId GetTypeId(void);

        inline static std::map<int, std::map<std::string, Type>> streamIdToFieldToType;
        inline static std::map<int, std::map<int, std::string>> streamIdToIndexToField;

        inline static std::map<std::string, int> streamNameToId;

        int reference_counter = 1;
        bool evicted = false;
        int id;
        int stream_id;
        std::string alias;
        std::vector <std::string> stringValues;
        std::vector<long> longValues;
        std::vector<double> doubleValues;
        Time external_timestamp = Time(0);
        int join_depth = 0;
        Ptr<TupleWrapper> tupleWrapper;
        bool shed = false;

        Tuple(Ptr<Tuple> t);

        Tuple(Ptr<Tuple> t, Ptr<StreamQuery> q);

        Tuple(Ptr<Tuple> t, Ptr<StreamQuery> q, bool shed_unessential_attributes);

        void decrementRef();

        long getLongValue(int index);

        double getDoubleValue(int index);

        std::string getStringValue(int index);

        bool is_essential_long_attribute(int index);

        bool is_essential_double_attribute(int index);

        bool is_essential_string_attribute(int index);

        Time getTime();
        void setTime(Time t);

        std::tuple<int, int, int> GetShedSizes(Ptr<StreamQuery> q) const;

        void setFieldsToValues(const std::map <std::string, std::any> &fieldsToValues);

        void setFieldsToValues(const std::vector<YAML::Node>& fieldsToValues);

        void PopulateTupleWithQuery(Ptr<Tuple> t, Ptr<StreamQuery> q);

        inline void addFieldValue(long value) {
            longValues.emplace_back(value);
        }

        inline void addFieldValue(const std::string &value) {
            stringValues.emplace_back(value);
        }

        inline void addFieldValue(double value) {
            doubleValues.emplace_back(value);
        }

        void setFieldToValueOfTuple(Ptr<SelectField> sf, Ptr<Tuple> t);

        friend std::ostream &operator<<(std::ostream &os, const Tuple &tuple);
    };

    class TupleWrapper : public PartialState {
    public:
        explicit TupleWrapper(std::vector<Ptr<Tuple>> tuples);

        TupleWrapper(Ptr<TupleWrapper> tupleWrapper, Ptr<StreamQuery> query);

        explicit TupleWrapper(Ptr<TupleWrapper> tupleWrapper);

        TupleWrapper(Ptr<TupleWrapper> tupleWrapper, bool shed_unessential_attributes, bool new_ids);

        TupleWrapper() = default;

        void add_tuple(Ptr<Tuple> t);

        static TypeId GetTypeId();

        Ptr<Group> group = Group::no_group;
        Ptr<Group> partition_group = Group::no_group;
        std::vector<Ptr<Tuple>> tuples;
        bool evicted = false;
        Ptr<StreamQuery> query;
        int next_operator_id = 0;
        Ptr<Node> node;
        bool shed = false;

        bool containsStreamId(int stream_id);

        void shed_unessential();

        bool cmp(std::vector<int> stream_ids) {
            if (stream_ids.size() != tuples.size()) {
                return false;
            }
            for (std::size_t i = 0; i < stream_ids.size(); i++) {
                bool found = false;
                for (std::size_t j = 0; j < stream_ids.size(); j++) {
                    if (stream_ids[i] == tuples[j]->stream_id) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return false;
                }
            }
            return true;
        }

        Time getTime();
        void setTime(Time t);
        size_t GetSerializedSize(bool with_group);
        size_t Serialize(bool with_group, Buffer::Iterator iterator);
        static Ptr<TupleWrapper> Deserialize(uint8_t *serialized_payload, size_t size);
        static Ptr<TupleWrapper> doDeserialize(Buffer::Iterator *iterator);
        static Ptr<TupleWrapper> DeserializeGroupLess(uint8_t *serialized_payload);
        size_t GetSerializedSize() override;
        size_t GetUtility() override;

        friend std::ostream &operator<<(std::ostream &os, const TupleWrapper &tupleWrapper);
    };

    class GroupedTuple : public Tuple {
    public:
        static TypeId GetTypeId();

        GroupedTuple();
        GroupedTuple(int stream_id, int id);

        std::vector<long> aggregated_long_values;
        std::vector<double> aggregated_double_values;
    };

    class MigrationPlan : public Object {
    public:
        static TypeId GetTypeId();

        MigrationPlan(std::string migration_plan);

        Ptr<PartialStateBatch> GetNextBatch();

        int old_host;
        int operator_id;
        std::string migration_mode;

        Ptr<MigrationMechanism> migration_mechanism;

        bool finished = false;
        bool operators_disabled = false;

        std::map<int, int> operatorIdToNumberPhases;
        std::map<int, int> operatorIdToOldHost;
        std::map<int, int> operatorIdToNewHost;

        bool preamble_states_sent = false;
        bool crit_states_sent = false;
        bool essential_states_sent = false;
        bool deadline_states_sent = false;
        bool unessential_states_sent = false;

        std::vector<Ptr<Operator>> operators;

        std::deque<Ptr<PartialStateBatch>> preambleQueue;
        std::deque<Ptr<PartialStateBatch>> critQueue;
        std::deque<Ptr<PartialStateBatch>> fetchQueue;
        std::deque<Ptr<PartialStateBatch>> normalEssentialQueue;
        std::deque<Ptr<PartialStateBatch>> normalUnessentialQueue;
        std::deque<Ptr<DeadlinePartialStateBatch>> deadlineQueue;
        //std::vector<int> operatorOrder;

        void InsertBatchCritQueue(Ptr<PartialStateBatch>);
        void InsertBatchFetchQueue(Ptr<PartialStateBatch>);
        void InsertBatchDeadlineQueue(Ptr<DeadlinePartialStateBatch>);
        void InsertOperator(Ptr<Operator> op);

        Ptr<PartialStateBatch> GetPreambleBatch();
        Ptr<PartialStateBatch> GetCritBatch();
        Ptr<PartialStateBatch> GetFetchBatch();
        Ptr<PartialStateBatch> GetDeadlineBatch();
        Ptr<PartialStateBatch> GetEssentialBatch();
        Ptr<PartialStateBatch> GetUnessentialBatch();
        bool Finished();
    };

    class MigrationMechanism : public Object {
    public:
        static TypeId GetTypeId();

        MigrationMechanism(Ptr<MigrationPlan> mp);

        virtual void OperatorFinalBatchReady(Ptr<Operator> op, Ptr<PartialStateBatch> batch);
        virtual void ExtractOperatorPreambleState(Ptr<Operator> old_host_op, Ptr<Operator> new_host_op);
        virtual void PrepareOperatorPreambleState(Ptr<Operator> op, Ptr<PartialStateBatch> batch);
        virtual void PrepareBatchForSending(Ptr<PartialStateBatch> batch);
        virtual void BeforeFirstMoveNextState();
        virtual void AllPreambleStatesSent() {}

        bool run_old_and_new_during_migration = false;

        Ptr<MigrationPlan> mp;
    };

    class MegaphoneMigrationMechanism : public MigrationMechanism {
    public:
        static TypeId GetTypeId();

        MegaphoneMigrationMechanism(Ptr<MigrationPlan> mp);
    };

    class MecesMigrationMechanism : public MigrationMechanism {
    public:
        static TypeId GetTypeId();

        MecesMigrationMechanism(Ptr<MigrationPlan> mp);
    };

    class RhinoMigrationMechanism : public MigrationMechanism {
    public:
        static TypeId GetTypeId();

        RhinoMigrationMechanism(Ptr<MigrationPlan> mp);

        void AllPreambleStatesSent() override;
    };

    class AllAtOnceMigrationMechanism : public MigrationMechanism {
    public:
        static TypeId GetTypeId();

        AllAtOnceMigrationMechanism(Ptr<MigrationPlan> mp);
    };

    class LazyLatencyMigrationMechanism : public MigrationMechanism {
    public:
        static TypeId GetTypeId();

        LazyLatencyMigrationMechanism(Ptr<MigrationPlan> mp);
    };

    class LazyUtilityMigrationMechanism : public MigrationMechanism {
    public:
        static TypeId GetTypeId();

        LazyUtilityMigrationMechanism(Ptr<MigrationPlan> mp);
    };

    class DropStateMigrationMechanism : public MigrationMechanism {
    public:
        static TypeId GetTypeId();

        DropStateMigrationMechanism(Ptr<MigrationPlan> mp);
    };
}

#endif //EP_SIM_OPERATOR_H
