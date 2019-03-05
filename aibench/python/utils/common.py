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

import inspect
import six
import os
import zipfile
import tarfile
from six.moves import urllib


################################
# log
################################
class CMDColors:
    PURPLE = '\033[95m'
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def get_frame_info(level):
    caller_frame = inspect.stack()[level]
    info = inspect.getframeinfo(caller_frame[0])
    return info.filename + ':' + str(info.lineno) + ': '


class AIBenchLogger:
    @staticmethod
    def header(message):
        six.print_(CMDColors.PURPLE + get_frame_info(2) + message
                   + CMDColors.ENDC)

    @staticmethod
    def summary(message):
        six.print_(CMDColors.GREEN + get_frame_info(2) + message
                   + CMDColors.ENDC)

    @staticmethod
    def info(message):
        six.print_(get_frame_info(2) + message)

    @staticmethod
    def warning(message):
        six.print_(CMDColors.YELLOW + 'WARNING:' + get_frame_info(2) + message
                   + CMDColors.ENDC)

    @staticmethod
    def error(message, location_info=""):
        if not location_info:
            location_info = get_frame_info(2)
        six.print_(CMDColors.RED + 'ERROR: ' + location_info + message
                   + CMDColors.ENDC)
        exit(1)


def aibench_check(condition, message):
    if not condition:
        AIBenchLogger.error(message, get_frame_info(2))


################################
# String Formatter
################################
class StringFormatter:
    @staticmethod
    def table(header, data, title, align="R"):
        data_size = len(data)
        column_size = len(header)
        column_length = [len(str(ele)) + 1 for ele in header]
        for row_idx in range(data_size):
            data_tuple = data[row_idx]
            ele_size = len(data_tuple)
            assert (ele_size == column_size)
            for i in range(ele_size):
                column_length[i] = max(column_length[i],
                                       len(str(data_tuple[i])) + 1)

        table_column_length = sum(column_length) + column_size + 1
        dash_line = '-' * table_column_length + '\n'
        header_line = '=' * table_column_length + '\n'
        output = ""
        output += dash_line
        output += str(title).center(table_column_length) + '\n'
        output += dash_line
        output += '|' + '|'.join([str(header[i]).center(column_length[i])
                                  for i in range(column_size)]) + '|\n'
        output += header_line

        for data_tuple in data:
            ele_size = len(data_tuple)
            row_list = []
            for i in range(ele_size):
                if align == "R":
                    row_list.append(str(data_tuple[i]).rjust(column_length[i]))
                elif align == "L":
                    row_list.append(str(data_tuple[i]).ljust(column_length[i]))
                elif align == "C":
                    row_list.append(str(data_tuple[i])
                                    .center(column_length[i]))
            output += '|' + '|'.join(row_list) + "|\n" + dash_line
        return output

    @staticmethod
    def block(message):
        line_length = 10 + len(str(message)) + 10
        star_line = '*' * line_length + '\n'
        return star_line + str(message).center(line_length) + '\n' + star_line


ABI_TYPES = [
    "armeabi-v7a",
    "arm64-v8a",
    "armhf",
    "aarch64",
    "host",
]

ABI_TOOLCHAIN_CONFIG = {
    "armeabi-v7a": "android",
    "arm64-v8a": "android",
    "armhf": "arm_linux_gnueabihf",
    "aarch64": "aarch64_linux_gnu",
    "host": "",
}


def download_and_extract_dataset(url, download_dir):
    filename = url.split('/')[-1]
    file_path = os.path.join(download_dir, filename)
    if not os.path.exists(file_path):
        if not os.path.exists(download_dir):
            os.makedirs(download_dir)

        print("Downloading %s" % url)
        file_path, _ = urllib.request.urlretrieve(url, file_path)

        if file_path.endswith(".zip"):
            zipfile.ZipFile(file=file_path, mode="r").extractall(download_dir)
        elif file_path.endswith((".tar.gz", ".tgz")):
            tarfile.open(name=file_path, mode="r:gz").extractall(download_dir)

        print("Done extracted to %s" % download_dir)
    else:
        print("Data has already downloaded and extracted.")
