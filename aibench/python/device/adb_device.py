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

import re
import sh

from aibench.proto import base_pb2
from aibench.python.utils import sh_commands
from device import Device
from device import YAMLKeyword


class AdbDevice(Device):

    def __init__(self, adb):
        self.address = adb[0]
        prop = self.get_props()
        Device.__init__(self, {
            YAMLKeyword.device_name: adb[1],
            YAMLKeyword.target_abis:
                prop['ro.product.cpu.abilist'].split(','),
            YAMLKeyword.target_socs: prop['ro.board.platform'],
            YAMLKeyword.models: prop['ro.product.model'],
            YAMLKeyword.address: adb[0],
        })

    def get_shell_prefix(self):
        return "adb -s %s shell" % self.address

    def exec_command(self, command, *args, **kwargs):
        return sh.adb('-s', self.address, 'shell',
                      command, *args, **kwargs)

    def push(self, src_path, dst_path, silent=False):
        sh_commands.adb_push(src_path, dst_path, self.address, silent)

    def pull(self, src_path, dst_path='.'):
        sh_commands.adb_pull(src_path, dst_path, self.address)

    def get_props(self):
        outputs = sh.adb("-s", self.address, "shell", "getprop")
        raw_props = sh_commands.split_stdout(outputs)
        props = {}
        p = re.compile(r'\[(.+)\]: \[(.+)\]')
        for raw_prop in raw_props:
            m = p.match(raw_prop)
            if m:
                props[m.group(1)] = m.group(2)
        return props

    def get_available_device_types(self, device_types, abi, executor):
        avail_device_types = Device.get_available_device_types(
            self, device_types, abi, executor)
        if base_pb2.GPU in device_types:
            avail_device_types.append(base_pb2.GPU)
        return avail_device_types

    def get_bench_path(self):
        return "/data/local/tmp/aibench"
