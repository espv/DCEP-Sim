//
// Created by espen on 27.06.23.
//

#include "ns3/Operator.h"

namespace ns3 {
    NS_OBJECT_ENSURE_REGISTERED(MigrationMechanism);
    NS_LOG_COMPONENT_DEFINE ("MigrationMechanism");

    TypeId
    MigrationMechanism::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::MigrationMechanism")
                .SetParent<Object> ()
        ;

        return tid;
    }

    TypeId
    AllAtOnceMigrationMechanism::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::AllAtOnceMigrationMechanism")
                .SetParent<MigrationMechanism> ()
        ;

        return tid;
    }

    TypeId
    RhinoMigrationMechanism::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::RhinoMigrationMechanism")
                .SetParent<MigrationMechanism> ()
        ;

        return tid;
    }

    TypeId
    LazyLatencyMigrationMechanism::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::LazyLatencyMigrationMechanism")
                .SetParent<MigrationMechanism> ()
        ;

        return tid;
    }

    TypeId
    LazyUtilityMigrationMechanism::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::LazyUtilityMigrationMechanism")
                .SetParent<MigrationMechanism> ()
        ;

        return tid;
    }

    TypeId
    MecesMigrationMechanism::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::MecesMigrationMechanism")
                .SetParent<MigrationMechanism> ()
        ;

        return tid;
    }

    TypeId
    MegaphoneMigrationMechanism::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::MegaphoneMigrationMechanism")
                .SetParent<MigrationMechanism> ()
        ;

        return tid;
    }

    TypeId
    DropStateMigrationMechanism::GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::DropStateMigrationMechanism")
                .SetParent<MigrationMechanism> ()
        ;

        return tid;
    }

    MigrationMechanism::MigrationMechanism(Ptr<MigrationPlan> mp) {
        this->mp = mp;
    }

    DropStateMigrationMechanism::DropStateMigrationMechanism(Ptr<MigrationPlan> mp) : MigrationMechanism(mp) {
        run_old_and_new_during_migration = false;
    }

    AllAtOnceMigrationMechanism::AllAtOnceMigrationMechanism(Ptr<MigrationPlan> mp) : MigrationMechanism(mp) {
        run_old_and_new_during_migration = false;
    }
    MegaphoneMigrationMechanism::MegaphoneMigrationMechanism(Ptr<MigrationPlan> mp) : MigrationMechanism(mp) {
        run_old_and_new_during_migration = true;
    }

    MecesMigrationMechanism::MecesMigrationMechanism(Ptr<MigrationPlan> mp) : MigrationMechanism(mp) {
        run_old_and_new_during_migration = false;
    }

    RhinoMigrationMechanism::RhinoMigrationMechanism(Ptr<MigrationPlan> mp) : MigrationMechanism(mp) {
        run_old_and_new_during_migration = true;
    }

    LazyLatencyMigrationMechanism::LazyLatencyMigrationMechanism(Ptr<MigrationPlan> mp) : MigrationMechanism(mp) {}
    LazyUtilityMigrationMechanism::LazyUtilityMigrationMechanism(Ptr<MigrationPlan> mp) : MigrationMechanism(mp) {}

    void MigrationMechanism::OperatorFinalBatchReady(Ptr<Operator> op, Ptr<PartialStateBatch> batch) {
        if (run_old_and_new_during_migration) {
            batch->op->SetInactive();
            // Forward tuples to new host
            int nh = mp->operatorIdToNewHost[batch->op->id];
            NS_ASSERT(nh != -1);
            batch->op->ForwardTuplesTo(nh);
            //next_batch->op->UnassignCpuCore();
        }
    }

    void MigrationMechanism::BeforeFirstMoveNextState() {
        if (mp->migration_mode != "rhino") {
            ExposeCoordinator::UpdateDeployedQueries();
            ExposeCoordinator::UpdateOperatorPlacements();
        }
    }

    void MigrationMechanism::ExtractOperatorPreambleState(Ptr<Operator> old_host_op, Ptr<Operator> new_host_op) {
        for (int i = 0; i < (int) old_host_op->stream->operators.size (); i++) {
            //mp->critQueue.emplace_back (static_batch);
            old_host_op->stream->operators[i]->ExtractPreambleState (this->mp);
            mp->InsertOperator(old_host_op->stream->operators[i]);

            new_host_op->stream->operators[i]->in_migration = true;
            new_host_op->stream->operators[i]->old_node_id = old_host_op->thread->resourceManager->node_id;
            old_host_op->stream->operators[i]->old_node_id = old_host_op->thread->resourceManager->node_id;
            new_host_op->stream->operators[i]->SetInactive();
            //new_host_op->stream->operators[i]->Assign();
            old_host_op->stream->operators[i]->in_migration = true;

            if (!run_old_and_new_during_migration) {
                old_host_op->SetInactive();
                // Forward tuples to new host
                int next_host = mp->operatorIdToNewHost[old_host_op->id];
                NS_ASSERT(next_host != -1);
                old_host_op->ForwardTuplesTo(next_host);
                //old_host_op->UnassignCpuCore();
            }
        }
    }

    void MigrationMechanism::PrepareOperatorPreambleState(Ptr<Operator> op, Ptr<PartialStateBatch> batch) {
        if (!op->stateful || mp->migration_mode != "rhino") {
            op->ExtractState(mp);
        }
        // For Rhino, this is important, because we haven't necessarily extracted all the state yet, but we might have
        //if (op->HasMoreState()) {
        if (op->stateful && mp->migration_mode == "rhino") {
            batch->active_op = false;
            batch->op_final_batch = false;
        }

        // TODO: This must be fixed. If we let the operator run while the migration runs,
        // TODO: the operator will build up state, and then unless it's Rhino, it will fail
        // TODO: So, basically, Rhino should be the exception when it comes to the "activate new host later".
        // TODO: That's the big problem here, that we apply it to all operators?????
        for (auto crit_batch: mp->critQueue) {
            if (crit_batch->op == batch->op) {
                batch->active_op = false;
                batch->op_final_batch = false;
                break;
            }
        }
    }

    void RhinoMigrationMechanism::AllPreambleStatesSent() {
        for (auto op: mp->operators) {
            if (mp->migration_mode == "rhino") {
                NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Rhino setting operator " << op->id
                                                               << " inactive");
                op->SetInactive();
                // Forward tuples to new host
                auto new_host = mp->operatorIdToNewHost[op->id];
                NS_ASSERT(new_host!= -1);
                op->ForwardTuplesTo(new_host);
                //op->UnassignCpuCore();
                op->ExtractDiffState(mp);
                //op->ClearState();
            }
            ExposeCoordinator::UpdateDeployedQueries();
            ExposeCoordinator::UpdateOperatorPlacements();
        }
    }

    void MigrationMechanism::PrepareBatchForSending(Ptr<PartialStateBatch> batch) {
        // TODO: Make the operator activation independent of the partial state queues that are empty or used
        // TODO: The current logic works for most cases, but not here, and I'd rather have something that is generalizable
        // The reason for this is that Utility mode uses the state queues for prioritization, not for minimizing downtime.
        // If the migration is interrupted, the operator is enabled again
        if (mp->migration_mode == "utility") {
            batch->dcepHeader.arguments["active_op"] = batch->op_final_batch;
        }
        batch->dcepHeader.arguments["batch-id"] = batch->id;
        batch->dcepHeader.arguments["op_final_batch"] = batch->op_final_batch;
        batch->dcepHeader.SetFromNode(batch->old_host);
        batch->dcepHeader.setContentSize(batch->GetSerializedSize());
        batch->in_migration = true;

        if (mp->migration_mode == "megaphone") {
            // First thing we're doing is telling the old host to forward tuples of certain keys to the new host
            for (auto &ps: batch->partial_states) {
                auto group = DynamicCast<Group>(ps->object);
                if (group) {
                    Operator::queryIdToOperatorIdToGroupIdToNewHost[batch->op->query->id][batch->op->id][group->id] = true;
                }
                auto tw = DynamicCast<TupleWrapper>(ps->object);
                if (tw) {
                    Operator::queryIdToOperatorIdToGroupIdToNewHost[batch->op->query->id][batch->op->id][tw->group->id] = true;
                }
                auto extent = DynamicCast<WindowExtent>(ps->object);
                if (extent) {
                    Operator::queryIdToOperatorIdToGroupIdToNewHost[batch->op->query->id][batch->op->id][extent->group->id] = true;
                }
            }
        }
    }
}
