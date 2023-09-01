import os
import subprocess
import sys
import matplotlib.pyplot as plt

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


# Declare the configurations
migration_modes_join = ["no-migration", "all-at-once", "rhino", "lazy"]
migration_modes_aggregation = ["no-migration", "all-at-once", "rhino", "lazy", "meces", "megaphone"]
migration_modes_join_aggregation = ["no-migration", "all-at-once", "rhino", "lazy", "meces", "megaphone"]
migration_modes_join_aggregation_max_utility = ["no-migration", "utility", "lazy"]
tuples_numbers = [1, 10, 100, 1000, 10000]
window_jump_times = [100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000]
proportion_joinable_tuples = [1, 10, 40, 100]
batch_size = 1
number_window_extents = 100

# Run Id counter
run_id_counter = 1

# Declare the YAML configuration files
yaml_files = ['join-test.yaml', 'aggregation-test.yaml', 'join-and-aggregation-test.yaml', 'join-and-aggregation-test-2.yaml', 'join-and-aggregation-max-utility-test.yaml']

# Process each type of query separately
queries = ['Join Query', 'Timed Aggregation Query', 'Join -> Timed Aggregation Query (Minimize Latency)', 'Join -> Timed Aggregation Query (Minimize Latency)', 'Join -> Timed Aggregation Query (Maximize Utility)']
configs = [migration_modes_join, migration_modes_aggregation, migration_modes_join_aggregation, migration_modes_join_aggregation, migration_modes_join_aggregation_max_utility]
x_params = ['percentage-A-tuples-join', 'window-jump', 'percentage-A-tuples-join', 'window-jump', 'percentage-A-tuples-join']
x_values = [proportion_joinable_tuples, window_jump_times, proportion_joinable_tuples, window_jump_times, proportion_joinable_tuples]

# Create dictionaries to store latency and deviation results
latency_results = {}
deviation_results = {}

results_path = f"raw-data"

no_migration_latency_results = {}
no_migration_deviation_results = {}

#index = int(sys.argv[1])

for index in range(len(queries)):
    query, migration_modes, x_param, xvalues, yaml_file = queries[index], configs[index], x_params[index], x_values[
        index], yaml_files[index]
    data_by_migration_mode = {}
    print()
    for x_value in xvalues:
        for migration_mode in migration_modes:
            joins_per_tuple = 1 if x_param != "joins-per-tuple" else x_value
            tuples_between_join = 1 if x_param != "tuples-between-join" else x_value
            percentage_A_tuples_join = 100 if x_param != "percentage-A-tuples-join" else x_value
            join_range = 1 if x_param != "join-range" else x_value
            window_jump = 100 if x_param != "window-jump" else x_value
            batch_size = 1 if x_param != "batch-size" else x_value

            cmd = f'nohup python3 ns3 run --no-build "lazy-migration-minimize-latency --yaml-config {yaml_file} --experiment-id 4 --migration-mechanism {migration_mode} --tuples-between-join {tuples_between_join} --joins-per-tuple {joins_per_tuple} --join-range {join_range} --percentage-A-tuples-join {percentage_A_tuples_join} --window-jump {window_jump} --run-id {run_id_counter}_{x_value} --batch-size {batch_size} --number-window-extents {number_window_extents}" > {run_id_counter}_{x_value}_{migration_mode}.nohup&'
            print(cmd)
            continue
            result = subprocess.run(cmd, capture_output=True, text=True, shell=True)

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
                            deviation_results[migration_mode][x_value] = deviation - no_migration_deviation_results.get(x_value, 0)
            else:
                print(f'Output files {latency_output_filename} and/or {deviation_output_filename} not found.')
                continue

            # Save results by migration mode
            if migration_mode not in data_by_migration_mode:
                data_by_migration_mode[migration_mode] = []
            data_by_migration_mode[migration_mode].append((x_value, latency, deviation))

    run_id_counter += 1  # Increase run_id_counter for each new configuration

# Plot the latency results
plt.figure()
for migration_mode, results in latency_results.items():
    x_values = list(results.keys())
    latencies = list(results.values())
    plt.plot(x_values, [l if l >= 0 else 0 for l in latencies], label=migration_mode)

plt.title('Average Latency for Different Queries and Migration Modes')
plt.xlabel('Query')
plt.ylabel('Average Latency')
plt.legend()
plt.show()

# Plot the deviation results
plt.figure()
for migration_mode, results in deviation_results.items():
    queries = list(results.keys())
    deviations = list(results.values())
    plt.plot(queries, deviations, label=migration_mode)

plt.title('Average Deviation for Different Queries and Migration Modes')
plt.xlabel('Query')
plt.ylabel('Average Deviation')
plt.legend()
plt.savefig(f"{results_path}/{query.replace(' ', '_')}_{run_id_counter}.png")
plt.show()
# plt.show()  # Display all the plots

latency_results = {}
deviation_results = {}


if __name__ == "__main__":
  pass
