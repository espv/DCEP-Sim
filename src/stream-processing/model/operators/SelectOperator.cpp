//
// Created by espen on 28.10.2021.
//

#include <algorithm>
#include <sstream>
#include "ns3/Operator.h"
#include "ns3/Tuple.h"
#include "ns3/string-functions.h"
#include "ns3/object.h"


namespace ns3 {
    NS_OBJECT_ENSURE_REGISTERED(SelectOperator);
    NS_LOG_COMPONENT_DEFINE ("SelectOperator");

    TypeId
    SelectOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::SelectOperator")
                .SetParent<Operator> ()
        ;

        return tid;
    }

    TypeId
    SelectField::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::SelectField")
                .SetParent<Object> ();

        return tid;
    }

    TypeId
    AggregateSelectField::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::AggregateSelectField")
                .SetParent<SelectField> ();

        return tid;
    }

    Ptr<Tuple> SelectOperator::selectFields(Ptr<TupleWrapper> tupleWrapper) {
        auto outputTuple = CreateObject<Tuple>(this->stream->output_stream_ids[0]);
        int long_agg_index = 0;
        //int double_agg_index = 0;
        auto group = tupleWrapper->group;
        for (Ptr<Tuple> tuple : tupleWrapper->tuples) {
            // We only need to set shed once, but it doesn't matter if it's done multiple times
            outputTuple->shed = tuple->shed;
            Ptr<GroupedTuple> groupedTuple = DynamicCast<GroupedTuple>(tuple);
            for (Ptr<SelectField> sf: fields) {
                // TODO: Check if the first condition "groupedTuple == nullptr" is necessary
                if (groupedTuple == nullptr && sf->isStreamIdSpecified() && tuple->stream_id != sf->input_stream_id) {
                    continue;
                }
                if (sf->aggregate == NONE) {
                    //
                    if (groupedTuple == nullptr) {
                        outputTuple->setFieldToValueOfTuple(sf, tuple);
                    } else {
                        if (tupleWrapper->partition_group == nullptr) {
                            outputTuple->setFieldToValueOfTuple(sf,group->first_tuple_wrapper->tuples[sf->tuple_index]);
                        } else {
                            outputTuple->setFieldToValueOfTuple(sf,tupleWrapper->partition_group->first_tuple_wrapper->tuples[sf->tuple_index]);
                        }
                    }
                } else {
                    auto agg = groupedTuple->aggregated_long_values[long_agg_index++];
                    outputTuple->addFieldValue(agg);
                }
            }

            tuple->decrementRef();
        }
        return outputTuple;
    }

    std::vector<Ptr<Tuple>> SelectOperator::selectPartitionedFields(Ptr<TupleWrapper> tupleWrapper) {
        std::vector<Ptr<Tuple>> output_tuples;
        output_tuples.emplace_back(CreateObject<Tuple>(this->stream->output_stream_ids[0]));
        int long_agg_index = 0;
//        int double_agg_index = 0;
        auto group = tupleWrapper->group;
        for (Ptr<Tuple> tuple : tupleWrapper->tuples) {
            Ptr<GroupedTuple> groupedTuple = DynamicCast<GroupedTuple>(tuple);
            for (Ptr<SelectField> sf: fields) {
                std::vector<Ptr<Tuple>> new_output_tuples;
                for (auto outputTuple : output_tuples) {
                    if (groupedTuple == nullptr && sf->isStreamIdSpecified() &&
                        tuple->stream_id != sf->input_stream_id) {
                        // This if-statement ensures basically that only one of the tuples in tupleWrapper will move on
                        // We skip the tuple inside tupleWrapper->tuples that doesn't correspond to sf->input_stream_id
                        continue;
                    }
                    // Each tuple will only reach this point once at most
                    if (sf->aggregate == NONE) {
                        // Check for partitioned field
                        if (groupedTuple == nullptr) {
                            output_tuples[0]->setFieldToValueOfTuple(sf, tuple);
                        } else {
                            if (tupleWrapper->partition_group != nullptr) {
                                bool first_tuple = true;
                                for (auto t : tupleWrapper->partition_group->tuples) {
                                    if (first_tuple) {
                                        output_tuples[0]->setFieldToValueOfTuple(sf, t->tuples[sf->tuple_index]);
                                        first_tuple = false;
                                    } else {
                                        // TODO: Create copy of t and insert into output_tuples at end of the loop
                                        auto new_t = CreateObject<Tuple>(outputTuple);
                                        new_output_tuples.emplace_back(new_t);
                                    }
                                }
                            } else {
                                output_tuples[0]->setFieldToValueOfTuple(sf, group->first_tuple_wrapper->tuples[sf->tuple_index]);
                            }
                        }
                    } else {
                        auto agg = groupedTuple->aggregated_long_values[long_agg_index++];
                        output_tuples[0]->addFieldValue(agg);
                    }
                }
                for (auto new_t : new_output_tuples) {
                    output_tuples.emplace_back(new_t);
                }
            }

            tuple->decrementRef();
        }
        return output_tuples;
    }

    void SelectOperator::process(Ptr<TupleWrapper> tupleWrapper) {
        Ptr<Tuple> outputTuple;
        outputTuple = selectFields(tupleWrapper);
        auto outputTupleWrapper = CreateObject<TupleWrapper>(std::vector < Ptr<Tuple>  > {outputTuple});
        outputTupleWrapper->group = tupleWrapper->group;
        outputTupleWrapper->node = tupleWrapper->node;
        static int cnt = 0;
        NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " SelectOperator processing tuple " << ++cnt);

        emit(outputTupleWrapper);
    }

    Ptr<SelectField> SelectField::createSf(std::string &field, Ptr<StreamQuery> query) {
        std::string alias;
        auto dot_position = field.find('.');
        NS_ASSERT (dot_position != std::string::npos);
        alias = field.substr(0, dot_position);
        field = field.substr(dot_position + 1);
        query->AddEssentialParameter(alias, field);
        // field, field is kind of a hack, where first is input and second is output field
        // It's because alias, field and query can't be used, because the signature is already used
        auto sf = CreateObject<SelectField>(alias, field, field, query);
        return sf;
    }

    int SelectField::getFieldIndex(std::string alias, const std::string &field, Ptr<StreamQuery> query) {
        auto stream_id = query->aliasToStreamId[alias];
        return Tuple::get_index(stream_id, alias, field, query);
    }

    SelectField::SelectField(const std::string &alias, const std::string &inputField, const std::string &outputField, Ptr<StreamQuery> query) {
        this->inputField = inputField;
        this->alias = alias;
        this->query = query;
        this->input_stream_id = query->aliasToStreamId[alias];
        this->inputFieldIndex = getFieldIndex(alias, inputField, query);
        this->aggregate = NONE;
        this->outputField = outputField;
        this->type = Tuple::streamIdToFieldToType[input_stream_id][this->inputField];
    }

    SelectField::SelectField(const std::string &alias, const std::string &inputField, Aggregation aggregate, const std::string &outputField, Ptr<StreamQuery> query) : SelectField(alias, inputField, outputField, query) {
        this->aggregate = aggregate;
    }

    Ptr<Operator> SelectOperator::copy(Ptr<Stream> s) {
        auto selectOperator = CreateObject<SelectOperator>(this->select_statement);
        selectOperator->init(windowOperator);
        selectOperator->fields = this->fields;
        Operator::copy(selectOperator, s);
        return selectOperator;
    }

    bool SelectField::same_as(Ptr<SelectField> sf) const {
        return this->inputField == sf->inputField &&
               this->outputField == sf->outputField &&
               this->input_stream_id == sf->input_stream_id &&
               this->aggregate == sf->aggregate &&
               this->type == sf->type;
    }

    void SelectOperator::init(Ptr<WindowOperator> windowOperator) {
        this->windowOperator = windowOperator;
        for (auto selectField: fields) {
            selectField->outputFieldIndex = selectField->getFieldIndex(selectField->alias,
                                                                       selectField->outputField, query);
            if (selectField->input_stream_id == -1) {
                // Search for input stream ID
                int stream_id_with_field = -1;
                for (int stream_id: this->stream->input_stream_ids) {
                    auto stream_field_map = Tuple::streamIdToFieldToType[stream_id];
                    if (stream_field_map.find(selectField->inputField) != stream_field_map.end()) {
                        // Stream has field!
                        if (stream_id_with_field == -1) {
                            stream_id_with_field = stream_id;
                            selectField->input_stream_id = stream_id_with_field;
                        } else {
                            // Conflict!
                            std::cout << "Error: Stream " << stream_id_with_field << " and " << stream_id
                                      << " have field "
                                      << selectField->inputField << "!" << std::endl;
                        }
                    }
                }
            }
        }

        // Reorder SelectField
        for (std::size_t i = 0; i < fields.size(); i++) {
            auto sf = fields[i];
            while ((std::size_t) sf->outputFieldIndex != i) {
                auto sf_swap = fields[sf->outputFieldIndex];
                fields[i] = sf_swap;
                fields[sf->outputFieldIndex] = sf;
                sf = sf_swap;
            }
        }
    }
    AggregateSelectField::AggregateSelectField(std::string alias, const std::string &field, Aggregation aggregate,
                                               const std::string &outputField, Ptr<StreamQuery> query)
            : SelectField(alias, field, aggregate, outputField, query) {
    }

    void SelectOperator::parseSelectStatement(std::vector<int> input_stream_ids, int output_stream_id, const std::string &select_statement) {
        std::vector <std::string> selects;
        std::istringstream iss(select_statement);
        std::string token;
        while (std::getline(iss, token, ',')) {
            selects.push_back(token);
        }

        for (auto &select: selects) {
            select = trim(select);
            std::string input_field;
            std::string output_field;
            std::string aggregate;
            std::vector <std::string> tokens;
            std::istringstream iss2(select);
            while (std::getline(iss2, token, ' ')) {
                tokens.push_back(token);
            }
            if (tokens.size() > 1) {
                // TODO: This has to be made more properly in the GUI, cause now, this was very confusing to get working
                int open_parenthesis_position = tokens[0].find("(");
                if (open_parenthesis_position) {
                    input_field = tokens[0].substr(open_parenthesis_position + 1,
                                                   tokens[0].find(")") - open_parenthesis_position - 1);
                    aggregate = tokens[0].substr(0, open_parenthesis_position);
                } else {
                    input_field = tokens[0];
                    aggregate = "";
                }
                output_field = tokens[1];
            } else {
                input_field = tokens[0];
                output_field = tokens[0];
            }

            std::string alias;
            int stream_id = -1;
            auto dot_position = input_field.find('.');
            NS_ASSERT(dot_position != std::string::npos);
            alias = input_field.substr(0, dot_position);
            stream_id = query->aliasToStreamId[alias];
            if (input_field == output_field) {
                input_field = input_field.substr(dot_position + 1, input_field.size() - dot_position);
                output_field = input_field;
            } else {
                input_field = input_field.substr(dot_position + 1, input_field.size() - dot_position);
            }
            query->AddUnessentialParameter(alias, input_field);

            int tuple_index = 0;
            if (stream_id > 0) {
                for (std::size_t i = 0; i < input_stream_ids.size(); i++) {
                    if (stream_id == input_stream_ids[i]) {
                        tuple_index = i;
                        break;
                    }
                }
            } else {
                // Fail!
                NS_FATAL_ERROR("Unspecified streams used to reference attributes is unsupported");
            }
            Ptr<SelectField> sf;
            if (aggregate.empty()) {
                // Regular SelectField
                sf = CreateObject<SelectField>(alias, input_field, NONE, output_field, this->query);
            } else {
                // Create AggregateSelectField
                sf = CreateObject<AggregateSelectField>(alias, input_field, aggregateStringToType[aggregate],
                                              output_field, this->query);
            }
            sf->tuple_index = tuple_index;
            this->fields.push_back(sf);
        }

        auto map_ = Tuple::streamIdToFieldToType[output_stream_id];
        for (std::size_t i = 0; i < this->fields.size(); i++) {
            bool right_position = false;
            while (!right_position) {
                auto field = this->fields[i];
                int j = 0;
                auto iter = map_.begin();
                for (; iter != map_.end(); ++iter, ++j) {
                    if (iter->first == field->outputField) {
                        break;
                    }
                }
                if (iter == map_.end()) {
                    NS_FATAL_ERROR("Could not map the selected field to an output field");
                }
                auto swapped_field = this->fields[j];
                this->fields[i] = swapped_field;
                this->fields[j] = field;
                auto iterator = map_.begin();
                std::advance(iterator, i);
                if (iterator->first == swapped_field->outputField) {
                    right_position = true;
                }
            }
        }
    }

    int SelectOperator::calculateMigrationSize(migration_types migration_type) {
        return 64;
    }

    void SelectOperator::ExtractState(Ptr<MigrationPlan> mp) {
        NS_LOG_INFO("Extracting state from SelectOperator");
    }

    void SelectOperator::ImportState(Ptr<PartialStateBatch> batch) {
        NS_LOG_INFO("Importing state to SelectOperator");
    }

    Time SelectOperator::get_base_processing_time() {
        return Operator::get_base_processing_time()/10;
        //return MicroSeconds(100);
    }

    Time SelectOperator::get_emit_time_added() {
        //return Operator::get_emit_time_added()/2;
        //return MilliSeconds(25);
        return MilliSeconds(0);
    }

    Time SelectOperator::get_forward_time_added() {
        //return Operator::get_forward_time_added()/2;
        return MilliSeconds(0);
    }
}
