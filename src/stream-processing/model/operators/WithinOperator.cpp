
#include <utility>
#include "ns3/Operator.h"

namespace ns3 {
    TypeId
    WithinOperator::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::WithinOperator")
                .SetParent<Operator> ()
        ;

        return tid;
    }

    WithinOperator::WithinOperator(long ms, std::string timestamp_attribute) : Operator(withinType) {
        this->ms = ms;
        this->timestamp_attribute = timestamp_attribute;
        if (timestamp_attribute.find('.') != std::string::npos) {
            std::istringstream iss(timestamp_attribute);
            std::string alias;
            std::getline(iss, alias, '.');
            std::getline(iss, this->timestamp_attribute, '.');
            this->index = SelectField::getFieldIndex(alias, this->timestamp_attribute, query);
        } else {
            this->timestamp_attribute = timestamp_attribute;
        }
    }

    Ptr<Operator> WithinOperator::copy(Ptr<Stream> s) {
        auto new_op = CreateObject<WithinOperator>(this->ms, this->timestamp_attribute);
        return new_op;
    }

    int WithinOperator::calculateMigrationSize(migration_types migration_type) {
        return 1;
    }

    void WithinOperator::ExtractState(Ptr<MigrationPlan> mp) {

    }

    void WithinOperator::ImportState(Ptr<PartialStateBatch> batch) {

    }
}
