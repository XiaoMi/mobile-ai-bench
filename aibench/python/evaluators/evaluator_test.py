import unittest
import tempfile

from aibench.python.utils import common
from aibench.python.evaluators.coco_evaluator import COCOObjectDetectionEvaluator

FAKE_BBOX_FILE = "https://raw.githubusercontent.com/cocodataset/cocoapi/master/results/instances_val2014_fakebbox100_results.json"  # noqa


class TestEvaluator(unittest.TestCase):

    def test_coco_object_detection_evaluator(self):
        coco_detection_evaluator = COCOObjectDetectionEvaluator()
        coco_detection_evaluator.prepare_dataset()
        common.download_and_extract_dataset(FAKE_BBOX_FILE, tempfile.gettempdir())
        fake_file = "%s/%s" % (tempfile.gettempdir(), FAKE_BBOX_FILE.split("/")[-1])
        coco_detection_evaluator.evaluate(fake_file)


if __name__ == '__main__':
    unittest.main()
