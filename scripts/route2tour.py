#!/usr/bin/env python3

import argparse
import os, json, time, sys
from datetime import datetime
from collections import OrderedDict

def get_args():
    parser = argparse.ArgumentParser(description="build LKH-3 TSPLIB files for each routing instance")
    parser.add_argument("--r_file", required=False, default="route_data.json", help="route file (default: route_data.json)", metavar='json')
    parser.add_argument("--s_file", required=False, default="", help="sequence file (default: none)", metavar='json')
    parser.add_argument("--out_dir", required=False, default="", help="sequence file (default: none)", metavar='json')

    return parser.parse_args()

args = get_args()
with open(args.s_file) as f:
  A = json.load(f, object_pairs_hook=OrderedDict)

with open(args.r_file) as f:
  R = json.load(f, object_pairs_hook=OrderedDict)

# print (len(A))

cnt = 0;
for tdata in A:
#  print(tdata)
  perm = []
  pname = args.out_dir + "/amz";
  if cnt < 10:
    pname += "000"
  elif cnt < 100:
    pname += "00"
  elif cnt < 1000:
    pname += "0"
  pname += str(cnt)

  f = open(pname+".tour","w")

  ncount = len(A[tdata]['actual'])
#  print("ncount =", ncount)

  f.write('NAME: ' + pname + '\n')
  f.write('TYPE: TOUR' + '\n');
  f.write('COMMENT: ' + tdata + '\n')
  route = R[tdata]
  if len(route['stops']) != ncount:
    print("ERROR: Route file does not match the time file\n")
    sys.exit()
  f.write('COMMENT: ' + route['route_score'] + '\n')

  f.write('DIMENSION: ' + str(ncount) + '\n')
  f.write('TOUR_SECTION\n')
  for i in range(ncount): perm.append(-1)
  k = 0
  for stop in A[tdata]['actual']:
     n = int(A[tdata]['actual'][stop])
     perm[n] = k;
     k = k+1
  for i in range(ncount): f.write(str(perm[i]+1) + '\n')
  f.write("-1\n");
  f.write("EOF\n");
  f.close()
  cnt += 1
  #if (cnt >= 1): break
