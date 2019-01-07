// Copyright 2018 Xiaomi, Inc.  All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "aibench/benchmark/imagenet/imagenet_preprocessor.h"

#include <functional>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "mace/utils/logging.h"
#include "opencv2/opencv.hpp"

namespace aibench {
namespace benchmark {

namespace imagenet {

const char *kImageNameHead = "ILSVRC2012_val_";
const char *kImageNameTail = ".JPEG";
const int kImageNameNumLen = 8;

void GetBlackFileList(const std::string &filename,
                      std::unordered_set<std::string> *blacklist) {
  std::ifstream in_file(filename, std::ios::in);
  if (in_file.is_open()) {
    int file_num;
    while (in_file >> file_num) {
      std::stringstream stream;
      stream << kImageNameHead
             << std::setfill('0') << std::setw(8) << file_num
             << kImageNameTail;
      blacklist->emplace(stream.str());
    }
    in_file.close();
  } else {
    LOG(FATAL) << filename << " not found.";
  }
}

bool isValidFileName(const std::unordered_set<std::string> &blacklist,
                     const std::string &filename) {
  if (filename.find(kImageNameHead) != std::string::npos
      && filename.find(kImageNameTail) != std::string::npos
      && blacklist.count(filename) == 0) {
    return true;
  } else {
    return false;
  }
}

void TransformCHWToHWC(const float *input_data,
                       const int64_t height,
                       const int64_t width,
                       const int64_t channels,
                       float *output_data) {
  for (int64_t h = 0; h < height; ++h) {
    for (int64_t w = 0; w < width; ++w) {
      for (int64_t c = 0; c < channels; ++c) {
        output_data[(h * width + w) * channels + c] =
            input_data[(c * height + h) * width + w];
      }
    }
  }
}

}  // namespace imagenet

ImageNetPreProcessor::ImageNetPreProcessor(
    const std::vector<DataFormat> &data_formats,
    const std::vector<std::vector<float>> &input_means,
    const std::vector<float> &input_var,
    const ChannelOrder channel_order)
    : data_formats_(data_formats),
      input_means_(input_means),
      input_var_(input_var),
      channel_order_(channel_order) {
  size_t formats_size = data_formats_.size();
  size_t means_size = input_means_.size();
  size_t var_size = input_var_.size();

  MACE_CHECK(
      ((formats_size == 1 || formats_size == 0)
          && (means_size == 1 || means_size == 0)
          && (var_size == 1 || var_size == 0)),
      "Size of data_formats(", formats_size,
      ") and input_means(", means_size,
      ") and input_var(", var_size,
      ") should be 1 or 0");
  if (formats_size == 0) {
    data_formats_.emplace_back(NHWC);
  }
  MACE_CHECK(data_formats_[0] == NHWC || data_formats_[0] == NCHW);
  if (means_size == 0) {
    input_means_.emplace_back(3, 127.5);
  } else {
    MACE_CHECK(input_means_[0].size() == 3, "Input means count should be 3");
  }
  if (var_size == 0) {
    input_var_.emplace_back(127.5);
  } else {
    MACE_CHECK(input_var_[0] > 1e-6,
               "Input standard deviation should be greater than zero");
  }
  MACE_CHECK(channel_order_ == RGB || channel_order_ == BGR);

  imagenet::GetBlackFileList("imagenet_blacklist.txt", &blacklist_);
}

Status ImageNetPreProcessor::Run(const std::string &filename,
                                 std::map<std::string, BaseTensor> *inputs) {
  if (!imagenet::isValidFileName(blacklist_, filename)) {
    return NOT_SUPPORTED;
  }
  MACE_CHECK(inputs->size() == 1);
  auto input = inputs->begin();
  auto &input_shape = input->second.shape();
  auto input_data = input->second.data().get();

  int64_t image_height, image_width, image_channels;
  if (data_formats_[0] == NHWC) {
    image_height = input_shape[1];
    image_width = input_shape[2];
    image_channels = input_shape[3];
  } else if (data_formats_[0] == NCHW) {
    image_channels = input_shape[1];
    image_height = input_shape[2];
    image_width = input_shape[3];
  }
  MACE_CHECK(image_channels == 3, "Input channels should be 3");

  const std::string input_dir("inputs/");
  std::string file_path = input_dir + filename;
  cv::Mat image, blob;
  image = cv::imread(file_path, cv::ImreadModes::IMREAD_COLOR);

  if (!image.data) {
    LOG(FATAL) << "Open input file failed " << file_path;
  } else {
    cv::dnn::blobFromImage(image,
                           blob,
                           1.0 / input_var_[0],
                           cv::Size(image_width, image_height),
                           cv::Scalar(input_means_[0][0],
                                      input_means_[0][1],
                                      input_means_[0][2]),
                           channel_order_ == RGB,
                           true);

    MACE_CHECK(blob.isContinuous(), "blob is not continuous.");
    auto input_size = std::distance(blob.begin<float>(), blob.end<float>());
    MACE_CHECK(input_size == image_channels * image_height * image_width,
               "Wrong input size: ", input_size);
    if (data_formats_[0] == NHWC) {
      imagenet::TransformCHWToHWC(blob.ptr<float>(),
                                  image_height,
                                  image_width,
                                  image_channels,
                                  input_data);
    } else if (data_formats_[0] == NCHW) {
      std::copy_n(blob.ptr<float>(), input_size, input_data);
    } else {
      LOG(FATAL) << "Wrong input format: " << data_formats_[0];
    }
  }

  return SUCCESS;
}

}  // namespace benchmark
}  // namespace aibench

