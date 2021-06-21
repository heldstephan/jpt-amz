#!/usr/bin/env python3

import argparse
import os, json, time, sys, math, random, itertools
import analyze_zone_id_order as zid
from datetime import datetime
from collections import OrderedDict
import build_TSPLIB as bTSP
import util_zone as uz
#from backports.datetime_fromisoformat import MonkeyPatch
# Get fromisoformat into python 3.6
#MonkeyPatch.patch_fromisoformat()

def get_args():
    parser = argparse.ArgumentParser(description="build LKH-3 TSPLIB files for each routing instance")
    parser.add_argument("--r_file", required=False, default="route_data.json", help="route file (default: route_data.json)", metavar='json')
    parser.add_argument("--s_file", required=False, default="actual_sequences.json", help="sequence file (default: actual_sequences.json)", metavar='json')
    parser.add_argument("--t_file", required=False, default="travel_times.json", help="times file (default: travel_times.json)", metavar='json')
    parser.add_argument("--p_file", required=False, default="package_data.json", help="package file (default: package_data.json)", metavar='json')
    parser.add_argument("-prune", required=False, type=int, default=0, help="specify 0 to not prune instances (default: 1)", metavar='0/1')
    parser.add_argument("-disjoint", required=False, type=int, default=1, help="Create Disjoint build and apply samples. R", metavar='0/1')
    parser.add_argument("DATAdir", type=str, help="specify directory for the DATA files", metavar='DATAdir')
    parser.add_argument("-seed", required=False, type=int, default=99, help='random seed (default: 99)', metavar='int')
    parser.add_argument("--build_sample", required=False, type=int, default=10000, help='random sample of build instances (default: 0 [all])', metavar='int')
    parser.add_argument("--apply_sample", required=False, type=int, default=6000, help='random sample of apply instances (default: 0 [all])', metavar='int')
    parser.add_argument("--station_code", required=False,  default="", help='Station code for which test should be extracted', metavar='string')
    parser.add_argument("--dry_run", required=False,  default="0", help='Just compute sample sizes without creating output', metavar='string')
    return parser.parse_args()


args = get_args()
random.seed(args.seed)
if args.station_code:
    print("Station code", args.station_code)
else:
    print("No Station code provided")
 


print( "Start json reading ... ")

with open(args.r_file) as f:
  R = json.load(f)
print("Done route data.", args.r_file)
if int(args.dry_run) == 0:
    with open(args.t_file) as f:
        T = json.load(f)
        print("Done travel times.",args.t_file )
    with open(args.p_file) as f:
        P = json.load(f)
        print("Done package data.", args.p_file)
    with open(args.s_file) as f:
        S = json.load(f)
        print("Done actual sequences.", args.s_file)
print( "Done json reading ... ")
  
# build list of routes (for possible sampling)
failedList = []

if args.prune ==1:
    failedList  = uz.markFailedRoutes(R,P)
noPruneArg = 1 - args.prune
print ("noPrune",noPruneArg)
initialApplyList =  bTSP.getRouteList(R, noPruneArg, failedList)


routeList = []
applyList = []
for tdata in R:
  route = R[tdata]
  if args.station_code:
      if args.station_code != route["station_code"]:
          continue
  if tdata in initialApplyList:
      applyList.append(tdata)
  routeList.append(tdata)    





nbuild = args.build_sample
napply = args.apply_sample
ntotal = nbuild+napply

print ("Start sampling  with nbuild/napply/ntotal/len(routeList):", nbuild, napply, ntotal, len(routeList))

if int(args.apply_sample) > 0:
    if args.disjoint == 1:
        ntotal = nbuild+napply
        nbuild = min (nbuild, int(len(routeList) * nbuild / ntotal))
        napply = min (nbuild, int(len(routeList) * napply / ntotal))
        nbuild = min (nbuild, len(routeList) - napply)
        ntotal = nbuild+napply
        sampleRoutes = random.sample(routeList, ntotal)
        sampleApplies = random.sample(sampleRoutes, napply)
        sampleBuilds = list(set(sampleRoutes)-set(sampleApplies))
    else:
        nbuild = min(nbuild, len(routeList))
        napply = min(napply, len(applyList))
        ntotal = nbuild+napply
        print ("Sampling  with build/apply/total size:", nbuild, napply, ntotal, len(routeList))
        sampleApplies = random.sample(applyList, napply)
        sampleBuilds = random.sample(routeList, nbuild)

print ("Generating instance with build size", len(sampleBuilds), "and apply size", len(sampleApplies), ntotal)

if int(args.dry_run) == 1:
    print ("Exiting")
    exit(0)


model_build_R  = dict()
model_build_T  = dict()
model_build_S  = dict()
model_build_P  = dict()



model_apply_R  = dict()
model_apply_T  = dict()
model_apply_P  = dict()


model_score_S  = dict()
model_score_I  = dict()


for tdata in sampleBuilds:
    model_build_R[tdata] = R[tdata]
    model_build_T[tdata] = T[tdata]
    model_build_P[tdata] = P[tdata]
    model_build_S[tdata] = S[tdata]

for tdata in sampleApplies :
    new_tdata = tdata # + "JPT"
   
    model_apply_R[new_tdata] = R[tdata]
    model_apply_T[new_tdata] = T[tdata]
    model_apply_P[new_tdata] = P[tdata]
    model_score_S[new_tdata] = S[tdata]
    model_score_I[new_tdata] = 1.2
    if R[tdata]["route_score"] == "Medium":
        model_score_I[new_tdata] = 0.9
    if R[tdata]["route_score"] == "Low":
        model_score_I[new_tdata] = 0.6



if not os.path.exists(args.DATAdir):
  os.makedirs(args.DATAdir)

model_build_dir = args.DATAdir + "/model_build_inputs"
if not os.path.exists(model_build_dir):
  os.makedirs(model_build_dir)

model_apply_dir = args.DATAdir + "/model_apply_inputs"
if not os.path.exists(model_apply_dir):
  os.makedirs(model_apply_dir)

model_score_dir = args.DATAdir + "/model_score_inputs"
if not os.path.exists(model_score_dir):
  os.makedirs(model_score_dir)


outfile = model_build_dir + "/route_data.json"
with open(outfile, 'w') as outfile:
    json.dump(model_build_R, outfile)

outfile = model_build_dir + "/actual_sequences.json"
with open(outfile, 'w') as outfile:
    json.dump(model_build_S, outfile)

outfile = model_build_dir + "/travel_times.json"
with open(outfile, 'w') as outfile:
    json.dump(model_build_T, outfile)

outfile = model_build_dir + "/package_data.json"
with open(outfile, 'w') as outfile:
    json.dump(model_build_P, outfile)


outfile = model_apply_dir + "/new_package_data.json"
with open(outfile, 'w') as outfile:
    json.dump(model_apply_P, outfile)

outfile = model_apply_dir + "/new_route_data.json"
with open(outfile, 'w') as outfile:
    json.dump(model_apply_R, outfile)

outfile = model_apply_dir + "/new_travel_times.json"
with open(outfile, 'w') as outfile:
    json.dump(model_apply_T, outfile)

outfile = model_score_dir + "/new_actual_sequences.json"
with open(outfile, 'w') as outfile:
    json.dump(model_score_S, outfile)

outfile = model_score_dir + "/new_invalid_sequence_scores.json"
with open(outfile, 'w') as outfile:
    json.dump(model_score_I, outfile)
