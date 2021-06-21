This is a collection of python scripts
developed by William Cook, Stephan Held, and Keld Helsgaun
during the competition in the AMAZON last mile routing challenge.

The main scripts are
 * analyze_zone_id_order_main.py  for analyzing zone information in the build phase that are the main ingredients to constrain the TSP instances in the apply phase
 * build_TSPLIB.py Translates the TSP instances from the python format into TSPLIB format and adds special constraints using the model computed in the build phase
 * analyze_zone_id_order.py subroutines for extracting and applying zone precedence information
 * util_zone.py subroutines for creating clusters and super clusters


In addition we used several scripts to visualize tours and get familiar with the instances:
 * analyze_repeated_deliveries.py -pa_file PA_FILE
    reads a package json file and counts repeated packages
 * analyze_zone_ids.py --rt_file ROUTE_FILE --se_file SEQUENCE_FILE
    reads a route json and a sequence json and counts the number of zone changes
    as well as the number of zones in a tour,
 * plot_tours_per_depot.py --rt_file ROUTE_FILE --se_file SEQUENCE_FILE
    reads a route json and a sequence json and plots all tours for each station/depot
    in a geojson file. 
 * plot_tours_from_tsplib.py --solfiles SOLFILES [SOLFILES ...]   --geomfile GEOMFILE --outfile OUTFILE
    Plots a list of tours given as SOLFILES in TSPLIB format using the geometric information from the TSP GEO instance GEOMFILE  in the geojson file OUTFILE.
