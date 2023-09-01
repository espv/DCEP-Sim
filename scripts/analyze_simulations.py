from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
import sys
import yaml
import os


args = sys.argv

yaml_config = args[1]

conf = yaml.safe_load(Path(yaml_config).read_text())

results_folder = "../expose/Results/"
raw_data_folder = results_folder + "/raw-data/"
figures_folder = results_folder + "/figures/"
control_experiment_id = -1

def inputLatency(plot, files):
    control_experiment = None
    control_experiment_lines = None
    # Initialize figure and axis
    fig, ax = plt.subplots(figsize=(8, 8))
    for file in files:
        if file.split("/")[-1].startswith(control_experiment_id):
            control_experiment = file
            with open(file) as f:
                control_experiment_lines = [int(x) for x in f.readlines()]
            break
    colors = ["red", "blue", "green", "yellow", "orange"]
    for i, file in enumerate(files):
        if file.split("/")[-1].startswith(control_experiment_id):
            continue
        with open(file) as f:
            lines = [int(x) for x in f.readlines()]
            modified_lines = [(x-y)/1000 for x, y in zip(lines, control_experiment_lines)]
            #ax.plot(lines, modified_lines, color=colors[i])
            ax.plot([x for x in range(len(modified_lines))], modified_lines, color=colors[i])

    plt.show()


def outputLatency(plot, files):
    control_experiment = None
    control_experiment_lines = None
    # Initialize figure and axis
    fig, ax = plt.subplots(figsize=(8, 8))
    for file in files:
        if file.split("/")[-1].startswith(control_experiment_id):
            control_experiment = file
            with open(file) as f:
                control_experiment_lines = [int(x) for x in f.readlines()]
            break
    colors = ["red", "blue", "green", "yellow", "orange"]
    for i, file in enumerate(files):
        if file.split("/")[-1].startswith(control_experiment_id):
            continue
        with open(file) as f:
            lines = [int(x) for x in f.readlines()]
            modified_lines = [(x-y)/1000 for x, y in zip(lines, control_experiment_lines)]
            #ax.plot(lines, modified_lines, color=colors[i])
            ax.plot([x for x in range(len(modified_lines))], modified_lines, color=colors[i])

    plt.show()


    # TODO: Read files and do a x=time, y=output latency, i.e., diff between input latency on sink for no-migration and the migration modes

    # TODO: Finally, write the figures to Results/figures/


def migrationTime(plot, files):

    for file in files:
        if file.startswith(control_experiment_id):
            continue
        with open(file) as f:
            lines = f.readlines()
    # TODO: Read files and do a bar chart where y-axis is migration time and the bars are the different experiments, except for control experiment


    # TODO: Finally, write the figures to Results/figures/


plots = conf["plots"]
for plot in plots:
    print(plot)
    control_experiment_id = plot["control-experiment-id"]
    folder_name = "../expose/Results/raw-data/" + plot["name"]
    plot_type = plot["type"]
    directory = os.fsencode(folder_name)
    files = []
    for f in os.listdir(directory):
        files.append(folder_name + "/" + f.decode("utf-8"))

    if plot_type == "Input-latency":
        # Read all files and just compare the input latency in a line diagram, except the control experiment
        inputLatency(plot, files)
    elif plot_type == "Output-latency":
        # Read all files and compare the times between control experiment and the others in a line diagram
        outputLatency(plot, files)
    elif plot_type == "Migration-time":
        # Read all files and just compare the migration stop time, in a bar chart, except the control experiment
        migrationTime(plot, files)
    else:
        print("Invalid plot type", plot_type)
        sys.exit(1)

