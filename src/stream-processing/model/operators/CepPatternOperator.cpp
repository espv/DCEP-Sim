//
// Created by espen on 26.11.2021.
//

#include <algorithm>
#include <boost/regex.hpp>
#include <cmath>
#include "ns3/Operator.h"

namespace ns3 {
    NS_OBJECT_ENSURE_REGISTERED(CepPatternOperator);
    NS_LOG_COMPONENT_DEFINE ("CepPatternOperator");
    Ptr<SequencePartialState> prev_sequence;
    int node = 0;
    int prev_sequence_cnt = 0;
    int cur_sequence_cnt = 0;

    std::vector<Ptr<PartialState>> CepPatternOperator::getPartialStates() {
        std::vector<Ptr<PartialState>> ret;
        for (auto &[group_id, sequences] : DynamicCast<CepWindowOperator>(this->windowOperator)->groupIdToSequences) {
            for (auto sequence : sequences) {
                if (this->withinOperator &&
                    sequence->first_ts + this->withinOperator->ms <
                    this->windowOperator->current_external_time) {
                    continue;
                }
                ret.emplace_back(sequence);
            }
        }
        return ret;
    }

    void AddTuple() {
        auto now = Simulator::Now();
        /*if (Metrics::cep_tuples_time.empty()) {
            Metrics::cep_tuples_time.emplace_back(std::make_pair(Simulator::Now(), 1));
        } else {
            auto last_record = Metrics::cep_tuples_time[Metrics::cep_tuples_time.size()-1];
            if (last_record.first.GetNanoSeconds() == now.GetNanoSeconds()) {
                Metrics::cep_tuples_time[Metrics::cep_tuples_time.size()-1] = std::make_pair(Simulator::Now(), last_record.second + 1);
            } else {
                Metrics::cep_tuples_time.emplace_back(std::make_pair(Simulator::Now(), last_record.second + 1));
            }
        }*/
    }

    void RemoveTuples(int number_tuples) {
        auto now = Simulator::Now();
        /*auto last_record = Metrics::cep_tuples_time[Metrics::cep_tuples_time.size()-1];
        if (last_record.first.GetNanoSeconds() == now.GetNanoSeconds()) {
            Metrics::cep_tuples_time[Metrics::cep_tuples_time.size()-1] = std::make_pair(Simulator::Now(), last_record.second - number_tuples);
        } else {
            Metrics::cep_tuples_time.emplace_back(std::make_pair(Simulator::Now(), last_record.second - number_tuples));
        }*/
    }

    void AddSequence() {
        auto now = Simulator::Now();
        /*if (Metrics::cep_sequences_time.empty()) {
            Metrics::cep_sequences_time.emplace_back(std::make_pair(Simulator::Now(), 1));
        } else {
            auto last_record = Metrics::cep_sequences_time[Metrics::cep_sequences_time.size()-1];
            if (last_record.first.GetNanoSeconds() == now.GetNanoSeconds()) {
                Metrics::cep_sequences_time[Metrics::cep_sequences_time.size()-1] = std::make_pair(Simulator::Now(), last_record.second + 1);
            } else {
                Metrics::cep_sequences_time.emplace_back(std::make_pair(Simulator::Now(), last_record.second + 1));
            }
        }*/
    }

    void RemoveSequences(int number_sequences) {
        auto now = Simulator::Now();
        /*auto last_record = Metrics::cep_sequences_time[Metrics::cep_sequences_time.size()-1];
        if (last_record.first.GetNanoSeconds() == now.GetNanoSeconds()) {
            Metrics::cep_sequences_time[Metrics::cep_sequences_time.size()-1] = std::make_pair(Simulator::Now(), last_record.second - number_sequences);
        } else {
            Metrics::cep_sequences_time.emplace_back(std::make_pair(Simulator::Now(), last_record.second - number_sequences));
        }*/
    }

    void CepPatternOperator::process(Ptr<TupleWrapper> tupleWrapper) {
        //NS_LOG_INFO(Simulator::Now() << ": Node " << this->thread->resourceManager->node_id << " CepPatternOperator::process " << tuples_processed);
        int group_id = -1;
        if (tupleWrapper->group != nullptr) {
            group_id = tupleWrapper->group->id;
        }
        // Use the defines
        // Find out where the tupleWrapper fits in in every sequence
        std::vector<int> sequences_to_delete;
        auto windowOperator = DynamicCast<CepWindowOperator>(this->windowOperator);
        for (std::size_t i = 0; i < windowOperator->groupIdToSequences[group_id].size(); i++) {
            auto sequence = windowOperator->groupIdToSequences[group_id][i];
            std::string alias;
            for (auto define: this->defineOperators) {
                if (define->match(sequence, tupleWrapper)) {
                    alias = define->alias;
                    break;
                }
            }
            prev_sequence_cnt = sequence->current_state.length();
            if (!alias.empty()) {
                // Execute pattern matching with tupleWrapper as alias
                // First, for this sequence, set map from alias to tupleWrapper
                if (sequence != prev_sequence) {
                    prev_sequence = sequence;
                    node = this->node_id;
                }
                AddTuple();

                boost::match_results<std::string::const_iterator> what;
                sequence->current_state += alias;
                if (!sequence->addToSequence(alias, tupleWrapper) || 0 == boost::regex_match(sequence->current_state, what, this->regex,
                                            boost::match_default | boost::match_partial)) {
                    // Delete sequence
                    sequences_to_delete.push_back(i);
                } else {
                    if (what[0].matched) {
                        // Sequence is fulfilled, now delete sequence
                        sequences_to_delete.push_back(i);
                        //RemoveTuples(sequence->current_state.length());
                        if (shutting_down) {
                            std::size_t number_sequences = 0;
                            for (auto &[k, v]: windowOperator->groupIdToSequences) {
                                number_sequences += v.size();
                            }
                            if (number_sequences == sequences_to_delete.size()) {
                                // Stop here
                                ExposeCoordinator::nodeIdsToWrappers[std::to_string(
                                        this->thread->resourceManager->node_id)]->FinalizeWindowRecreation(this);
                            }
                        }
                        if (emit_all_tuples) {
                            for (auto[key, val]: sequence->aliasToTuples) {
                                for (auto tw: val) {
                                    emit(tw);
                                }
                            }
                        } else {
                            emit(tupleWrapper);
                        }
                    }
                }
            }
        }

        if (((windowOperator->groupIdToSequences[group_id].empty() && this->parallel_old_node_id == -1) || this->multiple_sequences) && !this->shutting_down) {
            auto new_sequence = CreateObject<SequencePartialState>(this);
            std::string alias;
            for (auto define: this->defineOperators) {
                if (define->match(new_sequence, tupleWrapper)) {
                    alias = define->alias;
                    break;
                }
            }
            if (!alias.empty()) {
                new_sequence->addToSequence(alias, tupleWrapper);
                new_sequence->current_state += alias;
                boost::match_results<std::string::const_iterator> what;
                if (0 == boost::regex_match(new_sequence->current_state, what, this->regex,
                                            boost::match_default | boost::match_partial)) {
                    // Delete sequence
                    //delete new_sequence;
                } else {
                    if (what[0].matched) {
                        // Sequence is fulfilled, now delete sequence
                        //delete new_sequence;
                        emit(tupleWrapper);
                    } else {
                        AddSequence();
                        windowOperator->addSequence(new_sequence, group_id);
                        //this->sequences.push_back(new_sequence);
                    }
                }
            } else {
                //delete new_sequence;
                // The sequence is never added
            }
        } else if (windowOperator->groupIdToSequences[group_id].empty()) {
            NS_LOG_INFO(Simulator::Now() << " Error! The tuple wasn't used for any sequence whatsoever!");
        }

        if (!sequences_to_delete.empty()) {
            std::reverse(sequences_to_delete.begin(), sequences_to_delete.end());
            long number_tuples = 0;
            for (auto index: sequences_to_delete) {
                for (auto &[alias, tuples] : windowOperator->groupIdToSequences[group_id][index]->aliasToTuples) {
                    number_tuples += tuples.size();
                }
                RemoveSequences(1);
                windowOperator->groupIdToSequences[group_id].erase(windowOperator->groupIdToSequences[group_id].begin() + index);
            }
            //RemoveTuples(number_tuples);

        }
    }

    Ptr<Operator> CepPatternOperator::copy(Ptr<Stream> stream) {
        auto copy = new CepPatternOperator(this->pattern, this->multiple_sequences, this->emit_all_tuples);
        copy->pattern = pattern;
        for (auto defineOperator: defineOperators) {
            copy->defineOperators.push_back(DynamicCast<DefineOperator>(defineOperator->copy(defineOperator->stream)));
        }

        copy->regex = regex;
        copy->multiple_sequences = multiple_sequences;
        copy->emit_all_tuples = emit_all_tuples;
        copy->groupOperator = groupOperator;
        Operator::copy(copy, stream);
        return copy;
    }

    void CepPatternOperator::parsePattern(const std::string &pattern) {
        // TODO: Set up filter operators, tuple index within tuple wrapper to get tuple cep symbol, etc
        std::cout << "Parsing pattern " << pattern << std::endl;
        this->regex = boost::regex(pattern);
    }

    size_t SequencePartialState::GetSerializedSize() {
        size_t size = 1;
        for (auto &[alias, tuples] : this->aliasToTuples) {
            for (auto tuple : tuples) {
                size += tuple->GetSerializedSize(true);
            }
        }
        return size;
    }

    size_t CepPatternOperator::GetSerializedSize() {
        size_t size = 1;
        auto cepWindowOperator = DynamicCast<CepWindowOperator>(this->windowOperator);
        int cnt = 0;
        int total_tuples = 0;
        for (auto &[group_id, sequences] : cepWindowOperator->groupIdToSequences) {
            for (auto sequence : sequences) {
                if (withinOperator &&
                    sequence->first_ts + withinOperator->ms <
                    cepWindowOperator->current_external_time) {
                    continue;
                }
                cnt++;
                size += sequence->GetSerializedSize();
                for (auto &[alias, tuples] : sequence->aliasToTuples) {
                    total_tuples += tuples.size();
                }
            }
        }
        NS_LOG_INFO("Migrating " << cnt << " sequences with " << size << " bytes and " << total_tuples << " tuples");
        return size;
    }

    size_t SequencePartialState::GetUtility() {
        size_t utility;
        utility = rand()%100+1;
        //utility = rand()%3+1; //((partial_state_index+23)*961748941) % 100; //((size_t)GetPointer(sequence)) % 100;
        //utility *= pow(sequence->GetSerializedSize()/100+1, 1.5);
        /*if (utility < 1) {
            utility = 1;
        }*/
        /*if (utility > 50) {
            utility *= 10;
        }*/
        return utility;
    }

    size_t CepPatternOperator::calculateUtility(int group_id, int partial_state_index) {
        size_t utility;
        auto cepWindowOperator = DynamicCast<CepWindowOperator>(this->windowOperator);
        NS_ASSERT((int) cepWindowOperator->groupIdToSequences[group_id].size() > partial_state_index);
        auto sequence = cepWindowOperator->groupIdToSequences[group_id][partial_state_index];
        utility = rand()%100+1;
        //utility = rand()%3+1; //((partial_state_index+23)*961748941) % 100; //((size_t)GetPointer(sequence)) % 100;
        //utility *= pow(sequence->GetSerializedSize()/100+1, 1.5);
        /*if (utility < 1) {
            utility = 1;
        }*/
        /*if (utility > 50) {
            utility *= 10;
        }*/
        return utility;
    }

    int CepPatternOperator::calculateMigrationSize(migration_types migration_type) {
        int payload_size = 0;
        //auto serializedCepPatternOperator = CepPatternOperator::Serialize(cepPatternOperator);
        switch(migration_type) {
            case STANDARD_MOVING_STATE: {
                payload_size += this->GetSerializedSize(); //serializedCepPatternOperator->GetSize();
                break;
            } case WINDOW_RECREATION: {
                payload_size += 74;
                // The old host should stop creating new sequences, and the new host should start creating new sequences
                break;
            } case STATE_RECREATION: {
                payload_size += this->GetSerializedSize(); //serializedCepPatternOperator->GetSize();
                break;
            }
          case STATELESS:
            break;
          }
        return payload_size;
    }

    void CepPatternOperator::ExtractState(Ptr<MigrationPlan> mp) {
        auto batch = CreateObject<PartialStateBatch>(this, mp);
        for (auto &[group_id, sequences] : DynamicCast<CepWindowOperator>(this->windowOperator)->groupIdToSequences) {
            for (auto sequence : sequences) {
                auto ps_wrapper = CreateObject<PartialStateWrapper>(sequence);
                batch->partial_states.emplace_back(ps_wrapper);
            }
        }

        mp->critQueue.emplace_back(batch);
    }

    void CepPatternOperator::ImportState(Ptr<PartialStateBatch> batch) {

    }
}
