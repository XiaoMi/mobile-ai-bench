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

import sh

from aibench.python.device.adb_device import AdbDevice
from aibench.proto import base_pb2
from aibench.python.utils.sh_commands import *


class QualcommAdbDevice(AdbDevice):

    def get_available_device_types(self, device_types, abi, executor):
        avail_device_types = AdbDevice.get_available_device_types(
            self, device_types, abi, executor)
        if base_pb2.DSP in device_types and self._support_dev_dsp(executor):
            avail_device_types.append(base_pb2.DSP)

        if base_pb2.NPU in device_types and self._support_npu():
            avail_device_types.append(base_pb2.NPU)

        return avail_device_types

    def get_available_executors(self, executors, abi):
        avail_executors = \
                AdbDevice.get_available_executors(self, executors, abi)
        if base_pb2.SNPE in executors:
            avail_executors.append(base_pb2.SNPE)

        return avail_executors

    # TODO(luxuhui@xiaomi.com): optimize this method after qualcomm release
    # newer socs.
    def _support_npu(self):
        support = False
        # msmnile is 855, the only soc support NPU up to now.
        if self.target_soc == "msmnile":
            support = True
        return support

    def _support_dev_dsp(self, executor):
        support_dev_dsp = False
        if self.target_soc == "sdm660" and executor == base_pb2.SNPE:
            return support_dev_dsp
        if self.target_soc == "msmnile":
            return support_dev_dsp
        try:
            output = self.exec_command(
                "ls /system/vendor/lib/rfsa/adsp/libhexagon_nn_skel.so")  # noqa
        except sh.ErrorReturnCode_1:
            print("libhexagon_nn_skel.so does not exists, QualcommAdbDevice Skip DSP.")  # noqa
        else:
            if "No such file or directory" in output:
                print("libhexagon_nn_skel.so does not exists, QualcommAdbDevice Skip DSP.")  # noqa
            else:
                support_dev_dsp = True
        return support_dev_dsp
