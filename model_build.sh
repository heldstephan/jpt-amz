#!/bin/sh
date
readonly BASE_DIR=$(dirname $0)
readonly HOME_DIR="$(dirname ${BASE_DIR})"
readonly DATA_DIR="${HOME_DIR}/data/"
readonly OUT_FILE="${DATA_DIR}/model_build_outputs/model.json"

rm -rf ${DATA_DIR}/model_build_outputs/*

echo "Compiling C sources."
cd $BASE_DIR/LKH-AMZ
make clean
make
cd -

#Copy compiled binaries to model_build_outputs/bin
mkdir ${DATA_DIR}/model_build_outputs/bin
cp  ${BASE_DIR}/LKH-AMZ/get_Cost ${BASE_DIR}/LKH-AMZ/score  ${BASE_DIR}/LKH-AMZ/merge ${BASE_DIR}/LKH-AMZ/solve ${BASE_DIR}/LKH-AMZ/LKH ${DATA_DIR}/model_build_outputs/bin


date
echo "Copying route_data for zone fixup."
cp ${DATA_DIR}/model_build_inputs/route_data.json ${DATA_DIR}/model_build_outputs/

date

echo "Building model ..."
$BASE_DIR/scripts/analyze_zone_id_order_main.py --r_file ${DATA_DIR}/model_build_inputs/route_data.json  --s_file $DATA_DIR/model_build_inputs/actual_sequences.json --out_file $OUT_FILE

date
