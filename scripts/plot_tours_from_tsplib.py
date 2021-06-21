#!/usr/bin/env python3

import argparse
import json
import tsplib95  # see https://pypi.org/project/tsplib95 and https://tsplib95.readthedocs.io/
                 # need at least version 0.7.1, which fixed a bug found by Stephan: pip3 install --user git+https://github.com/rhgrant10/tsplib95
                 # (Stephans original fix: pip3 install --user git+https://github.com/heldstephan/tsplib95 )
import gzip

def get_args():

    parser = argparse.ArgumentParser()
    parser.add_argument("--solfiles", nargs='+', default=[], required=True)
    parser.add_argument("--geomfile", required=True)
    parser.add_argument("--outfile", required=True)
    parser.add_argument("--skip_start", required=False, default = 0)
    return parser.parse_args()

def run(args):
#    problem = tsplib95.load_problem(args.infile)

    # if problem.type not in ["TSP", "ATSP", "CVRP"]:
    #     raise NotImplementedError("Not implemented problem type " + problem.type)
    # if problem.type == "CVRP" and len(problem.depots) > 1:
    #     raise NotImplementedError("We only support CVRP instances with one depot")
    # if len(problem.fixed_edges) > 0:
    #     raise NotImplementedError("We do not support fixed edges")
    # if problem.dimension > int(args.maxnodes):
    #     raise RuntimeError("Instance larger than " + args.maxnodes + " nodes")

    # depot = problem.depots[0] if problem.type == "CVRP" else next(problem.get_nodes())

    geo = tsplib95.load_problem(args.geomfile)
    coords = geo.node_coords
    
    geoname = geo.name[:7]
    features = []
    colors = ["blue", "green","yellow","red"]
    soln = 0
    for solfile in args.solfiles:
        solution = tsplib95.load_problem(solfile)
        solname = solution.name[:7]
        if solname != geoname:
            print ("Solution file " + solname + "  and geo file + " + geoname + "  refer to different instances!")
            exit(1)
        
        tours = solution.as_dict().get("tours")
        for tour in tours:
            coordinates = []
            ncities=0
            for city in tour:
                ncities = ncities+1
                if   (ncities == 1) and (int(args.skip_start) == 1):
                    continue
                long = coords[city][0] + float(soln)/100000
                lat = coords[city][1] + float(soln)/100000
                coord = [lat, long]
                coordinates.append(coord)

        geometry = {
            "type": "LineString",
            "coordinates": coordinates,
            }

        feature = {
            "type": "Feature",
            "properties": {
                "stroke": colors[soln % len(colors)] ,
                "opacity": 0.5,
                "fill-opacity": 0.0
            },
            "geometry" : geometry
        }
        features.append(feature)

        soln = soln +1


    json_data = {
        "type": "FeatureCollection",
        "name": solution.name,
        "features" : features,
    }
    
    filename = args.outfile
    with open(filename, 'w') as outfile:
         json.dump(json_data, outfile)
            
def _main():
    args = get_args()
    run(args)

if __name__ == "__main__":  # don't run if this script is imported as module
    _main()
