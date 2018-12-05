#!/usr/bin/env bash

set -e -u -o pipefail

bazel build //aibench/python:benchmark

bazel-bin/aibench/python/benchmark "$@"
