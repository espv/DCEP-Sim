//
// Created by espen on 28.02.2022.
//

#ifndef NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_CSVITERATOR_H
#define NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_CSVITERATOR_H

#include <iterator>
#include <vector>

namespace ns3 {
    enum class CSVState {
        UnquotedField,
        QuotedField,
        QuotedQuote
    };

    class CSVReader {
    public:
        std::vector<std::string> readCSVRow(const std::string &row);
/// Read CSV file, Excel dialect. Accept "quoted fields ""with quotes"""
        std::vector<std::vector<std::string>> readCSV(std::istream &in, int n);
    };
}


#endif //NS_3_EXTENDED_WITH_EXECUTION_ENVIRONMENT_CSVITERATOR_H
