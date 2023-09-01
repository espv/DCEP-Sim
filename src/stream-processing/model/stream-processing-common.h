//
// Created by espen on 27.10.2021.
//

#ifndef STREAMPROCESSOR_COMMON_H
#define STREAMPROCESSOR_COMMON_H

#define NUM_PARTITIONS 1
#define MAXIMUM_CORES 1

namespace ns3 {
    enum OperatorType {
        windowType=1, filterType, groupType, selectType, joinType, printType, sinkType, cepType, defineType, forwardType, withinType, passthroughType, inputType
    };

    enum SelectorType {
        NOSELECTOR, SELECTFIRST, SELECTLAST
    };

    enum Type {
        NOTYPE, LONG, DOUBLE, STRING, TIMESTAMP
    };

    enum FilterArgumentType {
        ALIASFILTERARG, CONSTANTFILTERARG, ATTRIBUTEFILTERARG
    };

    enum TimeType {
        EXTERNAL_TIME_TYPE, PROCESSING_TIME_TYPE
    };

    enum Aggregation {
        NONE, AVG, MIN, MAX, SUM, COUNT
    };

    enum WindowType {
        NOWINDOWTYPE, INTERMEDIARY_JOIN, TUPLE_COUNT, EXTERNAL_TIME, PROCESSING_TIME
    };

    inline static std::map <std::string, Aggregation> aggregateStringToType = {{"",      NONE},
                                                                               {"none",  NONE},
                                                                               {"avg",   AVG},
                                                                               {"min",   MIN},
                                                                               {"max",   MAX},
                                                                               {"sum",   SUM},
                                                                               {"count", COUNT}};

    enum Op {
        larger_than, larger_than_or_equals, smaller_than, smaller_than_or_equals, equals, not_equals, in_list
    };

    enum ContainerType {
        BST, MAP
    };
}

#endif //STREAMPROCESSOR_COMMON_H
