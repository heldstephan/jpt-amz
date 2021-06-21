"""
   Just Passing Through: Zone utility functions

   Grab zone and cluster information from dict structures obtained by the
   the Amazon json files.

   Common input parameters have the following meaning:
       routes == dict from route_data.json
       packages == dict from package_data.json
       sequences == dict from actual_sequences.json
"""

import os, json, sys, math
from collections import OrderedDict

def markFailedRoutes(routes, packages):
# Return array marking routes with failed deliveries.
    rfail = []
    for id in routes:
        attempt = 0
        pack = packages[id]
        for s in pack:
            stop = pack[s]
            for package in stop:
                if "scan_status" in stop[package] and stop[package]['scan_status'] == 'DELIVERY_ATTEMPTED':
                    attempt = 1
                    break
            if attempt == 1: break
        rfail.append(attempt)
    return rfail


def getStationList(routes, sList = []):
# Return a sorted list of the stations for the set of routes.
    for id in routes:
        route = routes[id]
        code = route['station_code']
        if code not in sList:
            sList.append(code)
    sList.sort()
    return sList

def getStationZones(routes, stations):
# Create list of zone IDs for each station.
    szone = []
    for i in range(len(stations)): szone.append([])
    for id in routes:
        route = routes[id]
        code = route['station_code']
        c = stations.index(code)
        for s in route['stops']:
            stop = route['stops'][s]
            z = stop['zone_id']
            if not z or z != z: continue
            if z not in szone[c]:
                szone[c].append(z)
    return szone

def getStationStopsLatLng(routes, stations, sstop = []):
# Create list of lat-lng for the stops associated with each station.
    for i in range(len(sstop),len(stations)): sstop.append([])
    for id in routes:
        route = routes[id]
        code = route['station_code']

        k = stations.index(code)
        for stop in route['stops']:
            z = route['stops'][stop].get('zone_id')
            if not z or z != z:
                continue
            else:
                sstop[k].append([route['stops'][stop]['lat'],
                                 route['stops'][stop]['lng'], z])
    return sstop

def printStationStopZoneStat(stationStops, stationList):
    for station_index in range(len(stationStops)):
        print ("Station", station_index , "has" , len(stationStops[station_index]), "stops with zone id")

def addMissingLatLng(route):
    station_found = 0
    for stop in route['stops']:
        if "lat" not in route['stops'][stop]:
            route['stops'][stop]['lat'] = 0.0
        if "lng" not in route['stops'][stop]:
            route['stops'][stop]['lng'] = 0.0
        if route['stops'][stop].get('type') == "Station":
            station_found = 1
    return station_found
            
def addMissingData(R, prefix):
    cnt = 1
    delete_routes = dict()
    for id in R:
        route = R[id]
        if 'station_code' not in route:
            route['station_code'] = prefix+str(cnt)
            cnt += 1
        station_found = addMissingLatLng(route)                        
        if station_found == 0:
            delete_routes[id] = 1
            
    for id in delete_routes:
        print("ERROR found route without Station:", id)
        del R[id]
        
def addMissingZones(R, BR = None):
# Add missing zones to the route data. 
# If a stop is missing a zone ID, it is
# given the ID of the nearest stop that has an ID.
# Input: routes is a R of route data (typically from route_data.json).
#        BR is an optional route data to provide additional nearest zones.
    stationList = getStationList(R)
    if BR:
        stationList  = getStationList(BR, stationList)

    stationStops = getStationStopsLatLng(R,stationList)
    if BR:
        stationStops = getStationStopsLatLng(BR,stationList, stationStops)

#    printStationStopZoneStat(stationStops, stationList)
 
    for id in R:
        route = R[id]
        istation = stationList.index(route['station_code'])
        for stop in route['stops']:
            z = route['stops'][stop].get('zone_id')
            if not z or z != z:
                if route['stops'][stop]['type'] == "Station": continue
                dmin = 100000.0
                zmin = ''
                x = route['stops'][stop]['lat']
                y = route['stops'][stop]['lng']
                for i in range(len(stationStops[istation])):
                    xi = stationStops[istation][i][0]
                    yi = stationStops[istation][i][1]
                    if abs(x-xi) > dmin or abs(y-yi) > dmin: continue
                    dist = math.sqrt((x-xi)*(x-xi) + (y-yi)*(y-yi))
                    if dist < dmin:
                        dmin = dist
                        zmin = stationStops[istation][i][2]
                route['stops'][stop]['zone_id'] = zmin;

def getRouteZones(route, istation, sstops):
# Return list of zones in a route and the index of each stop's index into the
# list. Unlabeled nodes are assigned to the nearest zone for their station.
# Input: route ID, index of station, list from getStationStopsLatLng
    zone = []
    zd = []
    zcnt = 0
    for stop in route['stops']:
        z = route['stops'][stop]['zone_id']
        if not z or z != z:
            if route['stops'][stop]['type'] == "Station":
                q = str(zcnt)
                zcnt = zcnt+1
                zone.append('Depot')
                k = zone.index('Depot')
            else:
                dmin = 100000.0
                zmin = ''
                x = route['stops'][stop]['lat']
                y = route['stops'][stop]['lng']
                for i in range(len(sstops[istation])):
                    xi = sstops[istation][i][0]
                    yi = sstops[istation][i][1]
                    if abs(x-xi) > dmin or abs(y-yi) > dmin: continue
                    dist = math.sqrt((x-xi)*(x-xi) + (y-yi)*(y-yi))
                    if dist < dmin:
                        dmin = dist
                        zmin = sstops[istation][i][2]
                if zmin in zone:
                    k = zone.index(zmin)
                else:
                    zone.append(zmin)
                    k = zone.index(zmin)
                route['stops'][stop]['zone_id'] = zmin;
        elif z in zone:
            k = zone.index(z)
        else:
            zone.append(z)
            k = zone.index(z)
        zd.append(k)
    return zone, zd

def getRoutePermutation(seq):
# Return the permuation given by the route sequence
    perm = []
    for i in range(len(seq)): perm.append(-1)
    s = 0
    for stop in seq:
        perm[int(seq[stop])] = s
        s = s+1
    return perm

def getRouteZoneOrder(zone, zd, seq):
# Return the order of zones for a route sequence (there may be repeats).
# Input: the route zone list and index list from getRouteZones; sequence
    perm = getRoutePermutation(seq)
    last = -1
    zlist = []
    for s in perm:
        if zd[s] == last: continue
        last = zd[s]
        zlist.append(zone[zd[s]])
    return zlist

def getClusterKey(z,clusterKey):
# Return cluster key for zone ID z
    if len(z) >= 6:
        p = ''
        for i in range(len(clusterKey)):
            k = clusterKey[i]
            if k == 0:
                p += z[0]
            elif k == 1:
                p += z[2]
                if z[3] != ".": p += z[3]
            elif k == 2:
                if z[3] != ".":
                    p += z[5]
                else: p += z[4]
            else:
                p += z[-1] 
        return p
    else:
        return z

def getClusterKeyList(zoneList,clusterKey):
# Return a list of the zones grouped by their clusterKey
    pList = []
    pName = []
    for z in zoneList:
        if z != 'Depot': q = getClusterKey(z,clusterKey)
        else:            q = 'Depot'
        if q in pName:
            k = pName.index(q)
            pList[k].append(z)
        else:
            pName.append(q)
            pList.append([z])
    return pList

def getClusterOrder(zoneOrder,clusterKey):
# Return the order of clusters in a route sequence (with repeats, no Depot)
# Input: the order of zones in the sequence (from getRouteZoneOrder)
    clusterOrder = []
    last = -1
    for z in zoneOrder:
        if z == 'Depot': continue
        p = getClusterKey(z,clusterKey)
        if p == last: continue
        last = p
        clusterOrder.append(p)
    return clusterOrder

def getSuperSets(routes, sequences, stationList, stationStops, clusterKey):
# Return a list of super clusters information, used to find a best match
# reference route when creating super cluster precendence constraints
    superSets = dict()
    for id in routes:
        routeData = routes[id]
        if routeData['route_score'] == 'Low': continue
        zoneList, zoneIndex = getRouteZones(routeData,
                stationList.index(routeData['station_code']), stationStops)
        superList = []
        for z in zoneList:
            if z == 'Depot': continue
            p = getClusterKey(z,clusterKey)
            if p not in superList:
                superList.append(p)

        zoneOrder = getRouteZoneOrder(zoneList,zoneIndex,
                sequences[id]['actual'])
        superOrder = getClusterOrder(zoneOrder,clusterKey)
        station_code = routeData['station_code']
        if station_code not in superSets:
            superSets[station_code] = dict()
            
        superSets[station_code][id] = dict()
        superSets[station_code][id]["superList"]  = superList
        superSets[station_code][id]["superOrder"] = superOrder

    return superSets

def createClusterKeys(routes,sequences):
# Return a dictionary giving the best choice of selection keys for creating
# clusters, super clusters, and top-level clusters of zones. Clusters are
# created from 3 of the 4 items encoded in a zone ID. Super clusters are
# created from 2 of the 3 cluster items. Top-level clusters are created from 1
# of the 2 super-cluster items. The choices are made to minimize the number of
# clusters, super clusters, and top-level clusters in the historial data.

    # build a dictionary of the zone order for each route
    allZoneOrders = dict()
    for id in routes:
        zoneList = []
        stops = routes[id]["stops"]
        for s in stops:
            if stops[s]['type'] == 'Station':
                zoneList.append('Depot')
            else:
                z = stops[s]['zone_id']
                if not z or z != z:
                    zoneList.append('NoZoneID')
                else:
                    zoneList.append(z)

        zoneRoute = []
        seq = sequences[id]['actual']
        for i in range(len(seq)): zoneRoute.append('')
        k = 0
        for s in seq:
            zoneRoute[int(seq[s])] = zoneList[k]
            k = k+1

        zoneOrder = []
        last = -1
        for z in zoneRoute:
            if z == last: continue
            last = z
            zoneOrder.append(z)
        allZoneOrders[id] = zoneOrder

    # clusterKey will be a selection of 3 of the 4 items in zone IDs

    cKeys = [[0,1,2], [0,1,3], [0,2,3], [1,2,3]]
    ccount = [0,0,0,0]
    
    # For each zone order, count the number of clusters for each cKey.
    # Select the cKey having the greatest number of instances where it gives
    # the min nmber (recorded in ccount).

    for id in allZoneOrders:
        zoneOrder = allZoneOrders[id]
        cmin = 1000
        imin = 0
        for i in range(4):
            corder = getClusterOrder(zoneOrder,cKeys[i])
            if len(corder) < cmin:
                cmin = len(corder)
                imin = i
        ccount[imin] += 1
    max_c = max(ccount)
    imax_c = ccount.index(max_c)
    clusterKey = cKeys[imax_c]

    # superKey will be a subset of 2 items from the 3 in clusterKey

    sKeys = [[clusterKey[0],clusterKey[1]],
             [clusterKey[0],clusterKey[2]],
             [clusterKey[1],clusterKey[2]]]
    scount = [0,0,0]

    # For each zone order, count the number of super-clusters for each sKey.
    # Select the sKey having the greatest number of instances where it gives
    # the min number (recorded in scount).

    for id in allZoneOrders:
        zoneOrder = allZoneOrders[id]
        smin = 1000
        imin = 0
        for i in range(3):
            sorder = getClusterOrder(zoneOrder,sKeys[i])
            if len(sorder) < smin:
                smin = len(sorder)
                imin = i
        scount[imin] += 1
    max_s = max(scount)
    imax_s = scount.index(max_s)
    superKey = sKeys[imax_s]

    # topKey will be a 1 of the 2 items in superKey

    tKeys = [[superKey[0]],
             [superKey[1]]]
    tcount = [0,0]

    # For each zone order, count the number of top-level clusters for each tKey.
    # Select the tKey having the greatest number of instances where it gives
    # the min number (recorded in tcount).

    k = 0
    for id in allZoneOrders:
        zoneOrder = allZoneOrders[id]
        tmin = 1000
        imin = 0
        for i in range(2):
            torder = getClusterOrder(zoneOrder,tKeys[i])
            if len(torder) < tmin:
                tmin = len(torder)
                imin = i
        if tmin > 1: k += 1
        tcount[imin] += 1
    max_t = max(tcount)
    imax_t = tcount.index(max_t)
    topKey = tKeys[imax_t]

    print("Number of multiple top levels:", k)

    print(ccount, clusterKey)
    print(scount, superKey)
    print(tcount, topKey)
    clusterKeys = dict()
    clusterKeys["cluster"] = clusterKey
    clusterKeys["superCluster"] = superKey
    clusterKeys["topCluster"] = topKey
    print(list(set(clusterKey) - set(superKey)))
    return clusterKeys
