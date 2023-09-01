//
// Created by espen on 24.04.23.
//

#ifndef NS3_DATASET_MANIPULATOR_H
#define NS3_DATASET_MANIPULATOR_H

#include "ns3/object.h"
#include "ns3/Operator.h"

namespace ns3 {
    class DatasetManipulator : public Object {
    public:
        static TypeId GetTypeId();

        void ModifyTupleLists(int number_tuples_between_join,
                              int number_joins_per_tuple,
                              double percentage_A_tuples_join,
                              Time time_between_B_tuples,
                              std::vector <Ptr<TupleWrapper>> &tuples_A,
                              std::vector <Ptr<TupleWrapper>> &tuples_B,
                              std::string join_field_A,
                              std::string join_field_B,
                              int join_range);

        int ModifyDatasets(int number_tuples_between_join, int number_joins_per_tuple,
                          double percentage_A_tuples_join, Time time_to_send_B_tuples,
                          Time time_between_B_tuples,
                          std::vector<Ptr < TupleWrapper>>
                          tuples_A, std::vector<Ptr < TupleWrapper>> tuples_B,
                          std::string join_field_A, std::string join_field_B,
                          int join_range);
    };
}
#endif //NS3_DATASET_MANIPULATOR_H
