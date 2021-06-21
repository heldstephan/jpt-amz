#!/usr/bin/env python3

import argparse
import json
import gzip
import os

def read_tour_section(fp, tour_data):
    proposed_seq = dict()
    line = fp.readline()
    stopno = 0
    while line:
        tsp_i = str(line.split()[0])
        if tsp_i == "-1":
            break
        route_id = tour_data[tsp_i]
        proposed_seq[route_id] = stopno
        line = fp.readline()
        stopno += 1
        

    return proposed_seq
    
def add_tour_file(dir, file,sol_dict, T2A):

    filename = dir + "/" + file
    pname = file[:7]

    tour_data = T2A[pname]["stops"]

    with open(filename, 'r') as fp:
        line = fp.readline()
        
        while line:
            tokens = line.split()
            if tokens[0] == "TYPE":
                if tokens[2] != "TOUR":
                    print("ERROR wrong type ", tokens[2], " != TOUR")
            if tokens[0] == "DIMENSION":
                dimension = int(tokens[2])
                if dimension != len(tour_data):
                    print("ERROR", dimension, len(tour_data))
            if tokens[0] == "TOUR_SECTION":
                proposed_seq = read_tour_section(fp, tour_data)
            line = fp.readline()

    route_id = T2A[pname]["route_id"]         
    sol_dict[route_id]  = dict()
    sol_dict[route_id]["proposed"] = proposed_seq
        

def get_args():

    parser = argparse.ArgumentParser()
    parser.add_argument("--tour_dir",  default=[], required=True)
    parser.add_argument("--tsplib2amz_file", default=[], required=True)
    parser.add_argument("--out_json", required=True)
    return parser.parse_args()

def tsplib2json(args):
    with open(args.tsplib2amz_file) as f: T2A = json.load(f)

    sol_dict = dict()
    
    print("tour_dir",args.tour_dir)
    for file in os.listdir(args.tour_dir):
        if file.endswith(".tour"):

            add_tour_file(args.tour_dir, file, sol_dict, T2A)
           
    filename = args.out_json
    with open(filename, 'w') as outfile:
         json.dump(sol_dict, outfile)

        
def _main():
    args = get_args()
    tsplib2json(args)

if __name__ == "__main__":  # don't run if this script is imported as module
    _main()
