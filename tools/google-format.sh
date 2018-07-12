#!/bin/bash

clang-format-3.9 \
  -style="{BasedOnStyle: google,    \
           DerivePointerAlignment: false, \
           PointerAlignment: Right, \
           BinPackParameters: false}"  -i $1
