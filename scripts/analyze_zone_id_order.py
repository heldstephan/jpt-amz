#!/usr/bin/env python3

import operator
import argparse
import json
import math  
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
    
def collect_start_vertices(graph, verbosity):
    start_vertices = dict()
    for start in graph:
        start_vertices[start] = 1

    for start in graph:
        for end in graph[start]:
            if end in start_vertices:
                del start_vertices[end]

    return start_vertices

# solution is a rout entry from the actual_sequence json
def route_stops_by_order(solution, verbosity = 0):
    stops_by_order = dict()
    solution_seq = solution.get("actual")
    if solution_seq != solution_seq:
        print("Solution has no actual.", solution)
        return None
    try:
        some_object_iterator = iter(solution_seq)
    except TypeError as te:
        print('Is not iterable:', solution_seq)
        
    for stop in solution_seq:    
        position = solution_seq[stop];
        stops_by_order[position] = stop

    return stops_by_order

    
    
def run_dfs(graph, verbosity):
    marker = dict()
    root  = "Station"
    stack = []
    min_cost_edge = []
    for vertex in collect_start_vertices(graph,verbosity):
        stack.append(vertex)

    while len(stack) > 0:
        start = stack.pop();
        if start in marker  and marker[start] == "exploring":
            marker[start] = "visited"
        else:
            marker[start] = "exploring"
            stack.append(start)
            
#        print ("Scanning ", start)
        for end in graph[start]:
            if end in marker:
                if marker[end] == "exploring":
                    if verbosity >= 1 :
                        print ("Found cycle at start ", start, " end ", end)
                    head = end
                    min_cost_edge = (start,end)
                    min_cost = graph[start][end]
                    cycle = []
                    for zone in reversed(stack):
                        if zone in marker and marker[zone] == "exploring" and head in graph[zone]:
                            #                            print (zone, head, marker[zone], marker[head])
                            cycle.append((zone,head))
                            if min_cost > graph[zone][head]:
                                min_cost = graph[zone][head]
                                min_cost_edge = (zone,head)

                            if verbosity  >= 2:
                                print (zone, head, graph[zone][head])
                            head = zone
                        if zone == end:
                            break
                    if verbosity  >= 2:    
                        print("Min cost edge on cycle is ", min_cost_edge[0], min_cost_edge[1],graph[min_cost_edge[0]][min_cost_edge[1]])
                        print("Return min_cost_edge", min_cost_edge)
                    return min_cost_edge, cycle
            else:
                if end in graph:
                    stack.append(end)
    return None,None
 

def print_graph(graph):
    narcs = 0
    for start_zone in graph:
        outarcs = graph[start_zone]
        narcs += len(outarcs)
        for end_zone in outarcs:
            print ("e ", start_zone, " ", end_zone, outarcs[end_zone])

    print(narcs , "arcs")
    print ("====================================")

def print_station_graphs(station_graphs):
    for station_str in station_graphs:
        print ("Zone graph of station ", station_str)
        graph = station_graphs[station_str]
        print_graph(graph)

def full_zone_graphs(route_data, solution_data, verbosity = 0):
    stations= dict()

    last_zone_id = ""

    station_zones = dict()
    station_graphs = dict()    # for each station, a dict of zone ids each with a dict of outarcs
    num_edges = dict()
    for route_i in solution_data:

        stops = route_data[route_i].get("stops")
        station_str = str(route_data[route_i].get("station_code"))
        score = route_data[route_i].get("route_score")
        solution = solution_data[route_i]
        
        if station_str not in station_graphs:
            station_zones[station_str] = dict()
            station_graphs[station_str] = dict()
            num_edges[station_str] = 0
            
        graph = station_graphs[station_str]
        zones = station_zones[station_str]

        stop_dict = dict()
        for sol_t in solution:
            solution_seq = solution[sol_t]
            for stop in solution_seq:
                stop_dict[solution_seq[stop]] = stop

        coordinates = []
        zone_changes = 0
        last_zone_id = ""
        nstops = -1
        for stop_i in sorted(stop_dict):
            nstops+= 1    
            if nstops == 0:
                last_zone_id = "Station"
                continue
            
            stop = stop_dict[stop_i]

            zone_id =  stops[stop].get("zone_id")
            
            if  zone_id and zone_id == zone_id and zone_id != last_zone_id and last_zone_id != "" :
                num_edges[station_str] =  num_edges[station_str] + 1

                if last_zone_id not in graph:
                    outarcs = dict()
                    graph[last_zone_id] = outarcs

                if   zone_id in graph[last_zone_id]:
                    graph[last_zone_id][zone_id] = graph[last_zone_id][zone_id]  + 1
                else:
                    graph[last_zone_id][zone_id] = 1
                    
                
            if zone_id and zone_id == zone_id:
                zones[zone_id] = 1
                last_zone_id = zone_id
    return station_graphs



def delete_cycles(full_station_graphs, mode, verbosity = 0):
    station_graphs = full_station_graphs
    zone_orders = dict()
    for station_str in station_graphs:
        min_cost_edge = 1
        graph = station_graphs[station_str]
        if verbosity >= 1 :
            print ("Processing station", station_str)
            print_graph(graph)
        ndfs = 1
        while min_cost_edge:
            if verbosity >= 1 :
                print("DFS Call", ndfs)
            ndfs = ndfs+1
            min_cost_edge, cycle = run_dfs(graph,verbosity)
            if min_cost_edge:
                if mode == "min_cost_edge":
                    start = min_cost_edge[0]
                    end = min_cost_edge[1]
                    if verbosity >= 1 :
                        print ("Deleting edge ", start, end, graph[start][end])
                    del graph[start][end]
                if mode == "cycle":
                    for edge in cycle:
                        start = edge[0]
                        end = edge[1]
                        if verbosity >= 1 :
                            print ("Deleting edge ", start, end)
                        del graph[start][end]
        if verbosity >=1:       
            print_graph(graph)
        
    return station_graphs

        
def run(route_data, solution_data, mode = "cycle", verbosity = 0):


    station_graphs = full_zone_graphs(route_data, solution_data, verbosity)

    station_graphs = delete_cycles(station_graphs, mode, verbosity)
    return station_graphs


# Tarjan for zone list, i.e.  edge progressions
def tarjan_visit1(R, N, psi, psi_inverse, zone_id, zone_graph ):
    R[zone_id] = 1

    if zone_id in zone_graph:
        for head_zone in zone_graph[zone_id]:
            if head_zone not in R:
                N = tarjan_visit1(R, N, psi, psi_inverse, head_zone, zone_graph)
    N += 1
    psi[zone_id] = N
    psi_inverse[N] = zone_id
    return N
    
def tarjan_visit2(R, K, comp, zone_id, zone_graph):
    R[zone_id] = 1
    for tail in zone_graph:
        for head in zone_graph[tail]:
            if head == zone_id:
                if tail not in R:
                    tarjan_visit2(R,K,comp, tail,zone_graph)
    comp[zone_id] = K
      
def run_tarjan(zone_list, zone_graph,verbosity=0):
    R = dict()
    psi = dict()
    comp = dict()
    psi_inverse = dict()
    N = 0
    for i in  range(len(zone_list)):
        zone_id = zone_list[i]
        if zone_id not in R:
            N = tarjan_visit1(R, N, psi, psi_inverse, zone_id, zone_graph)

    R = dict()
    K = 0
    for i in reversed(range(1,len(psi)+1)):
        if psi_inverse[i] not in R:
            K += 1
            tarjan_visit2(R, K, comp, psi_inverse[i], zone_graph)
    return comp

def compute_proper_components(comps):
    proper_comps = dict()
    for zone_id in comps:
        c_i = comps[zone_id]
        if c_i not in proper_comps:
            proper_comps[c_i] = []
        proper_comps[c_i].append(zone_id)
                    
    return proper_comps

def prune_full_components(zone_graph, comps):
    for tail in zone_graph:
        deleted_heads = []
        for head in zone_graph[tail]:
            if comps[tail] == comps[head]:
                deleted_heads.append(head)
        for head in  deleted_heads:
            del zone_graph[tail][head]
    return zone_graph            


                

def compute_zone_distancegraph(route_data, travel_times):
    zone_matrices = dict()
    for route_i in route_data:
        
        stops = route_data[route_i].get("stops")
        station_str = str(route_data[route_i].get("station_code"))
        edges = []
        
        if station_str not in route_infos:
            zone_matrices[station_str] = dict()        
                          
        solution = solution_data[route_i]
        stops_by_order = route_stops_by_order(solution)
        zone_matrix = [station_str]

        for sol_i in sorted(stops_by_order):
            stop_i = stops_by_order[sol_i]
            zone_i = stops[stop_i].get("zone_id")
            if zone_i != zone_i:
                continue
            if not zone_i in zone_matrix:
                zone_matrix[zone_i] = Dict()
            for sol_j in sorted(stops_by_order):
                stop_j = stops_by_order[sol_j]
                zone_j = stops[stop_j].get("zone_id")
                if zone_j != zone_j:
                    continue
                if zone_j not in zone_matrix[zone_i]: 
                    zone_matrix[zone_i][zone_j] = 2000000000
                dist = min(travel_times[stop_i][stop_j],travel_times[stop_j][stop_i])
                zone_matrix[zone_i][zone_j] = min(zone_matrix[zone_i][zone_j],dist)

def get_accumulated_zones(route):

    stops = route.get("stops")
    accumulated_route_zones = dict()
    for stop in stops:
        zone_id = stops[stop].get("zone_id")
        if zone_id not in accumulated_route_zones:
            accumulated_route_zones[zone_id] = 0
        
        accumulated_route_zones[zone_id] += 1

    return accumulated_route_zones
    
def accumulated_zonelist2dict(accumulated_zone_list):
    accumulated_route_zones = dict()
    for (zone_id, number) in accumulated_zone_list:
        if zone_id not in accumulated_route_zones:
            accumulated_route_zones[zone_id] = number
        else:
            accumulated_route_zones[zone_id] += number

    return accumulated_route_zones
    

def station_route_zones(route_data, solution_data, verbosity = 0):

    route_infos = dict()
    for route_i in route_data:

        stops = route_data[route_i].get("stops")
        station_str = str(route_data[route_i].get("station_code"))
        edges = []
        
        if station_str not in route_infos:
            route_infos[station_str] = dict()        
        
        route_infos[station_str][route_i] = dict()
            
        this_info = route_infos[station_str][route_i]
        
        zone_list = []
        accumulated_zone_list = []
        solution = solution_data[route_i]
        stops_by_order = route_stops_by_order(solution)

        for sol_i in sorted(stops_by_order):
            stop = stops_by_order[sol_i]
            zone_id = stops[stop].get("zone_id")
            if zone_id == zone_id and (len(zone_list) == 0 or zone_list[-1] != zone_id):
                zone_list.append(zone_id)
                accumulated_zone_list.append( (zone_id,1))
            if len(accumulated_zone_list) > 0:
                last = accumulated_zone_list.pop()
                new_last = (last[0], last[1]+1)
                accumulated_zone_list.append(new_last)

        this_info["zone_list"] = zone_list
        this_info["accumulated_zone_list"] = accumulated_zone_list
        zone_graph = zone_list_to_graph(zone_list)
   

        comps = run_tarjan(zone_list, zone_graph,verbosity=1)
        zone_graph = prune_full_components(zone_graph, comps)


        this_info["zone_graph"] = zone_graph
        this_info["route_score"] = route_data[route_i].get("route_score")
        this_info["full_components"] = comps

    return route_infos

    
def zone_list_to_graph(zone_list):
    zone_graph = dict()
    for i in  range(1,len(zone_list)):
        v = zone_list[i-1]
        w = zone_list[i]
        if v not in zone_graph:
            zone_graph[v] = dict()
        zone_graph[v][w] = 1
    return zone_graph



def zone_size_match(accumulated_zones,ref_accumulated_zones):
    value = 0
    for zone_id in accumulated_zones:
        if zone_id in ref_accumulated_zones:
            value += min(accumulated_zones[zone_id],ref_accumulated_zones[zone_id])

    return value
    

# find & rank the zones with the best zone matching return the best match.
def find_pred_clusters(id,routeData, zoneList,stationInfos):
    best_fwd_match_num = 0
    best_bwd_match_num = 0
    best_zs_match = 0
    found_match = 0

    best_match_list = []

    accumulated_zones = get_accumulated_zones(routeData)

    for r_i in stationInfos:
        if r_i == id:
            continue
            # We must not collect information from the given route
        match_mult  = 1
        if stationInfos[r_i]['route_score'] == "High":
            match_mult = 2
        if stationInfos[r_i]['route_score'] == "Medium":
            match_mult = 1.5
        fwd_match_num = 0
        bwd_match_num = 0
        ref_zone_list = stationInfos[r_i]['zone_list']
        for zone in zoneList:
            if zone in ref_zone_list:
                fwd_match_num += 1
        for zone in ref_zone_list:
            if zone in zoneList:
                bwd_match_num += 1
        
        fwd_match_num *= match_mult
        bwd_match_num *= match_mult

        ref_accumulated_zones = accumulated_zonelist2dict(stationInfos[r_i]['accumulated_zone_list'])
        zs_match =  zone_size_match(accumulated_zones, ref_accumulated_zones)

        if ( (fwd_match_num > best_fwd_match_num)  or 
             ((fwd_match_num == best_fwd_match_num) and (bwd_match_num >  best_bwd_match_num)) or 
             ((fwd_match_num == best_fwd_match_num) and (bwd_match_num == best_bwd_match_num) and ( zs_match >= best_zs_match)) ):
            best_match_list.append(r_i)
            if ( (fwd_match_num > best_fwd_match_num) or 
                 ((fwd_match_num == best_fwd_match_num) and (bwd_match_num > best_bwd_match_num)) or
                 ((fwd_match_num == best_fwd_match_num) and (bwd_match_num == best_bwd_match_num) and (zs_match > best_zs_match))):

                best_match = r_i
                best_fwd_match_num = fwd_match_num
                best_bwd_match_num = bwd_match_num
                best_zs_match = zs_match
                found_match = 1

        
    if found_match:        
        return stationInfos[best_match]
    else:
        return None

def extract_proper_clusters(comps, zoneList):
    extractedZoneClusters = dict()
    singletons = []
    for zone_id in zoneList:
        if zone_id in comps:
            c_i = comps[zone_id]
            if c_i not in extractedZoneClusters:
                extractedZoneClusters[c_i] = []
            extractedZoneClusters[c_i].append(zone_id)
        else:
            singletons.append(zone_id)

    c_i = len(comps)+1            
    for zone_id in singletons:
        extractedZoneClusters[c_i] = {zone_id}
        c_i += 1

    return extractedZoneClusters
        
