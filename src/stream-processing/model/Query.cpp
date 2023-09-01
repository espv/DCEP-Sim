//
// Created by espen on 03.12.2021.
//
#include "ns3/Query.h"
#include "ns3/object.h"
#include "ns3/Operator.h"
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/topological_sort.hpp"
#include <map>


namespace ns3 {
    NS_OBJECT_ENSURE_REGISTERED(StreamQuery);

    NS_LOG_COMPONENT_DEFINE ("StreamQuery");

    TypeId
    StreamQuery::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::StreamQuery")
                .SetParent<Object>();

        return tid;
    }

    std::vector<Ptr<PartialStateWrapper>> StreamQuery::GetEssentialStates(std::string alias) {
        std::vector<Ptr<PartialStateWrapper>> ret;
        return ret;
    }

    void StreamQuery::addStream(Ptr<Stream> stream) {
        bool cur_op_source = true;
        bool cur_op_sink = true;
        auto last_stream = stream;
        for (; stream != nullptr; stream = stream->next_stream) {
            last_stream = stream;
            for (int i = 0; i < (int) stream->operators.size(); i++) {
                auto & partition = stream->operators[i];
                auto op = partition;
                if (!op->source_sinkable) {
                    if (i == 0) {
                        // This is bad, because it means we have no source
                        // Therefore, we create a passthrough operator

                    }
                    cur_op_source = false;
                }
                op->source = cur_op_source;
                auto op_id = op->previous_operator == nullptr ? 0 : op->id;
                op->query = this;
                //int node_id = partition->node_id;
                auto input_stream_ids = stream->getInputStreamId();
                for (auto stream_id : input_stream_ids) {
                    if (!this->streamIdToOperatorIdToOperator[stream_id][op_id]) {
                        // if (op->previous_operator == nullptr || op->previous_operator->node_id != op->node_id) {
                        this->streamIdToOperatorIdToOperator[stream_id][op_id] = partition;
                        //}
                    }
                }
                // TODO: Fix this. This is a hack
                if (op->type == joinType) {
                    // Also add join's input streams
                    auto input_stream_ids = DynamicCast<JoinOperator>(op)->stream2->input_stream_ids;
                    for (auto stream_id : input_stream_ids) {
                        if (!this->streamIdToOperatorIdToOperator[stream_id][op_id]) {
                            // if (op->previous_operator == nullptr || op->previous_operator->node_id != op->node_id) {
                            this->streamIdToOperatorIdToOperator[stream_id][op_id] = partition;
                            //}
                        }
                    }
                }
            }
        }
        for (; last_stream != nullptr; last_stream = last_stream->previous_stream) {
            for (auto & partition : last_stream->operators) {
                auto op = partition;
                // I've changed from allowing multiple sink operators to just having one
                // It doesn't make much sense to have more than one, since the sink node
                // should in reality just subscribe to the streaming data
                if (!op->source_sinkable) {
                    cur_op_sink = false;
                }
                op->sink = cur_op_sink;
            }
        }
    }

    Ptr<StreamQuery> StreamQuery::copy() {
        /*auto new_query = CreateObject<StreamQuery>(this->id, this->unique_query_id);
        new_query->sink_nodes = this->sink_nodes;
        new_query->source_nodes = this->source_nodes;
        for (auto &[stream_id, operatorIdToOperator] : this->streamIdToOperatorIdToOperator) {
            for (auto &[operator_id, op] : operatorIdToOperator) {
                new_query->streamIdToOperatorIdToOperator[stream_id][operator_id] = op->copy(op->stream);
            }
        }
        new_query->id = this->id;
        new_query->unique_query_id = this->unique_query_id;*/
        NS_FATAL_ERROR("Not implemented");
        return nullptr;
    }

    void StreamQuery::setTime(Ptr<TupleWrapper> tupleWrapper) {
        switch (this->time_type) {
            case EXTERNAL_TIME_TYPE: {
                this->current_time = tupleWrapper->getTime();
                break;
            } case PROCESSING_TIME_TYPE: {
                this->current_time = Simulator::Now();
                break;
            } default: {
                NS_LOG_ERROR("Wrong time type");
                exit(535);
            }
        }
    }

    Ptr<FilterArgument> buildFilterArgument(YAML::Node y, Ptr<StreamQuery> query) {
        Ptr<FilterArgument> fa;
        auto type = y["type"].as<std::string>();
        auto value_type = y["value-type"].as<std::string>();
        if (type == "constant") {
            if (value_type == "string") {
                auto arg = y["value"].as<std::string>();
                fa = new ConstantFilterArgument(arg);
            } else if (value_type == "int") {
                auto arg = y["value"].as<long>();
                if (y["offset"]) {
                    fa = new ConstantFilterArgument(arg, y["offset"].as<long>());
                } else {
                    fa = new ConstantFilterArgument(arg, 0);
                }
            } else if (value_type == "long") {
                auto arg = y["value"].as<long>();
                if (y["offset"]) {
                    fa = new ConstantFilterArgument(arg, y["offset"].as<long>());
                } else {
                    fa = new ConstantFilterArgument(arg, 0);
                }
            } else if (value_type == "timestamp") {
                auto arg = y["value"].as<long>();
                if (y["offset"]) {
                    fa = new ConstantFilterArgument(arg, y["offset"].as<long>());
                } else {
                    fa = new ConstantFilterArgument(arg, 0);
                }
            } else if (value_type == "float") {
                auto arg = y["value"].as<float>();
                if (y["offset"]) {
                    fa = new ConstantFilterArgument(arg, y["offset"].as<double>());
                } else {
                    fa = new ConstantFilterArgument(arg, 0);
                }
            } else if (value_type == "double") {
                auto arg = y["value"].as<double>();
                if (y["offset"]) {
                    fa = new ConstantFilterArgument(arg, y["offset"].as<double>());
                } else {
                    fa = new ConstantFilterArgument(arg, 0);
                }
            } else {
                NS_FATAL_ERROR("Invalid value type");
            }
        } else if (type == "attribute") {
            auto arg = y["value"].as<std::string>();
            double offset = 0;
            if (y["offset"]) {
                offset = y["offset"].as<double>();
            }
            auto dot_position = arg.find('.');
            auto alias = arg.substr(0, dot_position);
            auto field = arg.substr(dot_position + 1);
            query->AddEssentialParameter(alias, field);
            fa = CreateObject<AttributeFilterArgument>(alias, field, true, query, offset);
        } else {
            NS_FATAL_ERROR("Invalid argument type");
        }

        return fa;
    }

    class Vertex {
    public:
        bool intersection_point = false;
        Ptr<Stream> intersection_stream;
        int index = -1;

        Vertex(int i) {
            index = i;
        }
    };

    class Edge {
    public:
        std::string stream_name = "";
        int stream_id = -1;
        Vertex *from_vertex = nullptr;
        Vertex *to_vertex = nullptr;
        int from = -1;
        int to = -1;

        Edge *next_edge = nullptr;
        std::vector<Edge*> previous_edges;
        int number_roots_crossing = 0;
        bool intersection = false;
    };

    class Graph {
    public:
        std::vector<Vertex> vertices;
        std::map<int, Vertex*> indexToVertex;
        std::vector<Vertex*> roots;
        std::vector<Edge> edges;
        std::map<int, Edge*> inputStreamIdToEdge;
    };

    void traverse_tree_inc_cnt(Edge *e) {
        e->number_roots_crossing++;
        if (e->previous_edges.size() > 1) {
            // Intersection at from_node
            e->from_vertex->intersection_point = true;
        }
        if (e->next_edge) {
            traverse_tree_inc_cnt(e->next_edge);
        }
    }

    void buildOperatorGraph(Ptr<Stream> s, Edge *e, std::vector<YAML::Node> vertices_yaml, std::vector<YAML::Node> edges_yaml, Ptr<StreamQuery> q) {
        if (e == nullptr) {
            return;
        }
        YAML::Node operator_yaml = vertices_yaml[e->from];
        auto type = operator_yaml["type"].as<std::string>();
        if (e->from_vertex->intersection_point) {
            if (e->from_vertex->intersection_stream) {
                NS_ASSERT(type == "join");
            } else {
                e->from_vertex->intersection_stream = s;
                // We return here because it's the end of the line for the stream
                return;
            }
        }
        int output_stream_id = e->stream_id;

        auto op_name = operator_yaml["name"].as<std::string>();
        auto parameters = operator_yaml["parameters"].as<YAML::Node>();
        if (type == "filter") {
            Ptr<FilterArgument> fa1 = buildFilterArgument(parameters["arg1"].as<YAML::Node>(), q);
            Op op;
            Ptr<FilterArgument> fa2 = buildFilterArgument(parameters["arg2"].as<YAML::Node>(), q);
            auto op_str = parameters["op"].as<std::string>();
            if (op_str == "larger_than") {
                op = larger_than;
            } else if (op_str == "larger_than_or_equals") {
                op = larger_than_or_equals;
            } else if (op_str == "smaller_than") {
                op = smaller_than;
            } else if (op_str == "smaller_than_or_equals") {
                op = smaller_than_or_equals;
            } else if (op_str == "equals") {
                op = equals;
            } else if (op_str == "not_equals") {
                op = not_equals;
            } else if (op_str == "in_list") {
                op = in_list;
            } else {
                NS_FATAL_ERROR("Invalid operator");
            }
            s = s->filter(fa1, op, fa2);
        } else if (type == "groupby") {
            auto groups = parameters["fields"].as<std::vector<std::string>>();
            s = s->group(groups);
        } else if (type == "select") {
            auto fields = parameters["fields"].as<std::vector<std::string>>();
            std::string fields_str = "";
            for (int i = 0; i < (int) fields.size(); i++) {
                fields_str += fields[i];
                if (i < (int) fields.size()-1) {
                    fields_str += ",";
                }
            }
            s = s->select(fields_str, output_stream_id);
        } else if (type == "cep") {
            auto pattern = parameters["pattern"].as<std::string>();
            bool multiple_sequences = false;
            bool emit_all_tuples = false;
            s = s->cep(pattern, multiple_sequences, emit_all_tuples);
        } else if (type == "print") {
            s = s->print();
        } else if (type == "window") {
            int size = parameters["size"].as<int>();
            int jump = parameters["jump"].as<int>();
            auto st = parameters["size-type"].as<std::string>();
            int emit_size = parameters["emit-size"].as<int>();
            bool aggregation_window = true;
            if (emit_size == -1) {
                aggregation_window = false;
            }
            if (ExposeCoordinator::numeric_variables.count("emit_size") && aggregation_window) {
                emit_size = ExposeCoordinator::numeric_variables["emit_size"];
            }
            if (ExposeCoordinator::numeric_variables.count("number_window_extents") && aggregation_window) {
                size = emit_size * ExposeCoordinator::numeric_variables["number_window_extents"];
            }
            auto et = parameters["emit-type"].as<std::string>();
            auto field = parameters["external-timestamp-field"].as<std::string>();
            WindowType size_type;
            WindowType emit_type;
            if (st == "TUPLE_COUNT") {
                size_type = TUPLE_COUNT;
            } else if (st == "EXTERNAL_TIME") {
                size_type = EXTERNAL_TIME;
            } else if (st == "PROCESSING_TIME") {
                size_type = PROCESSING_TIME;
            } else {
                NS_FATAL_ERROR("Invalid window type for size type");
            }

            if (et == "TUPLE_COUNT") {
                emit_type = TUPLE_COUNT;
            } else if (et == "EXTERNAL_TIME") {
                emit_type = EXTERNAL_TIME;
            } else if (et == "PROCESSING_TIME") {
                emit_type = PROCESSING_TIME;
            } else {
                NS_FATAL_ERROR("Invalid window type for size type");
            }
            if (emit_size == -1) {
                s = s->window(size, jump, size_type, emit_type, field);
            } else {
                s = s->window(size, jump, size_type, emit_size, emit_type, field);
            }
        } else if (type == "aggregation-window") {
            int size = 0;
            int jump = 0;
            WindowType size_type = NOWINDOWTYPE;
            int emit_size = 0;
            WindowType emit_type = NOWINDOWTYPE;
            std::string field = "";
            s = s->window(size, jump, size_type, emit_size, emit_type, field);
        } else if (type == "storage-window") {
            int size = 0;
            int jump = 0;
            WindowType size_type = NOWINDOWTYPE;
            WindowType emit_type = NOWINDOWTYPE;
            std::string field = "";
            s = s->window(size, jump, size_type, emit_type, field);
        } else if (type == "join") {
            //int join_stream_id = 0;
            s = s->join(e->from_vertex->intersection_stream);
        } else if (type == "cep") {
            s = s->cep("", false, false);
        } else if (type == "input") {
            auto alias = parameters["alias"].as<std::string>();
            s = s->input(alias);
        } else if (type == "passthrough") {
            s = s->passthrough();
        } else if (type == "sink") {
            s = s->sink();
        } else if (type == "output") {
            s = s->sink();
        } else {
            NS_FATAL_ERROR("Error: unknown operator type " + type);
        }

        for (auto op : s->operators) {
            // If an operator doesn't have a name, it's completely new, and it should be named what the operator is
            // called in the configuration file
            if (op->name == "") {
                op->name = op_name;
            }
        }

        buildOperatorGraph(s, e->next_edge, vertices_yaml, edges_yaml, q);
    }

    Ptr<StreamQuery> StreamQuery::buildQuery(std::vector<YAML::Node> vertices_yaml, std::vector<YAML::Node> edges_yaml, int query_index, int unique_query_id) {
        Ptr<StreamQuery> q = CreateObject<StreamQuery>(query_index, unique_query_id);
        // TODO: fill input_stream_ids from the edges_yaml, i.e., the root edges "stream" tags
        std::vector<int> input_stream_ids;
        q->vertices_yaml = vertices_yaml;
        q->edges_yaml = edges_yaml;
        int host = 0;
        for (auto &[nodeId, node] : StreamQuery::nodeIdToNode) {
            host = nodeId;
            break;
        }
        Graph graph;
        // Create operators for the vertices and nextOperators from the edges
        boost::adjacency_list<> g;
        for (int i = 0; i < (int)vertices_yaml.size(); i++) {
            Vertex v{i};
            graph.vertices.emplace_back(v);
            graph.indexToVertex[i] = &graph.vertices[graph.vertices.size()-1];
            boost::add_vertex(g);
        }
        for (auto edge : edges_yaml) {
            auto from_edge = edge["from"].as<YAML::Node>();
            auto from_op = from_edge["name"].as<std::string>();
            auto to_edge = edge["to"].as<YAML::Node>();
            std::string to_op = "";
            if (to_edge) {
                to_op = to_edge["name"].as<std::string>();
            }
            auto stream_name = edge["stream"].as<std::string>();
            int stream_id = CEPEngine::stream_name_to_id[stream_name];
            Edge e;
            e.stream_id = stream_id;
            e.stream_name = stream_name;

            int from = -1;
            int to = -1;
            int j = 0;
            for (YAML::Node& operator_yaml : vertices_yaml) {
                auto op_name = operator_yaml["name"].as<std::string>();
                if (op_name == from_op) {
                    from = j;
                    if (operator_yaml["type"].as<std::string>() == "input") {
                        input_stream_ids.emplace_back(stream_id);
                    }
                } else if (op_name == to_op) {
                    to = j;
                }
                ++j;
            }
            e.from = from;
            e.from_vertex = &graph.vertices[from];
            if (to > -1) {
                e.to = to;
                e.to_vertex = &graph.vertices[to];
            }
            graph.edges.emplace_back(e);
            boost::add_edge(from, to, g);
        }

        for (int i = 0; i < (int)graph.edges.size(); i++) {
            Edge *e = &graph.edges[i];
            for (int j = 0; j < (int)graph.edges.size(); j++) {
                Edge *inner_edge = &graph.edges[j];
                if (e == inner_edge) {
                    continue;
                }
                if (e->to == inner_edge->from) {
                    e->next_edge = inner_edge;
                    inner_edge->previous_edges.emplace_back(e);
                }
            }
        }

        for (int input_stream_id : input_stream_ids) {
            for (int i = 0; i < (int)graph.edges.size(); i++) {
                if (graph.edges[i].stream_id == input_stream_id) {
                    graph.inputStreamIdToEdge[input_stream_id] = &graph.edges[i];
                }
            }
        }

        for (YAML::Node& operator_yaml : vertices_yaml) {
            auto op_name = operator_yaml["name"].as<std::string>();
            auto type = operator_yaml["type"].as<std::string>();
            auto parameters = operator_yaml["parameters"].as<YAML::Node>();
        }

        for (auto &[stream_id, root_edge] : graph.inputStreamIdToEdge) {
            //root_edge->number_roots_crossing++;
            graph.roots.emplace_back(root_edge->from_vertex);
            traverse_tree_inc_cnt(root_edge);
        }

        // Perform a topological sort.
        std::deque<int> topo_order;
        boost::topological_sort(g, std::front_inserter(topo_order));

        //auto in_stream = CreateObject<Stream>(input_stream_ids, host, q);
        // TODO: Allow for more than one "root" stream, i.e., for joining or pattern matching
        std::vector<Ptr<Stream>> roots;
        //Ptr<Stream> p = in_stream;

        for (auto &[stream_id, root_edge] : graph.inputStreamIdToEdge) {
            std::vector<int> input_stream_id;
            input_stream_id.emplace_back(stream_id);
            auto s = CreateObject<Stream>(input_stream_id, host, q);
            roots.emplace_back(s);
            buildOperatorGraph(s, root_edge, vertices_yaml, edges_yaml, q);
        }
        for (auto s : roots) {
            q->addStream(s->next_stream);
        }

        // Setting this query's attribute order for the tuples of this stream.
        // This order is enforced by InputOperator.
        q->setFieldIndexes();

        // Set all indexes in the operators
        /*for (auto &[stream_id, operatorIdToOperator] : q->streamIdToOperatorIdToOperator) {
            for (auto &[op_id, op] : operatorIdToOperator) {
                switch (op->type) {
                    case windowType:
                        auto windowOp = DynamicCast<WindowOperator>(op);
                        break;
                    case filterType:
                        auto filterOp = DynamicCast<FilterOperator>(op);
                        filterOp->arg1->index = filterOp->arg1->field
                        filterOp->arg2->index;
                        break;
                    case groupType:
                        auto groupOp = DynamicCast<GroupOperator>(op);
                        break;
                    case selectType:
                        auto selectOp = DynamicCast<SelectOperator>(op);
                        break;
                    case joinType:
                        auto joinOp = DynamicCast<JoinOperator>(op);
                        break;
                    case printType:
                        break;
                    case sinkType:
                        break;
                    case cepType:
                        auto cepOp = DynamicCast<CepPatternOperator>(op);
                        break;
                    case defineType:
                        break;
                    case forwardType:
                        break;
                    case withinType:
                        break;
                    case passthroughType:
                        break;
                    case inputType:
                        break;
                }
            }
        }*/

        for (auto &[alias, essentialAttributes] : q->aliasToEssentialAttributes) {
            //std::cout << alias << ": ";
            for (auto it = essentialAttributes.begin(); it != essentialAttributes.end();) {
                std::cout << *it;
                ++it;
                if (it != essentialAttributes.end()) {
                    //std::cout << ", ";
                }
            }
            //std::cout << std::endl;
        }

        return q;
    }

    void StreamQuery::AddEssentialParameter(std::string alias, std::string field) {
        if (!this->aliasToEssentialAttributes[alias].contains(field)) {
            int highest_index = -1;
            for (auto &[i, f] : aliasToIndexToField[alias]) {
                highest_index = std::max(i, highest_index);
            }
            aliasToIndexToField[alias][highest_index + 1] = field;
            this->aliasToEssentialAttributes[alias].insert(field);
        }
    }

    void StreamQuery::AddUnessentialParameter(std::string alias, std::string field) {
        if (!this->aliasToEssentialAttributes[alias].contains(field) && !this->aliasToUnssentialAttributes[alias].contains(field)) {
            int highest_index = -1;
            for (auto &[i, f] : aliasToIndexToField[alias]) {
                highest_index = std::max(i, highest_index);
            }
            aliasToIndexToField[alias][highest_index + 1] = field;
            this->aliasToUnssentialAttributes[alias].insert(field);
        }
    }

    void StreamQuery::setFieldIndexes() {
        int long_index = -1;
        int string_index = -1;
        int double_index = -1;
        int index = 0;
        for (auto &[alias, essentialAttributes]: this->aliasToEssentialAttributes) {
            auto stream_id = this->aliasToStreamId[alias];
            for (auto attr: essentialAttributes) {
                for (int i = 0; i < (int) Tuple::streamIdToIndexToField[stream_id].size(); i++) {
                    auto f = Tuple::streamIdToIndexToField[stream_id][i];
                    auto t = Tuple::streamIdToFieldToType[stream_id][f];
                    switch (t) {
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
                        switch (t) {
                            case NOTYPE:
                                break;
                            case LONG:
                                aliasToIndexToField[alias][index++] = attr;
                                break;
                            case DOUBLE:
                                aliasToIndexToField[alias][index++] = attr;
                                break;
                            case STRING:
                                aliasToIndexToField[alias][index++] = attr;
                                break;
                            case TIMESTAMP:
                                aliasToIndexToField[alias][index++] = attr;
                                break;
                        }
                    }
                }
            }
        }

        for (auto &[alias, essentialAttributes]: this->aliasToEssentialAttributes) {
            // Find all attributes in the stream that aren't essential
            int stream_id = aliasToStreamId[alias];
            for (auto &[field, type]: Tuple::streamIdToFieldToType[stream_id]) {
                bool found = false;
                for (auto essentialAttr: this->aliasToEssentialAttributes[alias]) {
                    if (essentialAttr == field) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    aliasToUnssentialAttributes[alias].insert(field);
                    aliasToIndexToField[alias][index++] = field;
                }
            }
        }
    }
}
