---
icon: kolena/media-16
---
# :kolena-media-20: Computer Vision

In this document we will review best practices when setting up Kolena datasets for computer vision
problems.

## Basics

### Supported File Data Formats

The Kolena SDK supports uploading of data in the Pandas
[`DataFrame`](https://pandas.pydata.org/docs/reference/frame.html) format.

The Kolena web app supports the following file formats.

| Format | Description |
| --- | --- |
| `.csv` | Comma-separated values file, ideal for tabular data |
| `.parquet` | Apache Parquet format, efficient for columnar storage |
| `.jsonl` | JSON Lines format, suitable for handling nested data |

Supported file types are:

| Type | Format |
| --- | --- |
| Images | `jpg`, `jpeg`, `png`, `gif`, `bmp` and other web browser supported images |
| Video | `mov`, `mp4`, `mpeg` and other web browser supported video types |
| Point Cloud | `.pcd` |

### Using the `locator`

Kolena uses references to files stored in your cloud storage to render them.
Refer to ["Connecting Cloud Storage"](../../../connecting-cloud-storage/index.md)
for details on how to configure this.

Computer Vision data is best visualized in Studio using the Gallery mode.
To enable the Gallery view store references to images in a column named `locator`. `locator` can be used as
the unique identifier of the datapoint which is also referenced by your model results.

Kolena supports `jpg`, `jpeg`, `png`, `gif`, `bmp` and other web browser supported images.

<figure markdown>
![Gallery View](../../../assets/images/gallery-view-dark.png#only-dark)
![Gallery View](../../../assets/images/gallery-view-light.png#only-light)
<figcaption>Gallery View</figcaption>
</figure>

### Using fields

You can add additional information about your image by
adding columns to the `.CSV` file with the metadata name and values in each row.
Below is an example datapoint:

| locator | ground_truth | image_brightness | image_contrast |
| --- | --- | --- | --- |
| `s3://kolena-public-examples/cifar10/data/horse0000.png` | horse | 153.994 | 84.126 |

!!! tip
    **Using thumbnails**

    In order to improve the loading performance of your image data, you can upload compressed versions of the image
    with the same dimensions as thumbnails. This results in an improved Studio experience due to faster image loading
    when filtering, sorting or using [embedding](../../../automations/set-up-natural-language-search.md) sort.

    Thumbnails are configured by adding a field called `thumbnail_locator` to the data, where the value points
    to a compressed version of the `locator` image.

    If you wanted to add a thumbnail to the classification data shown above it would look like:

    | locator| thumbnail_locator | ground_truth | image_brightness | image_contrast |
    | --- | --- | --- | --- | --- |
    | `s3://kolena-examples/data/h0.png`| `s3://kolena-examples/data/thumbnail/h0.png` | horse | 153.994 | 84.126 |

### Including Assets and Annotations

Kolena supports the inclusion of overlay annotations and asset files as fields in a dataset.

We recommend using the [annotation](../../../reference/annotation.md) and [asset](../../../reference/asset.md) dataclasses
for ease of annotation and asset manipulation:

```
# Creates a single-row DataFrame with an image datapoint, a `bbox` annotation field, and a `mesh` asset file.

import pandas as pd
from kolena.annotation import BoundingBox
from kolena.asset import MeshAsset

locator = "s3://kolena-public-examples/coco-2014-val/data/COCO_val2014_000000000294.jpg"
bbox = BoundingBox(top_left=(27.7, 69.83), bottom_right=(392.61, 427))
mesh = MeshAsset(locator="s3://kolena-public-examples/a-large-dataset-of-object-scans/data/mesh/00004.ply")
df = pd.DataFrame([dict(locator=locator, bbox=bbox, mesh=mesh)])

# DataFrame can now be directly uploaded as a dataset
from kolena.dataset import upload_dataset
upload_dataset("my-dataset", df, id_fields=["locator"])

# Or serialized to CSV and uploaded through the web UI.
# If serializing to CSV please use the provided `kolena.io.dataframe_to_csv` method. The Pandas provided `to_csv` method
# does not adhere to the JSON spec, and may serialize malformed objects.
from kolena.io import dataframe_to_csv

dataframe_to_csv(df, "my-dataset.csv", index=False)
```

## Specific Workflows

### 2D Object Detection

!!! example
    You can follow this [example 2D object detection ↗](https://github.com/kolenaIO/kolena/blob/trunk/examples/dataset/object_detection_2d/object_detection_2d/upload_dataset.py)

[`annotations`](../../../reference/annotation.md) are used to visualize overlays on top of images.
To render 2D bounding boxes you can use
[`LabeledBoundingBox`](../../../reference/annotation.md#kolena.annotation.LabeledBoundingBox) or
[`BoundingBox`](../../../reference/annotation.md#kolena.annotation.BoundingBox) annotations.

Consider a `.csv` file containing ground truth data in the form of bounding boxes for an Object Detection problem.

| locator | label | min_x | max_x | min_y | max_y |
| --- | --- | --- | --- | --- | --- |
| s3://kolena-public-examples/coco-2014-val/data/COCO_val2014_000000369763.jpg | motorcycle | 270.77 | 621.61 | 44.59 | 254.18 |
| s3://kolena-public-examples/coco-2014-val/data/COCO_val2014_000000369763.jpg | car | 538.03 | 636.85 | 8.86 | 101.93 |
| s3://kolena-public-examples/coco-2014-val/data/COCO_val2014_000000369763.jpg | trunk | 313.02 | 553.98 | 12.01 | 99.84 |

This looks like:
```python
from kolena.annotation import LabeledBoundingBox
bboxes = [
    LabeledBoundingBox(top_left=(270.77, 44.59), bottom_right=(621.61, 254.18), label="motorcycle"),
    LabeledBoundingBox(top_left=(538.03, 8.86), bottom_right=(636.85, 101.93), label="car"),
    LabeledBoundingBox(top_left=(313.02, 12.01), bottom_right=(553.98, 99.84), label="trunk"),
]
```

!!! tip
    **Using bounding box categories**

    If you wish to analyze your model results based on specific characteristics of your bounding boxes
    you can provide values representing those characteristics using additional key value pairs.
    For example if location of a bounding box is important you can construct your `LabeledBoundingBox` like this
    ```python
        LabeledBoundingBox(top_left=(313.02, 12.01), bottom_right=(553.98, 99.84), label="trunk", location="bottom-left")
    ```

!!! note
    When uploading `.csv` files for datasets that contain annotations, assets or nested values in a column use the
    [`dataframe_to_csv()`](../../../reference/io.md#kolena.io.dataframe_to_csv) function provided by Kolena
     to save a `.csv` file
    instead of [`pandas.to_csv()`](https://pandas.pydata.org/docs/reference/api/pandas.DataFrame.to_csv.html).
    `pandas.to_csv` does not serialize Kolena annotation objects in a way that is compatible with the platform.

#### Uploading Model Results

Model results contain your model inferences as well as any custom metrics that you wish to monitor on Kolena.
The data structure of model results is very similar to the structure of a dataset with minor differences.

* Ensure your results are using the same unique ID field (the `locator` for instance) you have selected for your dataset.

* Use [`ScoredBoundingBox`](../../../reference/annotation.md#kolena.annotation.ScoredBoundingBox) or
[`ScoredLabeledBoundingBox`](../../../reference/annotation.md#kolena.annotation.ScoredLabeledBoundingBox)
to pass on your model inferences confidence score for each bounding box.

* Use [`compute_object_detection_results`](../../../reference/experimental/index.md#kolena._experimental.object_detection.compute_object_detection_results)
to compute your metrics that are supported by Kolena's [Object Detection Task Metrics](../../advanced-usage/task-metrics.md#object-detection).

* OR include the following columns in your results. The values for each of the columns is a
[`List[ScoredLabeledBoundingBox]`](../../../reference/annotation.md#kolena.annotation.ScoredLabeledBoundingBox).

    | Column Name              | Description                                         |
    |--------------------------|-----------------------------------------------------|
    | `matched_inference`      | Inferences that were matched to a ground truth.     |
    | `unmatched_inference`    | Inferences that were not matched to a ground truth. |
    | `unmatched_ground_truth` | Ground truths with no matching inference.           |

* Leverage task metrics by adding the following columns to your CSV: `count_TP`, `count_FP`, `count_FN`, `count_TN`.

!!! note
    Once you have constructed your `DataFrame` use the
    [`upload_object_detection_results`](../../../reference/experimental/index.md#kolena._experimental.object_detection.upload_object_detection_results)
    wrapper function to simplify the upload process and enable the Object Detection Task metrics automatically.

!!! example
    Follow the [2D Object Detection result upload](https://github.com/kolenaIO/kolena/blob/trunk/examples/dataset/object_detection_2d/object_detection_2d/upload_results.py)
    example for optimal setup.

### 3D Object Detection

[`annotations`](../../../reference/annotation.md) are used to visualize overlays on top of images.
To render 3D Bounding boxes you can use
[`BoundingBox3D`](../../../reference/annotation.md#kolena.annotation.BoundingBox3D) or
[`LabeledBoundingBox3D`](../../../reference/annotation.md#kolena.annotation.LabeledBoundingBox3D)

!!! tip
    **Using bounding box categories**

    If you wish to analyze your model results based on specific characteristics of your bounding boxes
    you can provide values representing those characteristics using additional key value pairs.
    For example, if location of a bounding box is important you can construct your `LabeledBoundingBox3D` like this
    ```python
    LabeledBoundingBox3D(
        center=(313.02, 12.01, 15.5),
        dimensions=(553.98, 99.84, 231.17),
        rotations=(12, 16, 25),
        label="trunk",
        location="bottom-left"
    )
    ```

!!! note
    When uploading `.csv` files for datasets that contain annotations, assets or nested values in a column use the
    [`dataframe_to_csv()`](../../../reference/io.md#kolena.io.dataframe_to_csv) function provided by Kolena
    to save a `.csv` file instead of
    [`pandas.to_csv()`](https://pandas.pydata.org/docs/reference/api/pandas.DataFrame.to_csv.html).
    `pandas.to_csv` does not serialize Kolena annotation objects in a way that is compatible with the platform.

#### Uploading Model Results

Model results contain your model inferences as well as any custom metrics that you wish to monitor on Kolena.
The data structure of model results is very similar to the structure of a dataset with minor differences.

* Ensure your results are using the same unique ID field (the `locator` for instance) you have selected for your dataset.
* Use [`ScoredBoundingBox3D`](../../../reference/annotation.md#kolena.annotation.ScoredBoundingBox3D) or
[`ScoredLabeledBoundingBox3D`](../../../reference/annotation.md#kolena.annotation.ScoredLabeledBoundingBox3D)
to pass on your model inferences confidence score for each bounding box.
* Use [`compute_object_detection_results`](../../../reference/experimental/index.md#kolena._experimental.object_detection.compute_object_detection_results)
to compute your metrics that are supported by Kolena's [Object Detection Task Metrics](../../advanced-usage/task-metrics.md#object-detection).

!!! note
    Once you have constructed your `DataFrame` use the
    [`upload_object_detection_results`](../../../reference/experimental/index.md#kolena._experimental.object_detection.upload_object_detection_results)
    wrapper function to simplify the upload process and enable the Object Detection Task metrics automatically.

!!! example
    Follow the [3D Object Detection result upload script](https://github.com/kolenaIO/kolena/blob/trunk/examples/dataset/object_detection_3d/object_detection_3d/upload_results.py)
    on how to setup both 3D and 2D bounding boxes in your results for multi-modal 3D object detection data.

### Video

Videos are best represented in Kolena using the Gallery view. To setup the Gallery view, add links to your video files
stored on the cloud under the `locator` column. Kolena automatically looks for that column name and renders your video files
correctly.
Kolena supports `mov`, `mp4`, `mpeg` and other web browser supported video types.

!!! Note
    [Annotation](../../../reference/annotation.md) visualization over videos only works on videos with constant frame rates.
    For the best experience, include a `frame_rate` field on your video datapoints in frames per second as a `float`
    or `int` number.

#### Setting up bounding box annotations on videos

To overlay bounding boxes on videos, you will need to define a new class based on
[`LabeledBoundingBox`](../../../reference/annotation.md#kolena.annotation.LabeledBoundingBox) or
[`BoundingBox`](../../../reference/annotation.md#kolena.annotation.BoundingBox) annotations where a
`frame_id` property is added. You are able to add additional properties if you wish which can be used
for filtering and visualizations.

``` python
# video bounding box for pedestrians with pedestrian id,
# risk of collision and label indicating if bounding box
# is occluded.
class PedestrianBoundingBox(LabeledBoundingBox):
    frame_id: int
    ped_id: str
    occlusion: str
    risk: Optional[str] = None

    def set_risk(self, risk: str) -> None:
        object.__setattr__(self, "risk", risk)
```
!!! Note
    Kolena depends on the `frame_id` for rendering and it needs to be a zero-indexed integer.

To overlay bounding boxes with inferences (used when uploading model results) you will need a new class based on
[`ScoredBoundingBox`](../../../reference/annotation.md#kolena.annotation.ScoredBoundingBox) or
[`ScoredLabeledBoundingBox`](../../../reference/annotation.md#kolena.annotation.ScoredLabeledBoundingBox).
The requirements for rendering on a video is similar to the previous example:

``` python
# video bounding box with inference (represented by score),
# frame_id, pedestrian id, occlusion category,
# time_to_event (in this case potential collusion of
# a pedestrian and vehicle), failed_to_infer for capturing
# no inference cases

@dataclass(frozen=True)
class ScoredPedestrianBoundingBox(ScoredLabeledBoundingBox):
    frame_id: int
    ped_id: str
    occlusion: str
    time_to_event: Optional[float]
    failed_to_infer: bool

```

!!! example
    Follow the [Crossing Pedestrian Detection](https://github.com/kolenaIO/kolena/tree/trunk/examples/dataset/crossing_pedestrian_detection)
    example on how to setup video based dataset and model results.
