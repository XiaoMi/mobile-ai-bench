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
    "inception-v3-dsp"
    "mobilenet-v1"
    "mobilenet-v2"
    "squeezenet-v11"
    "vgg16-caffe-gpu"
    "vgg16-tensorflow-cpu"
)

# convert models to pb and data
for CONF_FILE in $CONF_FILES; do
    for MODEL in "${MODELS[@]}"; do
        if [ "$(basename $CONF_FILE .yml)" == "$MODEL" ]; then
            set +e
            python tools/converter.py convert --config=$CONF_FILE
            RESULT=$?
            set -e
            if [ $RESULT == 0 ]; then
                cp builds/$(basename $CONF_FILE .yml)/model/*.pb $2
                cp builds/$(basename $CONF_FILE .yml)/model/*.data $2
            fi
        fi
    done
done

# build and copy includes and libs to aibench
bash tools/build-standalone-lib.sh
cp -r builds/include ../third_party/mace/
cp -r builds/lib ../third_party/mace/
