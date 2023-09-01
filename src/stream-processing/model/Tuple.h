//
// Created by espen on 26.10.2021.
//

#ifndef STREAMPROCESSOR_TUPLE_H
#define STREAMPROCESSOR_TUPLE_H


#include <map>
#include <any>
#include <iostream>
#include <utility>
#include <vector>
#include <deque>
#include <yaml-cpp/yaml.h>
#include "ns3/stream-processing-common.h"
#include "ns3/Metrics.h"
#include "ns3/Operator.h"
#include "ns3/object.h"
#include "ns3/node.h"
#include "ns3/SerializedOperator.h"


namespace ns3 {
    class SelectField;
    class Group;
    class WindowOperator;
    class TupleWrapper;

}

#endif //STREAMPROCESSOR_TUPLE_H
