//
// Created by espen on 31.10.2021.
//
#include <sstream>
#include <set>
#include "ns3/Operator.h"


namespace ns3 {
    NS_OBJECT_ENSURE_REGISTERED(FilterOperator);

    NS_LOG_COMPONENT_DEFINE ("FilterOperator");

    TypeId
    FilterOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::FilterOperator")
                .SetParent<Operator> ()
        ;

        return tid;
    }

    TypeId
    JoinFilterOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::JoinFilterOperator")
                .SetParent<FilterOperator> ()
        ;

        return tid;
    }

    TypeId
    FilterArgument::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::FilterArgument")
                .SetParent<Object> ()
        ;

        return tid;
    }

    TypeId
    AttributeFilterArgument::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::AttributeFilterArgument")
                .SetParent<FilterArgument> ()
        ;

        return tid;
    }

    TypeId
    ConstantFilterArgument::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::ConstantFilterArgument")
                .SetParent<FilterArgument> ()
        ;

        return tid;
    }

    TypeId
    ListConstantFilterArgument::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::ListConstantFilterArgument")
                .SetParent<ConstantFilterArgument> ()
        ;

        return tid;
    }

    AttributeFilterArgument::AttributeFilterArgument(const std::string &alias, const std::string &filter, bool cacheArgument, Ptr<StreamQuery> query, double offset) : FilterArgument(ATTRIBUTEFILTERARG) {
        // Allow stream to be specified
        this->alias = alias;
        this->query = query;
        this->cacheArgument = cacheArgument;
        this->filter = filter;
        this->offset = offset;

        this->field = filter;
        this->stream_id = query->aliasToStreamId[this->alias];
        this->index = SelectField::getFieldIndex(alias, field, query);
        this->type = Tuple::streamIdToFieldToType[stream_id][field];
    }

    AttributeFilterArgument::AttributeFilterArgument(Aggregation aggregation, const std::string &alias, const std::string& filter, bool cacheArgument, Ptr<StreamQuery> query, double offset) : AttributeFilterArgument(alias, filter, cacheArgument, query, offset) {
        this->aggregation = aggregation;
    }

    std::string AttributeFilterArgument::getStringValue(Ptr<TupleWrapper> tupleWrapper) {
        return tupleWrapper->tuples[this->getTupleIndex()]->getStringValue(index);
    }

    long AttributeFilterArgument::getLongValue(Ptr<TupleWrapper> tupleWrapper) {
        return tupleWrapper->tuples[this->getTupleIndex()]->getLongValue(index) + offset;
    }

    double AttributeFilterArgument::getDoubleValue(Ptr<TupleWrapper> tupleWrapper) {
        return tupleWrapper->tuples[this->getTupleIndex()]->getDoubleValue(index) + offset;
    }

    bool FilterOperator::calculateCondition(Ptr<TupleWrapper> tupleWrapper) const {
        bool fulfilled = false;
        Type type = arg1->getType();
        if (type == TIMESTAMP) {
            type = LONG;
        }

        if (type == LONG) {
            if (op == larger_than) {
                return arg1->getLongValue(tupleWrapper) > arg2->getLongValue(tupleWrapper);
            } else if (op == larger_than_or_equals) {
                return arg1->getLongValue(tupleWrapper) >= arg2->getLongValue(tupleWrapper);
            } else if (op == smaller_than) {
                return arg1->getLongValue(tupleWrapper) < arg2->getLongValue(tupleWrapper);
            } else if (op == smaller_than_or_equals) {
                return arg1->getLongValue(tupleWrapper) >= arg2->getLongValue(tupleWrapper);
            } else if (op == equals) {
                return arg1->getLongValue(tupleWrapper) == arg2->getLongValue(tupleWrapper);
            } else if (op == not_equals) {
                return arg1->getLongValue(tupleWrapper) != arg2->getLongValue(tupleWrapper);
            } else if (op == in_list) {
                auto long_values = DynamicCast<ListConstantFilterArgument>(arg2)->getLongValues();
                for (int i = 0; i < (int)long_values.size(); i++) {
                    auto tw_arg1_long = arg1->getLongValue(tupleWrapper);
                    if (long_values[i] + arg2->offset == tw_arg1_long) {
                        return true;
                    }
                }
                return false;
            } else {
                // Invalid operator
                std::cout << "Invalid operator used" << std::endl;
                exit(-3);
            }
        } else if (type == DOUBLE) {
            if (op == larger_than) {
                return arg1->getDoubleValue(tupleWrapper) > arg2->getDoubleValue(tupleWrapper);
            } else if (op == larger_than_or_equals) {
                return arg1->getDoubleValue(tupleWrapper) >= arg2->getDoubleValue(tupleWrapper);
            } else if (op == smaller_than) {
                return arg1->getDoubleValue(tupleWrapper) < arg2->getDoubleValue(tupleWrapper);
            } else if (op == smaller_than_or_equals) {
                return arg1->getDoubleValue(tupleWrapper) >= arg2->getDoubleValue(tupleWrapper);
            } else if (op == equals) {
                return arg1->getDoubleValue(tupleWrapper) == arg2->getDoubleValue(tupleWrapper);
            } else if (op == not_equals) {
                return arg1->getDoubleValue(tupleWrapper) != arg2->getDoubleValue(tupleWrapper);
            } else if (op == in_list) {
                auto double_values = DynamicCast<ListConstantFilterArgument>(arg2)->getDoubleValues();
                for (int i = 0; i < (int)double_values.size(); i++) {
                    if (double_values[i] + arg2->offset == arg1->getDoubleValue(tupleWrapper)) {
                        return true;
                    }
                }
                return false;
            } else {
                // Invalid operator
                std::cout << "Invalid operator used" << std::endl;
                exit(-3);
            }
        } else if (type == STRING) {
            if (op == equals) {
                return arg1->getStringValue(tupleWrapper) == arg2->getStringValue(tupleWrapper);
            } else if (op == not_equals) {
                return arg1->getStringValue(tupleWrapper) != arg2->getStringValue(tupleWrapper);
            } else if (op == in_list) {
                auto string_values = DynamicCast<ListConstantFilterArgument>(arg2)->getStringValues();
                for (int i = 0; i < (int)string_values.size(); i++) {
                    if (string_values[i] == arg1->getStringValue(tupleWrapper)) {
                        return true;
                    }
                }
                return false;
            } else {
                // Invalid operator
                std::cout << "Invalid operator used" << std::endl;
                exit(36);
            }
        } else if (type == NOTYPE) {
            NS_LOG_INFO(Simulator::Now() << " Invalid filter argument type");
            exit(31);
        }
        return fulfilled;
    }

    void FilterOperator::process(Ptr<TupleWrapper> tupleWrapper) {
        bool condition = calculateCondition(tupleWrapper);
        NS_LOG_INFO(Simulator::Now() << ": FilterOperator processed " << this->tuples_processed << " tuples");
        if (condition) {
            emit(tupleWrapper);
        }
    }

    std::vector<Ptr<TupleWrapper> > JoinFilterOperator::getArg1Matches(Ptr<TupleWrapper> tupleWrapper) const {
        auto attribute_arg1 = DynamicCast<AttributeFilterArgument>(arg1);
        auto attribute_arg2 = DynamicCast<AttributeFilterArgument>(arg2);
        auto arg1_type = arg1->type;
        if (arg1_type == TIMESTAMP) {
            arg1_type = LONG;
        }
        switch (this->containerType) {
            case BST:
                switch (arg1_type) {
                    case LONG: {
                        std::set < std::pair < Ptr<TupleWrapper>  , long >> ::iterator
                        lower_bound_it;
                        std::set < std::pair < Ptr<TupleWrapper>  , long >> ::iterator
                        upper_bound_it;
                        auto tuples = attribute_arg1->bstLongTuples;
                        auto it = tuples.begin();
                        if (joinOperator->windowOperator1->number_evicted > attribute_arg1->number_evicted) {
                            while (it != tuples.end()) {
                                if (it->first->evicted) {
                                    tuples.erase(it);
                                    ++attribute_arg1->number_evicted;
                                }
                                ++it;
                            }
                        }
                        AttributeFilterArgument::custom_bound.second = attribute_arg2->getLongValue(tupleWrapper);
                        switch (this->op) {
                            case larger_than:
                                AttributeFilterArgument::custom_bound.second = attribute_arg2->getLongValue(tupleWrapper)+1;
                                lower_bound_it = attribute_arg1->bstLongTuples.lower_bound(
                                        AttributeFilterArgument::custom_bound);
                                upper_bound_it = attribute_arg1->bstLongTuples.upper_bound(
                                        AttributeFilterArgument::upper_bound);
                                break;
                            case larger_than_or_equals:
                                lower_bound_it = attribute_arg1->bstLongTuples.lower_bound(
                                        AttributeFilterArgument::custom_bound);
                                upper_bound_it = attribute_arg1->bstLongTuples.upper_bound(
                                        AttributeFilterArgument::upper_bound);
                                break;
                            case smaller_than:
                                AttributeFilterArgument::custom_bound.second = attribute_arg2->getLongValue(tupleWrapper)-1;
                                lower_bound_it = attribute_arg1->bstLongTuples.lower_bound(
                                        AttributeFilterArgument::lower_bound);
                                upper_bound_it = attribute_arg1->bstLongTuples.upper_bound(
                                        AttributeFilterArgument::custom_bound);
                                break;
                            case smaller_than_or_equals:
                                lower_bound_it = attribute_arg1->bstLongTuples.lower_bound(
                                        AttributeFilterArgument::lower_bound);
                                upper_bound_it = attribute_arg1->bstLongTuples.upper_bound(
                                        AttributeFilterArgument::custom_bound);
                                break;
                            case equals:
                              break;
                            case not_equals:
                              break;
                            case in_list:
                              break;
                            }
                        std::vector<Ptr<TupleWrapper>> results;
                        while (lower_bound_it != upper_bound_it) {
                            results.push_back(lower_bound_it->first);
                            ++lower_bound_it;
                        }
                        return results;
                    }
                    case DOUBLE: {
                        auto lower_bound_it = attribute_arg1->bstDoubleTuples.lower_bound(
                                AttributeFilterArgument::lower_bound);
                        auto upper_bound_it = attribute_arg1->bstDoubleTuples.upper_bound(
                                AttributeFilterArgument::upper_bound);
                        auto tuples = attribute_arg1->bstDoubleTuples;
                        auto it = tuples.begin();
                        if (joinOperator->windowOperator1->number_evicted > attribute_arg1->number_evicted) {
                            while (it != tuples.end()) {
                                if (it->first->evicted) {
                                    tuples.erase(it);
                                    ++attribute_arg1->number_evicted;
                                }
                                ++it;
                            }
                        }
                        AttributeFilterArgument::custom_bound.second = attribute_arg2->getLongValue(tupleWrapper);
                        switch (this->op) {
                            case larger_than:
                                AttributeFilterArgument::custom_bound.second = attribute_arg2->getLongValue(tupleWrapper)-1;
                                lower_bound_it = attribute_arg1->bstDoubleTuples.lower_bound(
                                        AttributeFilterArgument::custom_bound);
                                upper_bound_it = attribute_arg1->bstDoubleTuples.upper_bound(
                                        AttributeFilterArgument::upper_bound);
                                break;
                            case larger_than_or_equals:
                                lower_bound_it = attribute_arg1->bstDoubleTuples.lower_bound(
                                        AttributeFilterArgument::custom_bound);
                                upper_bound_it = attribute_arg1->bstDoubleTuples.upper_bound(
                                        AttributeFilterArgument::upper_bound);
                                break;
                            case smaller_than:
                                AttributeFilterArgument::custom_bound.second = attribute_arg2->getLongValue(tupleWrapper)+1;
                                lower_bound_it = attribute_arg1->bstDoubleTuples.lower_bound(
                                        AttributeFilterArgument::lower_bound);
                                upper_bound_it = attribute_arg1->bstDoubleTuples.upper_bound(
                                        AttributeFilterArgument::custom_bound);
                                break;
                            case smaller_than_or_equals:
                                lower_bound_it = attribute_arg1->bstDoubleTuples.lower_bound(
                                        AttributeFilterArgument::lower_bound);
                                upper_bound_it = attribute_arg1->bstDoubleTuples.upper_bound(
                                        AttributeFilterArgument::custom_bound);
                                break;
                            case equals:
                              break;
                            case not_equals:
                              break;
                            case in_list:
                              break;
                            }
                        std::vector<Ptr<TupleWrapper>> results;
                        while (lower_bound_it != upper_bound_it) {
                            results.push_back(lower_bound_it->first);
                            ++lower_bound_it;
                        }
                        return results;
                    }
                    default:
                        NS_FATAL_ERROR("Error!");
                }
            case MAP:
                switch (arg1_type) {
                    case LONG: {
                        if (this->op == equals) {
                            auto long_value = attribute_arg2->getLongValue(tupleWrapper);
                            auto &results = attribute_arg1->longToTuples[long_value];
                            while (!results.empty() && results.front()->evicted) {
                                results.erase(results.begin());
                            }
                            return results;
                        } else if (this->op == not_equals) {
                            std::vector<Ptr<TupleWrapper>> results;
                            auto long_value = attribute_arg2->getLongValue(tupleWrapper);
                            for (auto &[k, v] : attribute_arg1->longToTuples) {
                                if (k != long_value) {
                                    auto &long_to_tuples = attribute_arg1->longToTuples[k];
                                    while (!long_to_tuples.empty() && long_to_tuples.front()->evicted) {
                                        long_to_tuples.erase(long_to_tuples.begin());
                                    }
                                    for (auto t : long_to_tuples) {
                                        results.push_back(t);
                                    }
                                }
                            }

                            return results;
                        }
                    }
                    case DOUBLE: {
                        if (this->op == equals) {
                            auto double_value = attribute_arg2->getDoubleValue(tupleWrapper);
                            auto &results = attribute_arg1->doubleToTuples[double_value];
                            while (!results.empty() && results.front()->evicted) {
                                results.erase(results.begin());
                            }
                            return results;
                        } else if (this->op == not_equals) {
                            std::vector<Ptr<TupleWrapper>> results;
                            auto double_value = attribute_arg2->getDoubleValue(tupleWrapper);
                            for (auto &[k, v] : attribute_arg1->doubleToTuples) {
                                if (k != double_value) {
                                    auto &double_to_tuples = attribute_arg1->doubleToTuples[k];
                                    while (!double_to_tuples.empty() && double_to_tuples.front()->evicted) {
                                        double_to_tuples.erase(double_to_tuples.begin());
                                    }
                                    for (auto t : double_to_tuples) {
                                        results.push_back(t);
                                    }
                                }
                            }

                            return results;
                        }
                    }
                    case STRING: {
                        auto string_value = attribute_arg2->getStringValue(tupleWrapper);
                        auto &results = attribute_arg1->stringToTuples[string_value];
                        while (!results.empty() && results.front()->evicted) {
                            results.erase(results.begin());
                        }
                        return results;
                    }
                    default:
                        NS_FATAL_ERROR("Error!");
                }
        }

        return {};
    }

    std::vector<Ptr<TupleWrapper> > JoinFilterOperator::getArg2Matches(Ptr<TupleWrapper> tupleWrapper) const {
        auto attribute_arg1 = DynamicCast<AttributeFilterArgument>(arg1);
        auto attribute_arg2 = DynamicCast<AttributeFilterArgument>(arg2);
        auto arg1_type = arg1->type;
        if (arg1_type == TIMESTAMP) {
            arg1_type = LONG;
        }
        switch (this->containerType) {
            case BST:
                switch (arg1_type) {
                    case LONG: {
                        std::set < std::pair < Ptr<TupleWrapper>  , long >> ::iterator
                        lower_bound_it;
                        std::set < std::pair < Ptr<TupleWrapper>  , long >> ::iterator
                        upper_bound_it;
                        auto tuples = attribute_arg2->bstLongTuples;
                        auto it = tuples.begin();
                        if (joinOperator->windowOperator2->number_evicted > attribute_arg2->number_evicted) {
                            while (it != tuples.end()) {
                                if (it->first->evicted) {
                                    tuples.erase(it);
                                    ++attribute_arg2->number_evicted;
                                }
                                ++it;
                            }
                        }
                        AttributeFilterArgument::custom_bound.second = attribute_arg1->getLongValue(tupleWrapper);
                        switch (this->op) {
                            case larger_than:
                                AttributeFilterArgument::custom_bound.second = attribute_arg1->getLongValue(tupleWrapper)-1;
                                lower_bound_it = attribute_arg2->bstLongTuples.lower_bound(
                                        AttributeFilterArgument::lower_bound);
                                upper_bound_it = attribute_arg2->bstLongTuples.upper_bound(
                                        AttributeFilterArgument::custom_bound);
                                break;
                            case larger_than_or_equals:
                                lower_bound_it = attribute_arg2->bstLongTuples.lower_bound(
                                        AttributeFilterArgument::lower_bound);
                                upper_bound_it = attribute_arg2->bstLongTuples.upper_bound(
                                        AttributeFilterArgument::custom_bound);
                                break;
                            case smaller_than:
                                AttributeFilterArgument::custom_bound.second = attribute_arg1->getLongValue(tupleWrapper)+1;
                                lower_bound_it = attribute_arg2->bstLongTuples.lower_bound(
                                        AttributeFilterArgument::custom_bound);
                                upper_bound_it = attribute_arg2->bstLongTuples.upper_bound(
                                        AttributeFilterArgument::upper_bound);
                                break;
                            case smaller_than_or_equals:
                                lower_bound_it = attribute_arg2->bstLongTuples.lower_bound(
                                        AttributeFilterArgument::custom_bound);
                                upper_bound_it = attribute_arg2->bstLongTuples.upper_bound(
                                        AttributeFilterArgument::upper_bound);
                                break;
                            case equals:
                              break;
                            case not_equals:
                              break;
                            case in_list:
                              break;
                            }
                        std::vector<Ptr<TupleWrapper>> results;
                        while (lower_bound_it != upper_bound_it) {
                            results.push_back(lower_bound_it->first);
                            ++lower_bound_it;
                        }
                        return results;
                    }
                    case DOUBLE: {
                        auto lower_bound_it = attribute_arg2->bstDoubleTuples.lower_bound(
                                AttributeFilterArgument::lower_bound);
                        auto upper_bound_it = attribute_arg2->bstDoubleTuples.upper_bound(
                                AttributeFilterArgument::upper_bound);
                        auto tuples = attribute_arg2->bstDoubleTuples;
                        auto it = tuples.begin();
                        if (joinOperator->windowOperator2->number_evicted > attribute_arg2->number_evicted) {
                            while (it != tuples.end()) {
                                if (it->first->evicted) {
                                    tuples.erase(it);
                                    ++attribute_arg2->number_evicted;
                                }
                                ++it;
                            }
                        }
                        AttributeFilterArgument::custom_bound.second = attribute_arg1->getLongValue(tupleWrapper);
                        switch (this->op) {
                            case larger_than:
                                lower_bound_it = attribute_arg2->bstDoubleTuples.lower_bound(
                                        AttributeFilterArgument::custom_bound);
                                upper_bound_it = attribute_arg2->bstDoubleTuples.upper_bound(
                                        AttributeFilterArgument::upper_bound);
                                break;
                            case larger_than_or_equals:
                                lower_bound_it = attribute_arg2->bstDoubleTuples.lower_bound(
                                        AttributeFilterArgument::custom_bound);
                                upper_bound_it = attribute_arg2->bstDoubleTuples.upper_bound(
                                        AttributeFilterArgument::upper_bound);
                                break;
                            case smaller_than:
                                lower_bound_it = attribute_arg2->bstDoubleTuples.lower_bound(
                                        AttributeFilterArgument::lower_bound);
                                upper_bound_it = attribute_arg2->bstDoubleTuples.upper_bound(
                                        AttributeFilterArgument::custom_bound);
                                break;
                            case smaller_than_or_equals:
                                lower_bound_it = attribute_arg2->bstDoubleTuples.lower_bound(
                                        AttributeFilterArgument::lower_bound);
                                upper_bound_it = attribute_arg2->bstDoubleTuples.upper_bound(
                                        AttributeFilterArgument::custom_bound);
                                break;
                            case equals:
                              break;
                            case not_equals:
                              break;
                            case in_list:
                              break;
                            }
                        std::vector<Ptr<TupleWrapper>> results;
                        while (lower_bound_it != upper_bound_it) {
                            results.push_back(lower_bound_it->first);
                            ++lower_bound_it;
                        }
                        return results;
                    }
                    default:
                        NS_FATAL_ERROR("Error!");
                }
            case MAP:
                switch (arg1_type) {
                    case LONG: {
                        if (this->op == equals) {
                            auto long_value = attribute_arg1->getLongValue(tupleWrapper);
                            auto &results = attribute_arg2->longToTuples[long_value];
                            while (!results.empty() && results.front()->evicted) {
                                results.erase(results.begin());
                            }
                            return results;
                        } else if (this->op == not_equals) {
                            std::vector<Ptr<TupleWrapper>> results;
                            auto long_value = attribute_arg1->getLongValue(tupleWrapper);
                            for (auto &[k, v] : attribute_arg2->longToTuples) {
                                if (k != long_value) {
                                    while (!v.empty() && v.front()->evicted) {
                                        v.erase(v.begin());
                                    }
                                    for (auto t : v) {
                                        results.push_back(t);
                                    }
                                }
                            }

                            return results;
                        }
                    }
                    case DOUBLE: {
                        if (this->op == equals) {
                            auto double_value = attribute_arg1->getDoubleValue(tupleWrapper);
                            auto &results = attribute_arg2->doubleToTuples[double_value];
                            while (!results.empty() && results.front()->evicted) {
                                results.erase(results.begin());
                            }
                            return results;
                        } else if (this->op == not_equals) {
                            std::vector<Ptr<TupleWrapper>> results;
                            auto double_value = attribute_arg1->getDoubleValue(tupleWrapper);
                            for (auto &[k, v] : attribute_arg2->doubleToTuples) {
                                if (k != double_value) {
                                    while (!v.empty() && v.front()->evicted) {
                                        v.erase(v.begin());
                                    }
                                    for (auto t : v) {
                                        results.push_back(t);
                                    }
                                }
                            }

                            return results;
                        }
                    }
                    case STRING: {
                        auto string_value = attribute_arg1->getStringValue(tupleWrapper);
                        auto &results = attribute_arg2->stringToTuples[string_value];
                        while (!results.empty() && results.front()->evicted) {
                            results.erase(results.begin());
                        }
                        return results;
                    }
                    default:
                        NS_FATAL_ERROR("Error!");
                }
        }

        return {};
    }

    void AttributeFilterArgument::cache(Ptr<TupleWrapper> tupleWrapper) {
        auto t = type;
        if (t == TIMESTAMP) {
            t = LONG;
        }

        switch (DynamicCast<JoinFilterOperator>(this->filterOperator)->containerType) {
            case BST:
                switch (t) {
                    case LONG: {
                        bstLongTuples.emplace(std::make_pair(tupleWrapper, getLongValue(tupleWrapper)));
                        break;
                    }
                    case DOUBLE: {
                        bstDoubleTuples.insert(std::make_pair(tupleWrapper, getDoubleValue(tupleWrapper)));
                        break;
                    }
                    default:
                        NS_FATAL_ERROR("Error!");
                }
            break;
            case MAP:
                switch (t) {
                    case LONG: {
                        auto &results = this->longToTuples[getLongValue(tupleWrapper)];
                        results.push_back(tupleWrapper);
                        break;
                    }
                    case DOUBLE: {
                        auto &results = this->doubleToTuples[getDoubleValue(tupleWrapper)];
                        results.push_back(tupleWrapper);
                        break;
                    }
                    case STRING: {
                        auto &results = this->stringToTuples[getStringValue(tupleWrapper)];
                        results.push_back(tupleWrapper);
                        break;
                    }
                    default:
                        NS_FATAL_ERROR("Error!");
                }
            break;
        }
    }

    FilterOperator::FilterOperator(Ptr<FilterArgument> arg1, Op op, Ptr<FilterArgument> arg2) : Operator(filterType) {
        this->arg1 = arg1;
        this->op = op;
        this->arg2 = arg2;
    }

    void FilterOperator::init() {
        arg1->filterOperator = this;
        arg2->filterOperator = this;
        std::vector<int> input_stream_ids = this->stream->input_stream_ids;

        //if (!arg1->isInitialized()) {
            if (auto fa = DynamicCast<AttributeFilterArgument>(arg1)) {
                int input_stream_id = input_stream_ids[fa->getTupleIndex()];
                fa->index = SelectField::getFieldIndex(fa->alias, fa->field, query);
                fa->type = Tuple::streamIdToFieldToType[input_stream_id][fa->field];
            }
        //}
        if (arg1->aggregation != NONE) {
            // Create ASF
            NS_ASSERT(DynamicCast<AttributeFilterArgument>(arg1) != nullptr);
            auto asf = CreateObject<AggregateSelectField>(arg1->alias, arg1->field, arg1->aggregation, arg1->field, this->query);
            bool found_asf = false;
            auto windowOperator = DynamicCast<AggregationWindowOperator>(this->windowOperator);
            for (auto window_asf : windowOperator->aggregation_arguments) {
                if (asf->same_as(window_asf)) {
                    found_asf = true;
                    arg1->asf = window_asf;
                    break;
                }
            }
            if (!found_asf) {
                windowOperator->aggregation_arguments.push_back(asf);
                asf->aggregation_arguments_index = windowOperator->aggregation_arguments.size()-1;
                arg1->asf = asf;
            }
        }

        //if (!arg2->isInitialized()) {
            if (auto fa = DynamicCast<AttributeFilterArgument>(arg2)) {
                int input_stream_id = input_stream_ids[fa->getTupleIndex()];
                fa->index = SelectField::getFieldIndex(fa->alias, fa->field, query);
                fa->type = Tuple::streamIdToFieldToType[input_stream_id][fa->field];
            }
        //}
        if (arg2->aggregation != NONE) {
            NS_ASSERT(DynamicCast<AttributeFilterArgument>(arg2) != nullptr);
            // Create ASF
            auto asf = CreateObject<AggregateSelectField>(arg2->alias, arg2->field, arg2->aggregation, arg2->field, this->query);
            bool found_asf = false;
            auto windowOperator = DynamicCast<AggregationWindowOperator>(this->windowOperator);
            for (auto window_asf : windowOperator->aggregation_arguments) {
                if (asf->same_as(window_asf)) {
                    found_asf = true;
                    arg2->asf = window_asf;
                    break;
                }
            }
            if (!found_asf) {
                windowOperator->aggregation_arguments.push_back(asf);
                asf->aggregation_arguments_index = windowOperator->aggregation_arguments.size()-1;
                arg1->asf = asf;
            }
        }
    }

    void JoinFilterOperator::init() {
        arg1->filterOperator = this;
        arg2->filterOperator = this;
        std::vector<int> input_stream_ids = this->stream->input_stream_ids;
        switch (this->op) {
            case larger_than:
            case larger_than_or_equals:
            case smaller_than:
            case smaller_than_or_equals:
                containerType = BST;
                break;
            case equals:
            case not_equals:
                containerType = MAP;
                break;
            case in_list:
              break;
            }
        //if (!arg1->isInitialized()) {
            if (auto fa = DynamicCast<AttributeFilterArgument>(arg1)) {
                int input_stream_id = input_stream_ids[fa->getTupleIndex()];
                fa->index = SelectField::getFieldIndex(fa->alias, fa->field, query);
                fa->type = Tuple::streamIdToFieldToType[input_stream_id][fa->field];
            }
        //}

        //if (!arg2->isInitialized()) {
            if (auto fa = DynamicCast<AttributeFilterArgument>(arg2)) {
                int input_stream_id = this->joinOperator->stream2->input_stream_ids[fa->getTupleIndex()];
                fa->index = SelectField::getFieldIndex(fa->alias, fa->field, query);
                fa->type = Tuple::streamIdToFieldToType[input_stream_id][fa->field];
            }
        //}
    }

    Ptr<FilterArgument> ConstantFilterArgument::copy() {
        switch (type) {
            case LONG:
                return CreateObject<ConstantFilterArgument>(this->long_value);
            case DOUBLE:
                return CreateObject<ConstantFilterArgument>(this->double_value);
            case STRING:
                return CreateObject<ConstantFilterArgument>(this->string_value);
            case NOTYPE:
              break;
            case TIMESTAMP:
              break;
            }
        std::cout << "Error in copying ConstantFilterArgument" << std::endl;
        exit(94);
    }

    Ptr<FilterArgument> ListConstantFilterArgument::copy() {
        switch (type) {
            case LONG:
                return CreateObject<ListConstantFilterArgument>(this->long_values);
            case DOUBLE:
                return CreateObject<ListConstantFilterArgument>(this->double_values);
            case STRING:
                return CreateObject<ListConstantFilterArgument>(this->string_values);
            case NOTYPE:
              break;
            case TIMESTAMP:
              break;
            }
        std::cout << "Error in copying ConstantFilterArgument" << std::endl;
        exit(94);
    }

    Ptr<FilterArgument> AttributeFilterArgument::copy() {
        return CreateObject<AttributeFilterArgument>(this->aggregation, this->alias, this->filter, this->cacheArgument, this->query, this->offset);
    }

    Ptr<Operator> FilterOperator::copy(Ptr<Stream> s) {
        auto filterOperator = CreateObject<FilterOperator>();
        filterOperator->arg1 = this->arg1->copy();
        filterOperator->arg2 = this->arg2->copy();
        filterOperator->op = this->op;
        Operator::copy(filterOperator, s);
        filterOperator->init();
        return filterOperator;
    }

    int FilterOperator::calculateMigrationSize(migration_types migration_type) {
        return 1;
    }

    int JoinFilterOperator::calculateMigrationSize(migration_types migration_type) {
        return 1;
    }

    void FilterOperator::ExtractState(Ptr<MigrationPlan> mp) {

    }

    void JoinFilterOperator::ExtractState(Ptr<MigrationPlan> mp) {

    }

    void FilterOperator::ImportState(Ptr<PartialStateBatch> batch) {

    }

    void JoinFilterOperator::ImportState(Ptr<PartialStateBatch> batch) {

    }
}
