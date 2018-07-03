#!/usr/bin/env bash

wget https://cnbj1.fds.api.xiaomi.com/nnbench/third_party/tensorflow-1.9.0-rc1.zip

unzip -o tensorflow-1.9.0-rc1.zip -d third_party/tflite/

rm tensorflow-1.9.0-rc1.zip
