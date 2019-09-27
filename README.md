<div align="center">
<img src="logo.png" width="400" alt="Mobile AI Bench" />
</div>

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![pipeline status](https://gitlab.com/llhe/mobile-ai-bench/badges/master/pipeline.svg)](https://gitlab.com/llhe/mobile-ai-bench/pipelines)

[FAQ](#FAQ) |
[中文](README_zh.md)

In recent years, the on-device deep learning applications are getting more and
more popular on mobile phones or IoT devices. It's a challenging task for the developers to deploy their
deep learning models in their mobile applications or IoT devices.

They need to optionally choose a cost-effective hardware solution (i.e. chips and boards),
then a proper inference framework, optionally utilizing quantization or compression
techniques regarding the precision-performance trade-off, and finally
run the model on one or more of heterogeneous computing devices. How to make an
appropriate decision among these choices is a tedious and time-consuming task.

**Mobile AI Benchmark** (i.e. **MobileAIBench**) is an end-to-end benchmark tool
which covers different chips and inference frameworks, with results
include both speed and model accuracy, which will give insights for developers.

## Daily Benchmark Results
Please check *benchmark* step in [daily CI pipeline page](https://gitlab.com/llhe/mobile-ai-bench/pipelines), due to the lack of test devices, the CI result may not cover all hardwares and frameworks.

## FAQ
**Q: Why are benchmark results not stable on my device?**

**A**: Due to power save considerations, some SoCs have aggressive and advanced
power control scheduling to reduce power consumption which make performance
quite unstable (especially CPU). Benchmark results highly depend on
states of devices, e.g., running processes, temperature, power control policy.
It is recommended to disable power control policy (as shown in `tools/power.sh`) if possible (e.g., rooted phone).
Otherwise, keep your device at idle state with low temperature, and benchmark one model on one framework each time.

**Q: Why do some devices run faster (or slower) than expected in the CI benchmark result?**

**A**: Some devices is rooted and has some specialized performance tuning while some
others is not rooted and failed to make such tuning (see the code for more details).

**Q: Why is ncnn initialization time much less than others?**

**A**: ncnn benchmark uses fake model parameters and skips loading weights from filesystem.

**Q: Does benchmark use all available cores of devices?**

**A**: Most modern Android phones use [ARM big.LITTLE](https://en.wikipedia.org/wiki/ARM_big.LITTLE) architecture which can lead to significant variance between different runs of the benchmark, we use only available big cores to reduce this variance by `taskset` command for MACE/NCNN/TFLITE benchmark.
Moreover, there are no well-defined APIs for SNPE to bind to big cores and set thread count.
Thread count can be set by adding `--num_threads` to `tools/benchmark.sh` command.


## Environment requirement

MobileAIBench supports several deep learning frameworks (called `executor` in this project, i.e., [MACE](https://github.com/XiaoMi/mace), [SNPE](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk), [ncnn](https://github.com/Tencent/ncnn), [TensorFlow Lite](https://github.com/tensorflow/tensorflow/tree/master/tensorflow/contrib/lite) and [HIAI](https://developer.huawei.com/consumer/en/devservice/doc/2020314)) currently, which may require the following dependencies:

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
| SNPE (optional) | [download](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk) and uncompress  | 1.18.0  |

**Note 1:** [SNPE](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk)
has strict license that disallows redistribution, so the default link in the
Bazel `WORKSPACE` file is only accessible by the CI server. To benchmark SNPE
in your local system (i.e. set `--executors` with `all` or `SNPE` explicitly),
you need to download the SDK [here](https://developer.qualcomm.com/software/qualcomm-neural-processing-sdk),
uncompress it, [copy libgnustl_shared.so](https://developer.qualcomm.com/docs/snpe/setup.html)
 and modify `WORKSPACE` as the following:
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

**Note 2:** [HIAI](https://developer.huawei.com/consumer/en/devservice/doc/2020301)
has strict license that disallows redistribution, so the default link in the
Bazel `WORKSPACE` file is only accessible by the CI server. To benchmark HIAI
in your local system (i.e. set `--executors` with `all` or `HIAI` explicitly),
you need to login and download the SDK [here](https://developer.huawei.com/consumer/en/devservice/doc/2020301),
uncompress it and get the `HiAI_DDK_100.200.010.011.zip` file, uncompress it
 and modify `WORKSPACE` as the following:
```python
#new_http_archive(
#    name = "hiai",
#    build_file = "third_party/hiai/hiai.BUILD",
#    sha256 = "8da8305617573bc495df8f4509fcb1655ffb073d790d9c0b6ca32ba4a4e41055",
#    strip_prefix = "HiAI_DDK_100.200.010.011",
#    type = "zip",
#    urls = [
#        "http://cnbj1.fds.api.xiaomi.com/aibench/third_party/HiAI_DDK_100.200.010.011_LITE.zip",
#    ],
#)

new_local_repository(
    name = "hiai",
    build_file = "third_party/hiai/hiai.BUILD",
    path = "/path/to/hiai",
)
```

## Architecture
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
+-----------------+         |                  |      +---------------+
| - Run()         |         |                  | <--- | TfLiteExecutor|
+-----------------+         |                  |      +---------------+
        ^     ^             |                  |
        |     |             |                  |      +---------------+
        |     |             |                  | <--- | HiaiExecutor  |
        |     |             +------------------+      +---------------+
        |     |
        |     |             +--------------------+
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

## How To Use

### Benchmark Performance of all models on all executors

```bash
bash tools/benchmark.sh --benchmark_option=Performance \
                        --target_abis=armeabi-v7a,arm64-v8a,aarch64,armhf
```

The whole benchmark may take a few time, and continuous benchmarking may heat
the device very quickly, so you may set the following arguments according to your
interests. Only MACE supports precision benchmark right now.

| option         | type | default     | explanation |
| :-----------:  | :--: | :----------:| ------------|
| --benchmark_option | str | Performance | Benchmark options, Performance/Precision. |
| --output_dir   | str  | output      | Benchmark output directory. |
| --executors    | str  | all         | Executors(MACE/SNPE/NCNN/TFLITE/HIAI), comma separated list or all. |
| --device_types | str  | all         | DeviceTypes(CPU/GPU/DSP/NPU), comma separated list or all. |
| --target_abis  | str  | armeabi-v7a | Target ABIs(armeabi-v7a,arm64-v8a,aarch64,armhf), comma separated list. |
| --model_names  | str  | all         | Model names(InceptionV3,MobileNetV1...), comma separated list or all. |
| --run_interval | int  | 10          | Run interval between benchmarks, seconds. |
| --num_threads  | int  | 4           | The number of threads. |
| --input_dir    | str  | ""          | Input data directory for precision benchmark. |

### Configure ssh devices
For embedded ARM-Linux devices whose abi is aarch64 or armhf, ssh connection is supported.
Configure ssh devices in `generic-mobile-devices/devices_for_ai_bench.yml`, for example:
```yaml
devices:
  nanopi:
    target_abis: [aarch64, armhf]
    target_socs: RK3333
    models: Nanopi M4
    address: 10.231.46.118
    username: pi
```

### Adding a model to run on existing executor

* Add the new model name in `aibench/proto/base.proto` if not in there.

* Configure the model info in `aibench/proto/model.meta`.

* Configure the benchmark info in `aibench/proto/benchmark.meta`.

* Run benchmark

    Performance benchmark.

    ```bash
    bash tools/benchmark.sh --benchmark_option=Performance \
                            --executors=MACE --device_types=CPU --model_names=MobileNetV1 \
                            --target_abis=armeabi-v7a,arm64-v8a,aarch64,armhf
    ```

    Precision benchmark. Only supports ImageNet images as inputs for benchmarking MACE precision.

    ```bash
    bash tools/benchmark.sh --benchmark_option=Precision --input_dir=/path/to/inputs \
                            --executors=MACE --device_types=CPU --model_names=MobileNetV1 \
                            --target_abis=armeabi-v7a,arm64-v8a,aarch64,armhf
    ```
* Check benchmark result

    ```bash
    python report/csv_to_html.py
    ```

  Open the corresponding link in a browser to see the report.


### Adding a new AI executor

* Define `executor` and implement the interfaces:

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

* Include your executor header in `aibench/benchmark/benchmark_main.cc`:

    ```c++
    #ifdef AIBENCH_ENABLE_YOUR_EXECUTOR
    #include "aibench/executors/your_executor/your_executor.h"
    #endif
    ```
    
* Add dependencies to `third_party/your_executor`, `aibench/benchmark/BUILD` and `WORKSPACE`.
    Put macro `AIBENCH_ENABLE_YOUR_EXECUTOR` into `aibench/benchmark/BUILD` at `model_benchmark` target. 

* Benchmark a model on existing executor

    Refer to [Adding a model to run on existing executor](#Adding a model to run on existing executor).

## License
[Apache License 2.0](LICENSE).

## Notice
For [third party](third_party) dependencies, please refer to their licenses.
