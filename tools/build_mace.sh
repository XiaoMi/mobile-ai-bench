#!/usr/bin/env bash

set -e -u -o pipefail

trap "exit" INT

# get all model yamls
rm -rf mace-models
GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" git clone git@v9.git.n.xiaomi.com:deep-computing/mace-models.git
cd mace-models/
MODEL_ROOT_PATH=`pwd`
CONF_FILES=`find $MODEL_ROOT_PATH -name *.yml | { grep -v ".gitlab-ci.yml" || true; }`

# build all model yamls
cd ..
rm -rf mace
GIT_SSH_COMMAND="ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no" git clone git@v9.git.n.xiaomi.com:deep-computing/mace.git
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

for CONF_FILE in $CONF_FILES; do
    for MODEL in "${MODELS[@]}"; do
        if [ "$(basename $CONF_FILE .yml)" == "$MODEL" ]; then
            set +e
            python tools/converter.py build --config=$CONF_FILE --build_type=proto --target_abi=$1
            RESULT=$?
            set -e
            if [ $RESULT == 0 ]; then
                cp build/$(basename $CONF_FILE .yml)/model/*.pb $2
                cp build/$(basename $CONF_FILE .yml)/model/*.data $2
                if [ "$(basename $CONF_FILE .yml)" == "inception-v3-dsp" ]; then
                    COPIED_FILE=$CONF_FILE
                fi
            fi
        fi
    done
done

# copy headers and library to nnbench
cp -r build/$(basename $COPIED_FILE .yml)/include ../third_party/mace/
cp -r build/$(basename $COPIED_FILE .yml)/lib ../third_party/mace/
