#!/usr/bin/env bash

wget https://cnbj1.fds.api.xiaomi.com/nnbench/third_party/tensorflow-1.9.0-rc1.zip
unzip -o tensorflow-1.9.0-rc1.zip

# copy headers and library to nnbench
mv -f flatbuffers third_party/tflite/
mv -f tensorflow third_party/tflite/

rm tensorflow-1.9.0-rc1.zip
