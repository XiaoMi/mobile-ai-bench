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
import sh
import six
import sys
import yaml

from aibench.python.utils.common import *
from aibench.python.utils.sh_commands import *
from device import Device
from device import YAMLKeyword


class SshDevice(Device):

    def __init__(self, device_dict):
        Device.__init__(self, device_dict)
        self.username = device_dict[YAMLKeyword.username]
        if YAMLKeyword.device_types in device_dict.keys():
            self.device_types = device_dict[YAMLKeyword.device_types]
        else:
            self.device_types = ["cpu"]
        try:
            sh.ssh('-q', '%s@%s' % (self.username, self.address),
                   'exit')
        except sh.ErrorReturnCode as e:
            six.print_('device connect failed, '
                       'please check your authentication',
                       file=sys.stderr)
            raise e

    def get_shell_prefix(self):
        return "ssh %s@%s" % (self.username, self.address)

    def exec_command(self, command, *args, **kwargs):
        return sh.ssh('%s@%s' % (self.username, self.address),
                      command, *args, **kwargs)

    def push(self, src_path, dst_path, silent=False):
        ssh_push(src_path, dst_path, self.username, self.address, silent)

    def pull(self, src_path, dst_path='.'):
        if os.path.isdir(dst_path):
            exist_file = dst_path + '/' + src_path.split('/')[-1]
            if os.path.exists(exist_file):
                sh.rm('-rf', exist_file)
        elif os.path.exists(dst_path):
            sh.rm('-f', dst_path)
        try:
            sh.scp('-r',
                   '%s@%s:%s' % (self.username, self.address, src_path),
                   dst_path)
        except sh.ErrorReturnCode_1 as e:
            six.print_('Error msg {}'.format(e), file=sys.stderr)
            return

    def get_props(self):
        # TODO(luxuhui@xiaomi.com): read data from yml config
        props = {}
        props["ro.product.model"] = self.models
        return props

    def get_available_device_types(self, device_types, abi, executor):
        # TODO(luxuhui@xiaomi.com): get support device_types from config
        return Device.get_available_device_types(self, device_types,
                                                 abi, executor)

    def get_bench_path(self):
        return "/home/%s/tmp/aibench" % self.username
