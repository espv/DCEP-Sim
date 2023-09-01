
#include <filesystem>
#include "ns3/nstime.h"
#include "ns3/dcep-module.h"
#include "ns3/Metrics.h"
#include "ns3/Operator.h"
#include <iostream>

namespace ns3 {
    struct time_sort
    {
        inline bool operator() (const std::pair<Time, std::tuple<std::string, int, long>>& e1, const std::pair<Time, std::tuple<std::string, int, long>>& e2)
        {
            return e1.first < e2.first;
        }
    };

    void Metrics::PrintMetrics() {
        if (Metrics::result_deviation != -1) {
            std::cout << "RESULT DEVIATION: " << Metrics::result_deviation << std::endl;
        }
        std::cout << "RESULT LATENCY: " << Metrics::result_latency << std::endl;
        std::cout << "RESULT UTILITY: " << Metrics::count_accumulator << std::endl;
        std::cout << "RESULT EMISSION DELAY: " << Metrics::window_emission_delay << std::endl;
        Time total_input_tuple_latency = Time(0);
        for (auto &[op_type, input_tuple_latency] : Metrics::total_input_tuple_latency) {
            switch (op_type) {
                case windowType:
                    for (auto &[g, itl] : input_tuple_latency) {
                        std::cout << "RESULT INPUT TUPLE LATENCY FOR WINDOW OP WITH ID " << g << ": " << itl.GetMilliSeconds() << " ms" << std::endl;
                    }
                    break;
                case filterType:
                    for (auto &[g, itl] : input_tuple_latency) {
                        std::cout << "RESULT INPUT TUPLE LATENCY FOR FILTER OP WITH ID " << g << ": " << itl.GetMilliSeconds() << " ms" << std::endl;
                    }
                    break;
                case groupType:
                    for (auto &[g, itl] : input_tuple_latency) {
                        std::cout << "RESULT INPUT TUPLE LATENCY FOR GROUP OP WITH ID " << g << ": " << itl.GetMilliSeconds() << " ms" << std::endl;
                    }
                    break;
                case selectType:
                    for (auto &[g, itl] : input_tuple_latency) {
                        std::cout << "RESULT INPUT TUPLE LATENCY FOR SELECT OP WITH ID " << g << ": " << itl.GetMilliSeconds() << " ms" << std::endl;
                    }
                    break;
                case joinType:
                    for (auto &[g, itl] : input_tuple_latency) {
                        std::cout << "RESULT INPUT TUPLE LATENCY FOR JOIN OP WITH ID " << g << ": " << itl.GetMilliSeconds() << " ms" << std::endl;
                    }
                    break;
                case printType:
                    for (auto &[g, itl] : input_tuple_latency) {
                        std::cout << "RESULT INPUT TUPLE LATENCY FOR PRINT OP WITH ID " << g << ": " << itl.GetMilliSeconds() << " ms" << std::endl;
                    }
                    break;
                case sinkType:
                    for (auto &[g, itl] : input_tuple_latency) {
                        std::cout << "RESULT INPUT TUPLE LATENCY FOR SINK OP WITH ID " << g << ": " << itl.GetMilliSeconds() << " ms" << std::endl;
                    }
                    break;
                case cepType:
                    for (auto &[g, itl] : input_tuple_latency) {
                        std::cout << "RESULT INPUT TUPLE LATENCY FOR CEP OP WITH ID " << g << ": " << itl.GetMilliSeconds() << " ms" << std::endl;
                    }
                    break;
                case defineType:
                    for (auto &[g, itl] : input_tuple_latency) {
                        std::cout << "RESULT INPUT TUPLE LATENCY FOR DEFINE OP WITH ID " << g << ": " << itl.GetMilliSeconds() << " ms" << std::endl;
                    }
                    break;
                case forwardType:
                    for (auto &[g, itl] : input_tuple_latency) {
                        std::cout << "RESULT INPUT TUPLE LATENCY FOR FORWARD OP WITH ID " << g << ": " << itl.GetMilliSeconds() << " ms" << std::endl;
                    }
                    break;
                case withinType:
                    for (auto &[g, itl] : input_tuple_latency) {
                        std::cout << "RESULT INPUT TUPLE LATENCY FOR WITHIN OP WITH ID " << g << ": " << itl.GetMilliSeconds() << " ms" << std::endl;
                    }
                    break;
                case passthroughType:
                    for (auto &[g, itl] : input_tuple_latency) {
                        std::cout << "RESULT INPUT TUPLE LATENCY FOR PASSTHROUGH OP WITH ID " << g << ": " << itl.GetMilliSeconds() << " ms" << std::endl;
                    }
                    break;
                case inputType:
                    for (auto &[g, itl] : input_tuple_latency) {
                        std::cout << "RESULT INPUT TUPLE LATENCY FOR INPUT OP WITH ID " << g << ": " << itl.GetMilliSeconds() << " ms" << std::endl;
                    }
                    break;
            }

            for (auto &[g, itl] : input_tuple_latency) {
                total_input_tuple_latency += itl;
            }
        }
        std::cout << "RESULT TOTAL INPUT TUPLE LATENCY: " << total_input_tuple_latency.GetMilliSeconds() << std::endl;
        std::cout << "RESULT FORWARDED INPUT TUPLE LATENCY: " << Metrics::forwarded_tuples_input_latency.GetMilliSeconds() << std::endl;
    }

    void Metrics::WriteToFile(std::string run_id, std::string migration_mechanism) {
        std::cout << "Writing metrics from running the '" << migration_mechanism << "' simulation to file" << std::endl;

        // Update file paths to include run_id and migration_mechanism
        std::string path_prefix = "expose/Results/raw-data";
        std::string migration_time_path = path_prefix + "/migration_time";
        auto migration_time_filename = path_prefix + "/migration_time";
        std::string migration_time_file = migration_time_path + "/migration_time.csv";

        // Handle Migration Times
        if (Metrics::trace_migration) {
            std::filesystem::create_directories(migration_time_path);
            std::ofstream outfile_migration_time (migration_time_file);

            // Write CSV header
            outfile_migration_time << "start_time,stop_time,duration" << std::endl;

            for (int i = 0; i < (int) Metrics::migration_stops.size(); i++) {
                long start_time = Metrics::migration_starts[i].GetMicroSeconds();
                long stop_time = Metrics::migration_stops[i].GetMicroSeconds();
                long duration = stop_time - start_time;

                outfile_migration_time << start_time << "," << stop_time << "," << duration << std::endl;
            }

            outfile_migration_time.close();
        }

        std::string latency_trace_file = path_prefix + "/latency_trace_" + run_id + "_" + migration_mechanism + ".csv";
        std::ofstream outfile_latency_trace (latency_trace_file);

        // Write CSV header
        outfile_latency_trace << "timestamp,data" << std::endl;

        for (auto const &entry : Metrics::latency_data) { // Replace with your actual data source
            outfile_latency_trace << entry.timestamp.GetMilliSeconds() << "," << entry.data << std::endl;
        }

        outfile_latency_trace.close();


        std::string aggregation_latency_trace_file = path_prefix + "/aggregation-latency_trace_" + run_id + "_" + migration_mechanism + ".csv";
        std::ofstream outfile_aggregation_latency_trace (aggregation_latency_trace_file);

        // Write CSV header
        outfile_aggregation_latency_trace << "timestamp,data" << std::endl;

        for (auto const &entry : Metrics::aggregation_latency_data) { // Replace with your actual data source
            outfile_aggregation_latency_trace << entry.timestamp.GetMilliSeconds() << "," << entry.data << std::endl;
        }

        std::string emission_latency_trace_file = path_prefix + "/emission-latency_trace_" + run_id + "_" + migration_mechanism + ".csv";
        std::ofstream outfile_emission_latency_trace (emission_latency_trace_file);

        // Write CSV header
        outfile_emission_latency_trace << "timestamp,data" << std::endl;

        for (auto const &entry : Metrics::window_emission_delays) { // Replace with your actual data source
            outfile_emission_latency_trace << entry.timestamp.GetMilliSeconds() << "," << entry.data << std::endl;
        }

        outfile_emission_latency_trace.close();


        std::string deviation_trace_file = path_prefix + "/deviation_trace_" + run_id + "_" + migration_mechanism + ".csv";
        //if (!Metrics::deviation_data.empty()) {
        std::ofstream outfile_deviation_trace (deviation_trace_file);

        // Write CSV header
        outfile_deviation_trace << "timestamp,count,aggregate" << std::endl;

        for (auto const &entry : Metrics::deviation_data) { // Replace with your actual data source
            outfile_deviation_trace << entry.timestamp.GetMilliSeconds() << "," << entry.count << "," << entry.aggregate << std::endl;
        }

        outfile_deviation_trace.close();
        //}

        // Printout summary of the operations
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "Summary of Metrics::WriteToFile operation" << std::endl;
        std::cout << "-----------------------------------------" << std::endl;
        std::cout << "Simulation run ID: " << run_id << std::endl;
        std::cout << "Migration mechanism used: " << migration_mechanism << std::endl;

        std::string general_trace_file = path_prefix + "/general_trace_" + run_id + "_" + migration_mechanism + ".csv";
        std::ofstream  outfile_general_trace (general_trace_file);
        outfile_latency_trace << "type,data" << std::endl;
        if (Metrics::result_deviation != -1) {
            outfile_general_trace << "DEVIATION " << Metrics::result_deviation << std::endl;
        }
        outfile_general_trace << "LATENCY " << Metrics::result_latency << std::endl;
        outfile_general_trace << "UTILITY " << Metrics::count_accumulator << std::endl;
        outfile_general_trace << "EMISSION_DELAY " << Metrics::window_emission_delay << std::endl;
        long titl = 0;
        for (auto &[op_type, input_tuple_latency] : Metrics::total_input_tuple_latency) {
            for (auto &[g, itl] : input_tuple_latency) {
                titl += itl.GetMilliSeconds();
            }
        }
        outfile_general_trace << "INPUT_TUPLE_LATENCY " << titl << std::endl;
        outfile_general_trace << "FORWARDED_INPUT_TUPLE_LATENCY " << Metrics::forwarded_tuples_input_latency.GetMilliSeconds() << std::endl;
        outfile_general_trace.close();

        if (trace_migration) {
            std::cout << "Migration start and stop times have been traced and written to "
                      << migration_time_filename << std::endl;
        } else {
            std::cout << "Migration start and stop times have not been traced" << std::endl;
        }

        //if (!Metrics::latency_data.empty()) {
            std::cout << "Wrote output latency results to " << latency_trace_file << std::endl;
            std::cout << "Wrote emission delay results to " << emission_latency_trace_file << std::endl;
        /*} else {
            std::cout << "No output latency results to write" << std::endl;
        }*/

        //if (!Metrics::deviation_data.empty()) {
            std::cout << "Wrote output deviation results to " << deviation_trace_file << std::endl;
            std::cout << "Wrote general results to " << general_trace_file << std::endl;
        /*} else {
            std::cout << "No output deviation results to write" << std::endl;
        }*/


        //std::cout << "Tuple latencies have been written to " << tuple_latencies_filename << std::endl;
        std::cout << "-----------------------------------------" << std::endl;
    }




    /*void Metrics::WriteToFile(std::string run_id, std::string migration_mechanism) {
        std::cout << "Writing metrics from running the '" << migration_mechanism << "' simulation to file" << std::endl;
        // TODO: Write migration time to file
        if (Metrics::trace_migration) {
            for (int i = 0; i < (int) Metrics::migration_stops.size(); i++) {
                // Do write Metrics::migration_start and Metrics::migration_stop to file
                std::cout << "Migration start: " << Metrics::migration_starts[i].GetMilliSeconds() << std::endl;
                std::cout << "Migration stop: " << Metrics::migration_stops[i].GetMilliSeconds() << std::endl;
                std::cout << "Migration time: " << (Metrics::migration_stops[i] - Metrics::migration_starts[i]).GetMilliSeconds() << std::endl;

                // Write to file
                auto path = "expose/Results/raw-data/" + Metrics::migration_time_plot_name;
                std::filesystem::create_directories(path);
                std::ofstream outfile (path + "/" + std::to_string(ExposeCoordinator::experiment_id));

                outfile << (Metrics::migration_stops[i] - Metrics::migration_starts[i]).GetMicroSeconds() << std::endl;

                outfile.close();
            }
        }

        if (migration_mechanism == "no-migration") {
            // Write to file
            std::string path = "expose/Results/raw-data";
            std::filesystem::create_directories(path);
            auto filename = path + "/no-migration-received-times";
            try {
                if (std::filesystem::remove(filename))
                    std::cout << "file " << filename << " deleted.\n";
                else
                    std::cout << "file " << filename << " not found.\n";
            }
            catch(const std::filesystem::filesystem_error& err) {
                std::cout << "filesystem error: " << err.what() << '\n';
            }
            std::ofstream outfile (filename);

            for (auto &[group_id, tuples_received] : Metrics::sink_received_tuples) {
                for (int i = 0; i < (int) tuples_received.size(); i++) {
                    outfile << group_id << " " << tuples_received[i].t.GetNanoSeconds() << std::endl;
                }
            }

            outfile.close();
        } else {
            // Read from file to compare latency
            auto path = "expose/Results/raw-data/no-migration-received-times";
            std::ifstream no_migration_file (path);
            std::map<int, std::vector<long>> no_migration_received_times;
            std::string line;
            int group_id;
            while (no_migration_file >> group_id >> line) {
                no_migration_received_times[group_id].emplace_back(std::stol(line));
            }

            long diff = 0;

            for (auto &[group, tuples_received] : Metrics::sink_received_tuples) {
                long offset = 0;
                for (int i = 0; i < (int) tuples_received.size(); i++) {
                    auto one_diff = (tuples_received[i].t.GetNanoSeconds() - no_migration_received_times[group][i]);
                    offset += one_diff;
                    diff += one_diff;
                }
            }
            Metrics::result_latency = diff/1000000;
            std::cout << "Output tuple latency: " << ((double)diff/1000000000.0) << " seconds" << std::endl;
        }*/

        /*std::map<int, std::map<int, std::map<int, std::vector<std::pair<Time, std::tuple<std::string, int, long>>>>>> ingestion_times;
        std::map<int, std::map<int, std::map<int, std::vector<std::pair<Time, std::tuple<std::string, int, long>>>>>> received_times;
        // TODO: Write input tuple latency to file
        for (auto &[stream_id, queryIdToOperatorIdToTraced] : Metrics::traceEventToStreamIdToQueryIdToOperatorIdToTraced[TRACEINGESTION]) {
            for (auto &[query_id, operatorIdToTraced] : queryIdToOperatorIdToTraced) {
                for (auto &[operator_id, traced] : operatorIdToTraced) {
                    if (traced) {
                        std::string op_name;
                        for (auto &[node_id, query] : ExposeCoordinator::queryIdToNodeIdToQuery[query_id]) {
                            if (query->streamIdToOperatorIdToOperator.contains(stream_id) && query->streamIdToOperatorIdToOperator[stream_id][operator_id]) {
                                op_name = query->streamIdToOperatorIdToOperator[stream_id][operator_id]->name;
                                break;
                            }
                        }
                        NS_ASSERT_MSG(!op_name.empty(), "Failed to find operator " << operator_id << " with stream Id " << stream_id << " in query " << query_id);

                        // Write ingestion times to file
                        for (const auto& p : streamIdToQueryIdToOperatorIdToIngestionTimes[stream_id][query_id][operator_id]) {
                            auto t = p.first;
                            auto v = p.second;
                            ingestion_times[stream_id][query_id][operator_id].emplace_back(std::make_pair(t, std::tuple(op_name, operator_id, v)));
                        }
                        for (const auto& p : streamIdToQueryIdToOperatorIdToReceivedTimes[stream_id][query_id][operator_id]) {
                            auto t = p.first;
                            auto v = p.second;
                            received_times[stream_id][query_id][operator_id].emplace_back(std::make_pair(t, std::tuple(op_name, operator_id, v)));
                        }
                        std::sort(ingestion_times[stream_id][query_id][operator_id].begin(), ingestion_times[stream_id][query_id][operator_id].end(), time_sort());
                        std::sort(received_times[stream_id][query_id][operator_id].begin(), received_times[stream_id][query_id][operator_id].end(), time_sort());
                        // Write to file
                        auto results_directory = Metrics::traceEventToStreamIdToQueryIdToOperatorIdToFilename[TRACEINGESTION][stream_id][query_id][operator_id];

                        auto path = "expose/Results/raw-data/" + results_directory;
                        std::filesystem::create_directories("expose/Results/raw-data/" + results_directory);
                        std::ofstream outfile ("expose/Results/raw-data/" + results_directory + "/" + std::to_string(ExposeCoordinator::experiment_id));
                        std::cout << "Writing the following ingestion times to file " << path << std::endl;

                        std::cout << "Ingestion times: " << ingestion_times[stream_id][query_id][operator_id].size() << std::endl;
                        std::cout << "Received times: " << received_times[stream_id][query_id][operator_id].size() << std::endl;
                        NS_ASSERT(ingestion_times[stream_id][query_id][operator_id].size() == received_times[stream_id][query_id][operator_id].size());
                        for (int i = 0; i < (int) ingestion_times[stream_id][query_id][operator_id].size(); i++) {
                            auto &p = ingestion_times[stream_id][query_id][operator_id][i];
                            auto &p2 = received_times[stream_id][query_id][operator_id][i];
                            auto t = p.first;
                            auto tuple = p.second;
                            auto t2 = p2.first;
                            auto tuples2 = p2.second;
                            //auto op_id = std::get<1>(tuple);
                            //auto v = std::get<2>(tuple);

                            outfile << t.GetMicroSeconds() - t2.GetMicroSeconds() << std::endl; // "At time " << t << ", operator " << op_name << " (" << op_id << ")" << " received Tuple " << v << std::endl;
                        }
                        outfile.close();
                    }
                }
            }
        }*/

        //Time total_tuple_latency = Time(0);
        //std::cout << "Received tuple trace events: " << received_tuple_trace_events.size() << std::endl;
        //std::cout << "Processed tuple trace events: " << processed_tuple_trace_events.size() << std::endl;

        // IF MEGAPHONE
        /*for (auto &[id, received_tuple_event] : received_tuple_trace_events) {
            auto tw = received_tuple_event->tupleWrapper;
            auto group_id = tw->group->id;
            auto t_received_tuple = received_tuple_event->t;

            // Assume there's only 1 pw per group
            auto sent_ps_event = sent_partial_state_trace_events[group_id][0];
            if (!received_partial_state_trace_events.count(group_id)) {
                continue;
            }
            auto received_ps_event = received_partial_state_trace_events[group_id][0];
            if (t_received_tuple > sent_ps_event->t && t_received_tuple < received_ps_event->t) {
                // COLLISION
                total_tuple_latency += received_ps_event->t - t_received_tuple;
            }
        }*/

        /*for (auto &[id, received_tuple_event] : received_tuple_trace_events) {
            if (processed_tuple_trace_events.count(id)) {
                auto processed_tuple_event = processed_tuple_trace_events[(int) id];
                // Found match
                long tuple_latency = (processed_tuple_event->t - received_tuple_event->t).GetMilliSeconds();
                total_tuple_latency += MilliSeconds(tuple_latency);
                if (tuple_latency > 0) {
                    //std::cout << "Tuple latency: " << tuple_latency << std::endl;
                }
                break;
            }
        }

        //Metrics::result = total_tuple_latency.GetMilliSeconds();
        std::cout << "Total tuple latency: " << total_tuple_latency.GetMilliSeconds() << " milliseconds" << std::endl;
    }*/

    void Metrics::TraceIngestion(Ptr<TupleWrapper> tupleWrapper, int node_id, Ptr<StreamQuery> query, Ptr<Operator> op) {
        if (!trace_enabled) {
            return;
        }
        for (auto tuple : tupleWrapper->tuples) {
            int sid = tuple->stream_id;
            if (Metrics::traceEventToStreamIdToQueryIdToOperatorIdToTraced[TRACEINGESTION][sid][op->query->unique_query_id][op->id]) {
                Metrics::streamIdToQueryIdToOperatorIdToIngestionTimes[sid][op->query->unique_query_id][op->id].emplace_back(
                        std::make_pair(Simulator::Now(), 0 /*tupleWrapper->id*/));

                if (!Metrics::processed_tuple_trace_events.count(tupleWrapper->tuples[0]->id)) {
                    Metrics::processed_tuple_trace_events[tupleWrapper->tuples[0]->id] = new TraceProcessedTupleEvent(tupleWrapper);
                }
                return;
            }
        }
    }

    void Metrics::TraceReceived(Ptr<TupleWrapper> tupleWrapper, int node_id, Ptr<StreamQuery> query, Ptr<Operator> op) {
        if (!trace_enabled) {
            return;
        }
        for (auto tuple : tupleWrapper->tuples) {
            int sid = tuple->stream_id;
            if (Metrics::traceEventToStreamIdToQueryIdToOperatorIdToTraced[TRACERECEIVED][sid][op->query->unique_query_id][op->id]) {
                Metrics::streamIdToQueryIdToOperatorIdToReceivedTimes[sid][op->query->unique_query_id][op->id].emplace_back(
                        std::make_pair(Simulator::Now(), 0 /*tupleWrapper->id*/));
                if (!Metrics::received_tuple_trace_events.count(tupleWrapper->tuples[0]->id)) {
                    Metrics::received_tuple_trace_events[tupleWrapper->tuples[0]->id] = new TraceReceivedTupleEvent(tupleWrapper);
                }
                return;
            }
        }
    }

    void Metrics::TracePartialStateSent(Ptr<PartialStateWrapper> pw, int node_id, Ptr<StreamQuery> query, Ptr<Operator> op) {
        if (!trace_enabled || pw->object == nullptr || DynamicCast<Operator>(pw->object)) {
            return;
        }
        int group_id = -1;
        auto window_extent = DynamicCast<WindowExtent>(pw->object);
        if (window_extent != nullptr) {
            group_id = window_extent->group->id;
        }
        auto tupleWrapper = DynamicCast<TupleWrapper>(pw->object);
        if (tupleWrapper != nullptr) {
            group_id = tupleWrapper->group->id;
        }
        if (group_id == -1) {
            return;
        }

        Metrics::sent_partial_state_trace_events[group_id].emplace_back(new TraceSentPartialStateEvent(pw));
        if (Metrics::traceEventToStreamIdToQueryIdToOperatorIdToTraced[TRACEPARTIALSTATESENT][group_id][op->query->unique_query_id][op->id]) {
            Metrics::streamIdToQueryIdToOperatorIdToPartialStatesSentTimes[group_id][op->query->unique_query_id][op->id].emplace_back(
                    std::make_pair(Simulator::Now(), 0 /*tupleWrapper->id*/));
            Metrics::sent_partial_state_trace_events[group_id].emplace_back(new TraceSentPartialStateEvent(pw));
            return;
        }
    }

    void Metrics::TracePartialStateReceived(Ptr<PartialStateWrapper> pw, int node_id, Ptr<StreamQuery> query, Ptr<Operator> op) {
        if (!trace_enabled || pw->object == nullptr || true) {
            // TODO: Fix this function
            return;
        }
        int group_id = -1;
        auto window_extent = DynamicCast<WindowExtent>(pw->object);
        if (window_extent != nullptr) {
            group_id = window_extent->group->id;
        }
        auto tupleWrapper = DynamicCast<TupleWrapper>(pw->object);
        if (tupleWrapper != nullptr) {
            group_id = tupleWrapper->group->id;
        }
        if (group_id == -1) {
            return;
        }

        Metrics::received_partial_state_trace_events[group_id].emplace_back(new TraceReceivedPartialStateEvent(pw));
        if (Metrics::traceEventToStreamIdToQueryIdToOperatorIdToTraced[TRACEPARTIALSTATERECEIVED][group_id][op->query->unique_query_id][op->id]) {
            Metrics::streamIdToQueryIdToOperatorIdToPartialStatesReceivedTimes[group_id][op->query->unique_query_id][op->id].emplace_back(
                    std::make_pair(Simulator::Now(), 0 /*tupleWrapper->id*/));
            Metrics::received_partial_state_trace_events[group_id].emplace_back(new TraceReceivedPartialStateEvent(pw));
            return;
        }
    }
}
