#!/usr/bin/env bash

set -e -o pipefail

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

for CONF_FILE in $CONF_FILES; do
    if [ "$(basename $CONF_FILE .yml)" != "mobilenet-v2-host" ]; then
        python tools/converter.py build --config=$CONF_FILE --build_type=proto --target_abi=$1
        cp build/$(basename $CONF_FILE .yml)/model/*.pb $2
        cp build/$(basename $CONF_FILE .yml)/model/*.data $2
    fi
done

# copy headers and library to nnbench
cp -r build/$(basename $CONF_FILE .yml)/include ../third_party/mace/
cp -r build/$(basename $CONF_FILE .yml)/lib ../third_party/mace/