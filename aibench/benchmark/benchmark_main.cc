// Copyright 2018 The MobileAIBench Authors. All Rights Reserved.
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

#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <string>

#include "gflags/gflags.h"
#include "google/protobuf/message_lite.h"
#include "mace/utils/logging.h"
#include "mace/utils/utils.h"
#include "aibench/benchmark/benchmark.h"
#include "aibench/proto/aibench.pb.h"
#include "aibench/proto/base.pb.h"

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

#include "aibench/benchmark/imagenet/imagenet_preprocessor.h"
#include "aibench/benchmark/imagenet/imagenet_postprocessor.h"

namespace aibench {
namespace benchmark {

DEFINE_int32(benchmark_option,
             Performance, "benchmark option");
DEFINE_int32(run_interval,
             10, "run interval between benchmarks, seconds");
DEFINE_int32(num_threads,
             4, "number of threads");
DEFINE_int32(executor,
             MACE, "the executor to benchmark");
DEFINE_int32(model_name,
             MobileNetV1, "the model to benchmark");
DEFINE_int32(device_type,
             CPU, "the device_type to benchmark");
DEFINE_bool(quantize,
            false, "quantized or float");

namespace {

std::string GetFileName(const std::string &path) {
  auto index = path.find_last_of('/');
  return index != std::string::npos ? path.substr(index + 1) : path;
}

Status GetBenchInfo(const std::string &filename,
                    std::vector<std::string> *input_names,
                    std::vector<std::string> *output_names,
                    std::vector<std::vector<int64_t>> *output_shapes,
                    std::string *model_file,
                    std::string *weight_file) {
  std::vector<unsigned char> file_buffer;
  if (!mace::ReadBinaryFile(&file_buffer, filename)) {
    LOG(FATAL) << "Failed to read file: " << filename;
  }
  BenchFactory bench_factory;
  bench_factory.ParseFromArray(file_buffer.data(), file_buffer.size());
  for (const auto &benchmark : bench_factory.benchmarks()) {
    if (benchmark.executor() != FLAGS_executor) continue;
    for (const auto &model : benchmark.models()) {
      if (model.model_name() != FLAGS_model_name) continue;
      if (model.quantize() != FLAGS_quantize) continue;
      for (const auto &device : model.devices()) {
        if (device != FLAGS_device_type) continue;
        model_file->assign(GetFileName(model.model_path()));
        if (model.has_weight_path()) {
          weight_file->assign(GetFileName(model.weight_path()));
        }
        input_names->assign(model.input_names().begin(),
                            model.input_names().end());
        output_names->assign(model.output_names().begin(),
                             model.output_names().end());
        if (model.output_shape_size() > 0) {
          output_shapes->resize(model.output_shape_size());
          for (int i = 0; i < model.output_shape_size(); ++i) {
            (*output_shapes)[i].assign(model.output_shape(i).shape().begin(),
                                       model.output_shape(i).shape().end());
          }
        }
        return Status::SUCCESS;
      }
    }
  }
  return Status::NOT_SUPPORTED;
}

Status GetModelBaseInfo(
    const std::string &filename,
    ChannelOrder *channel_order,
    std::vector<DataFormat> *data_formats,
    std::vector<std::vector<int64_t>> *input_shapes,
    std::vector<std::vector<int64_t>> *output_shapes,
    std::vector<std::vector<float>> *input_means,
    std::vector<float> *input_var,
    PreProcessor_PreProcessorType *pre_type,
    PostProcessor_PostProcessorType *post_type,
    MetricEvaluator_MetricEvaluatorType *metric_type) {
  std::vector<unsigned char> file_buffer;
  if (!mace::ReadBinaryFile(&file_buffer, filename)) {
    LOG(FATAL) << "Failed to read file: " << filename;
  }
  ModelFactory model_factory;
  model_factory.ParseFromArray(file_buffer.data(), file_buffer.size());
  for (const auto &model : model_factory.models()) {
    if (model.model_name() != FLAGS_model_name) continue;
    *channel_order = model.channel_order(0);
    data_formats->resize(model.data_format_size());
    for (int i = 0; i < model.data_format_size(); ++i) {
      (*data_formats)[i] = model.data_format(i);
    }
    input_shapes->resize(model.input_shape_size());
    for (int i = 0; i < model.input_shape_size(); ++i) {
      (*input_shapes)[i].assign(model.input_shape(i).shape().begin(),
                                model.input_shape(i).shape().end());
    }
    output_shapes->resize(model.output_shape_size());
    for (int i = 0; i < model.output_shape_size(); ++i) {
      (*output_shapes)[i].assign(model.output_shape(i).shape().begin(),
                                 model.output_shape(i).shape().end());
    }
    const aibench::PreProcessor &pre_processor = model.pre_processor();
    *pre_type = pre_processor.type();
    input_means->resize(pre_processor.input_mean_size());
    for (int i = 0; i < pre_processor.input_mean_size(); ++i) {
      (*input_means)[i].assign(pre_processor.input_mean(i).mean().begin(),
                               pre_processor.input_mean(i).mean().end());
    }
    input_var->assign(pre_processor.var().begin(), pre_processor.var().end());
    *post_type = model.post_processor().type();
    *metric_type = model.metric_evaluator().type();
    return SUCCESS;
  }

  return NOT_SUPPORTED;
}

class PreProcessorFactory {
 public:
  static std::unique_ptr<PreProcessor> CreatePreProcessor(
      PreProcessor_PreProcessorType type,
      const std::vector<DataFormat> &data_formats,
      const std::vector<std::vector<float>> &input_means,
      const std::vector<float> &input_var,
      const ChannelOrder channel_order) {
    std::unique_ptr<PreProcessor> processor;

    switch (type) {
      case PreProcessor_PreProcessorType_DefaultProcessor:
        processor.reset(new ImageNetPreProcessor(data_formats,
                                                 input_means,
                                                 input_var,
                                                 channel_order));
        break;
      default:
        LOG(FATAL) << "Not supported PreProcessor type: " << type;
    }

    return std::move(processor);
  }
};

class PostProcessorFactory {
 public:
  static std::unique_ptr<PostProcessor> CreatePostProcessor(
      PostProcessor_PostProcessorType type) {
    std::unique_ptr<PostProcessor> processor;

    switch (type) {
      case PostProcessor_PostProcessorType_ImageClassification:
        processor.reset(new ImageNetPostProcessor());
        break;
      default:
        LOG(FATAL) << "Not supported PostProcessor type: " << type;
    }

    return std::move(processor);
  }
};

}  // namespace

int Main(int argc, char **argv) {
  std::string usage = "run benchmark, e.g. " + std::string(argv[0]) +
      " [flags]";
  gflags::SetUsageMessage(usage);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  auto benchmark_option = static_cast<BenchmarkOption>(FLAGS_benchmark_option);
  auto executor_type = static_cast<ExecutorType>(FLAGS_executor);
  auto device_type = static_cast<DeviceType>(FLAGS_device_type);
  auto model_name = static_cast<ModelName>(FLAGS_model_name);

  ChannelOrder channel_order;
  std::vector<DataFormat> data_formats;
  std::vector<std::vector<int64_t>> input_shapes;
  std::vector<std::vector<int64_t>> output_shapes;
  std::vector<std::vector<float>> input_means;
  std::vector<float> input_var;
  PreProcessor_PreProcessorType pre_processor_type;
  PostProcessor_PostProcessorType post_processor_type;
  MetricEvaluator_MetricEvaluatorType metric_evaluator_type;
  if (GetModelBaseInfo("model.pb",
                       &channel_order,
                       &data_formats,
                       &input_shapes,
                       &output_shapes,
                       &input_means,
                       &input_var,
                       &pre_processor_type,
                       &post_processor_type,
                       &metric_evaluator_type) != SUCCESS) {
    LOG(FATAL) << "Model info parse failed";
  }

  std::vector<std::string> input_names;
  std::vector<std::string> output_names;
  std::string model_file;
  std::string weight_file;
  if (GetBenchInfo("benchmark.pb",
                   &input_names,
                   &output_names,
                   &output_shapes,
                   &model_file,
                   &weight_file) != Status::SUCCESS) {
    LOG(FATAL) << "Bench info parse failed";
  }

  std::unique_ptr<aibench::BaseExecutor> executor;
#ifdef AIBENCH_ENABLE_MACE
  if (executor_type == aibench::MACE) {
    executor.reset(new aibench::MaceExecutor(device_type,
                                             model_file,
                                             weight_file,
                                             input_names,
                                             output_names));
  }
#endif
#ifdef AIBENCH_ENABLE_SNPE
  if (executor_type == aibench::SNPE) {
    executor.reset(new aibench::SnpeExecutor(device_type, model_file));
  }
#endif
#ifdef AIBENCH_ENABLE_NCNN
  if (executor_type == aibench::NCNN) {
    executor.reset(new aibench::NcnnExecutor(model_file));
  }
#endif
#ifdef AIBENCH_ENABLE_TFLITE
  if (executor_type == aibench::TFLITE) {
    executor.reset(new aibench::TfLiteExecutor(model_file));
  }
#endif

  std::unique_ptr<Benchmark> benchmark;
  if (benchmark_option == BenchmarkOption::Performance) {
    benchmark.reset(new PerformanceBenchmark(executor.get(),
                                             model_name,
                                             FLAGS_quantize,
                                             input_names,
                                             input_shapes,
                                             output_names,
                                             output_shapes,
                                             FLAGS_run_interval,
                                             FLAGS_num_threads));
  } else {
    std::unique_ptr<PreProcessor> pre_processor =
        PreProcessorFactory::CreatePreProcessor(pre_processor_type,
                                                data_formats,
                                                input_means,
                                                input_var,
                                                channel_order);
    std::unique_ptr<PostProcessor> post_processor =
        PostProcessorFactory::CreatePostProcessor(post_processor_type);
    benchmark.reset(new PrecisionBenchmark(executor.get(),
                                           model_name,
                                           FLAGS_quantize,
                                           input_names,
                                           input_shapes,
                                           output_names,
                                           output_shapes,
                                           FLAGS_run_interval,
                                           FLAGS_num_threads,
                                           std::move(pre_processor),
                                           std::move(post_processor),
                                           metric_evaluator_type));
  }

  Status status = benchmark->Run();
  return status;
}

}  // namespace benchmark
}  // namespace aibench

int main(int argc, char **argv) { aibench::benchmark::Main(argc, argv); }
