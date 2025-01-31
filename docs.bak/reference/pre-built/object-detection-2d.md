---
search:
  boost: -0.5
---

# :kolena-widget-20: Object Detection (2D)

!!! example "Experimental Feature"

    This pre-built workflow is an experimental feature. Experimental features are under active development and may
    occasionally undergo API-breaking changes.

Object Detection (OD) is a computer vision task that aims to classify and locate objects of interest presented in an
image. So, it can be viewed as a combination of localization and classification tasks.

This pre-built workflow is prepared for a 2D Object Detection problem and here is an example of using this workflow
on the COCO dataset.

<div class="grid cards" markdown>
- [:kolena-widget-20: Example: Object Detection (2D) ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/object_detection_2d)

    ![Example 2D bounding boxes from the COCO dataset.](../../assets/images/COCO-transportation.jpeg)

    ---

    2D Object Detection using the [COCO](https://cocodataset.org/#overview) dataset
</div>

::: kolena._experimental.object_detection
    options:
        members: ["TestSample", "GroundTruth", "Inference", "ThresholdConfiguration", "ObjectDetectionEvaluator"]
