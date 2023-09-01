//
// Created by espen on 28.11.2021.
//

//
// Created by espen on 27.11.2021.
//

#include <utility>
#include "ns3/Operator.h"

namespace ns3 {
    TypeId
    DefineOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::DefineOperator")
                .SetParent<Operator> ()
        ;

        return tid;
    }

    DefineOperator::DefineOperator(std::string alias, Ptr<FilterOperator> filterOperator) : Operator(defineType) {
        this->alias = alias;
        this->filterOperator = filterOperator;

        auto arg1 = filterOperator->arg1;
        auto arg2 = filterOperator->arg2;

        // Main filter argument is the argument that operates on the incoming tupleWrapper object, whether it's alias or attribute
        this->mainFilterArgument = nullptr;

        bool error_occurred = false;
        mainFilterArgument = arg1;

        if (mainFilterArgument != nullptr) {
            error_occurred = true;
        }
        mainFilterArgument = arg2;

        if (mainFilterArgument == nullptr) {
            error_occurred = true;
        }

        if (error_occurred) {
            std::cout << "Error occurred while setting up the define operator" << std::endl;
            exit(108);
        }
    }

    DefineOperator::DefineOperator(std::string alias) : Operator(defineType) {
        this->alias = std::move(alias);
    }

    bool DefineOperator::match(Ptr<SequencePartialState> sequence, Ptr<TupleWrapper> tupleWrapper) const {
        //if (aliasFilterArgument == nullptr) {
        // Easy, we know the arguments are attribute and constant
        if (filterOperator == nullptr) {
            return true;
        }

        auto windowOperator = DynamicCast<CepWindowOperator>(this->windowOperator);
        if (mainFilterArgument->aggregation != NONE) {
            windowOperator->insertTuple(sequence, tupleWrapper);
        }
        this->filterOperator->sequence = sequence;
        auto ret = this->filterOperator->calculateCondition(tupleWrapper);
        this->filterOperator->sequence = nullptr;
        if (!ret && mainFilterArgument->aggregation != NONE) {
            windowOperator->undoInsertTuple(sequence, tupleWrapper);
        }
        return ret;
        //}
        // Now we have to calculate condition for an alias and an attribute
        // The alias represents any of the sequence->aliasToTuples[alias] tuples
        // That means we have to calculate the condition for all of them
        // Essentially, for each tuple2 in sequence->aliasToTuples[alias] calculateCondition (tupleWrapper, tupleWrapper2) has to return true
        /*for (TupleWrapper *tupleWrapper2 : sequence->aliasToTuples[aliasFilterArgument->alias]) {
            if (attributeFilterArgumentIndex == 0) {
                if (!filterOperator->calculateCondition(tupleWrapper, tupleWrapper2)) {
                    return false;
                }
            } else if (attributeFilterArgumentIndex == 1) {
                if (!filterOperator->calculateCondition(tupleWrapper2, tupleWrapper)) {
                    return false;
                }
            } else {
                // Error!
                std::cout << "attributeFilterArgumentIndex is set up wrong!" << std::endl;
            }
        }*/
        return true;
    }

    Ptr<Operator> DefineOperator::copy(Ptr<Stream> s) {
        auto filterOperator_copy = DynamicCast<FilterOperator>(this->filterOperator->copy(this->filterOperator->stream));
        Ptr<DefineOperator> new_op = CreateObject<DefineOperator>(this->alias, filterOperator_copy);
        Operator::copy(new_op, s);
        new_op->alias = alias;
        new_op->filterOperator = DynamicCast<FilterOperator>(filterOperator->copy(filterOperator->stream));

        return new_op;
    }

    int DefineOperator::calculateMigrationSize(migration_types migration_type) {
        return 1;
    }

    void DefineOperator::ExtractState(Ptr<MigrationPlan> mp) {

    }

    void DefineOperator::ImportState(Ptr<PartialStateBatch> batch) {

    }
}
