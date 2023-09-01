//
// Created by espen on 24.04.23.
//

#include <iostream>
#include <vector>
#include "dataset-manipulator.h"
#include "ns3/Operator.h"

NS_OBJECT_ENSURE_REGISTERED(DatasetManipulator);

NS_LOG_COMPONENT_DEFINE ("DatasetManipulator");

namespace ns3 {

    TypeId
    DatasetManipulator::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::DatasetManipulator")
                .SetParent<Object>();
        return tid;
    }

    void assignKey(Ptr<Tuple> tuple, Type type, int attribute_index, long key) {
        switch (type) {
            case NOTYPE:
                break;
            case LONG:
                tuple->longValues[attribute_index] = key;
                break;
            case DOUBLE:
                tuple->doubleValues[attribute_index] = key;
                break;
            case STRING:
                NS_FATAL_ERROR("String join predicate not supported for DatasetManipulator");
            case TIMESTAMP:
                tuple->longValues[attribute_index] = key;
                break;
        }
    }

    void DatasetManipulator::ModifyTupleLists(int number_tuples_between_join,
                                              int number_joins_per_tuple,
                                              double percentage_A_tuples_join,
                                              Time time_between_B_tuples,
                                              std::vector<Ptr<TupleWrapper>> &tuples_A,
                                              std::vector<Ptr<TupleWrapper>> &tuples_B,
                                              std::string join_field_A,
                                              std::string join_field_B,
                                              int join_range) {
        int a_index = 0;
        int b_index = 0;
        long initial_join_Key = 0;
        long join_key = initial_join_Key;
        Type join_predicate_type;
        int join_attribute_index_A = Tuple::get_index(tuples_A[0]->tuples[0]->stream_id, join_field_A);
        int join_attribute_index_B = Tuple::get_index(tuples_B[0]->tuples[0]->stream_id, join_field_B);

        for (auto &[k, v] : Tuple::streamIdToIndexToField[tuples_A[0]->tuples[0]->stream_id]) {
            if (join_field_A == v) {
                join_predicate_type = Tuple::streamIdToFieldToType[tuples_A[0]->tuples[0]->stream_id][v];
                break;
            }
        }

        for (auto &[k, v] : Tuple::streamIdToIndexToField[tuples_B[0]->tuples[0]->stream_id]) {
            if (join_field_B == v) {
                join_predicate_type = Tuple::streamIdToFieldToType[tuples_B[0]->tuples[0]->stream_id][v];
                break;
            }
        }

        int number_A_tuples_join = (percentage_A_tuples_join / 100.0) * tuples_A.size();

        // Join key interval for A
        long join_key_interval_A = std::max(1L, (long)join_range / number_joins_per_tuple);

        // Join key interval for B
        //long join_key_interval_B = join_range * number_tuples_between_join;

        long max_join_key = -1;

        while (a_index < number_A_tuples_join || b_index < (int) tuples_B.size()) {
            int tmp_join_key = join_key;
            // For A tuples
            if (b_index == 9) {
                std::cout << std::endl;
            }
            for (int i = 0; i < number_joins_per_tuple && a_index < number_A_tuples_join && a_index < (int) tuples_A.size(); i++) {
                switch (join_predicate_type) {
                    case NOTYPE:
                        break;
                    case LONG: {
                        auto tuple = tuples_A[a_index]->tuples[0];
                        tuples_A[a_index]->tuples[0]->longValues[join_attribute_index_A] = tmp_join_key;
                        break;
                    } case DOUBLE:
                        tuples_A[a_index]->tuples[0]->doubleValues[join_attribute_index_A] = tmp_join_key;
                        break;
                    case STRING:
                        NS_FATAL_ERROR("String join predicate not supported for DatasetManipulator");
                    case TIMESTAMP:
                        tuples_A[a_index]->tuples[0]->longValues[join_attribute_index_A] = tmp_join_key;
                        break;
                }
                if (join_range == 1) {

                } else if (i == 0 || (tmp_join_key+join_key_interval_A % join_range != 0)) {
                    tmp_join_key += join_key_interval_A;
                } else {
                    tmp_join_key -= (join_range - join_key_interval_A);
                }
                if (++a_index == (int) tuples_A.size() || a_index == number_A_tuples_join) {
                    max_join_key = join_key;
                }
            }

            // For B tuples
            for (int i = 0; i < number_tuples_between_join && b_index < (int) tuples_B.size(); i++) {
                switch (join_predicate_type) {
                    case NOTYPE:
                        break;
                    case LONG:
                        tuples_B[b_index]->tuples[0]->longValues[join_attribute_index_B] = (i == 0) ? join_key : -2;
                        break;
                    case DOUBLE:
                        tuples_B[b_index]->tuples[0]->doubleValues[join_attribute_index_B] = (i == 0) ? join_key : -2;
                        break;
                    case STRING:
                        NS_FATAL_ERROR("String join predicate not supported for DatasetManipulator");
                    case TIMESTAMP:
                        tuples_B[b_index]->tuples[0]->longValues[join_attribute_index_B] = (i == 0) ? join_key : -2;
                        break;
                }

                tuples_B[b_index]->setTime(b_index * time_between_B_tuples); // Set the timestamp
                b_index++;
            }

            join_key += join_range;
            if (a_index >= number_A_tuples_join && join_key > max_join_key) {
                // TODO: Sometimes, initial_join_key is correct, and other times, join_range is correct
                join_key = initial_join_Key;
            }
        }

        for (int i = a_index; i < (int) tuples_A.size(); i++) {
            switch (join_predicate_type) {
                case NOTYPE:
                    break;
                case LONG:
                    tuples_A[i]->tuples[0]->longValues[join_attribute_index_A] = -1;
                    break;
                case DOUBLE:
                    tuples_A[i]->tuples[0]->doubleValues[join_attribute_index_A] = -1;
                    break;
                case STRING:
                    NS_FATAL_ERROR("String join predicate not supported for DatasetManipulator");
                case TIMESTAMP:
                    tuples_A[i]->tuples[0]->longValues[join_attribute_index_A] = -1;
                    break;
            }
        }
        for (int i = b_index; i < (int) tuples_B.size(); i++) {
            switch (join_predicate_type) {
                case NOTYPE:
                    break;
                case LONG:
                    tuples_B[i]->tuples[0]->longValues[join_attribute_index_B] = -2;
                    break;
                case DOUBLE:
                    tuples_B[i]->tuples[0]->doubleValues[join_attribute_index_B] = -2;
                    break;
                case STRING:
                    NS_FATAL_ERROR("String join predicate not supported for DatasetManipulator");
                case TIMESTAMP:
                    tuples_B[i]->tuples[0]->longValues[join_attribute_index_B] = -2;
                    break;
            }
        }
    }

    int DatasetManipulator::ModifyDatasets(
            int number_tuples_between_join,
            int number_joins_per_tuple,
            double percentage_A_tuples_join,
            Time time_to_send_B_tuples,
            Time time_between_B_tuples,
            std::vector<Ptr<TupleWrapper>> tuples_A,
            std::vector<Ptr<TupleWrapper>> tuples_B,
            std::string join_field_A,
            std::string join_field_B,
            int join_range // Added parameter to specify the join range
    ) {
        ModifyTupleLists(
                number_tuples_between_join,
                number_joins_per_tuple,
                percentage_A_tuples_join,
                time_between_B_tuples,
                tuples_A,
                tuples_B,
                join_field_A,
                join_field_B,
                join_range
        );
        Time stop_time = (tuples_B.size() - 1) * time_between_B_tuples;
        std::cout << "Stop time: " << stop_time << " ms" << std::endl;

        return 0;
    }
// This code assumes a uniform distribution for the keys of tuples in stream B that fall within the join range. If your data doesn't follow this distribution or if you want to model a different distribution, you may need to adjust the random key generation code.


}
