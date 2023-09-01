
#include "ns3/dcep-module.h"
#include "ns3/type-id.h"
#include "ns3/object.h"
#include "ns3/stream-processing-module.h"
#include "ns3/inet-socket-address.h"
#include "ns3/Query.h"
#include "ns3/Metrics.h"
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/write.hpp>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iostream>
#include <chrono>
#include <sstream>
#include <string>
#include <iomanip>
#include <ns3/data-rate.h>
#include <ns3/point-to-point-module.h>
#include <yaml-cpp/yaml.h>
#include <filesystem>

using namespace std;
using namespace ns3;

NS_OBJECT_ENSURE_REGISTERED(ExposeWrapper);
NS_LOG_COMPONENT_DEFINE ("ExposeWrapper");

TypeId
ExposeWrapper::GetTypeId(void) {
  static TypeId tid = TypeId("ns3::ExposeWrapper")
          .SetParent<Object> ()
  ;

  return tid;
}

TypeId
TupleProcessWrapper::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::TupleProcessWrapper")
            .SetParent<Object> ()
    ;

    return tid;
}

void ExposeWrapper::FinishTask() {
    // After changing the way deployQuery is called, some curTask's will be null
    if (curTask == nullptr) {
        return;
    }
    NS_LOG_INFO(Simulator::Now() << log_prefix << "FinishTask");
    if (curTask->GetYamlNode()["parallel"]) {
        if (curTask->GetYamlNode()["parallel"].as<bool>()) {
            return;
        }
    }
    Simulator::ScheduleNow (&ExposeCoordinator::FinishedTask, coordinator);
}

void
ExposeWrapper::FinalizeWindowRecreation(Ptr<Operator> op) {
    NS_LOG_INFO(Simulator::Now() << ": FinalizeWindowRecreation");
    auto base_query = ExposeCoordinator::queryIdToBaseQuery[op->query->unique_query_id];
    Ptr<Operator> base_op;
    for (auto stream_id : op->stream->input_stream_ids) {
        base_op = base_query->streamIdToOperatorIdToOperator[stream_id][op->id];
        if (base_op) {
            break;
        }
    }
    if (!base_op) {
        // Error
        NS_LOG_INFO("Error, operator not found in FinalizeWindowRecreation");
        exit(153);
    }
    base_op->parallel_old_node_id = -1;
    op->active = false;
    op->shutting_down = false;
    ExposeCoordinator::UpdateDeployedQueries();
    ExposeCoordinator::UpdateOperatorPlacements();
}

auto rng = std::default_random_engine {};

std::vector<std::tuple<int, int, double, size_t, double>>
CalculateUtility(Ptr<Operator> old_op, bool load_shed, long bitrate, long payload_size, DcepHeader &dcepHeader, Time deadline) {
    std::vector<std::tuple<int, int, double, size_t, double>> ret;
    std::vector<std::tuple<int, int, double, size_t, double>> ret_original;
    std::vector<std::tuple<int, int, double, size_t, double>> ret_pre_optimized;
    std::vector<std::tuple<int, int, double, size_t, double>> ret_optimized;
    auto dcepHeader_size = dcepHeader.GetSerializedSize();

    long double max_utility = 0;
    //int number_sequences = 0;
    auto partial_states = old_op->getPartialStates();
    for (int i = 0; i < (int)partial_states.size(); i++) {
        // Here we skip sequences that are outdated
        auto partial_state = partial_states[i];
        int utility = partial_state->GetUtility();
        if (utility > max_utility) {
            max_utility = utility;
        }
        double size = partial_state->GetSerializedSize();
        double utility_density = utility / size;
        ret_original.emplace_back(std::tuple(partial_state->group_id, i, utility, size, utility_density));
        //NS_LOG_INFO("Utility: " << utility << ", size: " << size << ", Utility density: " << utility_density);
    }
    // We sum up all the utilities because we already iterate through them and it reduces the space complexity of the knapsack solver
    uint64_t total_utility = 0;
    //int ret_index = 0;
    //int all_utilities[ret_original.size()];
    //float epsilon = 0.99;
    //float k = (1-epsilon)*max_utility/(float)ret.size();
    float k = 1;
    auto l = 0;
    for (auto entry: ret_original) {
        long double original_utility = std::get<2>(entry);
        long long utility = original_utility;
        //if (((double)original_utility) / ((double)max_utility) >= 0.5) {
        //    utility *= 10;
        //}
        //all_utilities[i] = utility;
        //total_utility += utility;
        double size = std::get<3>(entry);
        double utility_density = utility / size;

        //ret_pre_optimized.push_back(
        //        std::tuple(std::get<0>(entry), std::get<1>(entry), utility, size, utility_density));
        ret_original[l++] = std::tuple(std::get<0>(entry), std::get<1>(entry), utility, size, utility_density);
        //NS_LOG_INFO("Utility: " << utility << ", size: " << size << ", Utility density: " << utility_density);
    }
    max_utility *= 10;
    l = 0;
    for (auto entry: ret_original) {
        long double original_utility = std::get<2>(entry);
        //int utility = original_utility / (double)max_utility * 100.0 + 1;
        int utility = original_utility / max_utility*100.0+1;//std::floor(original_utility/k);
        //all_utilities[i] = utility;
        total_utility += utility;
        double size = std::get<3>(entry);
        double utility_density = utility / size;

        ret_pre_optimized.push_back(
                std::tuple(std::get<0>(entry), std::get<1>(entry), utility, size, utility_density));
        ret_original[l++] = std::tuple(std::get<0>(entry), std::get<1>(entry), utility, size, utility_density);
        //NS_LOG_INFO("Utility: " << utility << ", size: " << size << ", Utility density: " << utility_density);
    }
    //int gcd = Utility::findGCD(all_utilities, ret.size());
    load_shed = true;
    bool random = false;
    bool drop_all_state = true;
    //Metrics::cep_tuples_time.emplace_back(Simulator::Now(),Metrics::cep_tuples_time[Metrics::cep_tuples_time.size()-1].second);
    //Metrics::cep_tuples_time.emplace_back(Simulator::Now(),Metrics::cep_tuples_time[Metrics::cep_tuples_time.size()-1].second);

    //Metrics::cep_sequences_time.emplace_back(Simulator::Now(), Metrics::cep_sequences_time[
    //        Metrics::cep_sequences_time.size() - 1].second);
    //Metrics::cep_sequences_time.emplace_back(Simulator::Now(), Metrics::cep_sequences_time[
    //        Metrics::cep_sequences_time.size() - 1].second);

    NS_LOG_INFO(Simulator::Now() << " Number states: " << ret.size());
    bool running = true;
    for (int a = 0; a < 1 && running; a+=100) {
        /*if (a <= 100 || a <= 110) {
            a -= 99;
        } else if (a == 111) {
            a = 100;
            //continue;
        }*/
        ret.clear();
        for (auto entry : ret_original) {
            ret.emplace_back(entry);
        }
        ret_optimized.clear();
        auto new_deadline = deadline + MilliSeconds(a);
        std::sort(ret.begin(), ret.end(), UtilityDensityComparator());
        //KnapsackSolver knapsackSolver;
        double ms_until_deadline = new_deadline.GetMilliSeconds();
        double bits_per_ms = bitrate / 1000;
        long capacity = (ms_until_deadline) * bits_per_ms / 8;
        int number_packets = capacity / 536;
        // For instance, if capacity == 537, number_packets == 1, but capacity / 536.0 == 1.00186567164
        if (capacity / 536.0 > number_packets) {
            ++number_packets;
        }
        // Total packet size plus
        capacity -= 54 * number_packets;
        capacity -= dcepHeader_size;
        if (capacity < 0) {
            NS_LOG_INFO(Simulator::Now() << " Failed to move operator " << old_op->id);
            continue;
            //return {};
        }
        long greedy_total_utility = 0;
        long greedy_size_cnt = 0;
        for (int i = 0; i < (int)ret.size(); i++) {
            greedy_size_cnt += std::get<3>(ret[i]);
            if (greedy_size_cnt > capacity) {
                break;
            }
            greedy_total_utility += std::get<2>(ret[i]);
        }
        dcepHeader.setContentSize(std::min(payload_size, capacity));
        long result[ret.size()];
        for (int i = 0; i < (int)ret.size(); i++) {
            result[i] = 0;
        }
        //long achieved_utility = (long) knapsackSolver.dynamic_programming(ret_pre_optimized.size(), capacity,
        //                                                           ret_pre_optimized, result,
        //                                                           total_utility);

        long long ut = 0;
        long bytes_to_move = 0;
        //long long total_utility = 0;
        for (int i = 0; i < (int)ret_pre_optimized.size(); i++) {
            auto state_utility = std::get<2>(ret_pre_optimized[i]) * k;
            if (state_utility == 0) {
                NS_LOG_INFO("");
            }
            if (result[i] == 1) {
                ret_optimized.emplace_back(ret[i]);
                bytes_to_move += std::get<3>(ret_pre_optimized[i]);
                ut += state_utility;
            }
            total_utility += state_utility;
        }
        std::shuffle(std::begin(ret), std::end(ret), rng);
        long rand_bytes_to_move = 0;
        long rand_utility = 0;
        for (int i = 0; i < (int)ret.size(); i++) {
            long bytes_in_state = std::get<3>(ret[i]);
            if (rand_bytes_to_move + bytes_in_state > capacity) {
                break;
            }
            rand_bytes_to_move += bytes_in_state;
            rand_utility += std::get<2>(ret[i]);
        }
        if (load_shed) {
            ret.clear();
            for (auto entry : ret_optimized) {
                ret.emplace_back(entry);
            }
        } else if (random) {
            std::shuffle(std::begin(ret), std::end(ret), rng);
            for (int i = 0; i < (int)ret.size(); i++) {
                long bytes_in_state = std::get<3>(ret[i]);
                if (rand_bytes_to_move + bytes_in_state > capacity) {
                    while ((int)ret.size() > i) {
                        ret.pop_back();
                    }
                    break;
                }
                rand_bytes_to_move += bytes_in_state;
            }
        } else if (drop_all_state) {
            ret = {};
        }
        if ((uint64_t) ut == total_utility) {
            running = false;
        }
        //Metrics::cep_sequences_time.emplace_back(Simulator::Now(), ret.size());
        auto optimal_utility = ((double) ut / (double) total_utility);
        auto greedy_utility = ((double) greedy_total_utility / (double) total_utility);
        auto random_utility = ((double) rand_utility / (double) total_utility);
        NS_LOG_INFO(a << " " << ret_optimized.size() << " " << optimal_utility << " " << greedy_utility
                                 << " " << random_utility);
        //NS_LOG_INFO("Migrating " << ret_optimized.size() << " states, i.e., " << ut << " ("
        //                         << ((double) ut / (double) total_utility)
        //                         << ") utility using state shedding, greedy utility: "
        //                         << greedy_total_utility << "("
        //                         << ((double) greedy_total_utility / (double) total_utility)
        //                         << "), random utility: " << rand_utility << " ("
        //                         << ((double) rand_utility / (double) total_utility)
        //                         << "), total utility: "
        //                         << total_utility << " and " << bytes_to_move << " bytes, capacity of "
        //                         << capacity << ", iteration: " << a);
        /*for (int i = 0; i < ret.size(); i++) {
            if (result[i] == 0) {
                ret_optimized.emplace_back(ret[i]);
            }
        }*/
        //ret = ret_optimized;
        //NS_LOG_INFO("");
    }

    return ret;
    //return ret;
}

std::size_t sum_of_batches = 0;
void ExposeWrapper::MoveNextState() {
    auto cur_node = StreamQuery::nodeIdToNode[std::stoi(this->node_id)];
    auto dcep = Dcep::GetDcepFromNode(cur_node);
    if (dcep->c_communication->currently_migrating) {
        return;
    }
    auto next_batch = mp->GetNextBatch();
    if (next_batch == nullptr) {
        return;
    }

    auto comm = dcep->c_communication;
    static int cnt = 0;
    static int sum = 0;
    auto batch_id = next_batch->dcepHeader.arguments["batch-id"];
    sum += next_batch->GetSerializedSize();
    NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " Sending batch " << ++cnt << " with operator " << next_batch->op->id << " (batch-id: " << batch_id << ") with size " << next_batch->GetSerializedSize() << " bytes, number partial states: " << next_batch->partial_states.size() << ", header size: " << next_batch->dcepHeader.GetSerializedSize() << " and content size: " << next_batch->dcepHeader.GetContentSize() << ", total sum: " << sum << ", final batch: " << mp->Finished() << ", op final batch: " << next_batch->op_final_batch);
    Simulator::Schedule(MilliSeconds(0), &Communication::SendState, dcep->c_communication, next_batch->old_host, next_batch->next_host, next_batch->dcepHeader.GetContentSize(), next_batch->dcepHeader);
}

void ExposeWrapper::FetchNextPartialState() {
    auto comm = Dcep::GetDcepFromNode(StreamQuery::nodeIdToNode[std::stoi(node_id)])->c_communication;
    auto old_host = ps_fetch_queue.front().first;
    if (ps_fetch_queue.empty()) {
        return;
    }
    Ptr<PartialStateWrapper> ps;
    int count = 0;
    auto old_host_wrapper = ExposeCoordinator::nodeIdsToWrappers[std::to_string(old_host)];
    // TODO: Generalize the 10000 into a variable based on migration policy
    bool actually_fetch = false;
    while (count < 10000 && !ps_fetch_queue.empty()) {
        // TODO: Don't deque from the .second, but instead "fetch" the whole thing at the same time
        // TODO: But then, how do we ensure that we don't fetch the same thing again?
        // TODO: For join, we will only have one partial state wrapper, and these can be fetched multiple times
        // TODO: For aggregation, we can also end up fetching the same extents multiple times
        // TODO: But, I think we can generalize it by saying that if the first element has already been attempted fetched,
        // TODO: we don't fetch it again.
        auto &ps_queue = ps_fetch_queue.front().second;
        std::vector<Ptr<PartialStateWrapper>> fetch_ps_wrappers;
        for (auto it = ps_queue.begin(); it != ps_queue.end(); it++) {
            ps = ps_queue.front();
            ps_queue.pop_front();
            // TODO: Check if ps is currently being fetched
            if (std::any_of(ps_being_fetched.begin(), ps_being_fetched.end(),
                            [ps](const auto &t) { return t->object == ps->object; })) {
                //static int times_skipped = 0;
                //NS_LOG_INFO(Simulator::Now() << " Times skipped: " << ++times_skipped);
                continue;
            }
            fetch_ps_wrappers.emplace_back(ps);
            ps_being_fetched.emplace_back(ps);
        }
        if (!fetch_ps_wrappers.empty()) {
            actually_fetch = true;
            old_host_wrapper->nodeToMigrationFetchRequests[std::stoi(node_id)].emplace_back(fetch_ps_wrappers);
            count += fetch_ps_wrappers.size();
        }
        ps_fetch_queue.pop_front();
    }
    if (!actually_fetch) {
        return;
    }
    DcepHeader dcepHeader;
    dcepHeader.SetFromNode(std::stoi(this->node_id));
    dcepHeader.setContentSize(count);
    dcepHeader.SetContentType(MIGRATION_FETCH);

    auto op = ps->op;
    dcepHeader.arguments["query_type"] = op->type;
    dcepHeader.arguments["query_id"] = op->query->id;
    dcepHeader.arguments["unique_query_id"] = op->query->unique_query_id;
    dcepHeader.arguments["operator_id"] = op->id;
    dcepHeader.arguments["count"] = count;

    //void
    //Communication::SendState(int old_host, int new_host, size_t bytes_left, DcepHeader &dcepHeader)

    static int cnt_fetched = 0;
    NS_LOG_INFO(Simulator::Now().GetMicroSeconds() << " Actually fetch number " << ++cnt_fetched);
    comm->SendFetchRequest(std::stoi(node_id), old_host, dcepHeader.GetContentSize(), dcepHeader);
}

Time
ExposeWrapper::Migrate(std::map<int, int> operatorIdToNewHost, std::map<int, std::vector<int>> priority_to_operator_ids, int query_id, std::map<int, std::string> operatorIdToAlgorithm, Time deadline, std::string migration_mode) {
    //NS_LOG_INFO(Simulator::Now() << ": Migrating Operator " << migrating_operator_id << " to Node " << new_host << " using algorithm " << algorithm);
    auto base_query = ExposeCoordinator::queryIdToBaseQuery[query_id];
    this->mp = CreateObject<MigrationPlan>(migration_mode);
    int old_host = std::stoi(this->node_id);
    this->mp->old_host = old_host;
    if (mp->migration_mode != "drop-state") {
        Metrics::migration_starts.emplace_back(Simulator::Now());
    }
    for (auto &[stream_id, operatorIdToOperator] : base_query->streamIdToOperatorIdToOperator) {
        for (auto &[priority, operator_ids] : priority_to_operator_ids) {
            for (auto operator_id : operator_ids) {
                auto op = operatorIdToOperator[operator_id];
                if (op == nullptr || op->source_sinkable) {
                    continue;
                }
                if (operatorIdToNewHost.find (operator_id) != operatorIdToNewHost.end ()) {
                    bool already_migrating = false;
                    for (auto batch : mp->preambleQueue) {
                        if (batch->op == op) {
                            already_migrating = true;
                          }
                    }
                    if (already_migrating) {
                        continue;
                    }
                    auto algorithm = operatorIdToAlgorithm[operator_id];
                    //bool moving_state = algorithm.at(1) == 'M';
                    bool parallel_track = algorithm.at (2) == 'P';
                    auto previous_operator = op->previous_operator;
                    int new_host = operatorIdToNewHost[operator_id];
                    mp->operatorIdToNewHost[operator_id] = new_host;
                    ExposeCoordinator::nodeIdsToWrappers[std::to_string (new_host)]->mp = this->mp;
                    // Now, we fetch the actually deployed query to be migrated
                    auto old_host_query =
                        ExposeCoordinator::queryIdToNodeIdToQuery[query_id][old_host];
                    auto old_host_op =
                        old_host_query->streamIdToOperatorIdToOperator[stream_id][op->id];
                    auto new_host_op = ExposeCoordinator::queryIdToNodeIdToQuery[query_id][new_host]
                                           ->streamIdToOperatorIdToOperator[stream_id][operator_id];
                    op->SetNodeId (new_host);
                    if (parallel_track) {
                        op->parallel_old_node_id = old_host;
                    }
                    if (old_host_op->in_migration || new_host_op->in_migration) {
                        continue;
                    }
                    //auto resource_manager = StreamQuery::nodeIdToNode[op->node_id]->GetObject<NodeResourceManager>();
                    //op->SetCpuCore(resource_manager->AssignCore(op));
                    // The change here is important, but must be improved
                    // Explanation: Above, we modify the base query, and in the next line, we update all the deployed queries with proper placement and thread (only former is necessary, I think)

                    mp->migration_mechanism->ExtractOperatorPreambleState(old_host_op, new_host_op);
                    /*for (int i = 0; i < (int) old_host_op->stream->operators.size (); i++)
                    {
                        // TODO: Create batches in which to place the PartialState pointers
                        // TODO: First and easiest choice is to just place

                        //mp->critQueue.emplace_back (static_batch);
                        old_host_op->stream->operators[i]->ExtractPreambleState (this->mp);
                        mp->InsertOperator(old_host_op->stream->operators[i]);

                        new_host_op->stream->operators[i]->in_migration = true;
                        new_host_op->stream->operators[i]->old_node_id = old_host;
                        old_host_op->stream->operators[i]->old_node_id = old_host;
                        new_host_op->stream->operators[i]->SetInactive();
                        //new_host_op->stream->operators[i]->Assign();
                        old_host_op->stream->operators[i]->in_migration = true;

                        if (mp->migration_mode != "rhino" && mp->migration_mode != "megaphone") {
                            old_host_op->SetInactive();
                            // Forward tuples to new host
                            int next_host = mp->operatorIdToNewHost[old_host_op->id];
                            NS_ASSERT(next_host != -1);
                            old_host_op->ForwardTuplesTo(next_host);
                            //old_host_op->UnassignCpuCore();
                        }
                    }*/
                }
            }
        }
    }

    mp->migration_mechanism->BeforeFirstMoveNextState();
    // Moved into the BeforeFirstMoveNextState method
    /*if (mp->migration_mode != "rhino") {
        ExposeCoordinator::UpdateDeployedQueries();
        ExposeCoordinator::UpdateOperatorPlacements();
    }*/


    //ExposeCoordinator::UpdateDeployedQueries();
    //ExposeCoordinator::UpdateOperatorPlacements();
    MoveNextState();
    //MoveNextState(migration_plans);

    return deadline;
}

void
ExposeWrapper::Configure() {
  log_prefix = " Node " + this->node_id + ": ";
  NS_LOG_INFO(Simulator::Now() << log_prefix << "Configure");
  auto dcep = Dcep::GetDcepFromNode(node);
  dcep->GetObject<CEPEngine>()->SetNodeId(std::stoi(this->node_id));
  if (node->GetObject<ExposeWrapper>() == nullptr) {
      node->AggregateObject(this);
  }
  dcep->c_communication->SetupConnections();
}

void
ExposeWrapper::SetTupleBatchSize(int size) {
  NS_LOG_INFO(Simulator::Now() << log_prefix << "SetTupleBatchSize(" << size << ")");
  //Simulator::Schedule (Seconds (1.0), &ExposeWrapper::FinishTask, this);
}

void
ExposeWrapper::SetIntervalBetweenTuples(int iw) {
  NS_LOG_INFO(Simulator::Now() << log_prefix << "SetIntervalBetweenTuples(" << iw << ")");
  //Simulator::Schedule (Seconds (1.0), &ExposeWrapper::FinishTask, this);
}

void
ExposeWrapper::SendTupleTo(Ptr<TupleWrapper> tupleWrapper, int nid) {
    // Serialize payload
    std::vector<uint8_t> serialized_payload;
    auto size = tupleWrapper->GetSerializedSize(true);
    Buffer buffer(size);
    buffer.AddAtStart(size);
    tupleWrapper->Serialize(true, buffer.Begin());

    //auto payload_size = buffer.GetSerializedSize();
    Ptr<Packet> newPacket = Create<Packet>((uint8_t*) buffer.PeekData(), size);
    uint16_t msgType = EVENT;
    DcepHeader dcepHeader;
    dcepHeader.SetContentType(msgType);
    dcepHeader.setContentSize(size);
    dcepHeader.SetFromNode(std::stoi(this->node_id));
    newPacket->AddHeader (dcepHeader);
    auto dcep = Dcep::GetDcepFromNode(node);
    auto streamIdToNodes = dcep->GetObject<CEPEngine>()->GetStreamIdToNodes();
    auto nodeIdToIpAndPort = dcep->GetObject<CEPEngine>()->GetNodeIdToTuplesAddress();
    auto ipAndPort = nodeIdToIpAndPort[nid];
    InetSocketAddress address = InetSocketAddress(ipAndPort.first, ipAndPort.second+1);
    dcep->SendPacket(newPacket, address);
}

void
ExposeWrapper::PrepareSendTupleToNextHops() {
    if (sendTupleCallbacks.empty()) {
        return;
    }
    auto first_tuple = sendTupleCallbacks.front();
    if (first_tuple.being_sent) {
        return;
    }

    Ptr<Operator> op = first_tuple.emitting_operator;
    Ptr<TupleWrapper> tupleWrapper = first_tuple.emitted_tuple;
    int part = 0;
    if (tupleWrapper->group != nullptr) {
        part = (int) tupleWrapper->group->partition;
    }
    auto next_op = op->stream->next_stream->operators[part%op->stream->next_stream->operators.size()];
    int next_node_id = next_op->node_id;
    int next_node_old_host_id = next_op->parallel_old_node_id;
    tupleWrapper->next_operator_id = next_op->id;
    tupleWrapper->query = op->query;
    if (!op->sink && next_op->sink) {
        long delay_between_tuples = 5000000;
        for (int sink_node_id : op->query->sink_nodes) {
            Simulator::Schedule(MicroSeconds(delay_between_tuples), &ExposeWrapper::SendTupleTo, this, tupleWrapper, sink_node_id);
            delay_between_tuples += 5000000;
        }
    } else {
        if (next_node_old_host_id != -1) {
            SendTupleTo(tupleWrapper, next_node_old_host_id);
        }
        SendTupleTo(tupleWrapper, next_node_id);
    }
}

void ExposeWrapper::doLoadDataset(const YAML::Node& ds, int number_rows) {
  NS_LOG_INFO(Simulator::Now() << log_prefix << "SendDsAsStream(" << ds["id"].as<int>() << ")");
  std::string expose_path = "./expose/Datasets";
  auto file_name = expose_path + "/" + ds["file"].as<std::string>();
  std::filebuf fb;
  if (!std::filesystem::exists(file_name)) {
      NS_FATAL_ERROR("Dataset " << file_name << " does not exist!");
  }
  fb.open(file_name, std::ios::in);
  std::istream File(&fb);
  int dataset_id = ds["id"].as<int>();
  if (dataset_to_tuples.find(dataset_id) == this->dataset_to_tuples.end() || (int) dataset_to_tuples[dataset_id].size() < number_rows) {
      auto csv_output = CSVReader().readCSV(File, number_rows);
      auto dataset_type = ds["type"].as<std::string>();
      for (auto row: csv_output) {
          auto stream_id = std::stoi(row[0]);
          auto tuple = CreateObject<Tuple>(stream_id);
          // Start at index 1 because index 0 is the stream ID
          //Tuple::streamIdToIndexToField[stream_id].erase('');


          for (int i = 0; i < (int) Tuple::streamIdToIndexToField[stream_id].size(); i++) {
              auto field = Tuple::streamIdToIndexToField[stream_id][i];
              auto str = row[i + 1];
              auto type = Tuple::streamIdToFieldToType[stream_id][field];
              switch (type) {
                case LONG:
                  if (str == "NULL" || str == "null" || str.empty()) {
                      tuple->longValues.emplace_back(0);
                    } else {
                      tuple->longValues.emplace_back(std::stol(str));
                    }
                  break;
                case DOUBLE:
                  if (str == "NULL" || str == "null" || str.empty()) {
                      tuple->doubleValues.emplace_back(0.0);
                    } else {
                      tuple->doubleValues.emplace_back(std::stold(str));
                    }
                  tuple->doubleValues.emplace_back(std::stold(str));
                  break;
                  case TIMESTAMP: {
                    tuple->longValues.emplace_back(Utility::timestampToLong (str));
                    break;
                  }
                case STRING:
                  tuple->stringValues.emplace_back(str);
                  break;
                case NOTYPE:
                  --i;
                  break;
                }
            }

          dataset_to_tuples[dataset_id].emplace_back(CreateObject<TupleWrapper>((std::vector<Ptr<Tuple>>){tuple}));
        }
    }
}

void ExposeWrapper::LoadDataset(const YAML::Node& ds, int number_rows) {
    doLoadDataset(ds, number_rows);
}

void ExposeWrapper::LoadDataset(const YAML::Node& ds) {
    doLoadDataset(ds, -1);
}

void
ExposeWrapper::SendDsAsStream(YAML::Node& ds) {
  LoadDataset (ds);
  int dataset_id = ds["id"].as<int>();
  int delay = 1;
  auto cepEngine = Dcep::GetDcepFromNode(node)->GetObject<CEPEngine>();
  for (int i = 0; i < (int)dataset_to_tuples[dataset_id].size(); i++) {
      auto tupleWrapper = dataset_to_tuples[dataset_id][i];
      tupleWrapper->next_operator_id = 0;
      Simulator::Schedule(MilliSeconds(delay), &CEPEngine::ProcessCepEvent, cepEngine, tupleWrapper);
      //delay += 0.0001;
    }
  Metrics::total_time_end = std::chrono::steady_clock::now();
}

void
ExposeWrapper::SendDsAsSpecificStream(YAML::Node& ds, int stream_id) {
  LoadDataset (ds);
  int dataset_id = ds["id"].as<int>();
  int delay = 1;
  auto cepEngine = Dcep::GetDcepFromNode(node)->GetObject<CEPEngine>();
  auto tuple_process_wrapper = CreateObject<TupleProcessWrapper>();
  for (int i = 0; i < (int)dataset_to_tuples[dataset_id].size(); i++) {
      auto tupleWrapper = dataset_to_tuples[dataset_id][i];
      if (tupleWrapper->tuples[0]->stream_id != stream_id) {
        continue;
      }
      tupleWrapper->next_operator_id = 0;
      Simulator::Schedule(MilliSeconds(delay), &TupleProcessWrapper::ProcessEvent, tuple_process_wrapper, tupleWrapper, cepEngine, coordinator);
      //ProcessEvent(tupleWrapper);
      //delay += 0.1;
  }
  Metrics::total_time_end = std::chrono::steady_clock::now();
}

void
TupleProcessWrapper::ProcessEvent(Ptr<TupleWrapper> tupleWrapper, Ptr<CEPEngine> cepEngine, Ptr<ExposeCoordinator> coordinator) {
    if (coordinator->dataset_manipulation[tupleWrapper->tuples[0]->stream_id] == "modify-selectivity") {
        // Modify based on the numeric_variables
        auto tuples_between_joins = (int) coordinator->numeric_variables["tuples_between_joins"];
        auto joins_per_tuple = (int) coordinator->numeric_variables["joins_per_tuple"];
        switch (tupleWrapper->tuples[0]->stream_id) {
            case 2: {
                if (number_sent % joins_per_tuple == 0) {
                    // Set join predicate to number_sent / joins_per_tuple
                    // Problem is, we then need to limit groups to be sent

                    break;
                }
                case 3: {
                    if (number_sent % tuples_between_joins != 0) {

                    }
                    break;
                }
            }
        }
    }
    ++number_sent;
    cepEngine->ProcessCepEvent(tupleWrapper);
}

void
ExposeWrapper::SendNRowsDsAsSpecificStream(YAML::Node& ds, int stream_id, int number_rows) {
    NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " SendNRowsDsAsSpecificStream sending " << number_rows << " tuples");
    LoadDataset (ds, number_rows);
    int dataset_id = ds["id"].as<int>();
    //int delay_increment = 2;
    //int delay = 0;
    auto cepEngine = Dcep::GetDcepFromNode(node)->GetObject<CEPEngine>();
    auto tuple_process_wrapper = CreateObject<TupleProcessWrapper>();
    for (int i = 0; i < number_rows; i++) {
        auto tupleWrapper = CreateObject<TupleWrapper>(dataset_to_tuples[dataset_id][i], false, true);
        if (tupleWrapper->tuples[0]->stream_id != stream_id) {
            continue;
        }
        //NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << " SendNRowsDsAsSpecificStream sending tuple with stream ID " << tupleWrapper->tuples[0]->stream_id);
        if (tupleWrapper->tuples[0]->stream_id == 3) {
            //tupleWrapper->tuples[0]->longValues[0] = random() % 100;
            //tupleWrapper->tuples[0]->longValues[1] = random() % 100;
            //tupleWrapper->tuples[0]->longValues[2] = random() % 100;
            tupleWrapper->tuples[0]->longValues[3] = random() % 100;
        }
        tupleWrapper->next_operator_id = 0;
        //Simulator::Schedule(MicroSeconds(delay), &CEPEngine::ProcessCepEvent, cepEngine, tupleWrapper);
        //delay += delay_increment;
        tuple_process_wrapper->ProcessEvent(tupleWrapper, cepEngine, coordinator);
        //std::cout << "Sending tuple " << i << std::endl;
    }
    Metrics::total_time_end = std::chrono::steady_clock::now();
}

void
ExposeWrapper::AddSchemas(YAML::Node stream_schemas) {
  NS_LOG_INFO(Simulator::Now() << log_prefix << "AddSchemas");
  for (YAML::iterator it = stream_schemas.begin(); it != stream_schemas.end(); ++it) {
    const YAML::Node& stream_schema = *it;
    stream_id_to_schema[stream_schema["stream-id"].as<int>()] = stream_schema;
  }

  auto cepEngine = Dcep::GetDcepFromNode(node)->GetObject<CEPEngine>();
  cepEngine->AddSchemas(stream_schemas);
}

void
ExposeWrapper::DeployQuery(const Ptr<StreamQuery>& query, int unique_query_id) {
    auto dcep = Dcep::GetDcepFromNode(node);
    int int_node_id = std::stoi(node_id);
    auto cepEngine = Dcep::GetDcepFromNode(node)->GetObject<CEPEngine>();
    cepEngine->DeployQuery(query);
    ExposeCoordinator::queryIdToNodeIdToQuery[unique_query_id][int_node_id] = StreamQuery::buildQuery(query->vertices_yaml, query->edges_yaml, query->id, query->unique_query_id) /*cepEngine->queryIdToBaseQuery[unique_query_id]*/;

    for (auto &[stream_id, operatorIdToOperator] : query->streamIdToOperatorIdToOperator) {
        for (auto &[operator_id, op] : operatorIdToOperator) {
            for (auto &[trace_event, streamIdToQueryIdToOperatorNameToTraced] : Metrics::traceEventToStreamIdToQueryIdToOperatorNameToTraced) {
                for (auto &[sid, queryIdToOperatorNameToTraced] : streamIdToQueryIdToOperatorNameToTraced) {
                    for (auto &[operatorName, traced] : queryIdToOperatorNameToTraced[query->unique_query_id]) {
                        if (op->name == operatorName && traced) {
                            Metrics::traceEventToStreamIdToQueryIdToOperatorIdToTraced[trace_event][sid][query->unique_query_id][op->id] = Metrics::traceEventToStreamIdToQueryIdToOperatorNameToTraced[trace_event][sid][query->unique_query_id][op->name];
                            Metrics::traceEventToStreamIdToQueryIdToOperatorIdToFilename[trace_event][sid][query->unique_query_id][op->id] = Metrics::traceEventToStreamIdToQueryIdToOperatorNameToFilename[trace_event][sid][query->unique_query_id][op->name];
                            break;
                        }
                    }
                }
            }
        }
    }
}

void
ExposeWrapper::RegisterSource(int stream_id, int unique_query_id) {
    // TODO: deploy query
    //Simulator::Schedule (Seconds (1.0), &ExposeWrapper::FinishTask, this);
}

void
ExposeWrapper::RegisterSink(int stream_id, int unique_query_id) {
    //Simulator::Schedule (Seconds (1.0), &ExposeWrapper::FinishTask, this);
}

void
ExposeWrapper::AddNextHop(int stream_id, int nid) {
  NS_LOG_INFO(Simulator::Now() << log_prefix << "Node " << this->node_id << " adds Node " << nid << " as subscriber of stream with Id " << stream_id);
  // Need to find RequestHandler &parent of the node we want to add as subscriber.
  // We first get the IP and port from the slave, who gets it from the master,
  // and then we start a TCP connection via boost.
  auto dcep = Dcep::GetDcepFromNode(node);
  auto cepEngine = dcep->GetObject<CEPEngine>();
  cepEngine->AddNextHop(stream_id, nid);
  //Simulator::ScheduleNow (&ExposeWrapper::FinishTask, this);
}

void
ExposeWrapper::AddNextHopQuery(std::vector<int> nodes, Ptr<Operator> op) {
    //NS_LOG_INFO(Simulator::Now() << log_prefix << "Node " << this->node_id << " adds Node " << node_id << " as subscriber of stream with Id " << stream_id);
    // Need to find RequestHandler &parent of the node we want to add as subscriber.
    // We first get the IP and port from the slave, who gets it from the master,
    // and then we start a TCP connection via boost.
    auto dcep = Dcep::GetDcepFromNode(node);
    auto cepEngine = dcep->GetObject<CEPEngine>();
    //cepEngine->AddNextHop(stream_id, node_id);
    //Simulator::ScheduleNow (&ExposeWrapper::FinishTask, this);
}

void
ExposeWrapper::RemoveNextHop(std::vector<int> nodes, Ptr<Operator> op) {
    //NS_LOG_INFO(Simulator::Now() << log_prefix << "Node " << this->node_id << " adds Node " << node_id << " as subscriber of stream with Id " << stream_id);
    // Need to find RequestHandler &parent of the node we want to add as subscriber.
    // We first get the IP and port from the slave, who gets it from the master,
    // and then we start a TCP connection via boost.
    //auto dcep = Dcep::GetDcepFromNode(node);
    //auto cepEngine = dcep->GetObject<CEPEngine>();
    //cepEngine->AddNextHop(stream_id, node_id);
    //Simulator::ScheduleNow (&ExposeWrapper::FinishTask, this);
}

void ExposeWrapper::SetTupleDataRate(long datarate) {
    auto p2pNetDevice = DynamicCast<PointToPointNetDevice>(node->GetDevice(1));
    p2pNetDevice->SetAttribute("DataRate", DataRateValue(DataRate(datarate)));
}

void ExposeWrapper::SetMigrationDataRate(long datarate) {
    auto p2pNetDevice = DynamicCast<PointToPointNetDevice>(node->GetDevice(0));
    p2pNetDevice->SetAttribute("DataRate", DataRateValue(DataRate(datarate)));
}

void
ExposeWrapper::SetNidToMigrationAddress(std::map<int, std::pair<Ipv4Address, int>> newNodeIdToIpAndPort) {
  NS_LOG_INFO(Simulator::Now() << log_prefix << "Updating Node Id and address for Node " << this->node_id);

  //auto dcep = this->node->GetObject<Dcep>();
  auto cepEngine = Dcep::GetDcepFromNode(node)->GetObject<CEPEngine>();
  cepEngine->SetNodeIdToMigrationAddress(newNodeIdToIpAndPort);
}

void
ExposeWrapper::SetNidToTuplesAddress(std::map<int, std::pair<Ipv4Address, int>> newNodeIdToIpAndPort) {
    NS_LOG_INFO(Simulator::Now() << log_prefix << "Updating Node Id and address for Node " << this->node_id);

    //auto dcep = this->node->GetObject<Dcep>();
    auto cepEngine = Dcep::GetDcepFromNode(node)->GetObject<CEPEngine>();
    cepEngine->SetNodeIdToTuplesAddress(newNodeIdToIpAndPort);
}

void
ExposeWrapper::MigrateStateBeforeDisconnect(int ms_before_disconnect) {
    NS_LOG_INFO(Simulator::Now() << " Now migrating state from node " << this->node_id << " and it must be finished in " << ms_before_disconnect << " ms");
    for (auto &[query_id, baseQuery] : ExposeCoordinator::queryIdToBaseQuery) {
        for (auto it = baseQuery->potential_hosts.begin(); it != baseQuery->potential_hosts.end(); it++) {
            int nid = std::stoi(this->node_id);
            if (*it == nid) {
                NS_LOG_INFO("Removing node " << this->node_id << " from the potential host list of Query " << baseQuery->id);
                baseQuery->potential_hosts.erase(it);
                ExposeCoordinator::Adapt(query_id, false, nid, -1, ms_before_disconnect, "state-shedding");
                break;
            }
        }
    }
}

void
ExposeWrapper::DoDisconnectNode() {
    NS_LOG_INFO(Simulator::Now() << " Now disconnecting node " << this->node_id);
    // Run disable state on all queries
    // Iterate through all queries on this node
    for (auto &[query_id, node_id_to_query] : ExposeCoordinator::queryIdToNodeIdToQuery) {
        auto query = node_id_to_query[std::stoi(this->node_id)];
        // Iterate through all operators of this query
        for (auto &[stream_id, operator_id_to_operator] : query->streamIdToOperatorIdToOperator) {
            for (auto &[operator_id, op] : operator_id_to_operator) {
                // op->node_id should be the new host of the query
                auto new_query = ExposeCoordinator::queryIdToNodeIdToQuery[query_id][op->node_id];
                // Iterate through all operators of the new host query
                for (auto inner_op : new_query->streamIdToOperatorIdToOperator[stream_id][operator_id]->stream->operators) {
                    inner_op->SetActive();
                    inner_op->thread->processQueuedTuple();
                }
            }
        }
        for (auto &[stream_id, operator_id_to_operator] : query->streamIdToOperatorIdToOperator) {
            for (auto &[operator_id, op] : operator_id_to_operator) {
                for (auto actual_op : op->stream->operators) {
                    actual_op->ClearState();
                    actual_op->UnassignCpuCore();
                    actual_op->SetInactive();
                }
            }
        }
    }

    Dcep::GetDcepFromNode(this->node)->c_communication->ShutDown();
}

void
ExposeWrapper::DisconnectNode(int ms_before_disconnect) {
    NS_LOG_INFO(Simulator::Now() << " Disconnecting Node " << this->node_id << " in " << ms_before_disconnect << " ms");
    //Simulator::Schedule(MicroSeconds(ms_before_migration), &ExposeWrapper::MigrateStateBeforeDisconnect, this);
    //MigrateStateBeforeDisconnect(ms_before_migration);
    // TODO: Why does DoDisconnectNode cause no tuples from being sent even long after the migration is done? I don't get that, so fix it
    // TODO: When it's uncommented, no tuple is received by the sink, and if it's commented, it does work, even if the disconnect time is many many seconds
    Simulator::Schedule(MilliSeconds(ms_before_disconnect), &ExposeWrapper::DoDisconnectNode, this);
}

void
ExposeWrapper::AddTuples(const YAML::Node& tuple, int quantity) {

}

void
ExposeWrapper::ProcessTuples(int number_tuples, int stream_id) {

}

void
ExposeWrapper::ClearQueries() {
  NS_LOG_INFO(Simulator::Now() << log_prefix << "Clearing queries");
  //Simulator::Schedule (Seconds (1.0), &ExposeWrapper::FinishTask, this);
}

void
ExposeWrapper::StartRuntimeEnv() {
  NS_LOG_INFO(Simulator::Now() << log_prefix << "Running environment");
  //Simulator::Schedule (Seconds (1.0), &ExposeWrapper::FinishTask, this);
}

void
ExposeWrapper::StopRuntimeEnv() {
  NS_LOG_INFO(Simulator::Now() << log_prefix << "Stopping environment");
  //Simulator::Schedule (Seconds (1.0), &ExposeWrapper::FinishTask, this);
}

void
ExposeWrapper::EndExperiment() {
  NS_LOG_INFO(Simulator::Now() << log_prefix << "Node " << node_id << ": Cleanup after experiment");
}

void
ExposeWrapper::AddTpIds(std::vector<int> tracepointIds) {
  NS_LOG_INFO(Simulator::Now() << log_prefix << "AddTpIds();");
}

void
ExposeWrapper::RetSinkEndOfStream(int query_id, int milliseconds) {
    auto node_id_to_query = ExposeCoordinator::queryIdToNodeIdToQuery[query_id];
    long maximum_diff = -1;
    long maximum_received_tuples = 0;
    for (auto &[nid, query] : node_id_to_query) {
        for (auto &[stream_id, operator_id_to_operator] : query->streamIdToOperatorIdToOperator) {
            for (auto &[operator_id, op] : operator_id_to_operator) {
                if (op->type == sinkType) {
                    auto last_received = DynamicCast<SinkOperator>(op)->time_last_received;
                    auto diff = last_received == -1 ? -1 : Simulator::Now().GetMilliSeconds() - last_received;
                    maximum_diff = std::max(maximum_diff, diff);
                    maximum_received_tuples = std::max(maximum_received_tuples, DynamicCast<SinkOperator>(op)->number_tuples);
                }
            }
        }
    }

    if (maximum_diff < milliseconds) {
        Simulator::Schedule(MilliSeconds(milliseconds), &ExposeWrapper::RetSinkEndOfStream, this, query_id, milliseconds);
    } else {
        // Resetting time
        for (auto &[nid, query] : node_id_to_query) {
            for (auto &[stream_id, operator_id_to_operator] : query->streamIdToOperatorIdToOperator) {
                for (auto &[operator_id, op] : operator_id_to_operator) {
                    if (op->type == sinkType) {
                        DynamicCast<SinkOperator>(op)->time_last_received = -1;
                    }
                }
            }
        }
        NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << log_prefix << "RetSinkEndOfStream(" << milliseconds << "); diff: " << maximum_diff << ", total received tuples: " << maximum_received_tuples);
        //ExposeCoordinator::ResetTupleCount();
        FinishTask();
    }
}

void
ExposeWrapper::RetEndOfStream(int milliseconds) {
    auto dcep = Dcep::GetDcepFromNode(node);
    auto cepEngine = dcep->GetObject<CEPEngine>();
    long diff = Simulator::Now().GetMilliSeconds() - cepEngine->last_received_tuple;
    NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << log_prefix << "RetEndOfStream(" << milliseconds << "); diff: " << diff << ", total received tuples: " << cepEngine->number_tuples_received);
    if (diff < milliseconds || cepEngine->last_received_tuple == -1) {
        Simulator::Schedule(MilliSeconds(milliseconds), &ExposeWrapper::RetEndOfStream, this, milliseconds);
    } else {
        ExposeCoordinator::ResetTupleCount();
        FinishTask();
    }
    //Simulator::Schedule (Seconds (1.0), &ExposeWrapper::FinishTask, this);
}

void
ExposeWrapper::RetWhenReceived(int number_tuples) {
    //NS_LOG_INFO(Simulator::Now() << log_prefix << "RetEndOfStream(" << milliseconds << ");");
    auto dcep = Dcep::GetDcepFromNode(node);
    auto cepEngine = dcep->GetObject<CEPEngine>();
    if (cepEngine->number_tuples_received < number_tuples) {
        Simulator::Schedule(MilliSeconds(1), &ExposeWrapper::RetWhenReceived, this, number_tuples);
    } else {
        NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << ": Node " << this->node_id << " has received " << number_tuples << " tuples");
        FinishTask();
    }
    //Simulator::Schedule (Seconds (1.0), &ExposeWrapper::FinishTask, this);
}

void
ExposeWrapper::RetWhenProcessed(int number_tuples, int query_id, std::string operator_type) {
    //NS_LOG_INFO(Simulator::Now() << log_prefix << "RetEndOfStream(" << milliseconds << ");");
    auto dcep = Dcep::GetDcepFromNode(node);
    auto cepEngine = dcep->GetObject<CEPEngine>();
    Ptr<Operator> op = nullptr;
    for (auto &[stream_id, operator_id_to_operators] : ExposeCoordinator::queryIdToNodeIdToQuery[query_id][std::stoi(this->node_id)]->streamIdToOperatorIdToOperator) {
        for (auto &[op_id, inner_op] : operator_id_to_operators) {
            if (inner_op->type == joinType && operator_type == "join") {
                op = inner_op;
                break;
            } else if (DynamicCast<AggregationWindowOperator>(inner_op) != nullptr && operator_type == "aggregation") {
                op = inner_op;
                break;
            } else if (inner_op->type == selectType && operator_type == "select") {
                op = inner_op;
                break;
            }
        }
        if (op != nullptr) {
            break;
        }
    }
    NS_ASSERT(op != nullptr);

    if (op->tuples_processed < number_tuples) {
        Simulator::Schedule(MilliSeconds(1), &ExposeWrapper::RetWhenProcessed, this, number_tuples, query_id, operator_type);
    } else {
        NS_LOG_INFO(Simulator::Now().GetMilliSeconds() << ": Node " << this->node_id << " has received " << number_tuples << " tuples");
        FinishTask();
    }
    //Simulator::Schedule (Seconds (1.0), &ExposeWrapper::FinishTask, this);
}

void
ExposeWrapper::TraceTuple(int tracepointId, std::vector<std::string> traceArguments) {
  NS_LOG_INFO(Simulator::Now() << log_prefix << "TraceTuple(" << tracepointId << ");");
  // The empty vector is a vector of ints, but we use string arguments here
  //Simulator::Schedule (Seconds (1.0), &ExposeWrapper::FinishTask, this);
}
