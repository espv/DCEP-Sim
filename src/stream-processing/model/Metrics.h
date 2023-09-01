//
// Created by espen on 31.10.2021.
//

#ifndef STREAMPROCESSOR_METRICS_H
#define STREAMPROCESSOR_METRICS_H

#include <chrono>
#include "nstime.h"
#include "ns3/Operator.h"
#include "ns3/simulator.h"
#include "Operator.h"

namespace ns3 {
    class TupleWrapper;
    class PartialStateWrapper;
    class Operator;
    class StreamQuery;

    enum TraceEvent {
      TRACEMIGRATION,
      TRACEINGESTION,
      TRACERECEIVED,
      TRACEPARTIALSTATESENT,
      TRACEPARTIALSTATERECEIVED
    };

    class AbstractTupleTraceEvent {
    public:
        Time t;
        Ptr<TupleWrapper> tupleWrapper;
    };

    class TraceReceivedTupleEvent : public AbstractTupleTraceEvent {
    public:
        TraceReceivedTupleEvent(Ptr<TupleWrapper> tupleWrapper) {
            this->tupleWrapper = tupleWrapper;
            t = Simulator::Now();
        }
    };

    class TraceProcessedTupleEvent : public AbstractTupleTraceEvent {
    public:
        TraceProcessedTupleEvent(Ptr<TupleWrapper> tupleWrapper) {
            this->tupleWrapper = tupleWrapper;
            t = Simulator::Now();
        }
    };

    class TraceOutputLatencyEvent {
    public:
        Time timestamp;
        long data;
        Ptr<TupleWrapper> tuple_wrapper;

        TraceOutputLatencyEvent(Ptr<TupleWrapper> tuple_wrapper, long data) {
            this->timestamp = Simulator::Now();
            this->data = data;
            this->tuple_wrapper = tuple_wrapper;
        }
    };

    class TraceOutputDeviationEvent {
    public:
        Time timestamp;
        long count;
        double aggregate;
        Ptr<TupleWrapper> tuple_wrapper;

        TraceOutputDeviationEvent(Ptr<TupleWrapper> tuple_wrapper, long count, double aggregate) {
            this->timestamp = Simulator::Now();
            this->count = count;
            this->aggregate = aggregate;
            this->tuple_wrapper = tuple_wrapper;
        }
    };

    class AbstractPartialStateEvent {
    public:
        Time t;
        Ptr<PartialStateWrapper> partialStateWrapper;
    };

    class TraceReceivedPartialStateEvent : public AbstractPartialStateEvent {
    public:
        TraceReceivedPartialStateEvent(Ptr<PartialStateWrapper> partialStateWrapper) {
            this->partialStateWrapper = partialStateWrapper;
            t = Simulator::Now();
        }
    };

    class TraceSentPartialStateEvent : public AbstractPartialStateEvent {
    public:
        TraceSentPartialStateEvent(Ptr<PartialStateWrapper> partialStateWrapper) {
            this->partialStateWrapper = partialStateWrapper;
            t = Simulator::Now();
        }
    };

    class Metrics {
    public:
        // Start trace measurements
        inline static std::vector<std::vector<std::pair<Time, long>>> results;
        inline static std::map<TraceEvent, std::map<int, std::map<int, std::map<int, bool>>>> traceEventToStreamIdToQueryIdToOperatorIdToTraced;
        inline static std::map<TraceEvent, std::map<int, std::map<int, std::map<int, std::string>>>> traceEventToStreamIdToQueryIdToOperatorIdToFilename;
        inline static std::map<TraceEvent, std::map<int, std::map<int, std::map<std::string, bool>>>> traceEventToStreamIdToQueryIdToOperatorNameToTraced;
        inline static std::map<TraceEvent, std::map<int, std::map<int, std::map<std::string, std::string>>>> traceEventToStreamIdToQueryIdToOperatorNameToFilename;
        inline static std::map<int, std::map<int, std::map<int, std::vector<std::pair<Time, long>>>>> streamIdToQueryIdToOperatorIdToIngestionTimes;
        inline static std::map<int, std::map<int, std::map<int, std::vector<std::pair<Time, long>>>>> streamIdToQueryIdToOperatorIdToReceivedTimes;
        inline static std::map<int, std::map<int, std::map<int, std::vector<std::pair<Time, long>>>>> streamIdToQueryIdToOperatorIdToPartialStatesSentTimes;
        inline static std::map<int, std::map<int, std::map<int, std::vector<std::pair<Time, long>>>>> streamIdToQueryIdToOperatorIdToPartialStatesReceivedTimes;

        inline static std::map<int, std::vector<TraceReceivedTupleEvent>> sink_received_tuples;
        inline static std::map<int, TraceReceivedTupleEvent*> received_tuple_trace_events = {};
        inline static std::map<int, TraceProcessedTupleEvent*> processed_tuple_trace_events = {};
        inline static int extents_outputted = 0;
        inline static int tuples_in_extents_emitted = 0;

        inline static std::vector<TraceOutputLatencyEvent> latency_data = {};
        inline static std::vector<TraceOutputLatencyEvent> aggregation_latency_data = {};
        inline static std::vector<TraceOutputDeviationEvent> deviation_data = {};
        inline static long window_emission_delay = 0;
        inline static std::vector<TraceOutputLatencyEvent> window_emission_delays;
        inline static int count_accumulator = 0;

        inline static std::map<int, Time> forwarded_tuples;
        inline static Time forwarded_tuples_input_latency;
        inline static std::map<int, std::map<int, Time>> forwarded_tuple_input_latency;
        inline static std::map<OperatorType, std::map<int, Time>> total_input_tuple_latency;

        inline static std::map<int, std::vector<TraceReceivedPartialStateEvent*>> received_partial_state_trace_events = {};
        inline static std::map<int, std::vector<TraceSentPartialStateEvent*>> sent_partial_state_trace_events = {};

        inline static std::vector<Time> migration_starts;
        inline static std::vector<Time> migration_stops;
        inline static bool trace_migration = false;
        // Until we've enabled tracing, we don't trace anything
        inline static bool trace_enabled = false;
        inline static std::string migration_time_plot_name = "";
        // Stop trace measurements

        inline static int result_latency = -1;
        inline static int result_deviation = -1;

        inline static std::chrono::steady_clock::duration total_time_group =
                std::chrono::steady_clock::now() - std::chrono::steady_clock::now();
        inline static std::chrono::steady_clock::duration total_time_window =
                std::chrono::steady_clock::now() - std::chrono::steady_clock::now();
        inline static std::chrono::steady_clock::duration total_time_select =
                std::chrono::steady_clock::now() - std::chrono::steady_clock::now();
        inline static std::chrono::steady_clock::duration total_time_join =
                std::chrono::steady_clock::now() - std::chrono::steady_clock::now();
        inline static std::chrono::steady_clock::duration total_time_filter =
                std::chrono::steady_clock::now() - std::chrono::steady_clock::now();
        inline static std::chrono::steady_clock::duration total_time_setFieldToValueOfTuple =
                std::chrono::steady_clock::now() - std::chrono::steady_clock::now();
        inline static std::chrono::steady_clock::duration total_time =
                std::chrono::steady_clock::now() - std::chrono::steady_clock::now();
        inline static std::chrono::steady_clock::time_point total_time_end = std::chrono::steady_clock::now();
        inline static std::chrono::steady_clock::time_point total_time_begin;

        static void WriteToFile(std::string run_id, std::string migration_mechanism);

        static void AddTupleLatency(long delay) {
            while (results.empty()) {
                results.emplace_back();
            }
            results[0].emplace_back(std::make_pair(Simulator::Now(), delay));
        }
        static std::vector<std::pair<Time, long>> &GetTupleLatencies() {
            while (results.empty()) {
                results.emplace_back();
            }
            return results[0];
        }

        static void PrintMetrics();
        static void TraceIngestion(Ptr<TupleWrapper> tupleWrapper, int node_id, Ptr<StreamQuery> query, Ptr<Operator> op);
        static void TraceReceived(Ptr<TupleWrapper> tupleWrapper, int node_id, Ptr<StreamQuery> query, Ptr<Operator> op);
        static void TracePartialStateSent(Ptr<PartialStateWrapper> pw, int node_id, Ptr<StreamQuery> query, Ptr<Operator> op);
        static void TracePartialStateReceived(Ptr<PartialStateWrapper> pw, int node_id, Ptr<StreamQuery> query, Ptr<Operator> op);
    };
}

#endif //STREAMPROCESSOR_METRICS_H
