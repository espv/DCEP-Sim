import os
import subprocess
import matplotlib.pyplot as plt
from multiprocessing import Pool, Manager


def calculate_latency(file):
    """
    This function calculates the latency based on the data in the file.
    It assumes that the latency information is contained in the second column of the file (0-indexed).
    """
    latencies = []
    for line in file.readlines()[1:]:
        columns = line.split(",")
        latency = float(columns[0])  # Assuming the latency is the second column
        latencies.append(latency)

    # Calculate average latency
    average_latency = sum(latencies) / len(latencies) if latencies else None

    return average_latency


def calculate_deviation(file):
    """
    This function calculates the deviation based on the data in the file.
    It assumes that the deviation information is contained in the third column of the file (0-indexed).
    """
    deviations = []
    for line in file.readlines()[1:]:
        columns = line.split(",")
        deviation = float(columns[1])  # Assuming the deviation is the second column
        deviations.append(deviation)

    # Calculate average deviation
    average_deviation = sum(deviations) / len(deviations) if deviations else None

    return average_deviation



def run_simulation(args):
    # Here, args is a tuple of the arguments
    query, migration_mode, x_value, x_param, yaml_file, run_id_counter, results_path, latency_results, deviation_results, no_migration_latency_results, no_migration_deviation_results, data_by_migration_mode = args
    # ...
    joins_per_tuple = 1 if x_param != "joins-per-tuple" else x_value
    tuples_between_join = 1 if x_param != "tuples-between-join" else x_value
    percentage_A_tuples_join = 100 if x_param != "percentage-A-tuples-join" else x_value
    join_range = 1 if x_param != "join-range" else x_value
    window_jump = 100 if x_param != "window-jump" else x_value
    batch_size = 1 if x_param != "batch-size" else x_value
    number_window_extents = 100 if x_param != "number-window-extents" else x_value

    cmd = ['./ns3', 'run',
           f'lazy-migration-minimize-latency --yaml-config {yaml_file} --experiment-id 4 --migration-mechanism {migration_mode} --tuples-between-join {tuples_between_join} --joins-per-tuple {joins_per_tuple} --join-range {join_range} --percentage-A-tuples-join {percentage_A_tuples_join} --window-jump {window_jump} --run-id {run_id_counter} --batch-size {batch_size} --number-window-extents {number_window_extents}']
    print("Running", cmd[0], cmd[1], cmd[2])
    result = subprocess.run(cmd, capture_output=True, text=True)

    # Assuming the C++ program outputs latency and deviation data files with a specific name pattern.
    latency_output_filename = results_path + f'/latency_trace_{run_id_counter}_{migration_mode}.csv'
    deviation_output_filename = results_path + f'/deviation_trace_{run_id_counter}_{migration_mode}.csv'

    if os.path.exists(latency_output_filename) and os.path.exists(deviation_output_filename):
        with open(latency_output_filename, 'r') as latency_file, open(deviation_output_filename,
                                                                      'r') as deviation_file:
            latency = calculate_latency(latency_file)
            deviation = calculate_deviation(deviation_file)

            # Store the results
            if migration_mode == "no-migration":
                no_migration_latency_results[x_value] = latency
                no_migration_deviation_results[x_value] = deviation
            else:
                if migration_mode not in latency_results:
                    latency_results[migration_mode] = {}
                if migration_mode not in deviation_results:
                    deviation_results[migration_mode] = {}

                # Calculate latency and deviation relative to the no-migration results
                latency_results[migration_mode][x_value] = latency - no_migration_latency_results.get(x_value, 0)
                if deviation is not None:
                    deviation_results[migration_mode][x_value] = deviation - no_migration_deviation_results.get(x_value,
                                                                                                                0)
    else:
        print(f'Output files {latency_output_filename} and/or {deviation_output_filename} not found.')
        return

    # Save results by migration mode
    if migration_mode not in data_by_migration_mode:
        data_by_migration_mode[migration_mode] = []
    data_by_migration_mode[migration_mode].append((x_value, latency, deviation))
    return run_id_counter, no_migration_latency_results, no_migration_deviation_results, latency_results, deviation_results, data_by_migration_mode
