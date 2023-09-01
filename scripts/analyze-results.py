import os
import pandas as pd
import matplotlib.pyplot as plt
import glob

plt.rcParams.update({'font.size': 14})
plt.locator_params('x', nbins=4)
show_title = False
show_legend = False

def generate_plots():
    # Directory path
    results_path = "raw-data"

    all_files = glob.glob(os.path.join(results_path, "*.csv"))
    migration_modes = ["no-migration", "all-at-once", "lazy", "meces", "megaphone", "rhino", "utility"]
    migration_mode_plt = {"no-migration": 'dotted', "all-at-once": 'dotted', "lazy": 'dotted', "meces": 'dotted', "megaphone": "dotted", "rhino": "dotted", "utility": "dotted"}
    migration_mode_marker_plt = {"no-migration": 'o', "all-at-once": 'o', "lazy": '*', "meces": 'x', "megaphone": "D", "rhino": "+", "utility": "p"}
    migration_mode_to_display = {"no-migration": "$m_{nomigration}$", "all-at-once": "$m_{allatonce}$", "meces": "$m_{meces}$", "megaphone": "$m_{megaphone}$", "rhino": "$m_{rhino}$", "lazy": "$m_{lazylat}$", "utility": "$m_{lazyutil}$"}
    trace_types = ["aggregation-latency_trace", "emission-latency_trace", "latency_trace", "general_trace"]
    filename_map = {"EMISSION_DELAY": "emission-delay", "INPUT_TUPLE_LATENCY": "input-tuple-latency", "UTILITY": "utility", "latency_trace": "join-latency", "emission-latency_trace": "emission-latency", "aggregation-latency_trace": "aggregation-latency"}

    # Function to extract run_id and x-value from filename for sorting
    def get_sort_key(filename):
        filename = filename.split("/")[1]
        parts = filename.split('_')
        run_id = int(parts[2])
        x_value = int(parts[3])
        return (run_id, x_value)

    # Sort files based on run_id and x-value
    all_files = sorted(all_files, key=get_sort_key)


    # Initial setup
    runs = {}
    general_trace_types = ['UTILITY', 'EMISSION_DELAY', 'INPUT_TUPLE_LATENCY']


    for file in all_files:
        filename = os.path.basename(file)
        trace_type, run_id, figure_x_value, migration_mode = filename.split('_')[0] + "_" + filename.split('_')[1], \
                                                             filename.split('_')[2], filename.split('_')[3], \
                                                             filename.split('_')[-1].split('.')[0]

        if trace_type not in trace_types:
            continue
        if (run_id in ['1'] and trace_type in ["deviation_trace"]) or (run_id in ['5', '6'] and trace_type in ['deviation_trace']) or (run_id in ['2', '5', '6'] and trace_type in ['latency_trace']):
            continue
        if run_id not in runs:
            runs[run_id] = {}
        if run_id != '5':
            continue

        if trace_type not in runs[run_id]:
            runs[run_id][trace_type] = {}

        if figure_x_value not in runs[run_id][trace_type]:
            runs[run_id][trace_type][figure_x_value] = {}

        if migration_mode not in runs[run_id][trace_type][figure_x_value]:
            #print(f"Reading file {filename}")
            if trace_type == "general_trace":
                with open(file, 'r') as f:
                    data_dict = {line.split()[0]: float(line.split()[1]) for line in f}
                runs[run_id][trace_type][figure_x_value][migration_mode] = data_dict
            else:
                df = pd.read_csv(file)
                runs[run_id][trace_type][figure_x_value][migration_mode] = df

    # Accumulate differences and plot
    for run_id, run_data in runs.items():
        plt.figure(figsize=(6.4, 4.8))
        for trace_type, trace_data in run_data.items():
            #if run_id in ['0', '3'] and trace_type == 'deviation_trace' or run_id not in ['0', '2'] and trace_type == 'latency_trace':
            #    continue

            if trace_type == 'general_trace':
                for data_type in general_trace_types:
                    migration_modes_legend = []
                    plot_data = {migration_mode: {'x': [], 'y': []} for migration_mode in migration_modes}
                    for figure_x_value, migration_mode_data in trace_data.items():
                        for migration_mode, data_dict in migration_mode_data.items():
                            assert migration_mode in migration_modes
                            if migration_mode != "no-migration":
                                if migration_mode not in migration_modes_legend:
                                    migration_modes_legend.append(migration_mode_to_display[migration_mode])
                                plot_data[migration_mode]['x'].append(figure_x_value)
                                # Offsetting the value by 1 to get it working on a log scale, in case the results are 0
                                plot_data[migration_mode]['y'].append(data_dict[data_type]+1)

                    fig, ax = plt.subplots()

                    for migration_mode, data in plot_data.items():
                        if len(data['x']) == 0:
                            continue
                        ax.plot(data['x'], data['y'], marker=migration_mode_marker_plt[migration_mode], label=f"{migration_mode_to_display[migration_mode]}", alpha=0.7, linestyle='--', linewidth=3, markersize=10)
                    if show_title:
                        plt.title(f'{data_type.replace("_", " ").capitalize()} of Experiment {run_id}')
                    if run_id in ['2', '4']:
                        plt.xlabel("Window jump")
                    elif run_id in ['1', '3']:
                        plt.xlabel("Percentage of migrated tuples that are joined")
                    elif run_id in ['5', '6']:
                        plt.xlabel("Operational migration period")
                    plt.yscale('log')
                    if data_type == "EMISSION_DELAY":
                        plt.ylabel(data_type.replace("_", " ").capitalize() + " (ms)")
                    else:
                        plt.ylabel(data_type.replace("_", " ").capitalize())
                    if len(ax.xaxis.get_ticklabels()) > 6:
                        for n, label in enumerate(ax.xaxis.get_ticklabels()):
                            if n % 2 != 0:
                                label.set_visible(False)
                    if show_legend:
                        plt.legend()
                    plt.tight_layout()
                    plt.savefig(f"{results_path}/run-{run_id}-{filename_map[data_type]}.png", transparent=True, pad_inches=0)
                    plt.show()
                    continue
                continue
            else:
                max = 0
                sum = 0
                min = -1
                std = 0
                avg = 0
                median = 0
                fig, ax = plt.subplots()

                no_migration_data = {x_value: data.get("no-migration") for x_value, data in trace_data.items()}
                plot_data = {migration_mode: {'x': [], 'y': [], 'cnt': []} for migration_mode in migration_modes}
                plot_data2 = {migration_mode: {'x': [], 'y': [], 'cnt': []} for migration_mode in migration_modes}
                migration_modes_legend = []
                for figure_x_value, migration_mode_data in trace_data.items():
                    for migration_mode, df in migration_mode_data.items():
                        assert migration_mode in migration_modes
                        if migration_mode != "no-migration":
                            column = 'timestamp' if (trace_type == 'latency_trace' or trace_type == "aggregation-latency_trace" or trace_type == "emission-latency_trace") else 'aggregate'
                            #assert len(df[column]) == len(no_migration_data[figure_x_value][column])
                            #    continue
                            if migration_mode not in migration_modes_legend:
                                migration_modes_legend.append(migration_mode)

                            print(run_id, trace_type, "x:", figure_x_value, migration_mode, ":", df[column].sum(), "no migration:", no_migration_data[figure_x_value][column].sum())
                            sum = df[column].sum()
                            diff = sum - no_migration_data[figure_x_value][column].sum()
                            for r in df[column]:
                                if r > max:
                                    max = r
                                if r < min or min == -1:
                                    min = r
                            entry = df[column]
                            avg = df[column].mean()
                            median = df[column].median()
                            std = df[column].std()

                            #if diff < 0:
                            #    diff = 0
                            #if trace_type == "emission-latency_trace" or trace_type == "latency_trace":
                            #    plot_data[migration_mode]['x'].append(figure_x_value)
                            #    plot_data[migration_mode]['y'].append(avg)
                            #    plot_data[migration_mode]['cnt'].append(df[column].size)
                            #else:
                            plot_data[migration_mode]['x'].append(figure_x_value)
                            plot_data[migration_mode]['y'].append(diff)
                            plot_data[migration_mode]['cnt'].append(df[column].size)


                            if trace_type == "deviation_trace":
                                deviation = 0
                                for cnt1, cnt2 in zip(df[column], no_migration_data[figure_x_value][column]):
                                    deviation += abs(cnt1-cnt2)
                                total_sum = no_migration_data[figure_x_value][column].sum()
                                plot_data2[migration_mode]['x'].append(figure_x_value)
                                plot_data2[migration_mode]['y'].append(deviation/total_sum)
                for migration_mode, data in plot_data.items():
                    if len(data['x']) == 0:
                        continue
                    if trace_type == "emission-latency_trace":
                        for i in range(len(data['cnt'])):
                            ax.annotate(data['cnt'][i], (data['x'][i], data['y'][i]))
                    ax.plot(data['x'], data['y'], marker=migration_mode_marker_plt[migration_mode], label=f"{migration_mode_to_display[migration_mode]}", linestyle=migration_mode_plt[migration_mode], linewidth=3, markersize=10)
                if run_id in ['1']:
                    if show_title:
                        fig.title(f'Join latency in Experiment {run_id}')
                    plt.ylabel(f'Join latency (ms)')
                    plt.xlabel("Percentage of migrated tuples that are joined")
                    if show_legend:
                        ax.legend()
                    plt.tight_layout()
                    fig.savefig(f"{results_path}/run-{run_id}-{filename_map[trace_type]}.png", transparent=True, pad_inches=0)
                    fig.show()
                else:
                    if trace_type == "latency_trace":
                        if show_title:
                            plt.title(f'Join latency of Experiment {run_id}')
                        plt.ylabel(f'Join latency (ms)')
                    elif trace_type == "deviation_trace":
                        if show_title:
                            plt.title(f'Aggregate deviation of Experiment {run_id}')
                        plt.ylabel(f'Aggregate deviation')
                    elif trace_type == "aggregation-latency_trace":
                        if show_title:
                            plt.title(f'Aggregation latency of Experiment {run_id}')
                        plt.ylabel(f'Aggregation latency (ms)')
                    elif trace_type == "emission-latency_trace":
                        if show_title:
                            plt.title(f'Emission delay of Experiment {run_id}')
                        plt.ylabel(f'Emission delay (ms)')
                    else:
                        if show_title:
                            plt.title(f'Join latency of Experiment {run_id}')
                        plt.ylabel(f'Join latency (ms)')
                    if run_id in ['2', '4']:
                        plt.xlabel("Window jump")
                    else:
                        plt.xlabel("Percentage of migrated tuples that are joined")
                    if show_legend:
                        plt.legend()
                    if len(ax.xaxis.get_ticklabels()) > 6:
                        for n, label in enumerate(ax.xaxis.get_ticklabels()):
                            if n % 2 != 0:
                                label.set_visible(False)
                    plt.tight_layout()
                    plt.savefig(f"{results_path}/run-{run_id}-{filename_map[trace_type]}.png", transparent=True, pad_inches=0)
                    plt.show()

                    if run_id in ['2', '3', '4']:
                        fig, ax = plt.subplots()
                        for migration_mode, data in plot_data.items():
                            if len(data['x']) == 0:
                                continue
                            plot_data2[migration_mode]['x'] = data['x']
                            if run_id == '3':
                                plot_data2[migration_mode]['y'] = [float(data['y'][i]) / float(500) for i in range(len(data['x']))]
                            else:
                                plot_data2[migration_mode]['y'] = [float(data['y'][i]) / float(data['x'][i]) for i in
                                                                   range(len(data['x']))]
                        # Add "avg missed window extents"
                        for migration_mode, data in plot_data2.items():
                            if len(data['x']) == 0:
                                continue
                            ax.plot(data['x'], data['y'], marker=migration_mode_marker_plt[migration_mode], label=f"{migration_mode_to_display[migration_mode]}", linestyle=migration_mode_plt[migration_mode], linewidth=3, markersize=10)

                        #plt.yscale('log')
                        if show_title:
                            plt.title(f'Missed window extents of Experiment {run_id}')
                        plt.ylabel(f'Missed window extents')
                        if run_id == '3':
                            plt.xlabel("Percentage of migrated tuples that are joined")
                        elif run_id == '4':
                            plt.xlabel("Window jump")
                        if show_legend:
                            plt.legend()
                        if len(ax.xaxis.get_ticklabels()) > 6:
                            for n, label in enumerate(ax.xaxis.get_ticklabels()):
                                if n % 2 != 0:
                                    label.set_visible(False)
                        plt.tight_layout()
                        plt.savefig(f"{results_path}/run-{run_id}-missed-windows.png", transparent=True, pad_inches=0)
                        plt.show()
            plt.close()
            continue

if __name__ == "__main__":
    generate_plots()
    print("Finished")
    exit(0)
