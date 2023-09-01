
#include "ns3/Operator.h"

#include "ns3/random.h"
#include <utility>


namespace ns3 {
    TypeId Stream::GetTypeId() {
        static TypeId tid = TypeId("ns3::Stream")
                .SetParent<Operator> ()
        ;

        return tid;
    }

    Ptr<Stream> Stream::AddOperator(Ptr<Operator> new_op, const Ptr<Stream> &new_stream) {
        this->next_stream = new_stream;
        this->next_stream->input_stream_ids = this->output_stream_ids;
        this->next_stream->previous_stream = this;
        new_op->stream = new_stream;
        new_stream->query = this->query;
        new_op->query = this->query;
        ++query->operator_id_cnt;
        if (this->operators.empty()) {
            new_op->stream = new_stream;
            this->next_stream->operators.push_back(new_op);
            new_op->SetNodeId(this->node_id);
            new_op->id = query->operator_id_cnt;
            auto resource_manager = StreamQuery::nodeIdToNode[new_op->node_id]->GetObject<NodeResourceManager>();
            //new_op->SetCpuCore(resource_manager->AssignCore(new_op));
        } else {
            for (Ptr<Operator> op: this->operators) {
                int partition = op->partition;
                Ptr<WindowOperator> windowOperator = op->windowOperator;
                if (op->type == windowType) {
                    windowOperator = DynamicCast<WindowOperator>(op);
                }
                new_op->windowOperator = windowOperator;
                new_op->partition = partition;
                new_op->previous_operator = op;
                new_op->SetNodeId(op->node_id);
                //new_op->node_id = Operator::random->randInt()%5+1;
                bool new_host = new_op->node_id != op->node_id && new_op->node_id != -1;
                if (op->type == groupType) {
                    // Create one operator for each partition
                    op->forked = true;
                    for (int i = 0; i < NUM_PARTITIONS; i++) {
                        op->nextOperator.push_back(new_op->copy(new_stream));
                        auto nextOperator = op->nextOperator[i];
                        this->next_stream->operators.push_back(nextOperator);
                        nextOperator->SetNodeId(op->node_id);
                        nextOperator->id = query->operator_id_cnt;
                        //op->nextOperator->node_id = Operator::random->randInt()%5+1;
                        nextOperator->partition = i;
                        nextOperator->previous_operator = op;
                        auto resource_manager = StreamQuery::nodeIdToNode[nextOperator->node_id]->GetObject<NodeResourceManager>();
                    }
                } else if (new_host) {
                    op->nextOperator.push_back(new_op->copy(new_stream));
                    auto nextOperator = op->nextOperator[0];
                    nextOperator->id = query->operator_id_cnt;
                    this->next_stream->operators.push_back(nextOperator);
                } else { // !new_host
                    // Now, each operator has its nextOperator
                    op->nextOperator.push_back(new_op->copy(new_stream));
                    auto nextOperator = op->nextOperator[0];
                    nextOperator->id = query->operator_id_cnt;
                    //op->nextOperator->node_id = op->node_id;
                    this->next_stream->operators.push_back(nextOperator);
                }
            }
        }
        return new_stream;
    }

    Ptr<Stream> Stream::group(std::vector <std::string> groups) {
        auto groupOperator = CreateObject<GroupOperator>(std::move(groups));
        auto s = AddOperator(groupOperator, CreateObject<Stream>(output_stream_ids));
        return s;
    }

    Ptr<Stream> Stream::group() {
        auto groupOperator = CreateObject<GroupOperator>();
        groupOperator->single_group = true;
        return AddOperator(groupOperator, CreateObject<Stream>(output_stream_ids));
    }

    Ptr<Stream> Stream::cep(std::string pattern, bool multiple_sequences, bool emit_all_tuples) {
        auto cepPatternOperator = CreateObject<CepPatternOperator>(std::move(pattern), multiple_sequences, emit_all_tuples);
        auto cepWindow = new CepWindowOperator();
        cepPatternOperator->windowOperator = cepWindow;
        if (this->operators[0]->type == groupType) {
            cepPatternOperator->groupOperator = DynamicCast<GroupOperator>(this->operators[0]);
        }

        return AddOperator(cepPatternOperator, CreateObject<Stream>(output_stream_ids));
    }

    Ptr<Stream> Stream::within(long ms, std::string timestamp_attribute) {
        Ptr<Stream> s = this;
        while (true) {
            OperatorType operatorType = s->operators[0]->type;
            if (operatorType == cepType) {
                for (int i = 0; i < (int)s->operators.size(); i++) {
                    Ptr<CepPatternOperator> cepPatternOperator = DynamicCast<CepPatternOperator>(s->operators[i]);
                    auto withinOperator = CreateObject<WithinOperator>(ms, timestamp_attribute);
                    cepPatternOperator->withinOperator = withinOperator;
                }
                break;
            }
            s = s->previous_stream;
        }
        return s;
    }

    Ptr<Stream> Stream::define(std::string alias, Ptr<FilterArgument> fa1, Op op, Ptr<FilterArgument> fa2) {
        Ptr<Stream> s = this;
        while (true) {
            OperatorType operatorType = s->operators[0]->type;
            if (operatorType == cepType) {
                for (int i = 0; i < (int)s->operators.size(); i++) {
                    Ptr<CepPatternOperator> cepPatternOperator = DynamicCast<CepPatternOperator>(s->operators[i]);
                    auto filterOperator = CreateObject<FilterOperator>(fa1->copy(), op, fa2->copy());
                    filterOperator->windowOperator = cepPatternOperator->windowOperator;
                    filterOperator->stream = cepPatternOperator->stream;
                    filterOperator->init();
                    auto defineOperator = CreateObject<DefineOperator>(alias, filterOperator);
                    cepPatternOperator->defineOperators.push_back(defineOperator);
                }
                break;
            }
            s = s->previous_stream;
        }
        return s;
    }

    Ptr<Stream> Stream::define(std::string alias) {
        Ptr<Stream> s = this;
        while (true) {
            OperatorType operatorType = s->operators[0]->type;
            if (operatorType == cepType) {
                for (int i = 0; i < (int)s->operators.size(); i++) {
                    Ptr<CepPatternOperator> cepPatternOperator = DynamicCast<CepPatternOperator>(s->operators[i]);
                    auto defineOperator = CreateObject<DefineOperator>(alias);
                    cepPatternOperator->defineOperators.push_back(defineOperator);
                }
                break;
            }
            s = s->previous_stream;
        }
        return s;
    }

    Ptr<Stream> Stream::join(Ptr<Stream> joiningStream) {
        auto joinOperator = CreateObject<JoinOperator>(joiningStream);
        Ptr<Stream> s = this;

        // Check if we have previous join
        auto cur_operator = this->operators[0];
        if (cur_operator->type == joinType) {
            auto prev_join = DynamicCast<JoinOperator>(cur_operator);
            joinOperator->join_depth = prev_join->join_depth + 1;
            // Add window between current operator and new join
            // The window will have the same parameters as the original window
            s = window(9999999, 9999999, INTERMEDIARY_JOIN, TUPLE_COUNT, "");
            DynamicCast<StorageWindowOperator>(s->operators[0])->joinOperator = joinOperator;
        }

        std::unordered_map<int, int> output_stream_id_to_tuple_position;
        std::vector<int> output_stream_ids;
        for (int i = 0; i < (int)this->output_stream_ids.size(); i++) {
            int stream_id = this->output_stream_ids[i];
            output_stream_ids.push_back(stream_id);
            output_stream_id_to_tuple_position[output_stream_ids[i]] = i;
        }
        for (int i = 0; i < (int)joiningStream->output_stream_ids.size(); i++) {
            int stream_id = joiningStream->output_stream_ids[i];
            output_stream_ids.push_back(stream_id);
            output_stream_id_to_tuple_position[joiningStream->output_stream_ids[i]] =
                    i + this->output_stream_ids.size();
        }

        auto new_stream = CreateObject<Stream>(output_stream_ids);
        new_stream->output_stream_id_to_tuple_position = output_stream_id_to_tuple_position;
        joiningStream->next_stream = new_stream;
        new_stream = s->AddOperator(joinOperator, new_stream);
        auto joinWindow = DynamicCast<StorageWindowOperator>(joiningStream->operators[0]);
        joinOperator->windowOperator1 = DynamicCast<StorageWindowOperator>(joinOperator->windowOperator);
        //joinOperator->windowOperator = nullptr;
        joinOperator->windowOperator1->joinOperator = joinOperator;
        joinOperator->windowOperator2 = joinWindow;
        joinOperator->windowOperator2->joinOperator = joinOperator;
        return new_stream;
    }

    Ptr<Stream> Stream::sendTo(int node_id) {
        auto new_stream = CreateObject<Stream>(this->getOutputStreamId());
        auto forward_operator = CreateObject<ForwardOperator>();
        forward_operator->SetNodeId(node_id);
        return AddOperator(forward_operator, new_stream);
    }

    Ptr<Stream> Stream::filter(Ptr<FilterArgument> fa1, Op op, Ptr<FilterArgument> fa2) {
        bool joinFilter = false;
        Ptr<Stream> s = this;
        while (s != nullptr && !s->operators.empty()) {
            OperatorType operatorType = s->operators[0]->type;
            if (operatorType == joinType) {
                joinFilter = true;
                break;
            } else if (operatorType == windowType) {
                break;
            }
            s = s->previous_stream;
        }

        if (joinFilter) {
            auto joinFilterOperator = CreateObject<JoinFilterOperator>(fa1, op, fa2);
            // We create the FilterOperator and add it to the join
            auto joinOperator = DynamicCast<JoinOperator>(s->operators[0]);
            joinOperator->filterOperators.push_back(joinFilterOperator);
            joinFilterOperator->stream = joinOperator->stream;
            joinFilterOperator->query = joinOperator->query;
            joinFilterOperator->joinOperator = joinOperator;
            s = this;
            joinFilterOperator->init();
        } else {
            auto filterOperator = CreateObject<FilterOperator>(fa1, op, fa2);
            // We don't actually change the stream ID, so it should remain the same as the current stream ID
            s = AddOperator(filterOperator, CreateObject<Stream>(this->output_stream_ids));
            filterOperator->init();
        }
        return s;
    }

    Ptr<Stream> Stream::print() {
        auto printOperator = CreateObject<PrintOperator>();
        return AddOperator(printOperator, CreateObject<Stream>(this->output_stream_ids));
    }

    Ptr<Stream> Stream::passthrough() {
        auto passthroughOperator = CreateObject<PassthroughOperator>();
        return AddOperator(passthroughOperator, CreateObject<Stream>(this->output_stream_ids));
    }

    Ptr<Stream> Stream::input(std::string alias) {
        auto inputOperator = CreateObject<InputOperator>(alias);
        NS_ASSERT_MSG(this->output_stream_ids.size() == 1, "Only one output stream from the input operator is allowed");
        this->query->aliasToStreamId[alias] = output_stream_ids[0];
        return AddOperator(inputOperator, CreateObject<Stream>(this->output_stream_ids));
    }

    Ptr<Stream> Stream::sink() {
        auto sinkOperator = CreateObject<SinkOperator>();
        return AddOperator(sinkOperator, CreateObject<Stream>(this->output_stream_ids));
    }

    // Aggregation window operator
    Ptr<Stream> Stream::window(int size, int jump, WindowType size_type, int emit_size, WindowType emit_type,
                           const std::string &field) {
        if (size_type == EXTERNAL_TIME) {
            this->query->time_type = EXTERNAL_TIME_TYPE;
        }
        Ptr<Stream> s = this;
        auto windowOperator = CreateObject<AggregationWindowOperator>();
        if (!operators.empty() && operators.front()->type == groupType) {
            // Create partitions
            auto groupOperator = DynamicCast<GroupOperator>(operators.front());
            windowOperator->groupOperator = groupOperator;
            //groupOperator->windowOperator = windowOperator;
        } else {
            // Create group operator
            s = group();
            windowOperator->groupOperator = DynamicCast<GroupOperator>(s->operators[0]);
        }

        auto new_stream = CreateObject<Stream>(output_stream_ids);
        windowOperator->stream = new_stream;
        windowOperator->stream->input_stream_ids = this->getOutputStreamId();
        windowOperator->init(size, jump, size_type, emit_size, emit_type, field);
        auto stream = s->AddOperator(windowOperator, new_stream);
        return stream;
    }

    // Storage window operator
    Ptr<Stream> Stream::window(int size, int jump, WindowType size_type, WindowType emit_type, const std::string &field) {
        auto windowOperator = CreateObject<StorageWindowOperator>();

        auto new_stream = CreateObject<Stream>(output_stream_ids);
        windowOperator->stream = new_stream;
        windowOperator->stream->input_stream_ids = this->getOutputStreamId();
        windowOperator->init(size, jump, size_type, emit_type, field);
        auto stream = AddOperator(windowOperator, new_stream);
        return stream;
    }

    Ptr<Stream> Stream::select(std::string select_stmt, int output_stream_id) {
        auto selectOperator = CreateObject<SelectOperator>(select_stmt);
        Ptr<Stream> stream = AddOperator(selectOperator, CreateObject<Stream>((std::vector<int>){output_stream_id}));
        selectOperator->parseSelectStatement(this->getOutputStreamId(), output_stream_id, selectOperator->select_statement);
        for (std::size_t i = 0; i < selectOperator->fields.size(); i++) {
            auto sf = selectOperator->fields[i];
            if (sf->aggregate != NONE) {
                auto asf = DynamicCast<AggregateSelectField> (sf);
                bool found_asf = false;
                auto windowOperator = DynamicCast<AggregationWindowOperator>(selectOperator->windowOperator);
                for (int j = 0; j < (int) windowOperator->aggregation_arguments.size(); j++) {
                    // We update the window operator and select operator with the newest asf, because indexes might have changed
                    if (asf->same_as(windowOperator->aggregation_arguments[j])) {
                        found_asf = true;
                        selectOperator->fields[i] = asf;
                        windowOperator->aggregation_arguments[j] = asf;
                        break;
                    }
                }
                if (!found_asf) {
                    windowOperator->aggregation_arguments.push_back(asf);
                }
            }
        }
        for (auto op : stream->operators) {
            auto copiedSelectOp = DynamicCast<SelectOperator>(op);
            copiedSelectOp->parseSelectStatement(this->getOutputStreamId(), output_stream_id, selectOperator->select_statement);
            for (std::size_t i = 0; i < copiedSelectOp->fields.size(); i++) {
                auto sf = copiedSelectOp->fields[i];
                if (sf->aggregate != NONE) {
                    auto asf = DynamicCast<AggregateSelectField> (sf);
                    bool found_asf = false;
                    auto windowOperator = DynamicCast<AggregationWindowOperator>(copiedSelectOp->windowOperator);
                    for (int j = 0; j < (int) windowOperator->aggregation_arguments.size(); j++) {
                        // We update the window operator and select operator with the newest asf, because indexes might have changed
                        if (asf->same_as(windowOperator->aggregation_arguments[j])) {
                            found_asf = true;
                            copiedSelectOp->fields[i] = asf;
                            windowOperator->aggregation_arguments[j] = asf;
                            break;
                        }
                    }
                    if (!found_asf) {
                        windowOperator->aggregation_arguments.push_back(asf);
                    }
                }
            }
        }
        // Check if there is a GroupOperator
        Ptr<Stream> s = stream;
        while (s != nullptr && !s->operators.empty()) {
            if (s->operators[0]->type == groupType) {
                auto groupOperator = DynamicCast<GroupOperator>(s->operators[0]);
                groupOperator->init_partitions(selectOperator);
                groupOperator->init();
            }
            s = s->previous_stream;
        }

        return stream;
    }
}
