<div align="center">
<img src="logo.png" width="400" alt="Mobile AI Bench" />
</div>

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![build status](http://v9.git.n.xiaomi.com/deep-computing/mobile-ai-bench/badges/master/build.svg)](http://v9.git.n.xiaomi.com/deep-computing/mobile-ai-bench/commits/master)

[English](README.md)

近几年，设备上的深度学习应用越来越普遍。在应用中部署深度学习模型给开发者带来挑战。开发者们需要选择一个合适的框架，
选择性地利用量化压缩技术与模型精度进行权衡，最终将模型部署到设备上。对比测试这些框架，并从中选择是一个繁琐耗时的工作。

**MobileAIBench** 是一个端到端的测试工具，用于评测同一模型在不同框架上运行的性能表现，
希望测评结果可以提供给开发者一些指导。


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
| SNPE (可选)  | [下载](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk)并解压 | 1.15.0  |

**备注:** 鉴于SNPE的许可不允许第三方再分发, 目前Bazel WORKSPACE配置中的链接只能在CI Server中访问。
如果想测评SNPE(通过`--frameworks`指定`all`或者显式指定了`SNPE`)
，需从[官方地址](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk)
下载并解压，然后修改`WORKSPACE`文件如下。
```python
#new_http_archive(
#    name = "snpe",
#    build_file = "third_party/snpe/snpe.BUILD",
#    sha256 = "b11780e5e7f591e916c69bdface4a1ef75b0c19f7b43c868bd62c0f3747d3fbb",
#    strip_prefix = "snpe-1.15.0",
#    urls = [
#        "https://cnbj1-fds.api.xiaomi.net/aibench/third_party/snpe-1.15.0.zip",
#    ],
#)

new_local_repository(
    name = "snpe",
    build_file = "third_party/snpe/snpe.BUILD",
    path = "/path/to/snpe-1.15.0",
)
```

## 数据结构
```
+-----------------+         +------------------+      +---------------+
|   Benchmark     |         |   BaseExecutor   | <--- | MaceExecutor  |
+-----------------+         +------------------+      +---------------+
| - executor      |-------> | - framework      |
| - model_name    |         | - runtime        |      +---------------+
| - model_file    |         |                  | <--- | SnpeExecutor  |
| - input_names   |         +------------------+      +---------------+
| - input_files   |         | + Init()         |
| - input_shapes  |         | + Prepare()      |      +---------------+
| - output_names  |         | + Run()          | <--- | NcnnExecutor  |
| - output_shapes |         | + Finish()       |      +---------------+
+-----------------+         +------------------+               
| - Register()    |                                   +---------------+
| - Run()         |                              <--- | TfLiteExecutor|
+-----------------+                                   +---------------+

```

## 如何使用

### 测试所有模型在所有框架上的性能
```
python tools/benchmark.py --output_dir=output --frameworks=all \
                          --runtimes=all --model_names=all \
                          --target_abis=armeabi-v7a,arm64-v8a
```
运行时间可能比较长，如果只想测试指定模型和框架，可以添加如下选项： 

| option         | type | default     | explanation |
| :-----------:  | :--: | :----------:| ------------|
| --output_dir   | str  | output      | Benchmark output directory. |
| --frameworks   | str  | all         | Frameworks(MACE/SNPE/NCNN/TFLITE), comma separated list or all. |
| --runtimes     | str  | all         | Runtimes(CPU/GPU/DSP), comma separated list or all. |
| --target_abis  | str  | armeabi-v7a | Target ABIs(armeabi-v7a,arm64-v8a), comma separated list. |
| --model_names  | str  | all         | Model names(InceptionV3,MobileNetV1...), comma separated list or all. |
| --run_interval | int  | 10          | Run interval between benchmarks, seconds. |
| --num_threads  | int  | 4           | The number of threads. |


### 在已有框架中添加新模型评测
* 注册模型

	在 `aibench/benchmark/benchmark_main.cc` 中添加:
	```c++
	    #ifdef AIBENCH_ENABLE_YOUR_FRAMEWORK
	    std::unique_ptr<aibench::YourFrameworkExecutor>
	        your_framework_executor(new aibench::YourFrameworkExecutor());
	    AIBENCH_BENCHMARK(your_framework_executor.get(), MODEL_NAME, FRAMEWORK_NAME, RUNTIME,
	                      MODEL_FILE, (std::vector<std::string>{INPUT_NAME}),
	                      (std::vector<std::string>{INPUT_FILE}),
	                      (std::vector<std::vector<int64_t>>{INPUT_SHAPE}),
	                      (std::vector<std::string>{OUTPUT_NAME}),
	                      (std::vector<std::vector<int64_t>>{OUTPUT_SHAPE}));
	    #endif
	```
	e.g.
	```c++
	  AIBENCH_BENCHMARK(mobilenetv1_mace_cpu_executor.get(), MobileNetV1, MACE,
	                    CPU, mobilenet_v1, (std::vector<std::string>{"input"}),
	                    (std::vector<std::string>{"dog.npy"}),
	                    (std::vector<std::vector<int64_t>>{{1, 224, 224, 3}}),
	                    (std::vector<std::string>{
	                        "MobilenetV1/Predictions/Reshape_1"}),
	                    (std::vector<std::vector<int64_t>>{{1, 1001}}));
	```
* 在 `tools/model_list.py` 中注册模型名称
* 配置模型文件和输入文件 

	在 `tools/model_and_input.yml` 中配置 `MODEL_FILE` 和 `INPUT_FILE`。 

* 运行测试
	```
	python tools/benchmark.py --output_dir=output --frameworks=MACE \
	                          --runtimes=CPU --model_names=MobileNetV1 \
	                          --target_abis=armeabi-v7a,arm64-v8a
	```
	
* 查看结果
	```bash
	cat output/report.csv
	```


### 加入新的 AI 框架

* 定义 `executor` 并实现其接口:

    ```c++
    class YourFrameworkExecutor : public BaseExecutor {
     public:
      YourFrameworkExecutor() : BaseExecutor(FRAMEWORK_NAME, RUNTIME) {}
      
      // Init method should invoke the initializing process for your framework 
      // (e.g.  Mace needs to compile OpenCL kernel once per target). It will be
      // called only once when creating framework engine.
      virtual Status Init(const char *model_name, int num_threads);

      // Load model and prepare to run. It will be called only once before 
      // benchmarking the model.
      virtual Status Prepare(const char *model_name);
      
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
    #ifdef AIBENCH_ENABLE_YOUR_FRAMEWORK
    #include "aibench/executors/your_framework/your_framework_executor.h"
    #endif
    ```
    
* 添加依赖 `third_party/your_framework`, `aibench/benchmark/BUILD` 和 `WORKSPACE`.  

* 测试模型

	[在已有框架中添加新模型评测](#在已有框架中添加新模型评测).
    

## License
[Apache License 2.0](LICENSE).

