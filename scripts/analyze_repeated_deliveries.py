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
    parser.add_argument("--pa_file", required=True)
    return parser.parse_args()

def run(args):


        
    with open(args.pa_file) as f:
         parcel_data = json.load(f)

    last_zone_id = ""
    for route_i in parcel_data:
        invalid = 0
        parcels = dict()
        for address in parcel_data[route_i]:
            for parcel in parcel_data[route_i][address]:
                if parcel in parcels:
                    print ("Invalid route: ", route_i, parcel)
                    break
                else:
                    parcels[parcel] = 1

            if invalid == 1:
                break


def _main():
    args = get_args()
    run(args)

if __name__ == "__main__":  # don't run if this script is imported as module
    _main()
