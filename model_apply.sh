#!/bin/sh
date

pwd
readonly HOME_DIR=`pwd`
readonly BASE_DIR="${HOME_DIR}/src"
readonly DATA_DIR="${HOME_DIR}/data/"
readonly OUTPUTS_FILE="${HOME_DIR}/data/model_apply_outputs/proposed_sequences.json"

rm -rf ${DATA_DIR}/model_apply_outputs/*

${BASE_DIR}/scripts/manage_running_time.py -start_time  --time_file  data/model_apply_outputs/runtime.json  --r_file data/model_apply_inputs/new_route_data.json

# Remove any old solution if it exists
rm -f ${OUTPUTS_FILE} 2> /dev/null
rm -rf $DATA_DIR/model_apply_outputs/TOURS-TSPLIB*
rm -rf $DATA_DIR/model_apply_outputs/TSPLIB*
rm -rf $DATA_DIR/model_apply_outputs/TMP

date


echo "Creating TSPLIB1 instances to $DATA_DIR/model_apply_outputs/TSPLIB_1"
cmd="${BASE_DIR}/scripts/build_TSPLIB.py --r_file $DATA_DIR/model_apply_inputs/new_route_data.json --t_file  $DATA_DIR/model_apply_inputs/new_travel_times.json --p_file $DATA_DIR/model_apply_inputs/new_package_data.json --z_file $DATA_DIR/model_build_outputs/model.json --br_file $DATA_DIR/model_build_outputs/route_data.json -noPrune  -noPruneFailed -zoneNeighborTrans -superNeighborTrans  $DATA_DIR/model_apply_outputs/TSPLIB_1"
echo $cmd
$cmd &


echo "Creating TSPLIB2 instances to $DATA_DIR/model_apply_outputs/TSPLIB_2"
cmd="${BASE_DIR}/scripts/build_TSPLIB.py --r_file $DATA_DIR/model_apply_inputs/new_route_data.json --t_file  $DATA_DIR/model_apply_inputs/new_travel_times.json --p_file $DATA_DIR/model_apply_inputs/new_package_data.json --z_file $DATA_DIR/model_build_outputs/model.json --br_file $DATA_DIR/model_build_outputs/route_data.json  -noPrune -noPruneFailed -zoneNeighborTrans -superNeighborTrans  -superPred -trans   $DATA_DIR/model_apply_outputs/TSPLIB_2"
echo $cmd
$cmd

wait

date
TIME_LIMIT_1=`${BASE_DIR}/scripts/manage_running_time.py --time_file  $DATA_DIR/model_apply_outputs/runtime.json --r_file $DATA_DIR/model_apply_inputs/new_route_data.json | gawk '{print $1;}'`
TIME_LIMIT_2=`${BASE_DIR}/scripts/manage_running_time.py --time_file  $DATA_DIR/model_apply_outputs/runtime.json --r_file $DATA_DIR/model_apply_inputs/new_route_data.json  | gawk '{print $2;}'`
# Be robust to fails in the above  functions, 27/14 is impossible by the automatic setting
if [ -z "$TIME_LIMIT_1" ]; then    TIME_LIMIT_1 = 27 ; fi
if [ -z "$TIME_LIMIT_2" ]; then    TIME_LIMIT_2 = 14 ; fi

echo "Running with time limits $TIME_LIMIT_1 and $TIME_LIMIT_2"
date

cp  ${DATA_DIR}/model_build_outputs/bin/* $DATA_DIR/model_apply_outputs/
cd $DATA_DIR/model_apply_outputs/

echo "Solving TSPLIB_1 instances"
 cmd="./solve TSPLIB_1 $TIME_LIMIT_1"
echo $cmd
$cmd
cd -

date

mkdir $DATA_DIR/model_apply_outputs/TOURS-TSPLIB
cd  $DATA_DIR/model_apply_outputs/TOURS-TSPLIB
pwd
ln -fs $DATA_DIR/model_apply_outputs/TOURS-TSPLIB_1/* .
cd -
date

cd $DATA_DIR/model_apply_outputs/
echo "Solving TSPLIB_2 instances"
 cmd="./solve TSPLIB_2 $TIME_LIMIT_2"
echo $cmd
$cmd
echo "Directory: `pwd`"
date

cmd="./merge TSPLIB_1 TSPLIB_2 TSPLIB"
echo $cmd
$cmd

date

echo "Converting TSPLIB instances to  ${OUTPUTS_FILE}"
cmd="${BASE_DIR}/scripts/tsplib2json.py --tour_dir $DATA_DIR/model_apply_outputs/TOURS-TSPLIB --tsplib2amz_file $DATA_DIR/model_apply_outputs/TSPLIB_1/tsp2amz.json --out_json ${OUTPUTS_FILE}"
echo $cmd
$cmd

date

echo "Done!"
