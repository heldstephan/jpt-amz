#!/usr/bin/env python3

import argparse
import json
import math  # see https://pypi.org/project/tsplib95 and https://tsplib95.readthedocs.io/
                 # need at least version 0.7.1, which fixed a bug found by Stephan: pip3 install --user git+https://github.com/rhgrant10/tsplib95
                 # (Stephans original fix: pip3 install --user git+https://github.com/heldstephan/tsplib95 )
import gzip
import time
import datetime

    

def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--rt_file", required=True)
    parser.add_argument("--se_file", required=True)
    parser.add_argument("--pa_file", required=False)
    return parser.parse_args()

def run(args):

    with open(args.rt_file) as f:
        route_data = json.load(f)

    with open(args.se_file) as f:
        solution_data = json.load(f)

        
    # with open(args.pa_file) as f:
    #     parcel_data = json.load(f)

    last_zone_id = ""
    for route_i in solution_data:

        stops = route_data[route_i].get("stops")
        score = route_data[route_i].get("route_score")
        solution = solution_data[route_i]
        stop_dict = dict()
        for sol_t in solution:
            solution_seq = solution[sol_t]
            for stop in solution_seq:
                stop_dict[solution_seq[stop]] = stop
                zone_id =  str(stops[stop].get("zone_id")).split("-")[0]

        

        zone_ids = []
        zone_changes = 0
        for stop_i in sorted(stop_dict):
            stop = stop_dict[stop_i]

            zone_id =  str(stops[stop].get("zone_id"))
            if zone_id != "nan"  and zone_id != last_zone_id :
#                    print (zone_id , " != ", last_zone_id)
                zone_changes = zone_changes + 1
            zone_ids.append(zone_id)
            if zone_id != "nan":
                last_zone_id = zone_id

        zone_ids.remove("nan")        
        num_zones  = len(set(zone_ids))
#        if num_zones < zone_changes:

        print (zone_changes , " "  , len(solution_seq), zone_changes - num_zones +1, " ", num_zones, score, zone_ids, route_i)
        


def _main():
    args = get_args()
    run(args)

if __name__ == "__main__":  # don't run if this script is imported as module
    _main()
