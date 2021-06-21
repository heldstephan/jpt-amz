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

def normalize_tsplib_file(dir, file, out_dir):

    filename = dir + "/" + file
    out_txt = ""
    outfilename = "xxx"

    with open(filename, 'r') as fp:
        line = fp.readline()
        while line:
            tokens = line.split()
            if tokens[0] == "COMMENT:" and tokens[1][:7] == "RouteID":
                outfilename = out_dir + "/" + tokens[1] + ".ctsptw"
                out_txt = "NAME: " + tokens[1] + "\n"

            if tokens[0] != "NAME":
                out_txt += line

            line = fp.readline()


    with open(outfilename, 'wt') as fp:
        fp.write(out_txt)


def get_args():

    parser = argparse.ArgumentParser()
    parser.add_argument("--tsplib_dir",  default=[], required=True)
    parser.add_argument("--out_dir",  default=[], required=True)

    return parser.parse_args()

def normalize_tsplib(args):
    os.mkdir(args.out_dir)

    print("tsplib dir",args.tsplib_dir)
    for file in os.listdir(args.tsplib_dir):
        if file.endswith(".ctsptw"):
            normalize_tsplib_file(args.tsplib_dir, file, args.out_dir)


def _main():
    args = get_args()
    normalize_tsplib(args)

if __name__ == "__main__":  # don't run if this script is imported as module
    _main()
