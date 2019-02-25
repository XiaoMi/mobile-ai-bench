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

import os
import re
import six
import sh
import yaml

from adb_device import AdbDevice
from device import Device
from device import YAMLKeyword
from host_device import HostDevice
from huawei_adb_device import HuaweiAdbDevice
from qualcomm_adb_device import QualcommAdbDevice
from ssh_device import SshDevice


class DeviceManager:

    def list_adb_device(self):
        adb_list = sh.adb('devices').stdout.decode('utf-8') \
                     .strip().split('\n')[1:]
        adb_list = [tuple(pair.split('\t')) for pair in adb_list]
        devices = []
        for adb in adb_list:
            adb_device = self._create_adb_device(adb)
            devices.append(adb_device)
        return devices

    def list_ssh_device(self, yml):
        devices = []
        with open(yml) as f:
            yml_config = yaml.load(f.read())
            devices = yml_config['devices']
        device_list = []
        for name, dev in six.iteritems(devices):
            print("Find ssh device:%s" % name)
            dev[YAMLKeyword.device_name] = name
            device_list.append(SshDevice(dev))
        return device_list

    def list_devices(self,
                     yml='generic-mobile-devices/devices_for_ai_bench.yml'):
        devices_list = []
        devices_list.extend(self.list_adb_device())
        if os.path.exists(yml):
            devices_list.extend(self.list_ssh_device(yml))
        else:
            six.print_('No Arm linux device yaml file')

        return devices_list

    def _create_adb_device(self, adb):
        adb_device = AdbDevice(adb)
        # TODO(luxuhui@xiaomi.com): optimize this match after qualcomm release
        # newer socs
        if re.match("sdm\\d+$|msm\\d+$|msmnile", adb_device.target_soc, re.I):
            # ["sdm845", "sdm660", "msm8998", "msm8996", "msmnile"]
            adb_device = QualcommAdbDevice(adb)
            print("Find qualcomm adb device:%s" % adb[0])
        elif re.match("kirin\\d+$", adb_device.target_soc, re.I):
            # ["kirin980", "kirin970", "kirin960"]
            adb_device = HuaweiAdbDevice(adb)
            print("Find huawei adb device:%s" % adb[0])
        else:
            print("Find adb device:%s" % adb[0])

        return adb_device
