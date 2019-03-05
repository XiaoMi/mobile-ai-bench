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

from adb_device import AdbDevice
from aibench.proto import base_pb2
from aibench.python.utils.sh_commands import *


class HuaweiAdbDevice(AdbDevice):

    def get_available_device_types(self, device_types, abi, executor):
        avail_device_types = AdbDevice.get_available_device_types(
            self, device_types, abi, executor)

        if (base_pb2.NPU in device_types) and (abi == "arm64-v8a") and \
                self._support_hiai_ddk200():
            avail_device_types.append(base_pb2.NPU)

        return avail_device_types

    def get_available_executors(self, executors, abi):
        avail_executors = \
                AdbDevice.get_available_executors(self, executors, abi)
        if (base_pb2.HIAI in executors) and (abi == "arm64-v8a") and \
                self._support_hiai_ddk200():
            avail_executors.append(base_pb2.HIAI)

        return avail_executors

    def _support_hiai_ddk200(self):
        support = False
        soc_vers = re.findall("kirin(\\d+)$", self.target_soc, re.I)
        # DDK 200 matches the kirin980
        if len(soc_vers) > 0 and int(soc_vers[0]) >= 980:
            support = True
        return support
