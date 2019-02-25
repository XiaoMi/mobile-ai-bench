# Copyright 2018 The MobileAIBench Authors. All Rights Reserved.
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

import hashlib
import os
import sh

import bench_utils


def strip_invalid_utf8(str):
    return sh.iconv(str, "-c", "-t", "UTF-8")


def split_stdout(stdout_str):
    stdout_str = strip_invalid_utf8(stdout_str)
    # Filter out last empty line
    return [l.strip() for l in stdout_str.split('\n') if len(l.strip()) > 0]


def adb_push_file(src_file, dst_dir, serialno, silent=False):
    if not os.path.isfile(src_file):
        print("Not file, skip pushing " + src_file)
        return
    src_checksum = bench_utils.file_checksum(src_file)
    dst_file = os.path.join(dst_dir, os.path.basename(src_file))
    stdout_buff = []
    try:
        sh.adb("-s", serialno, "shell", "md5sum", dst_file,
               _out=lambda line: stdout_buff.append(line))
    except sh.ErrorReturnCode_1:
        print("Push %s to %s" % (src_file, dst_dir))
        sh.adb("-s", serialno, "push", src_file, dst_dir)
    else:
        dst_checksum = stdout_buff[0].split()[0]
        if src_checksum == dst_checksum:
            if not silent:
                print("Equal checksum with %s and %s" % (src_file, dst_file))
        else:
            if not silent:
                print("Push %s to %s" % (src_file, dst_dir))
            sh.adb("-s", serialno, "push", src_file, dst_dir)


def adb_push(src_path, dst_dir, serialno, silent=False):
    if os.path.isdir(src_path):
        for src_file in os.listdir(src_path):
            adb_push_file(os.path.join(src_path, src_file),
                          dst_dir, serialno, silent)
    else:
        adb_push_file(src_path, dst_dir, serialno, silent)


def adb_pull(src_path, dst_path, serialno):
    print("Pull %s to %s" % (src_path, dst_path))
    try:
        sh.adb("-s", serialno, "pull", src_path, dst_path)
    except Exception as e:
        print("Error msg: %s" % e.stderr)


def ssh_push_file(src_file, dst_dir, username, address, silent=False):
    if not os.path.isfile(src_file):
        print("Not file, skip pushing " + src_file)
        return
    src_checksum = bench_utils.file_checksum(src_file)
    dst_file = os.path.join(dst_dir, os.path.basename(src_file))
    stdout_buff = []
    try:
        sh.ssh('%s@%s' % (username, address), "md5sum", dst_file,
               _out=lambda line: stdout_buff.append(line))
    except sh.ErrorReturnCode_1:
        print("Scp %s to %s" % (src_file, dst_dir))
        sh.ssh('%s@%s' % (username, address), "mkdir -p %s" % dst_dir)
        sh.scp(src_file, '%s@%s:%s' % (username, address, dst_dir))
    else:
        dst_checksum = stdout_buff[0].split()[0]
        if src_checksum == dst_checksum:
            if not silent:
                print("Equal checksum with %s and %s" % (src_file, dst_file))
        else:
            if not silent:
                print("Scp %s to %s" % (src_file, dst_dir))
            sh.scp(src_file, '%s@%s:%s' % (username, address, dst_dir))


def ssh_push(src_path, dst_dir, username, address, silent=False):
    if os.path.isdir(src_path):
        print("Start scp dir %s=>%s, basename=%s"
              % (src_path, dst_dir, os.path.basename(src_path)))
        sh.scp("-r", src_path, '%s@%s:%s' % (username, address, dst_dir))
        tmp_dst_dir = os.path.join(dst_dir, os.path.basename(src_path))
        sh.ssh('%s@%s' % (username, address),
               "mv %s/* %s" % (tmp_dst_dir, dst_dir))
    else:
        ssh_push_file(src_path, dst_dir, username, address, silent)
