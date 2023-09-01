//
// Created by espen on 10.10.2022.
//

#include "ns3/Operator.h"

namespace ns3 {
    NS_OBJECT_ENSURE_REGISTERED(MigrationPlan);
    NS_LOG_COMPONENT_DEFINE ("MigrationPlan");

    TypeId
    MigrationPlan::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::MigrationPlan")
                .SetParent<Object>();

        return tid;
    }

    MigrationPlan::MigrationPlan(std::string migration_mode) {
        this->migration_mode = migration_mode;
        if (migration_mode == "rhino") {
            this->migration_mechanism = CreateObject<RhinoMigrationMechanism>(this);
        } else if (migration_mode == "lazy") {
            this->migration_mechanism = CreateObject<LazyLatencyMigrationMechanism>(this);
        } else if (migration_mode == "utility") {
            this->migration_mechanism = CreateObject<LazyUtilityMigrationMechanism>(this);
        } else if (migration_mode == "megaphone") {
            this->migration_mechanism = CreateObject<MegaphoneMigrationMechanism>(this);
        } else if (migration_mode == "meces") {
            this->migration_mechanism = CreateObject<MecesMigrationMechanism>(this);
        } else if (migration_mode == "all-at-once") {
            this->migration_mechanism = CreateObject<AllAtOnceMigrationMechanism>(this);
        } else if (migration_mode == "drop-state") {
            this->migration_mechanism = CreateObject<DropStateMigrationMechanism>(this);
        } else {
            NS_FATAL_ERROR(Simulator::Now().GetMilliSeconds() << " Invalid migration mechanism " << migration_mode << " selected!");
        }
    }

    void MigrationPlan::InsertBatchCritQueue(Ptr<PartialStateBatch> batch) {
        critQueue.push_back(batch);
    }

    void MigrationPlan::InsertBatchFetchQueue(Ptr<PartialStateBatch> batch) {
        fetchQueue.push_back(batch);
    }

    Ptr<PartialStateBatch> MigrationPlan::GetPreambleBatch() {
        Ptr<PartialStateBatch> next_batch = nullptr;
        if (!this->preambleQueue.empty()) {
            next_batch = this->preambleQueue.front();
            this->preambleQueue.pop_front();
            auto old_host_op = next_batch->op;
            NS_LOG_INFO(
                    Simulator::Now().GetMilliSeconds() << " Moving preamble batch " << next_batch->id << " of operator "
                                                       << old_host_op->id << " to "
                                                       << this->operatorIdToNewHost[old_host_op->id]);
            // No operator should have more than one preamble batch
            NS_ASSERT(std::find_if(this->preambleQueue.begin(), this->preambleQueue.end(), [old_host_op](Ptr<PartialStateBatch> b) { return b->op->id == old_host_op->id; }) == std::end(this->preambleQueue));
            // TODO: This can't be right for Megaphone or Rhino: //old_host_op->shutting_down = true;
            next_batch->next_host = this->operatorIdToNewHost[old_host_op->id];

            this->migration_mechanism->PrepareOperatorPreambleState(old_host_op, next_batch);

            //new_host_op->SetInactive ();
            /*if (this->migration_mode != "megaphone")
            {
                old_host_op->SetInactive ();
                // Forward tuples to new host
                NS_ASSERT(next_batch->next_host != -1);
                old_host_op->ForwardTuplesTo (next_batch->next_host);
                //old_host_op->UnassignCpuCore ();
            }
            if (this->preambleQueue.empty()) {
                ExposeCoordinator::UpdateDeployedQueries();
                ExposeCoordinator::UpdateOperatorPlacements();
            }*/
        }

        return next_batch;
    }

    Ptr<PartialStateBatch> MigrationPlan::GetCritBatch() {
        Ptr<PartialStateBatch> next_batch = nullptr;
        if (!this->critQueue.empty()/* && (next_batch == nullptr || (next_batch->partial_states.size() < 100 && this->critQueue.front()->op == next_batch->op))*/) {
            if (next_batch == nullptr) {
                next_batch = this->critQueue.front();
            } else {
                next_batch->merge(this->critQueue.front());
            }
            this->critQueue.pop_front();
            NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Moving critical batch " << next_batch->id
                                                           << " of operator " << next_batch->op->id << " to "
                                                           << this->operatorIdToNewHost[next_batch->op->id]);
            next_batch->op->DisableState(next_batch);
            if (!this->critQueue.empty() && this->critQueue.front()->op == next_batch->op) {
                next_batch->active_op = false;
                next_batch->op_final_batch = false;
            }
            /*if (next_batch->active_op && this->migration_mode == "rhino") {
                // Now, we extract the final state of the operator, which is the diff
                next_batch->op->ExtractDiffState(mp);
            }*/
            //next_batch->op->DisableState(next_batch);
            //break;
        }

        return next_batch;
    }

    Ptr<PartialStateBatch> MigrationPlan::GetFetchBatch() {
        Ptr<PartialStateBatch> next_batch = nullptr;

        if (!this->fetchQueue.empty()/* && (next_batch == nullptr || (next_batch->partial_states.size() < 100 &&
                                                                     this->fetchQueue.front()->op == next_batch->op))*/) {
            if (!this->deadlineQueue.empty() && !this->deadlineQueue.front()->canBeSentAfter(this->fetchQueue.front())) {
                if (next_batch == nullptr) {
                    next_batch = this->deadlineQueue.front();
                } else {
                    next_batch->merge(this->deadlineQueue.front());
                }
                NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Moving deadline batch " << next_batch->id
                                                               << " of operator " << next_batch->op->id << " to "
                                                               << this->operatorIdToNewHost[next_batch->op->id]);
                this->deadlineQueue.pop_front();
            } else {
                if (next_batch == nullptr) {
                    next_batch = this->fetchQueue.front();
                } else {
                    next_batch->merge(this->fetchQueue.front());
                }
                NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Moving fetch batch " << next_batch->id
                                                               << " of operator " << next_batch->op->id << " to "
                                                               << this->operatorIdToNewHost[next_batch->op->id]);
                this->fetchQueue.pop_front();
            }

            // Espen: Surely, we've already disabled the state?
            //next_batch->op->DisableState(next_batch);
        }

        return next_batch;
    }

    Ptr<PartialStateBatch> MigrationPlan::GetEssentialBatch() {
        Ptr<PartialStateBatch> next_batch = nullptr;

        if (!this->normalEssentialQueue.empty()/* && (next_batch == nullptr ||
                                                     (next_batch->partial_states.size() < 100 &&
                                                      this->normalEssentialQueue.front()->op == next_batch->op))*/) {
            if (!this->deadlineQueue.empty() &&
                !this->deadlineQueue.front()->canBeSentAfter(this->normalEssentialQueue.front())) {
                if (next_batch == nullptr) {
                    next_batch = this->deadlineQueue.front();
                } else {
                    next_batch->merge(this->deadlineQueue.front());
                }
                NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Moving deadline batch " << next_batch->id
                                                               << " of operator " << next_batch->op->id << " to "
                                                               << this->operatorIdToNewHost[next_batch->op->id]);
                this->deadlineQueue.pop_front();
            } else {
                if (next_batch == nullptr) {
                    next_batch = this->normalEssentialQueue.front();
                } else {
                    next_batch->merge(this->normalEssentialQueue.front());
                }
                NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Moving essential batch " << next_batch->id
                                                               << " of operator " << next_batch->op->id << " to "
                                                               << this->operatorIdToNewHost[next_batch->op->id]);
                this->normalEssentialQueue.pop_front();
            }
            next_batch->op->DisableState(next_batch);
        }

        return next_batch;
    }

    Ptr<PartialStateBatch> MigrationPlan::GetDeadlineBatch() {
        Ptr<PartialStateBatch> next_batch = nullptr;

        if (next_batch == nullptr) {
            if (!this->deadlineQueue.empty()/* && (next_batch == nullptr || (next_batch->partial_states.size() < 100 &&
                                                                        this->deadlineQueue.front()->op ==
                                                                        next_batch->op))*/) {
                if (next_batch == nullptr) {
                    next_batch = this->deadlineQueue.front();
                } else {
                    next_batch->merge(this->deadlineQueue.front());
                }
                NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Moving deadline batch " << next_batch->id
                                                               << " of operator " << next_batch->op->id << " to "
                                                               << this->operatorIdToNewHost[next_batch->op->id]);
                this->deadlineQueue.pop_front();
                next_batch->op->DisableState(next_batch);
            }
        }

        return next_batch;
    }

    Ptr<PartialStateBatch> MigrationPlan::GetUnessentialBatch() {
        Ptr<PartialStateBatch> next_batch = nullptr;

        if (!this->normalUnessentialQueue.empty()/* && (next_batch == nullptr || (next_batch->partial_states.size() < 100 &&
                                                                             this->normalUnessentialQueue.front()->op ==
                                                                             next_batch->op))*/) {
            if (next_batch == nullptr) {
                next_batch = this->normalUnessentialQueue.front();
            } else {
                next_batch->merge(this->normalUnessentialQueue.front());
            }
            NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Moving unessential batch " << next_batch->id
                                                           << " of operator " << next_batch->op->id << " to "
                                                           << this->operatorIdToNewHost[next_batch->op->id]);
            this->normalUnessentialQueue.pop_front();
            next_batch->op->DisableState(next_batch);
        }

        return next_batch;
    }

    Ptr<PartialStateBatch> MigrationPlan::GetNextBatch() {
        Ptr<PartialStateBatch> next_batch = GetPreambleBatch();
        if (next_batch == nullptr) {
            // Do the Rhino thing
            if (!this->preamble_states_sent) {
                // All preamble states have now been sent, and it's the first time the code has reached here
                this->preamble_states_sent = true;
                this->migration_mechanism->AllPreambleStatesSent();
            }
            next_batch = GetCritBatch();
        }
        if (next_batch == nullptr) {
            this->crit_states_sent = true;
            next_batch = GetFetchBatch();
        }
        if (next_batch == nullptr) {
            next_batch = GetDeadlineBatch();
        }
        if (next_batch == nullptr) {
            this->deadline_states_sent = true;
            next_batch = GetEssentialBatch();
        }
        if (next_batch == nullptr) {
            this->essential_states_sent = true;
            next_batch = GetUnessentialBatch();
        }

        if (next_batch == nullptr) {
            this->finished = true;
            std::cout << "Migration finished at " << Simulator::Now().GetMilliSeconds() << std::endl;
            return nullptr;
        }
        int op_id = next_batch->op->id;
        next_batch->old_host = this->old_host;
        next_batch->next_host = this->operatorIdToNewHost[op_id];
        NS_ASSERT(next_batch->old_host != -1 && next_batch->next_host != -1);

        for (auto batch: this->fetchQueue) {
            if (batch->op->id == next_batch->op->id) {
                next_batch->op_final_batch = false;
                break;
            }
        }
        for (auto batch: this->normalEssentialQueue) {
            if (batch->op->id == next_batch->op->id) {
                next_batch->op_final_batch = false;
                break;
            }
        }
        for (auto batch: this->deadlineQueue) {
            if (batch->op->id == next_batch->op->id) {
                next_batch->op_final_batch = false;
                break;
            }
        }
        for (auto batch: this->normalUnessentialQueue) {
            if (batch->op->id == next_batch->op->id) {
                next_batch->op_final_batch = false;
                break;
            }
        }
        switch (next_batch->op->type) {
            case joinType:
                if (this->migration_mode == "lazy" || this->migration_mode == "utility") {
                    this->operatorIdToNumberPhases[next_batch->op->id] = 2;
                } else {
                    this->operatorIdToNumberPhases[next_batch->op->id] = 1;
                }
                break;
            case groupType:
                if (this->migration_mode == "utility") {
                    this->operatorIdToNumberPhases[next_batch->op->id] = 2;
                } else {
                    this->operatorIdToNumberPhases[next_batch->op->id] = 1;
                }
                break;
            case windowType:
            case inputType:
            case filterType:
            case selectType:
            case printType:
            case sinkType:
            case cepType:
            case defineType:
            case forwardType:
            case withinType:
            case passthroughType:
                this->operatorIdToNumberPhases[this->operator_id] = 1;
                break;
        }
        next_batch->mp = this;
        next_batch->dcepHeader.arguments["last-batch"] = this->Finished();
        ExposeCoordinator::nodeIdsToWrappers[std::to_string(next_batch->next_host)]->nodeToMigrationPayloads[next_batch->old_host][next_batch->id] = next_batch;
        //std::cout << "Size of payload buffer: " << ExposeCoordinator::nodeIdsToWrappers[std::to_string(next_batch->next_host)]->nodeToMigrationPayloads[next_batch->old_host].size() << std::endl;
        next_batch->dcepHeader.arguments["active_op"] = next_batch->active_op;
        this->migration_mechanism->PrepareBatchForSending(next_batch);
        // This is not done at the moment
        /*for (auto &ps : next_batch->partial_states) {
            ps->object->in_migration = true;
        }*/
        // Second thing we're doing is disabling state related to the keys in the batch on the old host, e.g., processing time or external time windows
        /*next_batch->op->DisableState(next_batch);
        // We make sure that the below is only done if it's not the final batch of an operator, because we need to send those batches regardless
        // TODO: Fix the below. There's definitely some weird stuff going on here. If we do the below, then much fewer partial states may be sent, and results may be affected, even though it doesn't look like it should happen
        if (next_batch->partial_states.empty() && !op_final_batch && !this->Finished()) {
            // If now the batch is empty after disabling state and finding out that the partial states we had are no longer relevant (e.g., extents are already sent),
            // we just remove it and move to the next state
            MoveNextState();
            return;
        }*/
        for (auto &ps: next_batch->partial_states) {
            Metrics::TracePartialStateSent(ps, next_batch->op->old_node_id, next_batch->op->query, next_batch->op);
        }

        if (next_batch->op_final_batch) {
            this->migration_mechanism->OperatorFinalBatchReady(next_batch->op, next_batch);
        }

        return next_batch;
    }

    void MigrationPlan::InsertOperator(Ptr<Operator> op) {
        bool inserted = false;
        for (auto it = operators.rbegin(); it != operators.rend(); ++it) {
            auto other_op = *it;
            auto s = other_op->stream;
            bool is_after = true;
            while (s != nullptr) {
                for (auto inner_op: s->operators) {
                    if (inner_op == op) {
                        is_after = false;
                        break;
                    }
                }
                if (!is_after) {
                    break;
                }
                s = s->next_stream;
            }
            if (!is_after) {
                // Convert reverse_iterator to iterator for insert operation
                operators.insert(it.base(), op);
                inserted = true;
                break;
            }
        }
        if (!inserted) {
            operators.insert(operators.begin(), op);
        }
    }

    void MigrationPlan::InsertBatchDeadlineQueue(Ptr<DeadlinePartialStateBatch> batch) {
        deadlineQueue.push_back(batch);
    }

// This method does not consider state shedding, where you might explicitly decide to drop state
    bool MigrationPlan::Finished() {
        return critQueue.empty() && normalEssentialQueue.empty() && deadlineQueue.empty() && fetchQueue.empty();
    }

}
