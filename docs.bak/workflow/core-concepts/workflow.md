---
search:
  boost: -0.5
---

# :kolena-workflow-20: Workflow

Testing in Kolena is broken down by the type of ML problem you're solving, called a **workflow**. Any ML problem that
can be tested can be modeled as a workflow in Kolena.

Examples of workflows include:

<div class="grid cards" markdown>
- [:kolena-widget-20: Object Detection (2D)](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/object_detection_2d)
  using images
- [:kolena-text-summarization-20: Text Summarization](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/text_summarization)
  using articles/documents
- [:kolena-age-estimation-20: Age Estimation](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/age_estimation)
  (regression) using images
- [:kolena-video-20: Video Retrieval](https://paperswithcode.com/task/video-retrieval)
  using text queries on a corpus of videos
</div>

With the [`kolena.workflow`](../../reference/workflow/index.md) client module,
any arbitrary ML problem can be defined as a workflow and tested on Kolena.

There are three main components of a workflow:

!!! info inline end

    These three types can be thought of as the data model, or the schema, of a workflow.

1. [**Test Sample**](#test-sample): the inputs to a model, e.g. image, video, document
2. [**Ground Truth**](#ground-truth): the expected model outputs
3. [**Inference**](#inference): the actual model outputs

## Test Sample

In Kolena, "test sample" is the general term for the input to a model.

For standard computer vision (CV) models, the test sample is often a single [image][kolena.workflow.Image]. Video-based
computer vision models would have a [video][kolena.workflow.Video] test sample type, and stereo vision models would use
[image pairs][kolena.workflow.Composite]. For natural language processing models, the test sample may be a
[document][kolena.workflow.Document] or [text snippet][kolena.workflow.Text].

When [building a workflow](../building-a-workflow.md), you can [extend][kolena.workflow.TestSample] and
[compose][kolena.workflow.Composite] these base test sample types as necessary, or use the base types directly if no
customization is required.

### Metadata

Any additional information associated with a test sample, e.g. details about how it was collected, can be included as
[metadata][kolena.workflow.Metadata]. We recommend uploading any and all metadata that you have available, as metadata
can be useful for searching through data in the Studio, interpreting model results, and creating new test cases.

```python
from dataclasses import dataclass, field

from kolena.workflow import Document, Metadata

@dataclass(frozen=True)
class MyDocument(Document):
    # locator: str  # inherited from parent Document
    doc_id: int  # example of a field that is explicitly required
    metadata: Metadata = field(default_factory=dict)  # free-form, optional metadata
```

!!! tip "Use `pydantic` dataclasses"

    When building a workflow, object definitions can us [standard library `dataclasses`][dataclasses] or
    [Pydantic `dataclasses`](https://docs.pydantic.dev/latest/usage/dataclasses/). Pydantic brings helpful runtime type
    validation and coercion and can be used as a drop-in replacement for standard library `dataclasses`.

### Composite Test Samples

Kolena is not prescriptive about the shape of your ML problem. Test samples can be composed, using the
[`Composite`][kolena.workflow.Composite] test sample type, to mirror the shape of your problem directly.

Consider the example of an autonomous vehicle application that uses four cameras, one for each of the `front`, `right`,
`rear`, and `left` views:

```python
from dataclasses import dataclass

from kolena.workflow import Composite, Image

@dataclass(frozen=True)
class QuadImage(Composite):
    front: Image
    right: Image
    rear: Image
    left: Image
```

??? question "How can I specify annotations on `Composite` test samples?"

    Image-level (or video-level, document-level, etc.) annotations can be specified when using composite test samples.
    To specify image-level objets in each of the four images, ground truth or inference definitions may look like this:

    ```python
    from dataclasses import dataclass
    from typing import List

    from kolena.workflow import DataObject, GroundTruth
    from kolena.workflow.annotation import BoundingBox

    @dataclass(frozen=True)
    class SingleImageGroundTruth(DataObject):
        objects: List[BoundingBox]

    @dataclass(frozen=True)
    class QuadImageGroundTruth(GroundTruth):
        # attribute names matches attribute names in test sample
        front: SingleImageGroundTruth
        right: SingleImageGroundTruth
        rear: SingleImageGroundTruth
        left: SingleImageGroundTruth
    ```

## Ground Truth

The ground truth represents the expected output from a model when provided with a test sample. Ground truths are often
manually annotated and are used to determine the correctness of model predictions.

In the [:kolena-studio-16: Studio](https://app.kolena.com/redirect/studio), ground truths are always displayed alongside
their paired test samples. Any [annotations][kolena.workflow.annotation.Annotation], such as bounding boxes or polygons,
are visualized on top of the test sample.

The contents of a ground truth are driven by the requirements of the workflow. Take this example for a multiclass object
detection workflow:

```python
from dataclasses import dataclass
from typing import List

from kolena.workflow import GroundTruth
from kolena.workflow.annotation import LabeledBoundingBox

@dataclass(frozen=True)
class MyGroundTruth(GroundTruth):
    objects: List[LabeledBoundingBox]
```

??? question "Where should additional information that isn't used for model evaluation live?"

    We recommend scoping the ground truth to only the data required for model evaluation. Any additional metadata,
    annotations, or assets associated with a test sample can be included as a part of the test sample itself or in its
    free-form [metadata](#metadata).

    However, it isn't a strict requirement that ground truths only contain information used for model evaluation.
    Sometimes it makes sense to include additional information as optional fields inside a ground truth definition.

## Inference

A workflow's inference type contains the actual output produced by a model when given a test sample. Inferences are
also referred to as "raw inferences," as they represent the raw output from a model.

The inference type and ground truth type for a workflow will often look very similar to one another.

### Extending Annotation Types

[Annotation][kolena.workflow.annotation.Annotation] types can be extended to include additional fields, when necessary.

Consider the example of a [`Keypoints`][kolena.workflow.annotation.Keypoints] detection model that detects anywhere from
0 to N keypoints arrays when provided an image. Each keypoints array has an associated class label and confidence value.
This model's inference type could be defined as follows:

```python
from dataclasses import dataclass
from typing import List

from kolena.workflow import Inference
from kolena.workflow.annotation import Keypoints

@dataclass(frozen=True)
class ScoredLabeledKeypoints(Keypoints):
    # points: List[Tuple[float, float]]  # inherited from Keypoints
    score: float  # confidence score, between 0 and 1
    label: str  # predicted class

@dataclass(frozen=True)
class MyInference(Inference):
    predictions: List[ScoredLabeledKeypoints]
```

### Deduplication

Models are considered deterministic inputs from test samples to inferences. This means that, when testing in Kolena,
a given model only needs to process a given test sample once. Kolena uses this to speed up the process of running tests,
ensuring that compute cycles are not wasted processing a given test sample multiple times when test samples exist in
multiple test cases.

When calling [`test`][kolena.workflow.test], only samples that do not already have inferences uploaded from the given
model will be processed. To change this behavior and re-process all test samples, regardless of any uploaded inferences,
use the `reset` flag:

```python
# all test samples are processed and inferences [re]uploaded when reset=True
test(model, test_suite, evaluator, reset=True)
```

## Defining a Workflow

With [test sample](#test-sample), [ground truth](#ground-truth), and [inference](#inference) types declared,
defining a workflow provides the [`TestCase`][kolena.workflow.TestCase], [`TestSuite`][kolena.workflow.TestSuite], and
[`Model`][kolena.workflow.Model] definitions to use when creating tests and testing models with this workflow:

```python
from kolena.workflow import define_workflow

from my_workflow import MyTestSample, MyGroundTruth, MyInference

_, TestCase, TestSuite, Model = define_workflow(
    "My Example Workflow",
    MyTestSample,
    MyGroundTruth,
    MyInference,
)
```
