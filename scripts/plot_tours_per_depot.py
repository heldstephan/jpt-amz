#!/usr/bin/env python3

import argparse
import json
import math  # see https://pypi.org/project/tsplib95 and https://tsplib95.readthedocs.io/
                 # need at least version 0.7.1, which fixed a bug found by Stephan: pip3 install --user git+https://github.com/rhgrant10/tsplib95
                 # (Stephans original fix: pip3 install --user git+https://github.com/heldstephan/tsplib95 )
import gzip
import time
import datetime
import colorsys

def get_N_HexCol(N=5):
    HSV_tuples = [(x * 1.0 / N, 0.5, 0.5) for x in range(N)]
    hex_out = []
    for rgb in HSV_tuples:
        rgb = map(lambda x: int(x * 255), colorsys.hsv_to_rgb(*rgb))
        hex_out.append('#%02x%02x%02x' % tuple(rgb))
    return hex_out
    
def get_route_score_color(score_string):
    if score_string == "High":
        return "x00FF00"
    if score_string == "Medium":
        return "x0000FF"
    if score_string == "Low":
        return "xFF0000"
    return "x000000"


def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--rt_file", required=True)
    parser.add_argument("--se_file", required=True)
    parser.add_argument("--annotate_zones", required=False, type=int, default=0)
    parser.add_argument("--color_by_route_score", required=False, type=int, default=0)
    parser.add_argument("--routes", nargs='+', default=[], required=False)

    return parser.parse_args()

def run(args):

    with open(args.rt_file) as f:
        route_data = json.load(f)

    with open(args.se_file) as f:
        solution_data = json.load(f)

        
    # with open(args.pa_file) as f:
    #     parcel_data = json.load(f)

    stations= dict()

    last_zone_id = ""
    features = dict()

    zone_stops = dict()
    

    for route_i in solution_data:

        if args.routes and len(args.routes) and not route_i in args.routes:
            continue

        stops = route_data[route_i].get("stops")
        station_str = str(route_data[route_i].get("station_code"))
        score = route_data[route_i].get("route_score")
        solution = solution_data[route_i]
        
        if station_str not in features:
            features[station_str] = []
            zone_stops[station_str] = dict()

        
        coordinate_dict = dict()
        for sol_t in solution:
            solution_seq = solution[sol_t]

            for stop in solution_seq:
                lat =  stops[stop].get("lat")
                lng =  stops[stop].get("lng")
                coord = [lng, lat]
                coordinate_dict[solution_seq[stop]] = coord
                zone =  str(stops[stop].get("zone_id"))
                if zone not in zone_stops[station_str]:
                    zone_stops[station_str][zone] = []
                if solution_seq[stop] != 0:
                    zone_stops[station_str][zone].append(coord)

        coordinates = []
        for key in sorted(coordinate_dict):
            coordinates.append(coordinate_dict[key])
    
            
            
        geometry = {
            "type": "LineString",
            "coordinates": coordinates,
        }
        soln = len(features[station_str])
        feature = {
            "type": "Feature",
            "properties": {
                "opacity": 0.5,
                "fill-opacity": 0.0,
                "name": route_i
            },
            "geometry" : geometry
        }
        if args.color_by_route_score:
            properties = feature.get("properties")
            color = get_route_score_color(score)
            print ("Route score ", score, color, route_i)
            properties["stroke"] = color
            
        features[station_str].append(feature)



    for station_str in features:
        num_colors = len(features[station_str])
        colors = get_N_HexCol(min (num_colors,254))
        station_features = []
        i = 0
        for feature in features[station_str]:
            properties = feature.get("properties")
            if not properties["stroke"]:
                properties["stroke"] = colors[i % len(colors)]
                i = i + 1

            station_features.append(feature)
                
            first_geometry = {
                "type": "Point",
                "coordinates": feature.get("geometry").get("coordinates")[1]
            }
            first_feature = {
                "type": "Feature",
                "properties": {
                    "opacity": 1,
                    "strokeColor": "x000000",
                    "fill-opacity": 1.0
                },
                "geometry" : first_geometry
            }
            station_features.append(first_feature)

        if args.annotate_zones:    
            i = 0
            for zone in zone_stops[station_str]:
                num_colors = len(features[station_str])
                colors = get_N_HexCol(min (num_colors,254))
                color = colors[i % len(colors)]
                for coord in zone_stops[station_str][zone]:
                    zone_geometry = {
                        "type": "Point",
                        "coordinates": coord,
                    }
                    zone_feature = {
                        "type": "Feature",
                        "properties": {
                            "name": zone,
                            "opacity": 1,
                            "color":  color,
                            "fill":  color,
                            "fill-opacity": 0.3
                        },
                        "geometry" : zone_geometry
                    }
                    station_features.append(zone_feature)
                    i = i + 1

                    
        json_data = {
             "type": "FeatureCollection",
             "name": station_str,
             "features" : station_features,
        }

    

        filename = station_str + "_ALL_TOURS.geojson"
        with open(filename, 'w') as outfile:
            json.dump(json_data, outfile)
    
def _main():
    args = get_args()
    run(args)

if __name__ == "__main__":  # don't run if this script is imported as module
    _main()
