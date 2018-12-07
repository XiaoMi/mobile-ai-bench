from aibench.python.utils import common
from aibench.python.evaluators.base_evaluator import Evaluator

from pycocotools.coco import COCO
from pycocotools.cocoeval import COCOeval

COCO_EVAL_URL = "http://images.cocodataset.org/annotations/annotations_trainval2017.zip"
COCO_DIR = "dataset/coco"


class COCOEvaluator(Evaluator):
    def __init__(self):
        super(COCOEvaluator, self).__init__()

    def prepare_dataset(self):
        common.download_and_extract_dataset(COCO_EVAL_URL, COCO_DIR)

    def evaluate(self, result_file):
        pass


class COCOObjectDetectionEvaluator(COCOEvaluator):
    def __init__(self):
        super(COCOObjectDetectionEvaluator, self).__init__()

    def prepare_dataset(self):
        super(COCOObjectDetectionEvaluator, self).prepare_dataset()
        ann_file = '%s/annotations/%s_%s.json' % (COCO_DIR, "instances", "val2017")
        self._coco_gt = COCO(ann_file)

    def evaluate(self, result_file):
        img_ids = sorted(self._coco_gt.getImgIds())
        coco_dt = self._coco_gt.loadRes(result_file)

        coco_eval = COCOeval(self._coco_gt, coco_dt, "bbox")
        coco_eval.params.imgIds  = img_ids
        coco_eval.evaluate()
        coco_eval.accumulate()
        coco_eval.summarize()