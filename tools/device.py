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
import sys

import six
import sh
import yaml

import sh_commands

from common import *


class DeviceWrapper:
    def __init__(self, device_dict):
        """
        init device with device dict
        """
        diff = set(device_dict.keys()) - set(YAMLKeyword.__dict__.keys())
        if len(diff) > 0:
            six.print_('Wrong key detected:')
            six.print_(diff)
            raise KeyError(str(diff))
        self.__dict__.update(device_dict)
        if self.system == SystemType.android:
            pass
        elif self.system == SystemType.arm_linux:
            try:
                sh.ssh('-q', '%s@%s' % (self.username, self.address),
                       'exit')
            except sh.ErrorReturnCode as e:
                six.print_('device connect failed, '
                           'please check your authentication',
                           file=sys.stderr)
                raise e

    #####################
    #  public interface #
    #####################

    def lock(self):
        return sh_commands.device_lock(self.address)

    def exec_command(self, command, *args, **kwargs):
        if self.system == SystemType.android:
            return sh.adb('-s', self.address, 'shell',
                          command, *args, **kwargs)
        elif self.system == SystemType.arm_linux:
            return sh.ssh('%s@%s' % (self.username, self.address),
                          command, *args, **kwargs)

    def push(self, src_path, dst_path):
        if self.system == SystemType.android:
            sh_commands.adb_push(src_path, dst_path, self.address)
        elif self.system == SystemType.arm_linux:
            try:
                sh.scp(src_path, '%s@%s:%s' %
                       (self.username, self.address, dst_path))
            except sh.ErrorReturnCode_1 as e:
                six.print_('Error msg {}'.format(e), file=sys.stderr)

    def pull(self, src_path, dst_path='.'):
        if self.system == SystemType.android:
            sh_commands.adb_pull(src_path, dst_path, self.address)
        elif self.system == SystemType.arm_linux:
            if os.path.isdir(dst_path):
                exist_file = dst_path + '/' + src_path.split('/')[-1]
                if os.path.exists(exist_file):
                    sh.rm('-rf', exist_file)
            elif os.path.exists(dst_path):
                sh.rm('-f', dst_path)
            try:
                sh.scp('-r',
                       '%s@%s:%s' % (self.username,
                                     self.address,
                                     src_path),
                       dst_path)
            except sh.ErrorReturnCode_1 as e:
                six.print_('Error msg {}'.format(e), file=sys.stderr)
                return


class DeviceManager:
    @classmethod
    def list_adb_device(cls):
        adb_list = sh.adb('devices').stdout.decode('utf-8'). \
                       strip().split('\n')[1:]
        adb_list = [tuple(pair.split('\t')) for pair in adb_list]
        devices = []
        for adb in adb_list:
            prop = sh_commands.adb_getprop_by_serialno(adb[0])
            android = {
                YAMLKeyword.device_name: adb[1],
                YAMLKeyword.target_abis:
                    prop['ro.product.cpu.abilist'].split(','),
                YAMLKeyword.target_socs: prop['ro.board.platform'],
                YAMLKeyword.models: prop['ro.product.model'],
                YAMLKeyword.system: SystemType.android,
                YAMLKeyword.address: adb[0],
                YAMLKeyword.username: '',
                YAMLKeyword.password: ''
            }
            devices.append(android)
        return devices

    @classmethod
    def list_ssh_device(cls, yml):
        with open(yml) as f:
            devices = yaml.load(f.read())
        devices = devices['devices']
        device_list = []
        for name, dev in six.iteritems(devices):
            dev[YAMLKeyword.device_name] = name
            dev[YAMLKeyword.system] = SystemType.arm_linux
            if YAMLKeyword.password not in dev:
                dev[YAMLKeyword.password] = ''
            device_list.append(dev)
        return device_list

    @classmethod
    def list_devices(cls, yml='devices.yml'):
        devices_list = []
        devices_list.extend(cls.list_adb_device())
        if os.path.exists(yml):
            devices_list.extend(cls.list_ssh_device(yml))
        else:
            six.print_('no Arm linux device yaml file')
        host = {
            YAMLKeyword.target_abis: [ABIType.host],
            YAMLKeyword.target_socs: None,
            YAMLKeyword.system: SystemType.host,
            YAMLKeyword.models: None,
            YAMLKeyword.address: SystemType.host
        }
        devices_list.append(host)
        return devices_list


if __name__ == '__main__':
    devices_list_ = DeviceManager.list_devices()
    for dev_ in devices_list_:
        device = DeviceWrapper(dev_)
