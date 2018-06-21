MobileNNBench
=============
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![build status](http://v9.git.n.xiaomi.com/deep-computing/mobile-nn-bench/badges/master/build.svg)](http://v9.git.n.xiaomi.com/deep-computing/mobile-nn-bench/commits/master)

In recent years, the on device deep learning applications are getting more and
more popular. It's a challenging task for application developers to deploy their
deep learning models in their applications. They need to choose proper
inference framework, optionally utilizing quantization or compression
techniques regarding to the precision-performance trade-off, and finally
run the model on one or more of heterogeneous compute devices. How to make an
appropriate decision among these choices is a tedious and time consuming task.

The puropse of this project is to provide an end-to-end neural network benchmark
on mobile devices, which hopefully can provide insights for the developers.

## How to use
```
python tools/benchmark.py --output_dir=output --frameworks=all \
                          --runtimes=all --model_names=all
```
## Archetecture
```
+---------------+         +------------------+      +------------------+
|   Benchmark   |         |   BaseExecutor   | <--- | MaceGpuExecutor  |
+---------------+         +------------------+      +------------------+
| - executor    |-------> | - framework      |
| - model_name  |         | - runtime        |      +------------------+
| - model_file  |         |                  | <--- | SnpeGpuExecutor  |
| - input_names |         +------------------+      +------------------+
| - input_files |         | + Prepare()      |
| - input_shapes|         | + Run()          |      +------------------+
+---------------+         | + Finish()       | <--- |  TfLiteExecutor  |
| - Register()  |         +------------------+      +------------------+
| - Run()       |                                            .
+---------------+                                            .
                                                             .
```
## Adding a new NN framework

1. Add dependencies in `third_party/new_framework` and WORKSPACE.

2. Define executor and implement the interfaces:

    ```c++
    class NewFrameworkExecutor : public BaseExecutor {
     public:
      NewFrameworkExecutor() : BaseExecutor(FRAMEWORK_NAME, CPU) {}
      
      // Load model and prepare to run
      virtual Status Prepare(const char *model_name);
      
      // Run the model
      virtual Status Run(const std::map<std::string, BaseTensor> &inputs,
                         std::map<std::string, BaseTensor> *outputs);
      
      // Unload model and free the memory after running the model
      virtual void Finish();
    };
    ```

3. Register a new Benchmark in `nnbench/benchmark/benchmark_main.cc`

    ```c++
    std::unique_ptr<nnbench::NewFrameworkExecutor>
        new_framework_executor(new nnbench::NewFrameworkExecutor());
    NNBENCH_BENCHMARK(new_framework_executor.get(), MODEL_NAME, FRAMEWORK_NAME, CPU,
                      MODEL_FILE, (std::vector<std::string>{INPUT_NAMES}),
                      (std::vector<std::string>{INPUT_FILES}),
                      (std::vector<std::vector<int64_t>>{INPUT_SHAPES}));
    ```
   MODEL_FILE and INPUT_FILES can be configured in `tools/model_and_input.yml`


## License
[Apache License 2.0](LICENSE).