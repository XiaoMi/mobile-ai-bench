# Copyright 2018 Xiaomi, Inc.  All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


#################################
# YAMLKeyword
#################################
class YAMLKeyword:
    device_name = 'device_name'
    target_abis = 'target_abis'
    target_socs = 'target_socs'
    models = 'models'
    system = 'system'
    address = 'address'
    username = 'username'
    password = 'password'


#################################
# System type
#################################
class SystemType:
    android = 'android'
    arm_linux = 'arm_linux'
    host = 'host'


################################
# ABI Type
################################
class ABIType(object):
    armeabi_v7a = 'armeabi-v7a'
    arm64_v8a = 'arm64-v8a'
    arm64 = 'arm64'
    aarch64 = 'aarch64'
    armhf = 'armhf'
    host = 'host'


abi_types = [
    "armeabi-v7a",
    "arm64-v8a",
    'arm64',
    'armhf',
    "host",
]


#################################
# Tool chain Type
#################################
class ToolchainType:
    android = 'android'
    arm_linux_gnueabihf = 'arm_linux_gnueabihf'
    aarch64_linux_gnu = 'aarch64_linux_gnu'
    host = ''
