#!/usr/bin/env python3

import argparse
import os, json, time, sys, math, random, itertools, timeit
import analyze_zone_id_order as zid
import util_zone as uz
from datetime import datetime
from natsort import natsorted
from collections import OrderedDict

def get_args():
    parser = argparse.ArgumentParser(description="build LKH-3 TSPLIB files for each routing instance")
    parser.add_argument("--r_file", required=False, default="route_data.json", help="route file (default: route_data.json)", metavar='json')
    parser.add_argument("--t_file", required=False, default="travel_times.json", help="times file (default: travel_times.json)", metavar='json')
    parser.add_argument("--p_file", required=False, default="package_data.json", help="package file (default: package_data.json)", metavar='json')

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--s_file", required=False, default="", help="sequence file (default: none)", metavar='json')
    group.add_argument("--z_file", required=False, default="", help="package file (default: none)", metavar='json')
    parser.add_argument("--br_file", required=False, default="", help="rout file from the build phase", metavar='json')
    parser.add_argument("-noPrune", action="store_true", help='do not prune the medium and low instances')
    parser.add_argument("-noPruneFailed", action="store_true", help='do not prune the instances with failed deliveries')
    parser.add_argument("TSPLIBdir", type=str, help="specify directory for the TSPLIB files", metavar='TSPLIB_directory')
    parser.add_argument("-seed", required=False, type=int, default=99, help='random seed (default: 99)', metavar='int')
    parser.add_argument("-sample", required=False, type=int, default=0, help='random sample of instances (default: 0 [all])', metavar='int')
    parser.add_argument("-noCluster", action="store_true", help='do not create super clusters of zones')
    parser.add_argument("-noClusterNeighbors", action="store_true", help='do not add neighbor constraints for super cluster')
    parser.add_argument("-superPred", action="store_true", help='add precedence constraints for super clusters')
    parser.add_argument("-noSuperPath", action="store_true", help='do not create path constraints for super clusters')
    parser.add_argument("-noSuperSuper", action="store_true", help='do not create super super clusters')
    parser.add_argument("-superNeighborTrans", action="store_true", help='add transition constraints for super clusters')
    parser.add_argument("-zoneNeighborTrans", action="store_true", help='add transition constraints for zones')
    parser.add_argument("-Windows", action="store_true", help='turn on time windows')
    parser.add_argument("-noZonePred", action="store_true", help='turn off the zone precedence constraints')
    parser.add_argument("-noZone", action="store_true", help='turn off zones')
    parser.add_argument("-disjunctiveZonePred", action="store_true", help='turn on disjunctive precedence constraints')
    parser.add_argument("-paths", action="store_true", help='turn on path constraints')
    parser.add_argument("-trans", action="store_true", help='use transitive closoure precedence constraints')



    return parser.parse_args()

def getRouteList(routes, noPrune, failedList):
    rList = []
    r = 0
    for id in routes:
        route = routes[id]
        if ( (len(failedList) == 0) or (failedList[r]==0)) and (noPrune or (route['route_score']=='High')):
            rList.append(id)
        r = r+1
    return rList

def createProbName(cnt):
    pname = "amz";
    if cnt < 10:     pname += "000"
    elif cnt < 100:  pname += "00"
    elif cnt < 1000: pname += "0"
    pname += str(cnt)
    return pname

def writeHeader(f, pname, id, score, ncount):
    f.write('NAME: ' + pname + '\n')
    f.write('TYPE: TSPTW' + '\n');
    f.write('COMMENT: ' + id + '\n')
    f.write('COMMENT: ' + score + '\n')
    f.write('DIMENSION: ' + str(ncount) + '\n')

def writeEdgeWeightSection(f, routeTimes, ncount):
    f.write('EDGE_WEIGHT_TYPE: EXPLICIT\n')
    f.write('EDGE_WEIGHT_FORMAT: FULL_MATRIX\n')
    f.write('EDGE_WEIGHT_SECTION\n')
    for stop in routeTimes:
        timelist = routeTimes[stop]
        dline = ""
        k = 0
        for dest in timelist:
            dline += str(int(10*routeTimes[stop][dest]))
            k += 1
            if (k < ncount): dline += " "
            else:            dline += "\n"
        f.write(dline)

def writeTimeWindowSection(f, routeData, packageData):
    f.write("TIME_WINDOW_SECTION\n")
    ddd = datetime.fromisoformat(routeData['date_YYYY_MM_DD'] + " " +
                                 routeData['departure_time_utc'])
    truck_start = ddd.timestamp()
    stopcnt = 1
    for stop in packageData:
        window_start = 0
        window_end = 100000
        for package in packageData[stop]:
            ttt = packageData[stop][package]['time_window']
            if (ttt['start_time_utc'] and
                ttt['start_time_utc'] == ttt['start_time_utc']):
                ddd = datetime.fromisoformat(str(ttt['start_time_utc']))
                pstart = ddd.timestamp() - truck_start
                if pstart > window_start: window_start = pstart
            if (ttt['end_time_utc'] and
                ttt['end_time_utc'] == ttt['end_time_utc']):
                ddd = datetime.fromisoformat(str(ttt['end_time_utc']))
                pend = ddd.timestamp() - truck_start
                if pend < window_end: window_end = pend
        f.write(str(stopcnt) + " " + str(int(10*window_start)) +
                               " " + str(int(10*window_end)) + '\n')
        stopcnt += 1

    f.write("SERVICE_TIME_SECTION\n")
    stopcnt = 1
    for stop in packageData:
        service = 0
        for package in packageData[stop]:
            ts = packageData[stop][package]['planned_service_time_seconds']
            # if ts > service: service = ts
            service += ts
        f.write(str(stopcnt) + " " + str(int(10*service)) + '\n')
        stopcnt += 1

def writeDepotSection(f, routeData):
    f.write("DEPOT_SECTION\n")
    stopcnt = 1
    for stop in routeData["stops"]:
        if routeData["stops"][stop].get("type") == "Station":
            f.write(str(stopcnt) + "\n")
            break
        stopcnt += 1
    f.write("-1\n")

def writeSetSection(f, zoneList, zoneIndex):
    f.write("GTSP_SETS: " + str(len(zoneList)) +"\n")
    f.write("GTSP_SET_SECTION\n")
    for i in range(len(zoneList)):
        f.write(str(i+1) + ' ')
        for j in range(len(zoneIndex)):
            if zoneIndex[j] == i: f.write(str(j+1) + ' ')
        f.write("-1\n")

def writeSuperSetSection(f, patternList, zoneList):
    f.write("SUPER_GTSP_SETS: " + str(len(patternList)) +"\n")
    f.write("SUPER_GTSP_SET_SECTION\n")
    for i in range(len(patternList)):
        f.write(str(i+1) + ' ')
        for z in patternList[i]: f.write(str(zoneList.index(z)+1) + ' ')
        f.write("-1\n")

def writeSuperNeighborSection(f, zoneList, clusterKeys, args):
    # gather clusters with same type-2 pattern
    pName = []
    zName = []
    for z in zoneList:
        if z != 'Depot': q = uz.getClusterKey(z,clusterKeys["cluster"])
        else:            q = 'Depot'
        if q not in pName:
            pName.append(q)
            zName.append(z)

    pList2 = []
    pName2 = []
    zName2 = []
    for i in range(len(pName)):
        if zName[i] == 'Depot': continue
        p = pName[i]
        q = uz.getClusterKey(zName[i],clusterKeys["superCluster"])
        if q in pName2:
            k = pName2.index(q)
            pList2[k].append(p)
        else:
            pName2.append(q)
            pList2.append([p])
            zName2.append(zName[i])

    topName = []
    topList = []
    for i in range(len(pName2)):
        p = pName2[i]
        q = uz.getClusterKey(zName2[i],clusterKeys["topCluster"])
        if q in topName:
            k = topName.index(q)
            topList[k].append(p)
        else:
            topName.append(q)
            topList.append([p])

    f.write("SUPER_ZONE_NEIGHBOR_SECTION\n")

    neighborList = []

    # create neighbor constraints for super zones within a super-super zone,
    # where neigbhors are determined by the natural sorted order of the keys

    for i in range(len(pList2)):
        pList2[i] = natsorted(pList2[i])
    for nList in pList2:
        for kk in range(len(nList)-1):
            a = pName.index(nList[kk])+1
            b = pName.index(nList[kk+1])+1
            neighborList.append([a,b])

    # create extra neighbor constraints for super zones with matching
    # transition item in neighboring super-super zones

    if args.superNeighborTrans:
        disjList = []

        # check that the transition item is the last in the cluster key,
        # otherwise skip the contraints (the code can be extended to handle
        # any position of the transition item, taking care that item 1 may
        # have 2 digits in the zone ID)

        t = list(set(clusterKeys["cluster"]) - set(clusterKeys["superCluster"]))

        if t[0] == 3:
            for i in range(len(pList2)-1):
                for j in range(i+1,len(pList2)):
                    if (uz.getClusterKey(zName2[i],clusterKeys["topCluster"]) !=
                        uz.getClusterKey(zName2[j],clusterKeys["topCluster"])):
                        continue
                    tEdges = []
                    m = pList2[i]
                    n = pList2[j]
                    if m[0][-1] == n[0][-1]:
                        tEdges.append([m[0],n[0]])
                    if len(n) >= 2 and m[0][-1] == n[-1][-1]:
                        tEdges.append([m[0],n[-1]])
                    if len(m) >= 2:
                        if m[-1][-1] == n[0][-1]:
                            tEdges.append([m[-1],n[0]])
                        if len(n) >= 2 and m[-1][-1] == n[-1][-1]:
                            tEdges.append([m[-1],n[-1]])
                    if len(tEdges) == 1:
                        a = pName.index(tEdges[0][0])+1
                        b = pName.index(tEdges[0][1])+1
                        neighborList.append([a,b])
                    elif len(tEdges) == 2:
                        a = pName.index(tEdges[0][0])+1
                        b = pName.index(tEdges[0][1])+1
                        c = pName.index(tEdges[1][0])+1
                        d = pName.index(tEdges[1][1])+1
                        disjList.append([[a,b],[c,d]])

            for i in range(len(disjList)):
                a = disjList[i][0][0]
                b = disjList[i][0][1]
                f.write(str(a) + ' ' + str(b) + '\n')
                f.write("|\n")
                a = disjList[i][1][0]
                b = disjList[i][1][1]
                f.write(str(a) + ' ' + str(b) + '\n')

    for i in range(len(neighborList)):
        a = neighborList[i][0]
        b = neighborList[i][1]
        f.write(str(a) + ' ' + str(b) + '\n')

    f.write("-1\n")

    if not args.noSuperSuper:
        f.write("SUPER_SUPER_GTSP_SETS: " + str(len(pList2)+1) +"\n")
        f.write("SUPER_SUPER_GTSP_SET_SECTION\n")
        for i in range(len(pList2)):
            f.write(str(i+1) + ' ')
            for z in pList2[i]: f.write(str(pName.index(z)+1) + ' ')
            f.write("-1\n")
        # add a set with just the depot
        f.write(str(len(pList2)+1) + ' ')
        f.write(str(pName.index('Depot')+1) + ' -1\n')

        f.write("SUPER_SUPER_ZONE_NEIGHBOR_SECTION\n")
        for tL in topList:
            if len(tL) >= 3:
                sName = natsorted(tL)
                for i in range(len(sName)-1):
                    f.write(str(pName2.index(sName[i])+1) + ' ' +
                            str(pName2.index(sName[i+1])+1) + '\n')
        f.write("-1\n")

def writeZoneNeighborSection(f, patternList, zoneList, clusterKeys, args):
    f.write("ZONE_NEIGHBOR_SECTION\n")
    for i in range(len(patternList)):
        if len(patternList[i]) >= 3:
            slist = natsorted(patternList[i])
            for j in range(len(slist)-1):
                 f.write(str(zoneList.index(slist[j])+1) + ' ' +
                         str(zoneList.index(slist[j+1])+1) + '\n')

    if args.zoneNeighborTrans:
        # create extra zone neighbors from super-zone transitions
        pName = []
        zName = []
        for z in zoneList:
            if z != 'Depot': q = uz.getClusterKey(z,clusterKeys["cluster"])
            else:            q = 'Depot'
            if q not in pName:
                pName.append(q)
                zName.append(z)

        pList = []
        pName2 = []
        for i in range(len(pName)):
            if zName[i] == 'Depot': continue
            p = pName[i]
            q = uz.getClusterKey(zName[i],clusterKeys["superCluster"])
            if q in pName2:
                k = pName2.index(q)
                pList[k].append(p)
            else:
                pName2.append(q)
                pList.append([p])

        # create extra neighbor constraints for zones with matching
        # transition item in neighboring super zones

        # check that the transition item is next to last the zone ID.
        # otherwise skip the contraints (the code can be extended to handle
        # any position of the transition item, taking care that item 1 may
        # have 2 digits in the zone ID)

        t = list({0,1,2,3} - set(clusterKeys["cluster"]))
        if t[0] == 2:
            disjList = []
            for i in range(len(pList)):
                pList[i] = natsorted(pList[i])
            for nList in pList:
                for kk in range(len(nList)-1):
                    m = natsorted(patternList[pName.index(nList[kk])])
                    n = natsorted(patternList[pName.index(nList[kk+1])])
                    tEdges = []
                    if m[0][-2] == n[0][-2]:
                        tEdges.append([m[0],n[0]])
                    if len(n) >= 2 and m[0][-2] == n[-1][-2]:
                        tEdges.append([m[0],n[-1]])
                    if len(m) >= 2:
                        if m[-1][-2] == n[0][-2]:
                            tEdges.append([m[-1],n[0]])
                        if len(n) >= 2 and m[-1][-2] == n[-1][-2]:
                            tEdges.append([m[-1],n[-1]])
                    if len(tEdges) == 1:
                        a = zoneList.index(tEdges[0][0])+1
                        b = zoneList.index(tEdges[0][1])+1
                        f.write(str(a) + ' ' + str(b) + '\n')
                    elif len(tEdges) == 2:
                        a = zoneList.index(tEdges[0][0])+1
                        b = zoneList.index(tEdges[0][1])+1
                        c = zoneList.index(tEdges[1][0])+1
                        d = zoneList.index(tEdges[1][1])+1
                        disjList.append([[a,b],[c,d]])

            for i in range(len(disjList)):
                a = disjList[i][0][0]
                b = disjList[i][0][1]
                f.write(str(a) + ' ' + str(b) + '\n')
                f.write("|\n")
                a = disjList[i][1][0]
                b = disjList[i][1][1]
                f.write(str(a) + ' ' + str(b) + '\n')

    f.write("-1\n")

def add_disjunctive_pred_constraints(f, zoneList, comp_levels, use_disjunctive_pred_constraints
):
    if use_disjunctive_pred_constraints == 0:
        return

    level2zone = dict()
    for zone_id, level in comp_levels.items():
        if zone_id in zoneList:
            level2zone.setdefault(level, [])
            level2zone[level].append(zone_id)

    for level in level2zone:
        if len(level2zone[level]) <= 4:
            for i in range(len(level2zone[level])):
                tail   = level2zone[level][i]
                tail_i = zoneList.index(tail)
                for j in range(i+1,len(level2zone[level])):
                    head = level2zone[level][j]
                    head_i = zoneList.index(head)
                    f.write(str(tail_i + 1)+ " " + str(head_i + 1) + "\n")
                    f.write("|\n")
                    f.write(str(head_i + 1)+ " " + str(tail_i + 1) + "\n")


def writeZonePredSectionNonRedundant(f, zoneList, zoneInfo, use_disjunctive_pred_constraints):
  f.write("ZONE_PRECEDENCE_SECTION\n")
  comp_levels = zoneInfo["full_components"]

  add_disjunctive_pred_constraints(f, zoneList, comp_levels,use_disjunctive_pred_constraints)

  for tail in zoneList:
      if tail in comp_levels:
          tail_i = zoneList.index(tail)
          tail_level = comp_levels[tail]
          min_head_level = len(zoneList) + 1
          for head in zoneList:
              if head in comp_levels:
                  head_level = comp_levels[head]
                  if head_level > tail_level and head_level < min_head_level:
                      min_head_level = head_level

          for head in zoneList:
              if head in comp_levels:
                  head_level = comp_levels[head]
                  if head_level == min_head_level:
                      head_i = zoneList.index(head)
                      f.write(str(tail_i + 1)+ " " + str(head_i + 1) + "\n")
  f.write("-1\n")

def writeZonePredSectionTransitiveClosure(f, zoneList, zoneInfo, use_disjunctive_pred_constraints):
  f.write("ZONE_PRECEDENCE_SECTION\n")
  comp_levels = zoneInfo["full_components"]

  add_disjunctive_pred_constraints(f, zoneList, comp_levels, use_disjunctive_pred_constraints)

  for tail in zoneList:
      if tail in comp_levels:
          tail_i = zoneList.index(tail)
          tail_level = comp_levels[tail]
          min_head_level = len(zoneList) + 1
          for head in zoneList:
              if head in comp_levels:
                  head_level = comp_levels[head]
                  if head_level > tail_level:
                      head_i = zoneList.index(head)
                      f.write(str(tail_i + 1)+ " " + str(head_i + 1)  +"\n")
  f.write("-1\n")


def writeZonePathSectionFromZoneInfo(f, zoneList, zoneInfo):

    f.write("ZONE_PATH_SECTION\n")

    comp_levels = zoneInfo["full_components"]
    level2zone = dict()
    for key, value in comp_levels.items():
        level2zone.setdefault(value, [])
        level2zone[value].append(key)

    level2zone[0] = ["Depot"]
    level2zone[len(level2zone)] = ["Depot"]
    print("    level2zone",    level2zone)
    print("zoneList",  zoneList)
    for i in range(1,len(level2zone)):
        print ("level2zone[i-1]",level2zone[i-1])
        if len(level2zone[i-1]) != 1 or len(level2zone[i]) != 1:
            continue
        tail = level2zone[i-1][0]
        head = level2zone[i][0]
        if tail not in zoneList or head not in zoneList :
            continue
        tail_i = zoneList.index(tail)
        head_i = zoneList.index(head)
        print("Adding path ", tail, head)
        f.write(str(tail_i + 1)+ " " + str(head_i + 1) + "\n")

    f.write("-1\n")

def writeZoneNeighborSectionFromZoneInfo(f, zoneList, zoneInfo):

    f.write("ZONE_NEIGHBOR_SECTION\n")

    comp_levels = zoneInfo["full_components"]
    level2zone = dict()
    for key, value in comp_levels.items():
        level2zone.setdefault(value, [])
        level2zone[value].append(key)

    for key in level2zone:
        if len(level2zone[key]) == 2:
            tail = level2zone[key][0]
            head = level2zone[key][1]
            if tail not in zoneList or head not in zoneList :
                continue
            tail_i = zoneList.index(tail)
            head_i = zoneList.index(head)
            #print("Adding neighbor ", tail, head)
            f.write(str(tail_i + 1)+ " " + str(head_i + 1) + "\n")

    f.write("-1\n")


def  writeClusterSuperSetSection(f, zoneComponents, zoneList):
    if len(zoneComponents) == 0:
        return

    f.write("SUPER_GTSP_SETS: " + str(len(zoneComponents)) +"\n")
    f.write("SUPER_GTSP_SET_SECTION\n")

    total_comps = 0

    for c_i in zoneComponents:
        f.write(str(total_comps+1) + ' ')
        for z in zoneComponents[c_i]:
            if z in zoneList:
                f.write(str(zoneList.index(z)+1) + ' ')
        f.write("-1\n")
        total_comps += 1

def write_Bijection(routeData, packageData, dir):
    cnt = 0
    tsp2amz = dict()
    amz2tsp = dict()
    for route in routeData:
        stop_tsp2amz = dict()
        stop_amz2tsp = dict()
        pname =  createProbName(cnt)

        stop_cnt = 1
        for stop in packageData[route]:
            stop_tsp2amz[stop_cnt] = stop
            stop_amz2tsp[stop] = stop_cnt
            stop_cnt += 1
        tsp2amz[pname] = dict()
        tsp2amz[pname]["route_id"] = route
        tsp2amz[pname]["stops"] = stop_tsp2amz

        amz2tsp[route] = dict()
        amz2tsp[route]["route_id"] = pname
        amz2tsp[route]["stops"] = stop_amz2tsp

        cnt  += 1

    filename = dir + "/" + "tsp2amz.json"
    with open(filename, 'w') as f:
        json.dump(tsp2amz,f)


def extractSuperPredEdges(sorder, superList):
    predEdges = []
    for i in range(len(sorder)-1):
        a = sorder[i]
        b = sorder[i+1]
        if a in superList and b in superList:
            if sorder.count(a) == 1 and sorder.count(b) == 1:
                predEdges.append([superList.index(a),
                                  superList.index(b)])
    return predEdges

def buildRouteTSP(id, routeData, timeData, packageData, probNumber,
                  stationList, stationStops, stationInfos, superSets,
                  clusterKeys, args):
# write a TSPLIB for the route; id == Amazon routeID
    ncount = len(timeData)
    pname = createProbName(probNumber)

    f = open(args.TSPLIBdir+"/"+pname+".ctsptw","w")
    routeScore = "High"
    if 'route_score' in routeData:
        routeScore = routeData['route_score']
    writeHeader(f, pname, id, routeScore, ncount)
    writeEdgeWeightSection(f, timeData, ncount)

    if args.Windows: writeTimeWindowSection(f, routeData, packageData)

    writeDepotSection(f, routeData)

    if not args.noZone:
        zoneList, zoneIndex = uz.getRouteZones(routeData,
                   stationList.index(routeData['station_code']), stationStops)
        writeSetSection(f, zoneList, zoneIndex)

        if not args.noCluster:
            clusterList = uz.getClusterKeyList(zoneList, clusterKeys["cluster"])
            writeSuperSetSection(f, clusterList, zoneList)
            writeSuperNeighborSection(f, zoneList, clusterKeys, args)

            if not args.noClusterNeighbors:
                writeZoneNeighborSection(f,clusterList,zoneList,clusterKeys,
                                         args)

        if not args.noZonePred and routeData["station_code"] in stationInfos:
            best_zone_info = zid.find_pred_clusters(id,routeData, zoneList,stationInfos[routeData["station_code"]])
            if best_zone_info:
                if args.trans:
                    writeZonePredSectionTransitiveClosure(f, zoneList,
                                                          best_zone_info,
                                                          args.disjunctiveZonePred)
                else:
                    writeZonePredSectionNonRedundant(f, zoneList,
                                                     best_zone_info,
                                                     args.disjunctiveZonePred)
                if args.noClusterNeighbors:
                    writeZoneNeighborSectionFromZoneInfo(f, zoneList, best_zone_info)
                if args.paths:
                    writeZonePathSectionFromZoneInfo(f, zoneList, best_zone_info)
                # clusters = zid.extract_proper_clusters(best_zone_info['full_components'], zoneList)
                # writeClusterSuperSetSection(f,clusters, zoneList)

        if args.superPred or not args.noSuperPath:
            # find best match of super clusters for pred constraints
            predEdges = []
            superList = []
            for z in zoneList:
                if z != 'Depot': q = uz.getClusterKey(z,clusterKeys["cluster"])
                else:            q = 'Depot'
                if q not in superList: superList.append(q)

            max_id = None
            jmax = None
            card = -1
            s = set(superList)
            station_code = routeData['station_code']

            if station_code in superSets:
                for ss_id in superSets[station_code]:
                    if ss_id == id:
                        continue
                    k = len(s.intersection(set(superSets[station_code][ss_id]["superList"])))
                    if k >= card:
                        sorder = superSets[station_code][ss_id]["superOrder"]
                        # compute precedence constraints appearing in the reference superOrder
                        # (only for super clusters that appear exactly once in the route)
                        tmpPredEdges = extractSuperPredEdges(sorder, superList)

                        if ( (k > card)  or ((k == card) and (len(tmpPredEdges) > len(predEdges)))):
                            card = k
                            max_id = ss_id
                            predEdges = tmpPredEdges

                if args.superPred:
                    f.write("SUPER_ZONE_PRECEDENCE_SECTION\n")
                else:
                    f.write("SUPER_ZONE_PATH_SECTION\n")
                for i in range(len(predEdges)):
                    f.write(str(predEdges[i][0] + 1)+ " " +
                            str(predEdges[i][1] + 1) + "\n")
                f.write("-1\n")

    f.write("EOF" + '\n')
    f.close()

def run_build(args):
    random.seed(args.seed)
    if not os.path.exists(args.TSPLIBdir): os.makedirs(args.TSPLIBdir)


    print("Start json reading ... ")
    start = timeit.default_timer()
    with open(args.t_file) as f: T = json.load(f)
    end = timeit.default_timer()
    print("Done travel times.",args.t_file, end-start)

    start = timeit.default_timer()
    with open(args.r_file) as f: R = json.load(f, object_pairs_hook=OrderedDict)
    end = timeit.default_timer()
    print("Done route data.", args.r_file, end-start)
    start = timeit.default_timer()
    with open(args.p_file) as f: P = json.load(f)
    end = timeit.default_timer()
    print("Done package data.", args.p_file, end-start)

    if args.s_file:
        start = timeit.default_timer()
        with open(args.s_file) as f: S = json.load(f)
        end = timeit.default_timer()
        print("Done actual sequences.", args.s_file, end-start)
    if args.z_file:
        start = timeit.default_timer()
        with open(args.z_file) as f: Z = json.load(f)
        end = timeit.default_timer()
        print("Done zone_precedences.", args.s_file, end-start)

    BR = None
    if args.br_file:
        start = timeit.default_timer()
        with open(args.br_file) as f: BR = json.load(f)
        end = timeit.default_timer()
        print("Done build phase route data.", args.br_file, end-start)

    print("Done json reading ... ")

    if args.sample > 0:   print("Write", args.sample, "sample to",
                                 args.TSPLIBdir)
    elif not args.noPrune == 1: print("Write High+Good instances to",
                                       args.TSPLIBdir)
    else:                 print("Write all instances to", args.TSPLIBdir)


    start = timeit.default_timer()
    uz.addMissingData(R,"ATLANTIS\_R") # add missing station code, lat/lng, ... to prevent crashes
    end = timeit.default_timer()
    print("addMissingData took", end-start, "seconds")
    start = timeit.default_timer()
    uz.addMissingZones(R,BR) # add zone IDs to stops (if missing)
    end = timeit.default_timer()
    print("addMissingZones took", end-start, "seconds")
    stationList = uz.getStationList(R)     # build sorted list of stations
    if not args.noPruneFailed:
        failedList  = uz.markFailedRoutes(R,P) # mark failed delivery
    else:
        failedList = []

    if args.sample > 0:
        r = getRouteList(R, args.noPrune, failedList) # route IDs for sampling
        nsamples = min(len(r), args.sample)
        sampleRoutes = random.sample(r, nsamples)
        nbadsamples = 0
        minsamples = min(args.sample,int(len(R)/4))
        print ("MINSAMPLES", minsamples)
        if nsamples < minsamples:
            r = getRouteList(R, 1, failedList) # All route IDs for sampling
            bad_r = list(set(r) - set(sampleRoutes)) # subtract already sampled ids
            bad_r.sort() # Sorting is required for reproducible samples
            nbadsamples = minsamples - nsamples
            nbadsamples = min(len(bad_r), nbadsamples)
            badsampleRoutes = random.sample(bad_r, nbadsamples)
            sampleRoutes = sampleRoutes + badsampleRoutes
        print("Using", len(sampleRoutes), ", good", nsamples, "bad", nbadsamples, "R", len(R))
    else:
        sampleRoutes = []

    stationStops = []
    if not args.noZone:  # build list of zone_id locations for each station
        stationStops = uz.getStationStopsLatLng(R,stationList)

    superSets = dict()
    station_infos = dict()
    clusterKeys = dict()
    if not args.noZonePred:  # build historic zone adj information
        print( "Extracting zone ordering ... ")
        if args.z_file:
            station_infos = Z["station_infos"]
            clusterKeys = Z["clusterKeys"]
            superSets = Z["superSets"]
        else:
            station_infos = zid.station_route_zones(R, S)
            clusterKeys = uz.createClusterKeys(R,S)
            if (not args.noZone) and (args.superPred or not args.noSuperPath):
                superSets = uz.getSuperSets(R,S,stationList,stationStops,
                                            clusterKeys["cluster"])


        print("clusterKeys", clusterKeys)
        print( "Done zone ordering ... ")





    write_Bijection(R,P, args.TSPLIBdir)

    cnt = 0;
    for id in R:
        if ((len(sampleRoutes) > 0 and (id not in sampleRoutes)) or
            (len(sampleRoutes) == 0 and
            ((not args.noPrune) and (R[id]['route_score'] != 'High') or
             ((not args.noPruneFailed) and failedList[cnt] == 1)))):
            cnt = cnt+1
            continue

        buildRouteTSP(id, R[id], T[id], P[id], cnt, stationList,
                      stationStops, station_infos, superSets, clusterKeys, args)
        cnt += 1

def _main():
    args = get_args()

    if args.noZone: args.noZonePred = 1  # can't have zones pred if no zones
    run_build(args)

if __name__ == "__main__":  # don't run if this script is imported as module
    _main()
