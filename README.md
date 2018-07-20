<div align="center">
<img src="logo.png" width="400" alt="Mobile AI Bench" />
</div>

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![pipeline status](https://gitlab.com/llhe/mobile-ai-bench/badges/master/pipeline.svg)](https://gitlab.com/llhe/mobile-ai-bench/pipelines)

[FAQ](#FAQ) |
[中文](README_zh.md)

In recent years, the on-device deep learning applications are getting more and
more popular. It's a challenging task for application developers to deploy their
deep learning models in their applications. They need to choose a proper
inference framework, optionally utilizing quantization or compression
techniques regarding the precision-performance trade-off, and finally
run the model on one or more of heterogeneous computing devices. How to make an
appropriate decision among these choices is a tedious and time-consuming task.

**Mobile AI Benchmark** (i.e. **MobileAIBench**) is an end-to-end 
benchmark tool to test the models' runtime in the different neural network 
frameworks on mobile devices, which hopefully can provide insights for the 
developers.

## Daily Benchmark Results
Please check *benchmark* step in [daily CI pipeline page](https://gitlab.com/llhe/mobile-ai-bench/pipelines).

## Environment requirement

MobileAIBench supports several deep learning frameworks ([MACE](https://github.com/XiaoMi/mace), [SNPE](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk), [ncnn](https://github.com/Tencent/ncnn) and [TensorFlow Lite](https://github.com/tensorflow/tensorflow/tree/master/tensorflow/contrib/lite)) currently, which may require the following dependencies:

| Software  | Installation command  | Tested version  |
| :-------: | :-------------------: | :-------------: |
| Python  |   | 2.7  |
| ADB  | apt-get install android-tools-adb  | Required by Android run, >= 1.0.32  |
| Android NDK  | [NDK installation guide](https://developer.android.com/ndk/guides/setup#install) | Required by Android build, r15c |
| Bazel  | [bazel installation guide](https://docs.bazel.build/versions/master/install.html)  | 0.13.0  |
| CMake  | apt-get install cmake  | >= 3.11.3  |
| FileLock  | pip install -I filelock==3.0.0  | Required by Android run  |
| PyYaml  | pip install -I pyyaml==3.12  | 3.12.0  |
| sh  | pip install -I sh==1.12.14  | 1.12.14  |
| SNPE (optional) | [download](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk) and uncompress  | 1.15.0  |

**Note:** [SNPE](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk)
has strict license that disallows redistribution, so the default link in the
Bazel `WORKSPACE` file is only accessible by the CI server. To benchmark SNPE
in your local system (i.e. set `--frameworks` with `all` or `SNPE` explicitly),
you need to download the SDK [here](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk),
uncompress it and modify `WORKSPACE` as the following:
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

## Architecture
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

## How To Use

### Benchmark all models on all frameworks
```
python tools/benchmark.py --output_dir=output --frameworks=all \
                          --runtimes=all --model_names=all \
                          --target_abis=armeabi-v7a,arm64-v8a
```
The whole benchmark may take a few time, and continuous benchmarking may heat
the device very quickly, so you may set the following arguments according to your
interests. 

| option         | type | default     | explanation |
| :-----------:  | :--: | :----------:| ------------|
| --output_dir   | str  | output      | Benchmark output directory. |
| --frameworks   | str  | all         | Frameworks(MACE/SNPE/NCNN/TFLITE), comma separated list or all. |
| --runtimes     | str  | all         | Runtimes(CPU/GPU/DSP), comma separated list or all. |
| --target_abis  | str  | armeabi-v7a | Target ABIs(armeabi-v7a,arm64-v8a), comma separated list. |
| --model_names  | str  | all         | Model names(InceptionV3,MobileNetV1...), comma separated list or all. |
| --run_interval | int  | 10          | Run interval between benchmarks, seconds. |
| --num_threads  | int  | 4           | The number of threads. |


### Adding a model to run on existing framework
* Register model benchmark

	Register benchmark in `aibench/benchmark/benchmark_main.cc`:
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
* Register model in `tools/model_list.py`.

* Configure model file and input file 

	Configure `MODEL_FILE` and `INPUT_FILE` in `tools/model_and_input.yml`.

* Run benchmark
	```
	python tools/benchmark.py --output_dir=output --frameworks=MACE \
	                          --runtimes=CPU --model_names=MobileNetV1 \
	                          --target_abis=armeabi-v7a,arm64-v8a
	```
	
* Check benchmark result
	```bash
	cat output/report.csv
	```


### Adding your new AI framework

* Define `executor` and implement the interfaces:

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

* Include your framework header in `aibench/benchmark/benchmark_main.cc`:

    ```c++
    #ifdef AIBENCH_ENABLE_YOUR_FRAMEWORK
    #include "aibench/executors/your_framework/your_framework_executor.h"
    #endif
    ```
    
* Add dependencies to `third_party/your_framework`, `aibench/benchmark/BUILD` and `WORKSPACE`.
    Put macro `AIBENCH_ENABLE_YOUR_FRAMEWORK` into `aibench/benchmark/BUILD` at `model_benchmark` target. 

* Benchmark a model on existing framework

	Refer to [Adding a model to run on existing framework](#Adding a model to run on existing framework).
    
## FAQ
Q: Why are benchmark results not stable on my device?

A: Benchmark results highly depend on states of devices, e.g., running processes, temperature, power control policy.
It is recommended to disable power control policy (as shown in `tools/power.sh`) if possible (e.g., rooted phone).
Otherwise, keep your device at idle state with low temperature, and benchmark one model on one framework each time.

Q: Why does NCNN take so little time during initialization?

A: NCNN benchmark does not load weights from real models.

## License
[Apache License 2.0](LICENSE).

## Notice
For [third party](third_party) dependencies, please refer to their licenses.
