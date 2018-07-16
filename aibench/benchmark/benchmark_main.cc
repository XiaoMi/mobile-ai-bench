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

#include <string>
#include <iostream>

#include "gflags/gflags.h"
#include "aibench/benchmark/benchmark.h"
#include "aibench/executors/base_executor.h"
#ifdef AIBENCH_ENABLE_MACE
#include "aibench/executors/mace/mace_executor.h"
#endif
#ifdef AIBENCH_ENABLE_NCNN
#include "aibench/executors/ncnn/ncnn_executor.h"
#endif
#ifdef AIBENCH_ENABLE_SNPE
#include "aibench/executors/snpe/snpe_executor.h"
#endif
#ifdef AIBENCH_ENABLE_TFLITE
#include "aibench/executors/tflite/tflite_executor.h"
#endif

DEFINE_string(model_name, "all", "the model to benchmark");
DEFINE_string(framework, "all", "the framework to benchmark");
DEFINE_string(runtime, "all", "the runtime to benchmark");
DEFINE_string(product_soc, "", "product model and target soc");
DEFINE_int32(run_interval, 10, "run interval between benchmarks, seconds");
DEFINE_int32(num_threads, 4, "number of threads");

int main(int argc, char **argv) {
  std::string usage = "run benchmark, e.g. " + std::string(argv[0]) +
                      " --model_name=all";
  gflags::SetUsageMessage(usage);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  // define all benchmarks here
#ifdef AIBENCH_ENABLE_MACE
  std::unique_ptr<aibench::MaceExecutor> mobilenetv1_mace_cpu_executor(
      new aibench::MaceExecutor(aibench::CPU, FLAGS_product_soc, {"input"},
                                {"MobilenetV1/Predictions/Reshape_1"}));
  AIBENCH_BENCHMARK(mobilenetv1_mace_cpu_executor.get(), MobileNetV1, MACE,
                    CPU, mobilenet_v1, (std::vector<std::string>{"input"}),
                    (std::vector<std::string>{"dog.npy"}),
                    (std::vector<std::vector<int64_t>>{{1, 224, 224, 3}}),
                    (std::vector<std::string>{
                        "MobilenetV1/Predictions/Reshape_1"}),
                    (std::vector<std::vector<int64_t>>{{1, 1001}}));
  std::unique_ptr<aibench::MaceExecutor> mobilenetv2_mace_cpu_executor(
      new aibench::MaceExecutor(aibench::CPU, FLAGS_product_soc, {"input"},
                                {"MobilenetV2/Predictions/Reshape_1"}));
  AIBENCH_BENCHMARK(mobilenetv2_mace_cpu_executor.get(), MobileNetV2, MACE,
                    CPU, mobilenet_v2, (std::vector<std::string>{"input"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{1, 224, 224, 3}}),
                    (std::vector<std::string>{
                        "MobilenetV2/Predictions/Reshape_1"}),
                    (std::vector<std::vector<int64_t>>{{1, 1001}}));
  std::unique_ptr<aibench::MaceExecutor> squeezenetv11_mace_cpu_executor(
      new aibench::MaceExecutor(aibench::CPU, FLAGS_product_soc, {"data"},
                                {"prob"}));
  AIBENCH_BENCHMARK(squeezenetv11_mace_cpu_executor.get(), SqueezeNetV11, MACE,
                    CPU, squeezenet_v11, (std::vector<std::string>{"data"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{1, 227, 227, 3}}),
                    (std::vector<std::string>{"prob"}),
                    (std::vector<std::vector<int64_t>>{{1, 1, 1, 1000}}));
  std::unique_ptr<aibench::MaceExecutor> inceptionv3_mace_cpu_executor(
      new aibench::MaceExecutor(aibench::CPU, FLAGS_product_soc, {"input"},
                                {"InceptionV3/Predictions/Reshape_1"}));
  AIBENCH_BENCHMARK(inceptionv3_mace_cpu_executor.get(), InceptionV3, MACE,
                    CPU, inception_v3, (std::vector<std::string>{"input"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{1, 299, 299, 3}}),
                    (std::vector<std::string>{
                        "InceptionV3/Predictions/Reshape_1"}),
                    (std::vector<std::vector<int64_t>>{{1, 1001}}));
  std::unique_ptr<aibench::MaceExecutor> vgg16_mace_cpu_executor(
      new aibench::MaceExecutor(aibench::CPU, FLAGS_product_soc, {"input"},
                                {"vgg_16/fc8/BiasAdd"}));
  AIBENCH_BENCHMARK(vgg16_mace_cpu_executor.get(), VGG16, MACE,
                    CPU, vgg16, (std::vector<std::string>{"input"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{1, 224, 224, 3}}),
                    (std::vector<std::string>{"vgg_16/fc8/BiasAdd"}),
                    (std::vector<std::vector<int64_t>>{{1, 1, 1, 1000}}));

  std::unique_ptr<aibench::MaceExecutor> mobilenetv1_mace_gpu_executor(
      new aibench::MaceExecutor(aibench::GPU, FLAGS_product_soc, {"input"},
                                {"MobilenetV1/Predictions/Reshape_1"}));
  AIBENCH_BENCHMARK(mobilenetv1_mace_gpu_executor.get(), MobileNetV1, MACE,
                    GPU, mobilenet_v1, (std::vector<std::string>{"input"}),
                    (std::vector<std::string>{"dog.npy"}),
                    (std::vector<std::vector<int64_t>>{{1, 224, 224, 3}}),
                    (std::vector<std::string>{
                        "MobilenetV1/Predictions/Reshape_1"}),
                    (std::vector<std::vector<int64_t>>{{1, 1001}}));
  std::unique_ptr<aibench::MaceExecutor> mobilenetv2_mace_gpu_executor(
      new aibench::MaceExecutor(aibench::GPU, FLAGS_product_soc, {"input"},
                                {"MobilenetV2/Predictions/Reshape_1"}));
  AIBENCH_BENCHMARK(mobilenetv2_mace_gpu_executor.get(), MobileNetV2, MACE,
                    GPU, mobilenet_v2, (std::vector<std::string>{"input"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{1, 224, 224, 3}}),
                    (std::vector<std::string>{
                        "MobilenetV2/Predictions/Reshape_1"}),
                    (std::vector<std::vector<int64_t>>{{1, 1001}}));
  std::unique_ptr<aibench::MaceExecutor> squeezenetv11_mace_gpu_executor(
      new aibench::MaceExecutor(aibench::GPU, FLAGS_product_soc, {"data"},
                                {"prob"}));
  AIBENCH_BENCHMARK(squeezenetv11_mace_gpu_executor.get(), SqueezeNetV11, MACE,
                    GPU, squeezenet_v11, (std::vector<std::string>{"data"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{1, 227, 227, 3}}),
                    (std::vector<std::string>{"prob"}),
                    (std::vector<std::vector<int64_t>>{{1, 1, 1, 1000}}));
  std::unique_ptr<aibench::MaceExecutor> inceptionv3_mace_gpu_executor(
      new aibench::MaceExecutor(aibench::GPU, FLAGS_product_soc, {"input"},
                                {"InceptionV3/Predictions/Reshape_1"}));
  AIBENCH_BENCHMARK(inceptionv3_mace_gpu_executor.get(), InceptionV3, MACE,
                    GPU, inception_v3, (std::vector<std::string>{"input"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{1, 299, 299, 3}}),
                    (std::vector<std::string>{
                        "InceptionV3/Predictions/Reshape_1"}),
                    (std::vector<std::vector<int64_t>>{{1, 1001}}));
  std::unique_ptr<aibench::MaceExecutor> vgg16_mace_gpu_executor(
      new aibench::MaceExecutor(aibench::GPU, FLAGS_product_soc, {"data"},
                                {"prob"}));
  AIBENCH_BENCHMARK(vgg16_mace_gpu_executor.get(), VGG16, MACE,
                    GPU, vgg16_caffe_gpu, (std::vector<std::string>{"data"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{1, 224, 224, 3}}),
                    (std::vector<std::string>{"prob"}),
                    (std::vector<std::vector<int64_t>>{{1, 1, 1, 1000}}));
#ifdef AIBENCH_ENABLE_MACE_DSP
  std::unique_ptr<aibench::MaceExecutor> inceptionv3_mace_dsp_executor(
      new aibench::MaceExecutor(aibench::DSP, FLAGS_product_soc, {"Mul"},
                                {"softmax"}));
  if (FLAGS_product_soc.compare("polaris.sdm845") == 0) {
    AIBENCH_BENCHMARK(inceptionv3_mace_dsp_executor.get(), InceptionV3, MACE,
                      DSP, inception_v3_dsp, (std::vector<std::string>{"Mul"}),
                      (std::vector<std::string>{}),
                      (std::vector<std::vector<int64_t>>{{1, 299, 299, 3}}),
                      (std::vector<std::string>{"softmax"}),
                      (std::vector<std::vector<int64_t>>{{1, 1, 1, 1008}}));
  }
#endif  // AIBENCH_ENABLE_MACE_DSP
#endif
#ifdef AIBENCH_ENABLE_SNPE
  std::unique_ptr<aibench::SnpeExecutor>
      snpe_cpu_executor(new aibench::SnpeExecutor(aibench::CPU));
  AIBENCH_BENCHMARK(snpe_cpu_executor.get(), InceptionV3, SNPE, CPU,
                    inception_v3.dlc, (std::vector<std::string>{"Mul:0"}),
                    (std::vector<std::string>{"keyboard_299x299.dat"}),
                    (std::vector<std::vector<int64_t>>{{299, 299, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
  AIBENCH_BENCHMARK(snpe_cpu_executor.get(), MobileNetV1, SNPE, CPU,
                    mobilenet-v1.dlc, (std::vector<std::string>{"input:0"}),
                    (std::vector<std::string>{"chairs_224x224.raw"}),
                    (std::vector<std::vector<int64_t>>{{224, 224, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
  AIBENCH_BENCHMARK(snpe_cpu_executor.get(), MobileNetV2, SNPE, CPU,
                    mobilenet-v2.dlc, (std::vector<std::string>{"input:0"}),
                    (std::vector<std::string>{"chairs_224x224.raw"}),
                    (std::vector<std::vector<int64_t>>{{224, 224, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
  AIBENCH_BENCHMARK(snpe_cpu_executor.get(), VGG16, SNPE, CPU,
                    vgg16.dlc, (std::vector<std::string>{"input:0"}),
                    (std::vector<std::string>{"chairs_224x224.raw"}),
                    (std::vector<std::vector<int64_t>>{{224, 224, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
  AIBENCH_BENCHMARK(snpe_cpu_executor.get(), SqueezeNetV11, SNPE, CPU,
                    squeezenet_v11.dlc, (std::vector<std::string>{"data"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{227, 227, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));

  std::unique_ptr<aibench::SnpeExecutor>
      snpe_gpu_executor(new aibench::SnpeExecutor(aibench::GPU));
  AIBENCH_BENCHMARK(snpe_gpu_executor.get(), InceptionV3, SNPE, GPU,
                    inception_v3.dlc, (std::vector<std::string>{"Mul:0"}),
                    (std::vector<std::string>{"keyboard_299x299.dat"}),
                    (std::vector<std::vector<int64_t>>{{299, 299, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
  AIBENCH_BENCHMARK(snpe_gpu_executor.get(), MobileNetV1, SNPE, GPU,
                    mobilenet-v1.dlc, (std::vector<std::string>{"input:0"}),
                    (std::vector<std::string>{"chairs_224x224.raw"}),
                    (std::vector<std::vector<int64_t>>{{224, 224, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
  AIBENCH_BENCHMARK(snpe_gpu_executor.get(), MobileNetV2, SNPE, GPU,
                    mobilenet-v2.dlc, (std::vector<std::string>{"input:0"}),
                    (std::vector<std::string>{"chairs_224x224.raw"}),
                    (std::vector<std::vector<int64_t>>{{224, 224, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
  AIBENCH_BENCHMARK(snpe_gpu_executor.get(), SqueezeNetV11, SNPE, GPU,
                    squeezenet_v11.dlc, (std::vector<std::string>{"data"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{227, 227, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
  // TODO(wuchenghui): benchmark snpe + gpu + vgg16

  std::unique_ptr<aibench::SnpeExecutor>
      snpe_dsp_executor(new aibench::SnpeExecutor(aibench::DSP));
  AIBENCH_BENCHMARK(snpe_dsp_executor.get(), InceptionV3, SNPE, DSP,
                    inception_v3_quantized.dlc,
                    (std::vector<std::string>{"Mul:0"}),
                    (std::vector<std::string>{"keyboard_299x299.dat"}),
                    (std::vector<std::vector<int64_t>>{{299, 299, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
  AIBENCH_BENCHMARK(snpe_dsp_executor.get(), VGG16, SNPE, DSP,
                    vgg16_quantized.dlc, (std::vector<std::string>{"input:0"}),
                    (std::vector<std::string>{"chairs_224x224.raw"}),
                    (std::vector<std::vector<int64_t>>{{224, 224, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
#endif
#ifdef AIBENCH_ENABLE_NCNN
  std::unique_ptr<aibench::NcnnExecutor>
      ncnn_executor(new aibench::NcnnExecutor());
  AIBENCH_BENCHMARK(ncnn_executor.get(), MobileNetV1, NCNN, CPU,
                    mobilenet.param, (std::vector<std::string>{"data"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{224, 224, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
  AIBENCH_BENCHMARK(ncnn_executor.get(), MobileNetV2, NCNN, CPU,
                    mobilenet_v2.param, (std::vector<std::string>{"data"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{224, 224, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
  AIBENCH_BENCHMARK(ncnn_executor.get(), SqueezeNetV11, NCNN, CPU,
                    squeezenet.param, (std::vector<std::string>{"data"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{227, 227, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
  AIBENCH_BENCHMARK(ncnn_executor.get(), VGG16, NCNN, CPU,
                    vgg16.param, (std::vector<std::string>{"data"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{224, 224, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
  AIBENCH_BENCHMARK(ncnn_executor.get(), InceptionV3, NCNN, CPU,
                    inception_v3.param, (std::vector<std::string>{"data"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{299, 299, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
#endif
#ifdef AIBENCH_ENABLE_TFLITE
  std::unique_ptr<aibench::TfLiteExecutor>
      tflite_executor(new aibench::TfLiteExecutor());
  AIBENCH_BENCHMARK(tflite_executor.get(), MobileNetV1Quant, TFLITE, CPU,
                    mobilenet_quant_v1_224.tflite,
                    (std::vector<std::string>{"Placeholder"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{1, 224, 224, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
  AIBENCH_BENCHMARK(tflite_executor.get(), MobileNetV1, TFLITE, CPU,
                    mobilenet_v1_1.0_224.tflite,
                    (std::vector<std::string>{"input"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{1, 224, 224, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));
  AIBENCH_BENCHMARK(tflite_executor.get(), InceptionV3, TFLITE, CPU,
                    inception_v3.tflite,
                    (std::vector<std::string>{"input"}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{{1, 299, 299, 3}}),
                    (std::vector<std::string>{}),
                    (std::vector<std::vector<int64_t>>{}));

#endif
  aibench::Status status = aibench::benchmark::Benchmark::Run(
      FLAGS_model_name.c_str(), FLAGS_framework.c_str(), FLAGS_runtime.c_str(),
      FLAGS_run_interval, FLAGS_num_threads);
  return status;
}
