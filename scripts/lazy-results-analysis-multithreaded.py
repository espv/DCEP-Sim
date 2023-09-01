import os
import subprocess
import matplotlib.pyplot as plt
from multiprocessing import Pool, Manager
from simulation import *


if __name__ == "__main__":
    manager = Manager()
    # Declare the configurations
    migration_modes_join = ["no-migration", "all-at-once", "lazy"]
    migration_modes_aggregation = ["no-migration", "all-at-once", "megaphone", "meces", "lazy"]
    migration_modes_join_aggregation = ["no-migration", "all-at-once", "megaphone", "meces", "lazy"]
    migration_modes_join_aggregation_max_utility = ["no-migration", "utility", "lazy"]
    tuples_numbers = [1, 10, 100, 1000, 10000]
    window_jump_times = [100, 500, 1000, 5000, 10000]
    proportion_joinable_tuples = [0.001, 0.1, 0.4, 1]
    batch_size = 1
    number_window_extents = 100

    # Run Id counter
    run_id_counter = 1

    # Declare the YAML configuration files
    yaml_files = ['join-test.yaml', 'aggregation-test.yaml', 'join-and-aggregation-test.yaml',
                  'join-and-aggregation-test.yaml', 'join-and-aggregation-max-utility-test.yaml']

    # Process each type of query separately
    queries = ['Join Query', 'Timed Aggregation Query', 'Join -> Timed Aggregation Query (Minimize Latency)',
               'Join -> Timed Aggregation Query (Minimize Latency)',
               'Join -> Timed Aggregation Query (Maximize Utility)']
    configs = [migration_modes_join, migration_modes_aggregation, migration_modes_join_aggregation,
               migration_modes_join_aggregation, migration_modes_join_aggregation_max_utility]
    x_params = ['percentage-A-tuples-join', 'tuples-between-join', 'time-between-B-tuples', 'percentage-A-tuples-join']
    x_values = [proportion_joinable_tuples, window_jump_times, proportion_joinable_tuples, window_jump_times,
                proportion_joinable_tuples]

    # Create dictionaries to store latency and deviation results
    latency_results = manager.dict()
    deviation_results = manager.dict()

    results_path = f"expose/Results/raw-data"

    no_migration_latency_results = manager.dict()
    no_migration_deviation_results = manager.dict()

    pool = Pool()
    manager = Manager()
    run_id_counter = 1  # Initialize run ID counter
    async_results = []  # Store the AsyncResult objects here
    for query, migration_modes, x_param, x_values, yaml_file in zip(queries, configs, x_params, x_values, yaml_files):
        data_by_migration_mode = manager.dict()

        # For each x_value, run the simulations for all migration modes
        for x_value in x_values:
            # First, run the no-migration case
            simulation_args = (
                query, "no-migration", x_value, x_param, yaml_file, run_id_counter, results_path, latency_results,
                deviation_results, no_migration_latency_results, no_migration_deviation_results, data_by_migration_mode
            )
            print("Before pool.apply")
            async_result = pool.apply_async(run_simulation, (simulation_args,))
            async_results.append(async_result)
            print("After pool.apply")

            # Call get() on each AsyncResult to wait for the tasks to complete
            for async_result in async_results:
                async_result.get()

            # Then, run the other migration modes
            for migration_mode in migration_modes:
                if migration_mode != "no-migration":
                    simulation_args = (
                    query, migration_mode, x_value, x_param, yaml_file, run_id_counter, results_path, latency_results,
                    deviation_results, no_migration_latency_results, no_migration_deviation_results,
                    data_by_migration_mode)
                    async_result = pool.apply_async(run_simulation, (simulation_args,))
                    async_results.append(async_result)
                    # Call get() on each AsyncResult to wait for the tasks to complete
            for async_result in async_results:
                async_result.get()

            # After all simulations for this x_value, plot the results
            for result_type, results_dict, y_label in [("latency", latency_results, 'Average Latency'),
                                                       ("deviation", deviation_results, 'Average Deviation')]:
                plt.figure()
                for migration_mode in migration_modes:
                    # Get the results for this migration mode
                    results = results_dict[migration_mode]
                    # Create lists of x values and y_values
                    x_values = sorted(results.keys())
                    y_values = [results[x] for x in x_values]
                    plt.plot(x_values, y_values, label=migration_mode)

                plt.title(f'Average {result_type.capitalize()} for {query} and Different Migration Modes')
                plt.xlabel(x_param)
                plt.ylabel(y_label)
                plt.legend()
                plt.savefig(f"{results_path}/{result_type}_{query.replace(' ', '_')}_{run_id_counter}.png")
                plt.close()

            # Increment the run ID counter after all migration modes have been simulated for the current x_value
            run_id_counter += 1

    pool.close()
