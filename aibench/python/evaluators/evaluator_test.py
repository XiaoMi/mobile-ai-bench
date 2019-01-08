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


import unittest
import tempfile

from aibench.python.utils import common
from aibench.python.evaluators.coco_evaluator import COCOObjectDetectionEvaluator  # noqa

FAKE_BBOX_FILE = "https://raw.githubusercontent.com/cocodataset/cocoapi/master/results/instances_val2014_fakebbox100_results.json"  # noqa


class TestEvaluator(unittest.TestCase):

    def test_coco_object_detection_evaluator(self):
        coco_detection_evaluator = COCOObjectDetectionEvaluator()
        coco_detection_evaluator.prepare_dataset()
        common.download_and_extract_dataset(FAKE_BBOX_FILE,
                                            tempfile.gettempdir())
        fake_file = "%s/%s" % (tempfile.gettempdir(),
                               FAKE_BBOX_FILE.split("/")[-1])
        coco_detection_evaluator.evaluate(fake_file)


if __name__ == '__main__':
    unittest.main()
