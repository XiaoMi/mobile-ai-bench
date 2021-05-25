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


from aibench.proto import base_pb2
from aibench.python.utils import common
from aibench.python.utils import sh_commands


class YAMLKeyword(object):
    username = 'username'
    target_abis = 'target_abis'
    target_socs = 'target_socs'
    device_types = 'device_types'
    address = 'address'
    models = 'models'
    device_name = 'device_name'


class Device:

    def __init__(self, device_dict):
        self.device_name = device_dict[YAMLKeyword.device_name]
        self.target_abis = device_dict[YAMLKeyword.target_abis]
        self.target_soc = device_dict[YAMLKeyword.target_socs]
        self.address = device_dict[YAMLKeyword.address]
        self.models = device_dict[YAMLKeyword.models]

    def lock(self):
        return sh_commands.device_lock(self.address)

    def get_shell_prefix(self):
        return ""

    def exec_command(self, command, *args, **kwargs):
        pass

    def push(self, src_path, dst_path):
        pass

    def pull(self, src_path, dst_path='.'):
        pass

    def get_props(self):
        return {}

    def get_available_device_types(self, device_types, abi, executor):
        avail_device_types = []
        if base_pb2.CPU in device_types:
            avail_device_types.append(base_pb2.CPU)
        return avail_device_types

    def get_available_executors(self, executors, abi):
        avail_executors = []
        if base_pb2.MACE in executors:
            avail_executors.append(base_pb2.MACE)
        if base_pb2.TFLITE in executors:
            if abi != "aarch64" and abi != "armhf":
                avail_executors.append(base_pb2.TFLITE)
        if base_pb2.NCNN in executors:
            avail_executors.append(base_pb2.NCNN)
        if base_pb2.MNN in executors:
            if abi != "aarch64" and abi != "armhf":
                avail_executors.append(base_pb2.MNN)
        if base_pb2.TNN in executors:
            if abi != "aarch64" and abi != "armhf":
                avail_executors.append(base_pb2.TNN)

        return avail_executors

    def get_bench_path(self):
        pass
