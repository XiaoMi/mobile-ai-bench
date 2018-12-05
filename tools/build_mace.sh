#!/usr/bin/env bash

set -e -u -o pipefail

trap "exit" INT

# get mace-models
rm -rf mace-models
GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" git clone git@github.com:XiaoMi/mace-models.git
cd mace-models/
MODEL_ROOT_PATH=`pwd`
CONF_FILES=`find $MODEL_ROOT_PATH -name *.yml | { grep -v ".gitlab-ci.yml" || true; }`

# get mace
cd ..
rm -rf mace
GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" git clone git@github.com:XiaoMi/mace.git
cd mace/

MODELS=(
    "inception-v3"
    "mobilenet-v1"
    "mobilenet-v1-quantize-retrain"
    "mobilenet-v2"
    "mobilenet-v2-quantize-retrain"
    "squeezenet-v11"
)

# convert models to pb and data
TIMESTAMP=$(date +%s)
for CONF_FILE in $CONF_FILES; do
    for MODEL in "${MODELS[@]}"; do
        if [ "$(basename $CONF_FILE .yml)" == "$MODEL" ]; then
            set +e
            python tools/converter.py convert --config=$CONF_FILE
            RESULT=$?
            set -e
            if [ $RESULT == 0 ]; then
                FILE_NAME=$(basename $CONF_FILE .yml)
                cp builds/${FILE_NAME}/model/*.pb ../output/${FILE_NAME}_${TIMESTAMP}.pb
                cp builds/${FILE_NAME}/model/*.data ../output/${FILE_NAME}_${TIMESTAMP}.data
            fi
        fi
    done
done
