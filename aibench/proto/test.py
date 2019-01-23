from google.protobuf import text_format

from aibench.proto import base_pb2
from aibench.proto import aibench_pb2

model_factory = aibench_pb2.ModelFactory()
bench_factory = aibench_pb2.BenchFactory()

try:
    with open('aibench/proto/model.meta', 'rb') as fin:
        file_content = fin.read()
        text_format.Parse(file_content, model_factory)
        print(str(model_factory))
    with open('aibench/proto/benchmark.meta', 'rb') as fin:
        file_content = fin.read()
        text_format.Parse(file_content, bench_factory)
        print(str(bench_factory))
except text_format.ParseError as e:
    raise IOError("Cannot parse file.", e)
