//
// Created by espen on 28.02.2022.
//

#include "CSVIterator.h"
#include <iterator>
#include <vector>
#include <iostream>

namespace ns3 {
    std::vector<std::string> CSVReader::readCSVRow(const std::string &row) {
        CSVState state = CSVState::UnquotedField;
        std::vector<std::string> fields{""};
        size_t i = 0; // index of the current field
        for (char c: row) {
            switch (state) {
                case CSVState::UnquotedField:
                    switch (c) {
                        case ',': // end of field
                            fields.push_back("");
                            i++;
                            break;
                        case '"':
                            state = CSVState::QuotedField;
                            break;
                        default:
                            fields[i].push_back(c);
                            break;
                    }
                    break;
                case CSVState::QuotedField:
                    switch (c) {
                        case '"':
                            state = CSVState::QuotedQuote;
                            break;
                        default:
                            fields[i].push_back(c);
                            break;
                    }
                    break;
                case CSVState::QuotedQuote:
                    switch (c) {
                        case ',': // , after closing quote
                            fields.push_back("");
                            i++;
                            state = CSVState::UnquotedField;
                            break;
                        case '"': // "" -> "
                            fields[i].push_back('"');
                            state = CSVState::QuotedField;
                            break;
                        default:  // end of quote
                            state = CSVState::UnquotedField;
                            break;
                    }
                    break;
            }
        }
        return fields;
    }

/// Read CSV file, Excel dialect. Accept "quoted fields ""with quotes"""
    std::vector<std::vector<std::string>> CSVReader::readCSV(std::istream &in, int n) {
        std::vector<std::vector<std::string>> table;
        std::string row;
        int i = 0;
        // We don't do "i++ < n" because n=-1 means that we want to go all the way. We just stop when we reach n.
        while (!in.eof() && i++ != n) {
            std::getline(in, row);
            if (in.bad() || in.fail()) {
                break;
            }
            auto fields = readCSVRow(row);
            table.push_back(fields);
        }
        return table;
    }
}
