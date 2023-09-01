//
// Created by espen on 26.10.2021.
//

#include <yaml-cpp/yaml.h>
#include "ns3/Tuple.h"
#include "ns3/Operator.h"


namespace ns3 {
    TypeId
    Tuple::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::Tuple")
                .SetParent<Object>()
                .AddConstructor<Tuple>()
        ;

        return tid;
    }

    TypeId
    GroupedTuple::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::GroupedTuple")
                .SetParent<Tuple>()
                .AddConstructor<GroupedTuple>()
        ;

        return tid;
    }

    TypeId
    TupleWrapper::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::TupleWrapper")
                .SetParent<PartialState>()
                .AddConstructor<TupleWrapper>()
        ;

        return tid;
    }

    void Tuple::setFieldToValueOfTuple(Ptr<SelectField> sf, Ptr<Tuple> t) {
        switch (sf->type) {
            case TIMESTAMP:
            case LONG: {
                if ((int) t->longValues.size() <= sf->inputFieldIndex) {
                    // We assume this value has been shed and ignore it
                    return;
                }
                addFieldValue(t->getLongValue(sf->inputFieldIndex));
                break;
            }
            case DOUBLE: {
                if ((int) t->doubleValues.size() <= sf->inputFieldIndex) {
                    // We assume this value has been shed and ignore it
                    return;
                }
                addFieldValue(t->getDoubleValue(sf->inputFieldIndex));
                break;
            }
            case STRING: {
                if ((int) t->stringValues.size() <= sf->inputFieldIndex) {
                    // We assume this value has been shed and ignore it
                    return;
                }
                addFieldValue(t->getStringValue(sf->inputFieldIndex));
                break;
            }
            default: {

            }
        }
    }

    bool Tuple::is_essential_long_attribute(int index) {
        int long_index = -1;

        for (auto attr: this->tupleWrapper->query->aliasToEssentialAttributes[alias]) {
            auto type = Tuple::streamIdToFieldToType[this->stream_id][attr];
            switch (type) {
                case LONG:
                    ++long_index;
                    if (long_index == index) {
                        break;
                    }
                    break;
                case NOTYPE:
                case DOUBLE:
                case STRING:
                case TIMESTAMP:
                    break;
            }
        }
        return long_index < (int) this->longValues.size();
    }

    void TupleWrapper::add_tuple(Ptr<Tuple> t) {
        t->tupleWrapper = this;
        this->tuples.emplace_back(t);
    }

    bool Tuple::is_essential_double_attribute(int index) {
        int double_index = -1;

        for (auto attr: this->tupleWrapper->query->aliasToEssentialAttributes[alias]) {
            auto type = Tuple::streamIdToFieldToType[this->stream_id][attr];
            switch (type) {
                case DOUBLE:
                    ++double_index;
                    if (double_index == index) {
                        break;
                    }
                    break;
                case NOTYPE:
                case LONG:
                case STRING:
                case TIMESTAMP:
                    break;
            }
        }
        return double_index < (int) this->doubleValues.size();
    }

    bool Tuple::is_essential_string_attribute(int index) {
        int string_index = -1;

        for (auto attr: this->tupleWrapper->query->aliasToEssentialAttributes[alias]) {
            auto type = Tuple::streamIdToFieldToType[this->stream_id][attr];
            switch (type) {
                case STRING:
                    ++string_index;
                    if (string_index == index) {
                        break;
                    }
                    break;
                case NOTYPE:
                case LONG:
                case DOUBLE:
                case TIMESTAMP:
                    break;
            }
        }
        return string_index < (int) this->stringValues.size();
    }

    long Tuple::getLongValue(int index) {
        if (index < (int) longValues.size()) {
            return longValues[index];
        }
        NS_ASSERT(!is_essential_long_attribute(index));
        return -1;
    }

    double Tuple::getDoubleValue(int index) {
        if (index < (int) doubleValues.size()) {
            return doubleValues[index];
        }
        NS_ASSERT(!is_essential_double_attribute(index));
        return -1;
    }

    std::string Tuple::getStringValue(int index) {
        if (index < (int) stringValues.size()) {
            return stringValues[index];
        }
        NS_ASSERT(!is_essential_string_attribute(index));
        return "";
    }

    std::ostream &operator<<(std::ostream &os, Tuple &tuple) {
        int string_index = 0;
        int long_index = 0;
        int double_index = 0;

        for (auto const&[key, val]: Tuple::streamIdToFieldToType[tuple.stream_id]) {
            if (val == STRING) {
                if ((int) tuple.stringValues.size() <= string_index) {
                    continue;
                }
                os << key << ": " << tuple.getStringValue(string_index++) << ", ";
            } else if (val == LONG || val == TIMESTAMP) {
                if ((int) tuple.longValues.size() <= long_index) {
                    continue;
                }
                os << key << ": " << tuple.longValues[long_index++] << ", ";
            } else if (val == DOUBLE) {
                if ((int) tuple.doubleValues.size() <= double_index) {
                    continue;
                }
                os << key << ": " << tuple.doubleValues[double_index++] << ", ";
            }
        }

        return os;
    }

    std::ostream &operator<<(std::ostream &os, const TupleWrapper &tupleWrapper) {
        auto group = tupleWrapper.group;

        if (group) {
            os << "Group " << group->id << ", ";
        }
        for (auto &tuple : tupleWrapper.tuples) {
            os << *tuple;
        }

        return os;
    }

    void Tuple::decrementRef() {
        --reference_counter;
    }

    Tuple::Tuple(Ptr<Tuple> t) {
        this->reference_counter = t->reference_counter;
        this->evicted = t->evicted;
        this->id = t->id;
        this->alias = t->alias;
        this->stream_id = t->stream_id;
        this->stringValues = t->stringValues;
        this->longValues = t->longValues;
        this->doubleValues = t->doubleValues;
    }

    int Tuple::get_index(int stream_id, std::string alias, std::string field, Ptr<StreamQuery> q) {
        auto sid = q->aliasToStreamId[alias];
        int long_index = -1;
        int string_index = -1;
        int double_index = -1;
        for (auto attr: q->aliasToEssentialAttributes[alias]) {
            auto type = Tuple::streamIdToFieldToType[sid][attr];
            switch (type) {
                case NOTYPE:
                    break;
                case LONG:
                    ++long_index;
                    break;
                case DOUBLE:
                    ++double_index;
                    break;
                case STRING:
                    ++string_index;
                    break;
                case TIMESTAMP:
                    ++long_index;
                    break;
            }
            if (field == attr) {
                switch (type) {
                    case NOTYPE:
                        break;
                    case TIMESTAMP:
                    case LONG:
                        return long_index;
                    case DOUBLE:
                        return double_index;
                    case STRING:
                        return string_index;
                }
            }
        }

        for (auto attr: q->aliasToUnssentialAttributes[alias]) {
            auto type = Tuple::streamIdToFieldToType[sid][attr];
            switch (type) {
                case NOTYPE:
                    break;
                case LONG:
                    ++long_index;
                    break;
                case DOUBLE:
                    ++double_index;
                    break;
                case STRING:
                    ++string_index;
                    break;
                case TIMESTAMP:
                    ++long_index;
                    break;
            }
            if (field == attr) {
                switch (type) {
                    case NOTYPE:
                        break;
                    case TIMESTAMP:
                    case LONG:
                        return long_index;
                    case DOUBLE:
                        return double_index;
                    case STRING:
                        return string_index;
                }
            }
        }

        NS_FATAL_ERROR("Field " << field << " not found in stream " << stream_id);
    }

    int Tuple::get_index(int stream_id, std::string field) {
        int long_index = -1;
        int string_index = -1;
        int double_index = -1;
        for (int i = 0; i < (int) Tuple::streamIdToIndexToField[stream_id].size(); i++) {
            auto f = Tuple::streamIdToIndexToField[stream_id][i];
            auto type = Tuple::streamIdToFieldToType[stream_id][f];
            switch (type) {
                case NOTYPE:
                    break;
                case TIMESTAMP:
                case LONG:
                    ++long_index;
                    if (f == field) {
                        return long_index;
                    }
                    break;
                case DOUBLE:
                    ++double_index;
                    if (f == field) {
                        return double_index;
                    }
                    break;
                case STRING:
                    ++string_index;
                    if (f == field) {
                        return string_index;
                    }
                    break;
            }
        }

        NS_FATAL_ERROR("Field " << field << " not found in stream " << stream_id);
    }

    Tuple::Tuple(Ptr<Tuple> t, Ptr<StreamQuery> q, bool shed_unessential_attributes) {
        this->reference_counter = t->reference_counter;
        this->evicted = t->evicted;
        this->id = t->id;
        this->alias = t->alias;
        this->stream_id = t->stream_id;

        if (shed_unessential_attributes) {
            for (auto attr: q->aliasToEssentialAttributes[alias]) {
                auto type = Tuple::streamIdToFieldToType[t->stream_id][attr];
                switch (type) {
                    case NOTYPE:
                        break;
                    case TIMESTAMP:
                    case LONG:
                        this->longValues.emplace_back(t->longValues[this->longValues.size()]);
                        break;
                    case DOUBLE:
                        this->doubleValues.emplace_back(t->doubleValues[this->doubleValues.size()]);
                        break;
                    case STRING:
                        this->stringValues.emplace_back(t->stringValues[this->stringValues.size()]);
                        break;
                }
            }
        } else {
            this->longValues = t->longValues;
            this->doubleValues = t->doubleValues;
            this->stringValues = t->stringValues;
        }
    }

    Tuple::Tuple(Ptr<Tuple> t, Ptr<StreamQuery> q) {
        this->reference_counter = t->reference_counter;
        this->evicted = t->evicted;
        this->id = t->id;
        this->stream_id = t->stream_id;
        this->alias = t->alias;

        // We assign the attributes in order of essential to unessential attributes, according to the query
        PopulateTupleWithQuery(t, q);
        //this->stringValues = t->stringValues;
        //this->longValues = t->longValues;
        //this->doubleValues = t->doubleValues;
    }

    void Tuple::PopulateTupleWithQuery(Ptr<Tuple> t, Ptr<StreamQuery> q) {
        auto sid = q->aliasToStreamId[t->alias];
        for (auto attr: q->aliasToEssentialAttributes[t->alias]) {
            int long_index = -1;
            int string_index = -1;
            int double_index = -1;
            for (int i = 0; i < (int) Tuple::streamIdToIndexToField[sid].size(); i++) {
                auto f = Tuple::streamIdToIndexToField[sid][i];
                auto type = Tuple::streamIdToFieldToType[sid][f];
                switch (type) {
                    case NOTYPE:
                        break;
                    case LONG:
                        ++long_index;
                        break;
                    case DOUBLE:
                        ++double_index;
                        break;
                    case STRING:
                        ++string_index;
                        break;
                    case TIMESTAMP:
                        ++long_index;
                        break;
                }
                if (f == attr) {
                    switch (type) {
                        case NOTYPE:
                            break;
                        case LONG:
                            this->longValues.emplace_back(t->longValues[long_index]);
                            break;
                        case DOUBLE:
                            this->doubleValues.emplace_back(t->doubleValues[double_index]);
                            break;
                        case STRING:
                            this->stringValues.emplace_back(t->stringValues[string_index]);
                            break;
                        case TIMESTAMP:
                            this->longValues.emplace_back(t->longValues[long_index]);
                            break;
                    }
                    break;
                }
            }
        }

        for (auto attr: q->aliasToUnssentialAttributes[t->alias]) {
            int long_index = -1;
            int string_index = -1;
            int double_index = -1;
            for (int i = 0; i < (int) Tuple::streamIdToIndexToField[sid].size(); i++) {
                auto f = Tuple::streamIdToIndexToField[sid][i];
                auto type = Tuple::streamIdToFieldToType[sid][f];
                switch (type) {
                    case NOTYPE:
                        break;
                    case LONG:
                        ++long_index;
                        break;
                    case DOUBLE:
                        ++double_index;
                        break;
                    case STRING:
                        ++string_index;
                        break;
                    case TIMESTAMP:
                        ++long_index;
                        break;
                }
                if (f == attr) {
                    switch (type) {
                        case NOTYPE:
                            break;
                        case LONG:
                            this->longValues.emplace_back(t->longValues[long_index]);
                            break;
                        case DOUBLE:
                            this->doubleValues.emplace_back(t->doubleValues[double_index]);
                            break;
                        case STRING:
                            this->stringValues.emplace_back(t->stringValues[string_index]);
                            break;
                        case TIMESTAMP:
                            this->longValues.emplace_back(t->longValues[long_index]);
                            break;
                    }
                    break;
                }
            }
        }
    }

    std::tuple<int, int, int> Tuple::GetShedSizes(Ptr<StreamQuery> q) const {
        auto sid = q->aliasToStreamId[alias];
        int max_long_index = -1;
        int max_double_index = -1;
        int max_string_index = -1;
        for (auto attr: q->aliasToEssentialAttributes[alias]) {
            int long_index = -1;
            int string_index = -1;
            int double_index = -1;
            for (int i = 0; i < (int) Tuple::streamIdToIndexToField[sid].size(); i++) {
                auto f = Tuple::streamIdToIndexToField[sid][i];
                auto type = Tuple::streamIdToFieldToType[sid][f];
                switch (type) {
                    case NOTYPE:
                        break;
                    case LONG:
                        ++long_index;
                        break;
                    case DOUBLE:
                        ++double_index;
                        break;
                    case STRING:
                        ++string_index;
                        break;
                    case TIMESTAMP:
                        ++long_index;
                        break;
                }
                if (f == attr) {
                    switch (type) {
                        case NOTYPE:
                            break;
                        case LONG:
                            max_long_index = std::max(long_index, max_long_index);
                            break;
                        case DOUBLE:
                            max_double_index = std::max(double_index, max_double_index);
                            break;
                        case STRING:
                            max_string_index = std::max(string_index, max_string_index);
                            break;
                        case TIMESTAMP:
                            max_long_index = std::max(long_index, max_long_index);
                            break;
                    }
                    break;
                }
            }
        }

        //NS_ASSERT(max_long_index > -1 && max_double_index > -1 && max_string_index > -1);
        return {max_long_index + 1, max_double_index + 1, max_string_index + 1};
    }

    TupleWrapper::TupleWrapper(std::vector<Ptr<Tuple>> tuples) {
        for (auto t : tuples) {
            this->add_tuple(t);
        }
    }

    TupleWrapper::TupleWrapper(Ptr<TupleWrapper> tupleWrapper, Ptr<StreamQuery> query) {
        this->next_operator_id = tupleWrapper->next_operator_id;
        this->node = tupleWrapper->node;
        this->evicted = tupleWrapper->evicted;
        this->in_migration = tupleWrapper->in_migration;
        this->current_migration_phase = tupleWrapper->current_migration_phase;
        this->group_id = tupleWrapper->group_id;
        this->group = tupleWrapper->group;
        this->query = tupleWrapper->query;
        this->partition_group = tupleWrapper->partition_group;
        for (auto tuple : tupleWrapper->tuples) {
            auto t = CreateObject<Tuple>(tuple, query);
            t->tupleWrapper = this;
            this->add_tuple(t);
        }
    }

    void TupleWrapper::shed_unessential() {
        // Iterate through each tuple
        for (auto tuple : tuples) {
            int sid = tuple->stream_id;
            int long_index = -1;
            int string_index = -1;
            int double_index = -1;
            for (int i = 0; i < (int) Tuple::streamIdToIndexToField[sid].size(); i++) {
                bool shed_attribute = false;
                auto f = Tuple::streamIdToIndexToField[sid][i];
                auto type = Tuple::streamIdToFieldToType[sid][f];
                for (auto &[alias, unessential_attributes] : this->query->aliasToEssentialAttributes) {
                    for (auto unessential_attr : unessential_attributes) {
                        if (f == unessential_attr) {
                            shed_attribute = true;
                            break;
                        }
                    }
                }
                switch (type) {
                    case NOTYPE:
                        break;
                    case LONG:
                        ++long_index;
                        break;
                    case DOUBLE:
                        ++double_index;
                        break;
                    case STRING:
                        ++string_index;
                        break;
                    case TIMESTAMP:
                        ++long_index;
                        break;
                }
                if (!shed_attribute) {
                    continue;
                }
                switch (type) {
                    case NOTYPE:
                        break;
                    case LONG:
                        tuple->longValues[long_index] = -1;
                        break;
                    case DOUBLE:
                        tuple->doubleValues[double_index] = -1;
                        break;
                    case STRING:
                        tuple->stringValues[string_index] = "";
                        break;
                    case TIMESTAMP:
                        tuple->longValues[long_index] = -1;
                        break;
                }
            }
        }
    }

    TupleWrapper::TupleWrapper(Ptr<TupleWrapper> tupleWrapper) {
        this->next_operator_id = tupleWrapper->next_operator_id;
        this->node = tupleWrapper->node;
        this->evicted = tupleWrapper->evicted;
        this->in_migration = tupleWrapper->in_migration;
        this->current_migration_phase = tupleWrapper->current_migration_phase;
        this->group_id = tupleWrapper->group_id;
        this->group = tupleWrapper->group;
        this->partition_group = tupleWrapper->partition_group;
        this->query = tupleWrapper->query;
        for (auto tuple : tupleWrapper->tuples) {
            auto t = CreateObject<Tuple>(tuple);
            t->tupleWrapper = this;
            this->add_tuple(t);
        }
    }

    TupleWrapper::TupleWrapper(Ptr<TupleWrapper> tupleWrapper, bool shed_unessential_attributes, bool new_ids) {
        this->next_operator_id = tupleWrapper->next_operator_id;
        this->node = tupleWrapper->node;
        this->evicted = tupleWrapper->evicted;
        this->in_migration = tupleWrapper->in_migration;
        this->current_migration_phase = tupleWrapper->current_migration_phase;
        this->group_id = tupleWrapper->group_id;
        this->group = tupleWrapper->group;
        this->query = tupleWrapper->query;
        this->partition_group = tupleWrapper->partition_group;
        for (auto tuple : tupleWrapper->tuples) {
            auto t = CreateObject<Tuple>(tuple, query, shed_unessential_attributes);
            t->tupleWrapper = this;
            if (new_ids) {
                t->id = Tuple::get_next_id();
            }
            this->add_tuple(t);
        }
    }

    size_t TupleWrapper::GetSerializedSize(bool with_group) {
        // next_operator_id
        size_t size = sizeof(uint32_t);
        // Group or not
        size += sizeof(uint16_t);
        if (with_group && group != nullptr && group != Group::no_group) {
            // Group ID, partition
            size += sizeof(uint32_t) * 2;
            // Now the values of the first tuple
            auto tupleWrapper = group->first_tuple_wrapper;
            size += tupleWrapper->GetSerializedSize(false);
        }
        // Partition or not
        size += sizeof(uint16_t);
        if (with_group && partition_group != nullptr && partition_group != Group::no_group) {
            // Group ID, partition
            size += sizeof(uint32_t) * 2;
            // Now the values of the first tuple
            auto tupleWrapper = partition_group->first_tuple_wrapper;
            size += tupleWrapper->GetSerializedSize(false);
        }
        // number of tuples
        size += sizeof(uint16_t);
        for (auto tuple : tuples) {
            // Length of alias and the alias itself
            size += sizeof(uint16_t);
            size += sizeof(char) * tuple->alias.length();
            // stream_id, id, join_depth
            size += sizeof(uint32_t)*3;
            // Length of longValues, doubleValues and stringValues
            size += sizeof(uint32_t)*3;

            auto grouped_tuple = DynamicCast<GroupedTuple>(tuple);
            // grouped_tuple == nullptr
            size += sizeof(uint16_t);
            if (grouped_tuple) {
                // Size of aggregated_long_values
                size += sizeof(uint16_t);
                size += sizeof(uint64_t) * grouped_tuple->aggregated_long_values.size();
                // Size of aggregated_double_values
                size += sizeof(uint16_t);
                size += sizeof(uint64_t) * grouped_tuple->aggregated_double_values.size();
            }

            auto long_size = tuple->longValues.size();
            auto double_size = tuple->doubleValues.size();
            auto string_size = tuple->stringValues.size();

            // TODO: shed tuples if they should be shed
            if (needs_shedding) {
                auto t = tuple->GetShedSizes(query);
                long_size = std::get<0>(t);
                double_size = std::get<1>(t);
                string_size = std::get<2>(t);
            }
            // Size of longValues
            size += sizeof(uint64_t) * long_size;
            // Size of doubleValues
            size += sizeof(uint64_t) * double_size;
            // Size of stringValues
            for (int i = 0; i < (int) string_size; i++) {
                auto str = tuple->getStringValue(i);
                // Length of string
                size += sizeof(uint16_t);
                size += sizeof(char) * str.length();
            }
        }
        return size;
    }

    size_t TupleWrapper::Serialize(bool with_group, Buffer::Iterator iterator) {
        iterator.WriteHtonU32(next_operator_id);
        with_group = with_group && this->group != nullptr && this->group != Group::no_group;
        iterator.WriteHtonU16(with_group);
        if (with_group) {
            auto group_buffer_size = sizeof(uint32_t) * 2 + group->first_tuple_wrapper->GetSerializedSize(false);
            Buffer group_buffer(group_buffer_size);
            group_buffer.AddAtStart(group_buffer_size);
            auto group_iterator = group_buffer.Begin();
            group_iterator.WriteHtonU32(group->id);
            group_iterator.WriteHtonU32(group->partition);
            group->first_tuple_wrapper->Serialize(false, group_iterator);
            iterator.Write(group_buffer.PeekData(), group_buffer_size);
        }
        auto with_partition = with_group && this->partition_group != nullptr && this->partition_group != Group::no_group;
        iterator.WriteHtonU16(with_partition);
        // We send partition_group if we send group. Here we don't check if partition_group == Group::no_group,
        // because it won't happen. partition_group is nullptr if not in use
        if (with_partition) {
            auto group_buffer_size = sizeof(uint32_t) * 2 + partition_group->first_tuple_wrapper->GetSerializedSize(false);
            Buffer group_buffer(group_buffer_size);
            group_buffer.AddAtStart(group_buffer_size);
            auto group_iterator = group_buffer.Begin();
            group_iterator.WriteHtonU32(partition_group->id);
            group_iterator.WriteHtonU32(partition_group->partition);
            partition_group->first_tuple_wrapper->Serialize(false, group_iterator);
            iterator.Write(group_buffer.PeekData(), group_buffer_size);
        }
        iterator.WriteHtonU16(this->tuples.size());
        for (auto &tuple : this->tuples) {
            iterator.WriteHtonU16(tuple->alias.length());
            iterator.Write((uint8_t*)tuple->alias.c_str(), tuple->alias.length());
            iterator.WriteHtonU32(tuple->stream_id);
            iterator.WriteHtonU32(tuple->id);

            auto grouped_tuple = DynamicCast<GroupedTuple>(tuple);
            iterator.WriteHtonU16(grouped_tuple != nullptr);
            if (grouped_tuple) {
                // Size of aggregated_long_values
                iterator.WriteHtonU16(grouped_tuple->aggregated_long_values.size());
                for (auto longValue : grouped_tuple->aggregated_long_values) {
                    iterator.WriteHtonU64(longValue);
                }
                // Size of aggregated_double_values
                iterator.WriteHtonU16(grouped_tuple->aggregated_double_values.size());
                for (auto doubleValue : grouped_tuple->aggregated_double_values) {
                    iterator.WriteHtonU64(doubleValue);
                }
            }

            iterator.WriteHtonU32(tuple->join_depth);

            int long_size = tuple->longValues.size();
            int double_size = tuple->doubleValues.size();
            int string_size = tuple->stringValues.size();
            // TODO: shed tuples if they should be shed
            if (needs_shedding) {
                auto t = tuple->GetShedSizes(query);
                long_size = std::get<0>(t);
                double_size = std::get<1>(t);
                string_size = std::get<2>(t);
            }
            iterator.WriteHtonU32(long_size);
            iterator.WriteHtonU32(double_size);
            iterator.WriteHtonU32(string_size);

            for (int i = 0; i < (int) long_size; i++) {
                auto longValue = tuple->longValues[i];
                iterator.WriteHtonU64(longValue);
            }
            for (int i = 0; i < (int) double_size; i++) {
                auto doubleValue = tuple->doubleValues[i];
                iterator.WriteHtonU64(doubleValue);
            }
            for (int i = 0; i < (int) string_size; i++) {
                auto stringValue = tuple->getStringValue(i);
                iterator.WriteHtonU16(stringValue.length());
                iterator.Write((uint8_t*)stringValue.c_str(), stringValue.length());
            }
        }
        return GetSerializedSize(with_group);
    }

    Ptr<TupleWrapper> TupleWrapper::doDeserialize(Buffer::Iterator *iterator) {
        //auto beginning_of_iterator = iterator;
        auto deserializedTupleWrapper = CreateObject<TupleWrapper>((std::vector<Ptr<Tuple>>){});
        deserializedTupleWrapper->next_operator_id = iterator->ReadNtohU32();
        bool with_group = iterator->ReadNtohU16();
        if (with_group) {
            int group_id = iterator->ReadNtohU32();
            int partition = iterator->ReadNtohU32();
            deserializedTupleWrapper->group = CreateObject<Group>(group_id, partition);
            deserializedTupleWrapper->group->first_tuple_wrapper = TupleWrapper::doDeserialize(iterator);
        }
        bool with_partition = iterator->ReadNtohU16();
        if (with_partition) {
            int group_id = iterator->ReadNtohU32();
            int partition = iterator->ReadNtohU32();
            deserializedTupleWrapper->partition_group = CreateObject<Group>(group_id, partition);
            deserializedTupleWrapper->partition_group->first_tuple_wrapper = TupleWrapper::doDeserialize(iterator);
        }

        int number_tuples = iterator->ReadNtohU16();
        deserializedTupleWrapper->tuples.resize(number_tuples);
        for (int i = 0; i < number_tuples; i++) {
            int str_length = iterator->ReadNtohU16();
            char str[str_length+1];
            str[str_length] = '\0';
            iterator->Read((uint8_t*)str, str_length);
            auto alias = std::string(str);
            int stream_id = iterator->ReadNtohU32();
            int id = iterator->ReadNtohU32();
            Ptr<Tuple> tuple;
            bool is_grouped_tuple = iterator->ReadNtohU16();
            if (is_grouped_tuple) {
                Ptr<GroupedTuple> grouped_tuple = CreateObject<GroupedTuple>(stream_id, id);
                tuple = grouped_tuple;
                // Size of aggregated_long_values
                int number_long_values = iterator->ReadNtohU16();
                grouped_tuple->aggregated_long_values.resize(number_long_values);
                for (int j = 0; j < number_long_values; j++) {
                    grouped_tuple->aggregated_long_values[j] = iterator->ReadNtohU64();
                }
                // Size of aggregated_double_values
                int number_double_values = iterator->ReadNtohU16();
                grouped_tuple->aggregated_double_values.resize(number_double_values);
                for (int j = 0; j < number_double_values; j++) {
                    grouped_tuple->aggregated_double_values[j] = iterator->ReadNtohU64();
                }
            } else {
                tuple = CreateObject<Tuple>(stream_id, id);
            }
            int join_depth = iterator->ReadNtohU32();
            tuple->alias = alias;
            tuple->join_depth = join_depth;
            int longValues_size = iterator->ReadNtohU32();
            int doubleValues_size = iterator->ReadNtohU32();
            int stringValues_size = iterator->ReadNtohU32();
            tuple->longValues.resize(longValues_size);
            tuple->doubleValues.resize(doubleValues_size);
            tuple->stringValues.resize(stringValues_size);
            for (int j = 0; j < longValues_size; j++) {
                tuple->longValues[j] = iterator->ReadNtohU64();
            }
            for (int j = 0; j < doubleValues_size; j++) {
                tuple->doubleValues[j] = iterator->ReadNtohU64();
            }
            for (int j = 0; j < stringValues_size; j++) {
                int attr_str_length = iterator->ReadNtohU16();
                char attr_str[attr_str_length+1];
                attr_str[attr_str_length] = '\0';
                iterator->Read((uint8_t*)attr_str, attr_str_length);
                tuple->stringValues[j] = std::string(attr_str);
            }
            tuple->tupleWrapper = deserializedTupleWrapper;
            deserializedTupleWrapper->tuples[i] = tuple;
        }
        return deserializedTupleWrapper;
    }

    Ptr<TupleWrapper> TupleWrapper::Deserialize(uint8_t *serialized_payload, size_t size) {
        //static int deserialize_cnt = 0;
        //NS_LOG_INFO("Deserializing tuple " << ++deserialize_cnt);
        auto buffer = new Buffer(size);
        buffer->AddAtStart(size);
        buffer->Begin().Write(serialized_payload, size);
        auto iterator = buffer->Begin();
        auto tw = doDeserialize(&iterator);
        delete buffer;
        return tw;
    }

    void Tuple::setFieldsToValues(const std::map <std::string, std::any> &fieldsToValues) {
        for (auto const&[key, val]: fieldsToValues) {
            switch (Tuple::streamIdToFieldToType[stream_id][key]) {
                case TIMESTAMP:
                case LONG:
                    longValues.emplace_back(std::any_cast<long>(val));
                    break;
                case DOUBLE:
                    doubleValues.emplace_back(std::any_cast<double>(val));
                    break;
                case STRING:
                    stringValues.emplace_back(std::any_cast<std::string>(val));
                    break;
                case NOTYPE:
                  break;
                }
        }
    }

    void Tuple::setFieldsToValues(const std::vector<YAML::Node>& fieldsToValues) {
        std::map<std::string, std::any> convertedFieldsToValues;
        for (const auto& it : fieldsToValues) {
            auto key = it["name"].as<std::string>();
            switch (Tuple::streamIdToFieldToType[stream_id][key]) {
                case TIMESTAMP:
                case LONG:
                    convertedFieldsToValues[key] = it["value"].as<long>();
                    break;
                case DOUBLE:
                    convertedFieldsToValues[key] = it["value"].as<double>();
                    break;
                case STRING:
                    convertedFieldsToValues[key] = it["value"].as<std::string>();
                    break;
                case NOTYPE:
                  break;
                }
        }
        setFieldsToValues(convertedFieldsToValues);
    }

    Time Tuple::getTime() {
        return external_timestamp;
    }

    void Tuple::setTime(Time t) {
        this->external_timestamp = t;
    }

    bool TupleWrapper::containsStreamId(int stream_id) {
        for (auto tuple : tuples) {
            if (tuple->stream_id == stream_id) {
                return true;
            }
        }

        return false;
    }

    Time TupleWrapper::getTime() {
        Time time = Time(0);
        for (auto tuple : tuples) {
            time = std::max(tuple->getTime(), time);
        }
        return time;
    }

    void TupleWrapper::setTime(Time t) {
        Time time = Time(t);
        for (auto tuple : tuples) {
            tuple->setTime(t);
        }
    }

    size_t TupleWrapper::GetSerializedSize() {
        return this->GetSerializedSize(true);
    }

    size_t TupleWrapper::GetUtility() {
        return 0;
    }
}
