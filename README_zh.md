<div align="center">
<img src="logo.png" width="400" alt="Mobile AI Bench" />
</div>

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![pipeline status](https://gitlab.com/llhe/mobile-ai-bench/badges/master/pipeline.svg)](https://gitlab.com/llhe/mobile-ai-bench/pipelines)

[English](README.md)

近年来，智能手机以及IoT设备上的离线深度学习应用变得越来越普遍。在设备上部署深度学习模型给开发者带来挑战，对于手机应用开发者，需要在众多深度学习框架中选择一款合适的框架，对于IoT硬件开发者而言，则还需要从不同的芯片方案中做出选择。同时，除了软硬件的选择之外，开发者还需要从模型量化压缩与模型精度角度进行权衡，最终将模型部署到设备上。对比测试这些不同的芯片，框架以及量化方案，并从中选择最佳组合是一个非常繁琐耗时的工作。

**MobileAIBench** 是一个端到端的测试工具，用于评测相同的模型在不同硬件和软件框架上运行的性能和精度表现，对开发者的技术选型给出客观参考数据。

## 每日评测结果
请查看最新的[CI Pipeline页面](https://gitlab.com/llhe/mobile-ai-bench/pipelines)中的*benchmark*步骤的运行结果。

## FAQ
参考英文文档。

## 准备环境

MobileAIBench 现在支持多种框架 ([MACE](https://github.com/XiaoMi/mace), [SNPE](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk), [ncnn](https://github.com/Tencent/ncnn) 以及 [TensorFlow Lite](https://github.com/tensorflow/tensorflow/tree/master/tensorflow/contrib/lite))，需要安装以下的依赖：

| 依赖  | 安装命令  | 验证可用的版本  |
| :-------: | :-------------------: | :-------------: |
| Python  |   | 2.7  |
| ADB  | apt-get install android-tools-adb  | Required by Android run, >= 1.0.32  |
| Android NDK  | [NDK installation guide](https://developer.android.com/ndk/guides/setup#install) | Required by Android build, r15c |
| Bazel  | [bazel installation guide](https://docs.bazel.build/versions/master/install.html)  | 0.13.0  |
| CMake  | apt-get install cmake  | >= 3.11.3  |
| FileLock  | pip install -I filelock==3.0.0  | Required by Android run  |
| PyYaml  | pip install -I pyyaml==3.12  | 3.12.0  |
| sh  | pip install -I sh==1.12.14  | 1.12.14  |
| SNPE (可选)  | [下载](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk)并解压 | 1.18.0  |

**备注:** 鉴于SNPE的许可不允许第三方再分发, 目前Bazel WORKSPACE配置中的链接只能在CI Server中访问。
如果想测评SNPE(通过`--executors`指定`all`或者显式指定了`SNPE`)
，需从[官方地址](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk)
下载并解压，[复制libgnustl_shared.so](https://developer.qualcomm.com/docs/snpe/setup.html)，然后修改`WORKSPACE`文件如下。
```python
#new_http_archive(
#    name = "snpe",
#    build_file = "third_party/snpe/snpe.BUILD",
#    sha256 = "8f2b92b236aa7492e4acd217a96259b0ddc1a656cbc3201c7d1c843e1f957e77",
#    strip_prefix = "snpe-1.22.2.233",
#    urls = [
#        "https://cnbj1-fds.api.xiaomi.net/aibench/third_party/snpe-1.22.2_with_libgnustl_shared.so.zip",
#    ],
#)

new_local_repository(
    name = "snpe",
    build_file = "third_party/snpe/snpe.BUILD",
    path = "/path/to/snpe",
)
```

## 数据结构
```
+-----------------+         +------------------+      +---------------+
|   Benchmark     |         |   BaseExecutor   | <--- | MaceExecutor  |
+-----------------+         +------------------+      +---------------+
| - executor      |-------> | - executor       |
| - model_name    |         | - device_type    |      +---------------+
| - quantize      |         |                  | <--- | SnpeExecutor  |
| - input_names   |         +------------------+      +---------------+
| - input_shapes  |         | + Init()         |
| - output_names  |         | + Prepare()      |      +---------------+
| - output_shapes |         | + Run()          | <--- | NcnnExecutor  |
| - run_interval  |         | + Finish()       |      +---------------+
| - num_threads   |         |                  |
+-----------------+         +------------------+      +---------------+
| - Run()         |                              <--- | TfLiteExecutor|
+-----------------+                                   +---------------+
        ^     ^             +--------------------+     
        |     |             |PerformanceBenchmark|      
        |     --------------+--------------------+       
        |                   | - Run()            |       
        |                   +--------------------+ 
        |
        |                   +---------------+      +---------------------+                           
+--------------------+ ---> |PreProcessor   | <--- |ImageNetPreProcessor |                       
| PrecisionBenchmark |      +---------------+      +---------------------+                         
+--------------------+                           
| - pre_processor    |      +---------------+      +---------------------+                         
| - post_processor   | ---> |PostProcessor  | <--- |ImageNetPostProcessor|                       
| - metric_evaluator |      +---------------+      +---------------------+                    
+--------------------+                       
| - Run()            |      +---------------+                     
+--------------------+ ---> |MetricEvaluator|                     
                            +---------------+    
```

## 如何使用

### 测试所有模型在所有框架上的性能

```bash
bash tools/benchmark.sh --benchmark_option=Performance \
                        --target_abis=armeabi-v7a,arm64-v8a
```

运行时间可能比较长，如果只想测试指定模型和框架，可以添加如下选项。当前只有MACE支持精度测试。 

| option         | type | default     | explanation |
| :-----------:  | :--: | :----------:| ------------|
| --benchmark_option | str | Performance | Benchmark options, Performance/Precision. |
| --output_dir   | str  | output      | Benchmark output directory. |
| --executors    | str  | all         | Executors(MACE/SNPE/NCNN/TFLITE), comma separated list or all. |
| --device_types | str  | all         | DeviceTypes(CPU/GPU/DSP), comma separated list or all. |
| --target_abis  | str  | armeabi-v7a | Target ABIs(armeabi-v7a,arm64-v8a), comma separated list. |
| --model_names  | str  | all         | Model names(InceptionV3,MobileNetV1...), comma separated list or all. |
| --run_interval | int  | 10          | Run interval between benchmarks, seconds. |
| --num_threads  | int  | 4           | The number of threads. |
| --input_dir    | str  | ""          | Input data directory for precision benchmark. |


### 在已有框架中添加新模型评测

* 在`aibench/proto/base.proto`添加新模型名。

* 在 `aibench/proto/model.meta`配置模型信息。

* 在`aibench/proto/benchmark.meta`配置benchmark信息。 

* 运行测试
    性能benchmark:

    ```bash
    bash tools/benchmark.sh --benchmark_option=Performance \
                            --executors=MACE --device_types=CPU --model_names=MobileNetV1 \
                            --target_abis=armeabi-v7a,arm64-v8a
    ```

    精度benchmark。当前仅支持以ImageNet图像为输入测试MACE精度。

    ```bash
    bash tools/benchmark.sh --benchmark_option=Precision --input_dir=/path/to/inputs \
                            --executors=MACE --device_types=CPU --model_names=MobileNetV1 \
                            --target_abis=armeabi-v7a,arm64-v8a
    ```
    
* 查看结果

    ```bash
    cat output/report.csv
    ```


### 加入新的 AI 框架

* 定义 `executor` 并实现其接口:

    ```c++
    class YourExecutor : public BaseExecutor {
     public:
      YourExecutor() :
          BaseExecutor(executor_type, device_type, model_file, weight_file) {}
      
      // Init method should invoke the initializing process for your executor 
      // (e.g.  Mace needs to compile OpenCL kernel once per target). It will be
      // called only once when creating executor engine.
      virtual Status Init(int num_threads);
       // Load model and prepare to run. It will be called only once before 
      // benchmarking the model.
      virtual Status Prepare();
      
      // Run the model. It will be called more than once.
      virtual Status Run(const std::map<std::string, BaseTensor> &inputs,
                         std::map<std::string, BaseTensor> *outputs);
      
      // Unload model and free the memory after benchmarking. It will be called
      // only once.
      virtual void Finish();
    };
    ```

* 在 `aibench/benchmark/benchmark_main.cc` 中包含头文件:

    ```c++
    #ifdef AIBENCH_ENABLE_YOUR_EXECUTOR
    #include "aibench/executors/your_executor/your_executor.h"
    #endif
    ```
    
* 添加依赖 `third_party/your_executor`, `aibench/benchmark/BUILD` 和 `WORKSPACE`。

* 测试模型

    [在已有框架中添加新模型评测](#在已有框架中添加新模型评测)。
    
## License
[Apache License 2.0](LICENSE)。

