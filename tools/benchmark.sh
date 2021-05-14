#!/usr/bin/env bash

set -e -u -o pipefail
set -x

bazel build //aibench/python:benchmark

bazel-bin/aibench/python/benchmark "$@"
