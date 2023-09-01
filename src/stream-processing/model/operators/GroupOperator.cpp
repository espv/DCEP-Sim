//
// Created by espen on 27.10.2021.
//

#include "ns3/Operator.h"
#include "ns3/log.h"
#include <map>


namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED(GroupOperator);
    NS_LOG_COMPONENT_DEFINE("GroupOperator");

    TypeId
    GroupOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::GroupOperator")
                .SetParent<Operator> ()
                .AddConstructor<GroupOperator> ()
        ;

        return tid;
    }

    TypeId
    Group::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::Group")
                .SetParent<PartialState> ()
                .AddConstructor<Group> ()
        ;

        return tid;
    }

    TypeId
    Grouper::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::Grouper")
                .SetParent<PartialState>()
        ;

        return tid;
    }

    TypeId
    NoGrouper::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::NoGrouper")
                .SetParent<Grouper>()
        ;

        return tid;
    }

    TypeId
    LongGrouper::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::LongGrouper")
                .SetParent<Grouper>()
        ;

        return tid;
    }

    TypeId
    DoubleGrouper::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::DoubleGrouper")
                .SetParent<Grouper>()
        ;

        return tid;
    }

    TypeId
    StringGrouper::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::StringGrouper")
                .SetParent<Grouper>()
        ;

        return tid;
    }

    TypeId
    Partitioner::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::Partitioner")
                .SetParent<PartialState>()
        ;

        return tid;
    }

    TypeId
    LongPartitioner::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::LongPartitioner")
                .SetParent<Partitioner>()
        ;

        return tid;
    }

    TypeId
    DoublePartitioner::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::DoublePartitioner")
                .SetParent<Partitioner>()
        ;

        return tid;
    }

    TypeId
    StringPartitioner::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::StringPartitioner")
                .SetParent<Partitioner>()
        ;

        return tid;
    }

    Group::Group() {
        this->id = ++group_cnt;
        partition = rand() % (NUM_PARTITIONS+1);
    }

    Group::Group(int id, int partition) {
        this->id = id;
        this->partition = partition;
    }

    Ptr<Group> Group::copy(Ptr<GroupOperator> group_operator, bool shed_unessential_attributes) {
        if (this == Group::no_group) {
            return this;
        }

        auto group = CreateObject<Group>(id, partition);
        group->tuples = this->tuples;
        group->first_tuple_wrapper = CreateObject<TupleWrapper>(this->first_tuple_wrapper, shed_unessential_attributes, false);
        group->first_tuple_wrapper->group = group;
        for (auto internal_group : this->internal_groups) {
            auto new_group = internal_group->copy(group_operator, shed_unessential_attributes);
            group->internal_groups.push_back(new_group);
            //NS_ASSERT(!group_operator->group_map.contains(group->id));
            //group_operator->group_map[new_group->id] = new_group;
            if (group->first_tuple_wrapper->partition_group == internal_group) {
                group->first_tuple_wrapper->partition_group = new_group;
            }
        }

        group->double_values = this->double_values;
        group->long_values = this->long_values;
        group->string_values = this->string_values;

        group->key = this->key;

        return group;
    }

    Ptr<Grouper> LongGrouper::copy(Ptr<GroupOperator> group_operator, bool shed_unessential_attributes) {
        auto long_grouper = CreateObject<LongGrouper>(group_operator, 0);
        /*for (auto &[k, v] : this->longToGrouper) {
            long_grouper->longToGrouper[k] = v->copy(group_operator, shed_unessential_attributes);
        }
        if (first_partitioner != nullptr) {
            long_grouper->first_partitioner = this->first_partitioner->copy(group_operator, shed_unessential_attributes);
        }*/
        long_grouper->depth = this->depth;
        long_grouper->group = this->group->copy(group_operator, shed_unessential_attributes);
        long_grouper->groupOperator->group_map[long_grouper->group->id] = long_grouper->group;
        long_grouper->max_depth = this->max_depth;

        long_grouper->double_value = this->double_value;
        long_grouper->long_value = this->long_value;
        long_grouper->string_value = this->string_value;

        return long_grouper;
    }

    Ptr<Grouper> DoubleGrouper::copy(Ptr<GroupOperator> group_operator, bool shed_unessential_attributes) {
        auto double_grouper = CreateObject<DoubleGrouper>(group_operator, 0);
        /*for (auto &[k, v] : this->doubleToGrouper) {
            double_grouper->doubleToGrouper[k] = v->copy(group_operator, shed_unessential_attributes);
        }
        if (first_partitioner != nullptr) {
            double_grouper->first_partitioner = this->first_partitioner->copy(group_operator, shed_unessential_attributes);
        }*/
        double_grouper->depth = this->depth;
        double_grouper->group = this->group->copy(group_operator, shed_unessential_attributes);
        double_grouper->groupOperator->group_map[double_grouper->group->id] = double_grouper->group;
        double_grouper->max_depth = this->max_depth;

        double_grouper->double_value = this->double_value;
        double_grouper->long_value = this->long_value;
        double_grouper->string_value = this->string_value;

        return double_grouper;
    }

    Ptr<Grouper> StringGrouper::copy(Ptr<GroupOperator> group_operator, bool shed_unessential_attributes) {
        auto string_grouper = CreateObject<StringGrouper>(group_operator, 0);
        /*for (auto &[k, v] : this->stringToGrouper) {
            string_grouper->stringToGrouper[k] = v->copy(group_operator, shed_unessential_attributes);
        }
        if (first_partitioner != nullptr) {
            string_grouper->first_partitioner = this->first_partitioner->copy(group_operator, shed_unessential_attributes);
        }*/
        string_grouper->depth = this->depth;
        string_grouper->group = this->group->copy(group_operator, shed_unessential_attributes);
        string_grouper->groupOperator->group_map[string_grouper->group->id] = string_grouper->group;
        string_grouper->max_depth = this->max_depth;

        string_grouper->double_value = this->double_value;
        string_grouper->long_value = this->long_value;
        string_grouper->string_value = this->string_value;

        return string_grouper;
    }

    Ptr<Partitioner> LongPartitioner::copy(Ptr<GroupOperator> group_operator, bool shed_unessential_attributes) {
        auto long_partitioner = CreateObject<LongPartitioner>(group_operator, 0);
        /*for (auto &[k, v] : this->longToPartitioner) {
            long_partitioner->longToPartitioner[k] = v->copy(group_operator, shed_unessential_attributes);
        }*/
        long_partitioner->depth = this->depth;
        long_partitioner->group = this->group->copy(group_operator, shed_unessential_attributes);
        long_partitioner->groupOperator->group_map[long_partitioner->group->id] = long_partitioner->group;
        long_partitioner->max_depth = this->max_depth;

        long_partitioner->double_value = this->double_value;
        long_partitioner->long_value = this->long_value;
        long_partitioner->string_value = this->string_value;

        return long_partitioner;
    }

    Ptr<Partitioner> DoublePartitioner::copy(Ptr<GroupOperator> group_operator, bool shed_unessential_attributes) {
        auto double_partitioner = CreateObject<DoublePartitioner>(group_operator, 0);
        /*for (auto &[k, v] : this->doubleToPartitioner) {
            double_partitioner->doubleToPartitioner[k] = v->copy(group_operator, shed_unessential_attributes);
        }*/
        double_partitioner->depth = this->depth;
        double_partitioner->group = this->group->copy(group_operator, shed_unessential_attributes);
        double_partitioner->groupOperator->group_map[double_partitioner->group->id] = double_partitioner->group;
        double_partitioner->max_depth = this->max_depth;

        double_partitioner->double_value = this->double_value;
        double_partitioner->long_value = this->long_value;
        double_partitioner->string_value = this->string_value;

        return double_partitioner;
    }

    Ptr<Partitioner> StringPartitioner::copy(Ptr<GroupOperator> group_operator, bool shed_unessential_attributes) {
        auto string_partitioner = CreateObject<StringPartitioner>(group_operator, 0);
        /*for (auto &[k, v] : this->stringToPartitioner) {
            string_partitioner->stringToPartitioner[k] = v->copy(group_operator, shed_unessential_attributes);
        }*/
        string_partitioner->depth = this->depth;
        string_partitioner->group = this->group->copy(group_operator, shed_unessential_attributes);
        string_partitioner->groupOperator->group_map[string_partitioner->group->id] = string_partitioner->group;
        string_partitioner->max_depth = this->max_depth;

        string_partitioner->double_value = this->double_value;
        string_partitioner->long_value = this->long_value;
        string_partitioner->string_value = this->string_value;

        return string_partitioner;
    }

    size_t Grouper::GetSerializedSize() {return 1000;}
    size_t Grouper::GetUtility() {return 1;}
    size_t Partitioner::GetSerializedSize() {return 1000;}
    size_t Partitioner::GetUtility() {return 1;}

    GroupOperator::GroupOperator(std::vector <std::string> group_keys) : Operator(groupType) {
        this->group_keys = std::move(group_keys);
    }

    void GroupOperator::init() {
        group_map[Group::no_group->id] = Group::no_group;

        if (group_keys.empty()) {
            first_grouper = CreateObject<NoGrouper>(this, 0);
        }
        for (auto key: group_keys) {
            // Find stream ID of key to find the tuple index
            auto stream_ids = this->stream->input_stream_ids;
            auto sf = SelectField::createSf(key, query);
            auto stream_id = sf->input_stream_id;

            for (std::size_t j = 0; j < stream_ids.size(); j++) {
                if (stream_id == stream_ids[j]) {
                    tuple_indexes.push_back(j);
                    break;
                }
            }
            NS_ASSERT(!tuple_indexes.empty());
            Type type = Tuple::streamIdToFieldToType[stream_id][key];
            group_types.push_back(type);
            switch (type) {
                case NOTYPE:
                    break;
                        case TIMESTAMP:
                        case LONG:
                            if (!first_grouper) {
                                first_grouper = CreateObject<LongGrouper>(this, 0);
                            }
                            depth_indexes.push_back(SelectField::getFieldIndex(sf->alias, key, query));
                            break;
                        case DOUBLE:
                            if (!first_grouper) {
                                first_grouper = CreateObject<DoubleGrouper>(this, 0);
                            }
                            depth_indexes.push_back(SelectField::getFieldIndex(sf->alias, key, query));
                            break;
                        case STRING:
                            if (!first_grouper) {
                                first_grouper = CreateObject<StringGrouper>(this, 0);
                            }
                            depth_indexes.push_back(SelectField::getFieldIndex(sf->alias, key, query));
                            break;
            }
        }
    }

    void GroupOperator::init_partitions(Ptr<SelectOperator> select_operator) {
        // TODO: Create partitions IFF select_operator selects attributes that are not grouped on
        // TODO: Use the group_keys to compare the selected fields in select_operator
        // TODO: If an attribute is NOT in group_keys, Partition objects must be created to cover
        // TODO: that attribute

        // TODO: first build up the partition_keys
        for (auto sf: select_operator->fields) {
            bool found = false;
            auto select_field = sf->inputField;
            auto dot_position = select_field.find('.');
            if (dot_position != std::string::npos) {
                select_field = select_field.substr(dot_position + 1, select_field.size() - dot_position);
            }
            for (auto group_key : group_keys) {
                auto group_field = group_key;
                dot_position = group_field.find('.');
                if (dot_position != std::string::npos) {
                    group_field = group_field.substr(dot_position + 1, group_field.size() - dot_position);
                }
                if (group_field == select_field) {
                    found = true;
                    break;
                }
            }
            if (!found && sf->aggregate == NONE) {
                partition_keys.emplace_back(select_field);
                partition_tuple_indexes.emplace_back(sf->tuple_index);
                auto type = Tuple::streamIdToFieldToType[sf->input_stream_id][select_field];
                partition_types.emplace_back(type);
                switch (type) {
                    case NOTYPE:
                        break;
                    case TIMESTAMP:
                    case LONG:
                        if (!first_partitioner) {
                            first_partitioner = CreateObject<LongPartitioner>(this, 0);
                        }
                        partition_depth_indexes.push_back(SelectField::getFieldIndex(sf->alias, select_field, query));
                        break;
                    case DOUBLE:
                        if (!first_partitioner) {
                            first_partitioner = CreateObject<DoublePartitioner>(this, 0);
                        }
                        partition_depth_indexes.push_back(SelectField::getFieldIndex(sf->alias, select_field, query));
                        break;
                    case STRING:
                        if (!first_partitioner) {
                            first_partitioner = CreateObject<StringPartitioner>(this, 0);
                        }
                        partition_depth_indexes.push_back(SelectField::getFieldIndex(sf->alias, select_field, query));
                        break;
                }
            }
        }

        /*for (int i = 0; i < partition_keys.size(); i++) {
                // Problem, because now we're selecting a field that's not grouped.
                // In a regular SQL relational database, this is grounds for error
                // Here, however, we convert this to a "partition by" scheme, where
                // the selected fields that aren't grouped yield an output tuple for
                // each unique combination of fields, and the aggregation is unique
                // for the group keys.
                // This results in SelectOperator into possibly multiple output tuples
                // with the same aggregate results, but with different select fields
                auto type = partition_types[i];
                switch (type) {
                  case NOTYPE:
                    break;
                  case TIMESTAMP:
                  case LONG:
                      if (!first_partitioner) {
                          first_partitioner = CreateObject<LongPartitioner>(this, 0);
                      }
                      partition_depth_indexes.push_back(SelectField::getFieldIndex(stream_id, field));
                      break;
                  case DOUBLE:
                      if (!first_partitioner) {
                          first_partitioner = CreateObject<DoublePartitioner>(this, 0);
                      }
                      partition_depth_indexes.push_back(SelectField::getFieldIndex(stream_id, field));
                      break;
                  case STRING:
                      if (!first_partitioner) {
                          first_partitioner = CreateObject<StringPartitioner>(this, 0);
                      }
                      partition_depth_indexes.push_back(SelectField::getFieldIndex(stream_id, field));
                      break;
                }
            }
        }*/
    }

    void GroupOperator::process(Ptr<TupleWrapper> tupleWrapper) {
        if (!first_grouper) {
            init();
        }
        first_grouper->groupTuple(tupleWrapper);
        //static int cnt = 0;
        //NS_LOG_INFO(Simulator::Now() << " Node " << this->thread->resourceManager->node_id << " groups tuple " << ++cnt);
        emit(tupleWrapper);
    }

    void GroupOperator::process(const std::vector<Ptr<Tuple> > &tuple) {

    }

    GroupedTuple::GroupedTuple() : Tuple() {
        this->stream_id = 0;
    }

    GroupedTuple::GroupedTuple(int stream_id, int id) : GroupedTuple() {
        this->stream_id = stream_id;
        this->id = id;
    }

    void Grouper::partitionTuple(Ptr<TupleWrapper> t) {
        if (first_partitioner == nullptr) {
            if (DynamicCast<LongPartitioner>(groupOperator->first_partitioner)) {
                first_partitioner = CreateObject<LongPartitioner>(groupOperator->first_partitioner);
            } else if (DynamicCast<DoublePartitioner>(groupOperator->first_partitioner)) {
                first_partitioner = CreateObject<DoublePartitioner>(groupOperator->first_partitioner);
            } else if (DynamicCast<StringPartitioner>(groupOperator->first_partitioner)) {
                first_partitioner = CreateObject<StringPartitioner>(groupOperator->first_partitioner);
            } else {
                NS_FATAL_ERROR("Invalid first partitioner type " << groupOperator->first_partitioner);
            }
        }
        first_partitioner->partitionTuple(t);
    }

    void LongGrouper::groupTuple(Ptr<TupleWrapper> t) {
        if (depth < max_depth) {
            int tuple_index = groupOperator->tuple_indexes[depth];
            auto tuple = t->tuples[tuple_index];
            int depth_index = groupOperator->depth_indexes[depth];
            long long_value = tuple->getLongValue(depth_index);
            if (longToGrouper.count(long_value) == 0) {
                switch (groupOperator->group_types[depth]) {
                    case LONG:
                        longToGrouper[long_value] = CreateObject<LongGrouper>(groupOperator, depth + 1, this, long_value);
                        break;
                    case DOUBLE:
                        longToGrouper[long_value] = CreateObject<DoubleGrouper>(groupOperator, depth + 1, this, long_value);
                        break;
                    case STRING:
                        longToGrouper[long_value] = CreateObject<StringGrouper>(groupOperator, depth + 1, this, long_value);
                        break;
                    default: {
                        NS_LOG_INFO(Simulator::Now() << " Unsupported group type " << groupOperator->group_types[depth]);
                        exit(112);
                    }
                }
            }
            longToGrouper[long_value]->groupTuple(t);
            return;
        }

        if (group == Group::no_group) {
            if (t->group == Group::no_group) {
                group = CreateObject<Group>();
            } else {
                group = t->group;
            }
            NS_ASSERT(!groupOperator->group_map.contains(group->id));
            groupOperator->group_map[group->id] = group;
            group->first_tuple_wrapper = t;
            group->first_tuple_wrapper->group = group;
            group->groupOperator = this->groupOperator;
            groupOperator->groups.emplace_back(group);
        }
        t->group = group;
        //t->group->tuples.emplace_back(t);

        if (!groupOperator->partition_depth_indexes.empty()) {
            // All partitions should be placed within t->group, such that we can iterate through them
            // These should be placed in internal_groups
            partitionTuple(t);
        }
    }

    void NoGrouper::groupTuple(Ptr<TupleWrapper> t) {
        t->group = Group::no_group;
        //t->group->tuples.emplace_back(t);

        if (!groupOperator->partition_depth_indexes.empty()) {
            // All partitions should be placed within t->group, such that we can iterate through them
            // These should be placed in internal_groups
            partitionTuple(t);
        }
    }

    void LongPartitioner::partitionTuple(Ptr<TupleWrapper> t) {
        if (depth < max_depth) {
            int tuple_index = groupOperator->partition_tuple_indexes[depth];
            long long_value = t->tuples[tuple_index]->getLongValue(groupOperator->partition_depth_indexes[depth]);
            if (longToPartitioner.count(long_value) == 0) {
                switch (groupOperator->partition_types[depth]) {
                    case LONG:
                        longToPartitioner[long_value] = CreateObject<LongPartitioner>(groupOperator, depth + 1, long_value);
                        break;
                    case DOUBLE:
                        longToPartitioner[long_value] = CreateObject<DoublePartitioner>(groupOperator, depth + 1, long_value);
                        break;
                    case STRING:
                        longToPartitioner[long_value] = CreateObject<StringPartitioner>(groupOperator, depth + 1, long_value);
                        break;
                    default: {
                        NS_LOG_INFO(Simulator::Now() << " Unsupported group type " << groupOperator->partition_types[depth]);
                        exit(112);
                    }
                }
            }
            longToPartitioner[long_value]->partitionTuple(t);
            return;
        }

        if (group == Group::no_group) {
            if (t->partition_group == Group::no_group) {
                group = CreateObject<Group>();
            } else {
                group = t->partition_group;
            }
            NS_ASSERT(!groupOperator->group_map.contains(group->id));
            groupOperator->group_map[group->id] = group;
            group->first_tuple_wrapper = t;
            group->first_tuple_wrapper->partition_group = group;
            group->groupOperator = this->groupOperator;
            groupOperator->partitions.emplace_back(group);
        }
        t->group->internal_groups.emplace_back(group);
        t->partition_group = group;
        //t->partition_group->tuples.emplace_back(t);

        //return group;
    }

    void DoubleGrouper::groupTuple(Ptr<TupleWrapper> t) {
        if (depth < max_depth) {
            int tuple_index = groupOperator->tuple_indexes[depth];
            double double_value = t->tuples[tuple_index]->getDoubleValue(groupOperator->depth_indexes[depth]);
            if (doubleToGrouper.count(double_value) == 0) {
                switch (groupOperator->group_types[depth]) {
                    case LONG:
                        doubleToGrouper[double_value] = CreateObject<LongGrouper>(groupOperator, depth + 1, this, double_value);
                        break;
                    case DOUBLE:
                        doubleToGrouper[double_value] = CreateObject<DoubleGrouper>(groupOperator, depth + 1, this, double_value);
                        break;
                    case STRING:
                        doubleToGrouper[double_value] = CreateObject<StringGrouper>(groupOperator, depth + 1, this, double_value);
                        break;
                    default: {
                        NS_LOG_INFO(Simulator::Now() << " Unsupported group type " << groupOperator->group_types[depth]);
                        exit(112);
                    }
                }
            }
            doubleToGrouper[double_value]->groupTuple(t);
            return;
        }

        if (group == Group::no_group) {
            if (t->group == Group::no_group) {
                group = CreateObject<Group>();
            } else {
                group = t->group;
            }
            NS_ASSERT(!groupOperator->group_map.contains(group->id));
            groupOperator->group_map[group->id] = group;
            group->first_tuple_wrapper = t;
            group->first_tuple_wrapper->group = group;
            group->groupOperator = this->groupOperator;
            groupOperator->groups.emplace_back(group);
        }
        t->group = group;
        //t->group->tuples.emplace_back(t);

        if (!groupOperator->partition_depth_indexes.empty()) {
            partitionTuple(t);
        }
    }

    void DoublePartitioner::partitionTuple(Ptr<TupleWrapper> t) {
        if (depth < max_depth) {
            int tuple_index = groupOperator->partition_tuple_indexes[depth];
            double double_value = t->tuples[tuple_index]->getDoubleValue(groupOperator->partition_depth_indexes[depth]);
            if (doubleToPartitioner.count(double_value) == 0) {
                switch (groupOperator->partition_types[depth]) {
                    case LONG:
                        doubleToPartitioner[double_value] = CreateObject<LongPartitioner>(groupOperator, depth + 1, double_value);
                        break;
                    case DOUBLE:
                        doubleToPartitioner[double_value] = CreateObject<DoublePartitioner>(groupOperator, depth + 1, double_value);
                        break;
                    case STRING:
                        doubleToPartitioner[double_value] = CreateObject<StringPartitioner>(groupOperator, depth + 1, double_value);
                        break;
                    default: {
                        NS_LOG_INFO(Simulator::Now() << " Unsupported group type " << groupOperator->partition_types[depth]);
                        exit(112);
                    }
                }
            }
            doubleToPartitioner[double_value]->partitionTuple(t);
            return;
        }

        if (group == Group::no_group) {
            if (t->partition_group == Group::no_group) {
                group = CreateObject<Group>();
            } else {
                group = t->partition_group;
            }
            NS_ASSERT(!groupOperator->group_map.contains(group->id));
            groupOperator->group_map[group->id] = group;
            group->first_tuple_wrapper = t;
            group->first_tuple_wrapper->partition_group = group;
            group->groupOperator = this->groupOperator;
            groupOperator->partitions.emplace_back(group);
        }
        t->group->internal_groups.emplace_back(group);
        t->partition_group = group;
        //t->partition_group->tuples.emplace_back(t);

        //return group;
    }

    void StringGrouper::groupTuple(Ptr<TupleWrapper> t) {
        if (depth < max_depth) {
            int tuple_index = groupOperator->tuple_indexes[depth];
            std::string string_value = t->tuples[tuple_index]->getStringValue(groupOperator->depth_indexes[depth]);
            if (stringToGrouper.count(string_value) == 0) {
                switch (groupOperator->group_types[depth]) {
                    case LONG:
                        stringToGrouper[string_value] = CreateObject<LongGrouper>(groupOperator, depth + 1, this, string_value);
                        break;
                    case DOUBLE:
                        stringToGrouper[string_value] = CreateObject<DoubleGrouper>(groupOperator, depth + 1, this, string_value);
                        break;
                    case STRING:
                        stringToGrouper[string_value] = CreateObject<StringGrouper>(groupOperator, depth + 1, this, string_value);
                        break;
                    default: {
                        NS_LOG_INFO(Simulator::Now() << " Unsupported group type " << groupOperator->group_types[depth]);
                        exit(112);
                    }
                }
            }
            stringToGrouper[string_value]->groupTuple(t);
            return;
        }

        if (group == Group::no_group) {
            if (t->group == Group::no_group) {
                group = CreateObject<Group>();
            } else {
                group = t->group;
            }
            NS_ASSERT(!groupOperator->group_map.contains(group->id));
            groupOperator->group_map[group->id] = group;
            group->first_tuple_wrapper = t;
            group->first_tuple_wrapper->group = group;
            group->groupOperator = this->groupOperator;
            groupOperator->groups.emplace_back(group);
        }

        t->group = group;
        //t->group->tuples.emplace_back(t);

        if (!groupOperator->partition_depth_indexes.empty()) {
            partitionTuple(t);
        }
    }

    void StringPartitioner::partitionTuple(Ptr<TupleWrapper> t) {
        if (depth < max_depth) {
            int tuple_index = groupOperator->partition_tuple_indexes[depth];
            std::string string_value = t->tuples[tuple_index]->getStringValue(groupOperator->partition_depth_indexes[depth]);
            if (stringToPartitioner.count(string_value) == 0) {
                switch (groupOperator->partition_types[depth]) {
                    case LONG:
                        stringToPartitioner[string_value] = CreateObject<LongPartitioner>(groupOperator, depth + 1, string_value);
                        break;
                    case DOUBLE:
                        stringToPartitioner[string_value] = CreateObject<DoublePartitioner>(groupOperator, depth + 1, string_value);
                        break;
                    case STRING:
                        stringToPartitioner[string_value] = CreateObject<StringPartitioner>(groupOperator, depth + 1, string_value);
                        break;
                    default: {
                        NS_LOG_INFO(Simulator::Now() << " Unsupported group type " << groupOperator->partition_types[depth]);
                        exit(112);
                    }
                }
            }
            stringToPartitioner[string_value]->partitionTuple(t);
            return;
        }

        if (group == Group::no_group) {
            if (t->partition_group == Group::no_group) {
                group = CreateObject<Group>();
            } else {
                group = t->partition_group;
            }
            NS_ASSERT(!groupOperator->group_map.contains(group->id));
            groupOperator->group_map[group->id] = group;
            group->first_tuple_wrapper = t;
            group->first_tuple_wrapper->partition_group = group;
            group->groupOperator = this->groupOperator;
            groupOperator->partitions.emplace_back(group);
        }

        t->group->internal_groups.emplace_back(group);
        t->partition_group = group;
        //t->partition_group->tuples.emplace_back(t);

        //return group;
    }


    Grouper::Grouper(Ptr<GroupOperator> groupOperator, int depth, Ptr<Grouper> previous_grouper, long long_value) {
        this->groupOperator = groupOperator;
        this->depth = depth;
        this->max_depth = groupOperator->group_types.size();
        //this->previous_grouper = previous_grouper;
        this->long_value = long_value;
    }

    Grouper::Grouper(Ptr<GroupOperator> groupOperator, int depth, Ptr<Grouper> previous_grouper, double double_value) {
        this->groupOperator = groupOperator;
        this->depth = depth;
        this->max_depth = groupOperator->group_types.size();
        //this->previous_grouper = previous_grouper;
        this->double_value = double_value;
    }

    Grouper::Grouper(Ptr<GroupOperator> groupOperator, int depth, Ptr<Grouper> previous_grouper, std::string string_value) {
        this->groupOperator = groupOperator;
        this->depth = depth;
        this->max_depth = groupOperator->group_types.size();
        //this->previous_grouper = previous_grouper;
        this->string_value = string_value;
    }

    Partitioner::Partitioner(Ptr<GroupOperator> groupOperator, int depth, long long_value) {
        this->groupOperator = groupOperator;
        this->depth = depth;
        this->max_depth = groupOperator->partition_types.size();
        //this->previous_grouper = previous_grouper;
        this->long_value = long_value;
    }

    Partitioner::Partitioner(Ptr<GroupOperator> groupOperator, int depth, double double_value) {
        this->groupOperator = groupOperator;
        this->depth = depth;
        this->max_depth = groupOperator->partition_types.size();
        //this->previous_grouper = previous_grouper;
        this->double_value = double_value;
    }

    Partitioner::Partitioner(Ptr<GroupOperator> groupOperator, int depth, std::string string_value) {
        this->groupOperator = groupOperator;
        this->depth = depth;
        this->max_depth = groupOperator->partition_types.size();
        //this->previous_grouper = previous_grouper;
        this->string_value = string_value;
    }

    int GroupOperator::calculateMigrationSize(migration_types migration_type) {
        return 1;
    }

    Ptr<PartialStateBatch> GroupOperator::ExtractPreambleState(Ptr<MigrationPlan> mp) {
        auto batch = Operator::ExtractPreambleState(mp);
        if (mp->migration_mode == "rhino" || mp->migration_mode == "lazy") {
            doExtractState(mp);
        }
        return batch;
    }

    void GroupOperator::ExtractDiffState(Ptr<MigrationPlan> mp) {
        auto b = this->first_rhino_batch;
        auto new_b = CreateObject<PartialStateBatch>(this, mp);

        Ptr<PartialStateWrapper> ps_first_grouper = CreateObject<PartialStateWrapper>();
        ps_first_grouper->object = first_grouper;
        if (first_grouper != nullptr) {
            ps_first_grouper->size = first_grouper->GetSerializedSize();
        }
        new_b->partial_states.emplace_back(ps_first_grouper);

        for (auto &[group_id, group] : group_map) {
            if (group == Group::no_group) {
                continue;
            }
            if (std::find_if(b->partial_states.begin(), b->partial_states.end(), [group](Ptr<PartialStateWrapper> ps_wrapper) {return ps_wrapper->object == group;}) != std::end(b->partial_states)) {
                continue;
            }
            Ptr<PartialStateWrapper> ps_group = CreateObject<PartialStateWrapper>();
            auto group_copy = group->copy(this, false);
            group_copy->first_tuple_wrapper = CreateObject<TupleWrapper>(group_copy->first_tuple_wrapper, false, false);
            //group_map[group_id]->first_tuple_wrapper = CreateObject<TupleWrapper>(group_copy->first_tuple_wrapper, shed_unessential_attributes);
            ps_group->object = group_copy;
            ps_group->size = group_copy->GetSerializedSize() - group_copy->first_tuple_wrapper->GetSerializedSize(false) + CreateObject<TupleWrapper>(group_copy->first_tuple_wrapper, true, false)->GetSerializedSize(false);
            new_b->partial_states.emplace_back(ps_group);
        }

        mp->critQueue.emplace_back(new_b);
    }

    void GroupOperator::doExtractState(Ptr<MigrationPlan> mp) {
        auto batch = CreateObject<PartialStateBatch>(this, mp);
        Ptr<PartialStateWrapper> ps_first_grouper = CreateObject<PartialStateWrapper>();
        ps_first_grouper->object = first_grouper;
        if (first_grouper != nullptr) {
            ps_first_grouper->size = first_grouper->GetSerializedSize();
        }
        batch->partial_states.emplace_back(ps_first_grouper);

        bool shed_unessential_attributes = mp->migration_mode == "utility";
        for (auto &[group_id, group] : group_map) {
            if (group == Group::no_group) {
                continue;
            }
            Ptr<PartialStateWrapper> ps_group = CreateObject<PartialStateWrapper>();
            auto group_copy = group->copy(this, shed_unessential_attributes);
            ps_group->object = group_copy;
            group_copy->first_tuple_wrapper = CreateObject<TupleWrapper>(group_copy->first_tuple_wrapper, shed_unessential_attributes, false);
            if (mp->migration_mode == "utility") {
                //group_map[group_id]->first_tuple_wrapper = CreateObject<TupleWrapper>(group_copy->first_tuple_wrapper, shed_unessential_attributes);
                ps_group->size =
                        group_copy->GetSerializedSize() - group_copy->first_tuple_wrapper->GetSerializedSize(false) +
                        CreateObject<TupleWrapper>(group_copy->first_tuple_wrapper, true, false)->GetSerializedSize(false);
            }
            batch->partial_states.emplace_back(ps_group);
        }

        if (mp->migration_mode == "rhino" || mp->migration_mode == "lazy") {
            auto preamble_batch = mp->preambleQueue.back();
            NS_ASSERT(preamble_batch->op == this);
            for (auto ps_wrapper : batch->partial_states) {
                preamble_batch->partial_states.emplace_back(ps_wrapper);
            }
            if (mp->migration_mode == "rhino") {
                this->first_rhino_batch = batch;
            }
        } else {
            mp->critQueue.emplace_back(batch);
        }

        if (mp->migration_mode == "utility") {
            // Second batch for unessential attributes
            batch = CreateObject<PartialStateBatch>(this, mp);
            batch->partial_states.emplace_back(ps_first_grouper);

            for (auto &[group_id, group] : group_map) {
                if (group == Group::no_group) {
                    continue;
                }
                Ptr<PartialStateWrapper> ps_group = CreateObject<PartialStateWrapper>();
                auto group_copy = group->copy(this, false);
                //group_map[group_id]->first_tuple_wrapper = CreateObject<TupleWrapper>(group_copy->first_tuple_wrapper, shed_unessential_attributes);
                ps_group->object = group_copy;
                // TODO: add the size of the unessential attributes, and not just all the attributes
                // We're subtracting the size of the essential attributes of the first tuple wrapper, because it's already migrated
                ps_group->size = group_copy->GetSerializedSize() - CreateObject<TupleWrapper>(group_copy->first_tuple_wrapper, true, false)->GetSerializedSize(false);
                batch->partial_states.emplace_back(ps_group);
            }

            std::cout << "Size of unessential batch: " << batch->GetSerializedSize() << std::endl;
            mp->normalUnessentialQueue.emplace_back(batch);
        }
    }

    void GroupOperator::ExtractState(Ptr<MigrationPlan> mp) {
        if (mp->migration_mode != "rhino" && mp->migration_mode != "lazy") {
            doExtractState(mp);
        }
    }

    // Get size of Group
    size_t Group::GetSerializedSize() {
        size_t size = 0;
        if (this->first_tuple_wrapper != nullptr) {
            size += this->first_tuple_wrapper->GetSerializedSize(false);
        }
        size += sizeof(this->id);
        size += sizeof(this->partition);
        return size;
    }

    Ptr<Group> GroupOperator::GetGroup(int group_id) {
        return first_grouper->GetGroup(group_id);
    }

    Ptr<Group> LongGrouper::GetGroup(int group_id) {
        if (group != Group::no_group) {
            if (group->id == group_id) {
                return group;
            }
            if (first_partitioner != nullptr) {
                return first_partitioner->GetGroup(group_id);
            }
            return nullptr;
        }
        for (auto &[k, v] : longToGrouper) {
            auto g = v->GetGroup(group_id);
            if (g != nullptr) {
                return g;
            }
        }

        return nullptr;
    }

    Ptr<Group> LongPartitioner::GetGroup(int group_id) {
        if (group != Group::no_group) {
            if (group->id == group_id) {
                return group;
            }
            return nullptr;
        }
        for (auto &[k, v] : longToPartitioner) {
            auto g = v->GetGroup(group_id);
            if (g != nullptr) {
                return g;
            }
        }

        return nullptr;
    }

    Ptr<Group> DoubleGrouper::GetGroup(int group_id) {
        if (group != Group::no_group) {
            if (group->id == group_id) {
                return group;
            }
            if (first_partitioner != nullptr) {
                return first_partitioner->GetGroup(group_id);
            }
            return nullptr;
        }
        for (auto &[k, v] : doubleToGrouper) {
            auto g = v->GetGroup(group_id);
            if (g != nullptr) {
                return g;
            }
        }

        return nullptr;
    }

    Ptr<Group> DoublePartitioner::GetGroup(int group_id) {
        if (group != Group::no_group) {
            if (group->id == group_id) {
                return group;
            }
            return nullptr;
        }
        for (auto &[k, v] : doubleToPartitioner) {
            auto g = v->GetGroup(group_id);
            if (g != nullptr) {
                return g;
            }
        }

        return nullptr;
    }

    Ptr<Group> StringGrouper::GetGroup(int group_id) {
        if (group != Group::no_group) {
            if (group->id == group_id) {
                return group;
            }
            if (first_partitioner != nullptr) {
                return first_partitioner->GetGroup(group_id);
            }
            return nullptr;
        }
        for (auto &[k, v] : stringToGrouper) {
            auto g = v->GetGroup(group_id);
            if (g != nullptr) {
                return g;
            }
        }

        return nullptr;
    }

    Ptr<Group> StringPartitioner::GetGroup(int group_id) {
        if (group != Group::no_group) {
            if (group->id == group_id) {
                return group;
            }
            return nullptr;
        }
        for (auto &[k, v] : stringToPartitioner) {
            auto g = v->GetGroup(group_id);
            if (g != nullptr) {
                return g;
            }
        }

        return nullptr;
    }

    void GroupOperator::ImportState(Ptr<PartialStateBatch> batch) {
        if (batch->partial_states.empty()) {
            return;
        }
        auto op = DynamicCast<GroupOperator>(batch->partial_states[0]->object);
        int offset = 0;
        if (op != nullptr) {
            // Static state
            // TODO: Include parameters such as current group ID counter, etc.
            this->depth_indexes = op->depth_indexes;
            this->partition_depth_indexes = op->partition_depth_indexes;
            this->tuple_indexes = op->tuple_indexes;
            this->group_keys = op->group_keys;
            this->partition_keys = op->partition_keys;
            if (batch->partial_states.size() == 1) {
                return;
            }
            offset = 1;
        }
        auto original_first_grouper = DynamicCast<Grouper>(batch->partial_states[offset + 0]->object);
        bool shed_unessential_attributes = ++original_first_grouper->current_migration_phase < batch->mp->operatorIdToNumberPhases[this->id] && batch->mp->migration_mode == "utility";
        int cnt = 0;
        if (batch->mp->migration_mode == "utility") {
            if (shed_unessential_attributes) {
                first_grouper = original_first_grouper->copy(this, shed_unessential_attributes);
                for (int i = offset+1; i < (int) batch->partial_states.size(); i++) {
                    auto group = DynamicCast<Group>(batch->partial_states[i]->object);
                    if (group == Group::no_group) {
                        continue;
                    }
                    auto group_copy = group->copy(this, false);
                    // Step 1: Create a group based on the original tuple
                    first_grouper->groupTuple(CreateObject<TupleWrapper>(group_copy->first_tuple_wrapper));
                    // Step 2: Strip the group's first tuple wrapper's unessential attributes
                    group_map[group->id]->first_tuple_wrapper = CreateObject<TupleWrapper>(
                            group_copy->first_tuple_wrapper, shed_unessential_attributes, false);
                }
            } else {
                // TODO: Update the first_tuple_wrapper for all groups
                for (auto &[group_id, updated_group] : original_first_grouper->groupOperator->group_map) {
                    if (updated_group == Group::no_group) {
                        continue;
                    }
                    auto group_to_update = this->group_map[group_id];
                    group_to_update->first_tuple_wrapper = CreateObject<TupleWrapper>(updated_group->first_tuple_wrapper);
                    std::cout << "Completing group " << ++cnt << std::endl;
                }
            }
        } else {
            // TODO: Update the first_tuple_wrapper for all groups
            for (int i = offset + 1; i < (int) batch->partial_states.size(); i++) {
                auto group = DynamicCast<Group>(batch->partial_states[i]->object);
                if (group == Group::no_group) {
                    continue;
                }
                // We create a copy of the group, group a copy of the first tuple wrapper, and assert that our group map points to this group with the expected group ID
                auto group_copy = group->copy(this, false);
                first_grouper->groupTuple(CreateObject<TupleWrapper>(group_copy->first_tuple_wrapper));
                // If this assertion fails, it probably means that the GroupOperator has created a new group for the set of keys that it is represented by. This would mean that the migration has failed.
                //NS_ASSERT(group_map[group->id] != nullptr);
            }
        }
        /*Ptr<Grouper> grouper = DynamicCast<LongGrouper>(original_first_grouper);
        if (grouper == nullptr) {
            grouper = DynamicCast<DoubleGrouper>(original_first_grouper);
            if (grouper == nullptr) {
                grouper = DynamicCast<StringGrouper>(original_first_grouper);
                NS_ASSERT(grouper != nullptr);
                first_grouper = CreateObject<StringGrouper>(this, 0);
            } else {
                first_grouper = CreateObject<DoubleGrouper>(this, 0);
            }
        } else {
            first_grouper = CreateObject<LongGrouper>(this, 0);
        }

        // Copy attributes of grouper into first_grouper
        first_grouper->depth = grouper->depth;
        first_grouper->max_depth = grouper->max_depth;
        first_grouper->long_value = grouper->long_value;
        first_grouper->double_value = grouper->double_value;
        first_grouper->string_value = grouper->string_value;*/
    }

    Time GroupOperator::get_base_processing_time() {
        return Operator::get_base_processing_time();
        //return MilliSeconds(0);
    }

    Time GroupOperator::get_emit_time_added() {
        return Operator::get_emit_time_added();
        //return MilliSeconds(0);
    }

    Time GroupOperator::get_forward_time_added() {
        return Operator::get_forward_time_added();
        //return MilliSeconds(0);
    }
}
