#!/usr/bin/env python3

import argparse
import json
import math
import util_zone as uz

import gzip
import time, timeit
import datetime
import colorsys
import analyze_zone_id_order as zid

def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--r_file", required=False, default="route_data.json")
    parser.add_argument("--nr_file", required=False, default="" , help = 'new_route_data.json')
    parser.add_argument("--s_file", required=False, default="actual_sequences.json")
    parser.add_argument("--out_file", required=False, default="station_zone_paths.json")

    return parser.parse_args()

                    
def _main():
    args = get_args()

    with open(args.r_file) as f:
        R = json.load(f)
        
    with open(args.s_file) as f:
        S = json.load(f)

    new_R = None
    if args.nr_file != "":
        with open(args.nr_file) as f:
            new_R = json.load(f)

    print ("Adding missing zones.")
    start = timeit.default_timer()
    uz.addMissingData(R, "ATLANTIS\_BR") # add missing station code, lat/lng, ... to prevent crashes
    
    uz.addMissingZones(R,new_R)                  # add zone IDs to stops (if missing)
    end = timeit.default_timer()
    print("Adding missing zones took", end-start, "seconds.")
    # verbosity = 2
    # zone_graphs = zid.run(R, S, "min_cost_edge", verbosity)

    # for station_str in zone_graphs:
    #     print(station_str, zone_graphs[station_str])
    station_infos = zid.station_route_zones(R, S)

    print("Creating cluster keys ...")
    clusterKeys = uz.createClusterKeys(R, S)

    print("Creating super sets ...")
    stationList = uz.getStationList(R)     # build sorted list of stations
    stationStops = uz.getStationStopsLatLng(R,stationList)
    superSets = uz.getSuperSets(R,S,stationList,stationStops,
                                    clusterKeys["cluster"])

    outjson = dict()
    outjson["station_infos"] = station_infos
    outjson["clusterKeys"]   = clusterKeys
    outjson["superSets"]     = superSets

    
    with open(args.out_file, 'w') as f:
        json.dump(outjson,f)
       

if __name__ == "__main__":  # don't run if this script is imported as module
    _main()
