#!/usr/bin/env python3

import argparse, json
import time 
import sys, os


def isLKHRunning():
    f = os.popen("ps auxw|grep LKH", "r")
    text = f.read()
    linenum = text.count('\n')
    return linenum > 2

def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--time_file", default="run_time.json")
    parser.add_argument("--r_file", required=False, default="route_data.json", metavar='json')
    parser.add_argument("-start_time", action="store_true")
    return parser.parse_args()

def _main():
    start_time=time.time()  - 10
    
    args = get_args()
    

    if args.start_time == 1:
        outjson = dict()
        outjson["start_time"] = start_time
        
    elif args.time_file:
        with open(args.time_file, "r") as f: outjson = json.load(f)
        start_time = outjson["start_time"]
        if (args.r_file):
            with open(args.r_file) as f: R = json.load(f)
            
        # the tsplib2json conversion time can be reduced after further experiments    
        tsplib2json_time = 1800    
        end_time = start_time + 4 * 3600 - tsplib2json_time
        remaining_time = end_time - time.time()
        
        # merge time assuming 60 s / 1000 merges 
        merge_time = 60 * len(R) / 1000 + 10
        remaining_time -= merge_time

        # distribute the remaining walltime to 16 vCores per number of instances
        time_per_run = remaining_time * 16 / len(R);

        # parallelization pessimism per run
        par_overhead_per_run = 1
        time_per_run1 = int(time_per_run*2/3 - par_overhead_per_run)
        time_per_run2 = int(time_per_run/3   - par_overhead_per_run)

        # we didn't experiment  with > 60 seconds
        # so let's not go beyond
        time_per_run1 = min(60,time_per_run1)
        time_per_run2 = min(60,time_per_run2)

        time_per_run1 = max(1,time_per_run1)
        time_per_run2 = max(1,time_per_run2)

        
        # print ("remaining_time,per run", merge_time, remaining_time, len(R), remaining_time * 16 ,
        #        time_per_run, time_per_run1, time_per_run2)
        outjson["time_per_run1"] = time_per_run1
        outjson["time_per_run2"] = time_per_run2
        print(time_per_run1,time_per_run2)
        
    with open(args.time_file, 'w') as f:
        json.dump(outjson,f)

if __name__ == "__main__":  # don't run if this script is imported as module
    _main()
