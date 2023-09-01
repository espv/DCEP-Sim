from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
import sys
import yaml
import os


args = sys.argv

yaml_config = args[1]
file1 = args[1]
file2 = args[2]

f1_created = {}
f1_emitted = {}
f1_sent_when = {}

f2_created = {}
f2_emitted = {}
f2_sent_when = {}

with (open("../" + file1) as f1):
    for l in f1:
        print(l)
        timestamp = l.split(" Node ")[0]
        node = l.split(" Node ")[1].split(" ")[0]
        sending = l.find("Sending") != -1
        creating = l.find("Creating") != -1
        group = l.split(" of group ")[1].split(" and ")[0]
        lower_bound = l.split(" lower_bound ")[1].split(" for the")[0]
        if sending:
            if f1_emitted.get(group) == None:
                f1_emitted[group] = 1
            else:
                f1_emitted[group] += 1

            if f1_sent_when.get(lower_bound) == None:
                f1_sent_when[lower_bound] = 1
            else:
                f1_sent_when[lower_bound] += 1
        elif creating:
            if f1_created.get(group) == None:
                f1_created[group] = 1
            else:
                f1_created[group] += 1
        else:
            print("ERROR")
            exit(0)

with (open("../" + file2) as f2):
    for l in f2:
        print(l)
        timestamp = l.split(" Node ")[0]
        node = l.split(" Node ")[1].split(" ")[0]
        sending = l.find("Sending") != -1
        creating = l.find("Creating") != -1
        group = l.split(" of group ")[1].split(" and ")[0]
        lower_bound = l.split(" lower_bound ")[1].split(" for the")[0]
        if sending:
            if (f2_emitted.get(group) == None):
                f2_emitted[group] = 1
            else:
                f2_emitted[group] += 1

            if f2_sent_when.get(lower_bound) == None:
                f2_sent_when[lower_bound] = 1
            else:
                f2_sent_when[lower_bound] += 1
        elif creating:
            if (f2_created.get(group) == None):
                f2_created[group] = 1
            else:
                f2_created[group] += 1
        else:
            print("ERROR")
            exit(0)

print("f1 created")
for group, cnt in f1_created.items():
    if cnt == 280:
        continue
    print(group, ":", cnt)

print("f2 created")
for group, cnt in f2_created.items():
    if cnt == 280:
        continue
    print(group, ":", cnt)


print("f1 emitted")
for group, cnt in f1_emitted.items():
    if cnt == 280:
        continue
    print(group, ":", cnt)

print("f2 emitted")
for group, cnt in f2_emitted.items():
    if cnt == 280:
        continue
    print(group, ":", cnt)


print("f1 sent when")
f1_total = 0
f1_cnt_sent = 0
f2_total = 0
f2_cnt_sent = 0
for lower_bound, cnt in f1_sent_when.items():
    f1_total += cnt
    f1_cnt_sent += 1
    if f1_sent_when[lower_bound] == f2_sent_when[lower_bound]:
        continue
    print(lower_bound, "Lazy:", cnt, ", no migration:", f2_sent_when[lower_bound])

print("f2 sent when")
for lower_bound, cnt in f2_sent_when.items():
    f2_total += cnt
    f2_cnt_sent += 1
    if cnt <= 1000:
        continue
    print(lower_bound, ":", cnt)

print("f1 total:", f1_total, ", number submissions:", f1_total)
print("f2 total:", f2_total, ", number submissions:", f2_total)

if __name__ == "__main__":
    print("Running")
