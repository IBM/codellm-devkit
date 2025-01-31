---
search:
  boost: -0.5
---

# :kolena-cube-20: Building a Workflow

!!! note
    Kolena Workflows are simplified in Kolena Datasets.
    If you are setting up your Kolena environments for the first time, please refer to the
    [Datasets Quickstart](../dataset/quickstart.md).

In this tutorial we'll learn how to use the [`kolena.workflow`](../reference/workflow/index.md) workflow builder
definitions to test a [Keypoint Detection](https://keras.io/examples/vision/keypoint_detection/) model on the
[300-W](https://ibug.doc.ic.ac.uk/resources/300-W/) facial keypoint dataset. This demonstration will show us how we can
build a workflow to test any arbitrary ML problem on Kolena.

### Getting Started

With the `kolena` Python client [installed](../installing-kolena.md#installation),
first let's initialize a client session:

```python
import kolena

kolena.initialize(verbose=True)
```

The data used in this tutorial is publicly available in the `kolena-public-datasets` S3 bucket in a `metadata.csv` file:

```python
import pandas as pd

DATASET = "300-W"
BUCKET = "s3://kolena-public-datasets"

df = pd.read_csv(f"{BUCKET}/{DATASET}/meta/metadata.csv", storage_options={"anon": True})
```

!!! note "Note: `s3fs` dependency"

    To load CSVs directly from S3, make sure to install the `s3fs` Python module: `pip3 install s3fs[boto3]` and
    [set up AWS credentials](https://docs.aws.amazon.com/cli/latest/userguide/cli-configure-files.html).

This **`metadata.csv`** file describes a keypoint detection dataset with the following columns:

!!! note inline end "Note: Five-point facial keypoints array"

    For brevity, the 300-W dataset has been pared down to only 5 keypoints: outermost corner of each eye, bottom of
    nose, and corners of the mouth.

    <figure markdown>
      ![Example image and five-point facial keypoints array from 300-W.](../assets/images/300-W.jpg)
      <figcaption>Example image and five-point facial keypoints array from 300-W.</figcaption>
    </figure>

- **`locator`**: location of the image in S3
- **`normalization_factor`**: normalization factor of the image. This is used to normalize the error by providing a
    factor for each image. Common techniques for computation include the Euclidean distance between two points or the
    diagonal measurement of the image.
- **`points`**: stringified list of coordinates corresponding to the `(x, y)` coordinates of the keypoint ground truths

Each `locator` is present exactly one time and contains the keypoint ground truth for that image. In this tutorial,
we're implementing our workflow with support for only a single keypoint instance per image, but we could easily adapt
our ground truth, inference, and metrics types to accommodate a variable number of keypoint arrays per image.

### Step 1: Defining Data Types

When building your own workflow you have control over the [`TestSample`][kolena.workflow.TestSample] (e.g. image),
[`GroundTruth`][kolena.workflow.GroundTruth] (e.g. 5-element facial keypoint array), and
[`Inference`][kolena.workflow.Inference] types used in your project.

#### Test Sample Type

For the purposes of this tutorial, let's assume our model takes a single image as input along with an optional bounding
box around the face in question, produced by an upstream model in our pipeline. We can import and extend the
[`kolena.workflow.Image`][kolena.workflow.Image] test sample type for this purpose:

```python
from dataclasses import dataclass
from typing import Optional

from kolena.workflow import Image
from kolena.workflow.annotation import BoundingBox

@dataclass(frozen=True)
class TestSample(Image):
    bbox: Optional[BoundingBox] = None
```

#### Ground Truth Type

Next, let's define our [`GroundTruth`][kolena.workflow.GroundTruth] type, typically containing the manually-annotated
information necessary to evaluate model inferences:

```python
from kolena.workflow import GroundTruth as GT
from kolena.workflow.annotation import Keypoints

@dataclass(frozen=True)
class GroundTruth(GT):
    keypoints: Keypoints

    # In order to compute normalized error, some normalization factor describing
    # the size of the face in the image is required.
    normalization_factor: float
```

#### Inference Type

Lastly, we'll define our [`Inference`][kolena.workflow.Inference] type, containing model outputs that are evaluated
against a ground truth. Note that our model produces not only a [`Keypoints`][kolena.workflow.annotation.Keypoints]
array, but also an associated `confidence` value that we may use to ignore low-confidence predictions:

```python
from kolena.workflow import Inference as Inf

@dataclass(frozen=True)
class Inference(Inf):
    keypoints: Keypoints
    confidence: float
```

With our [test sample](#test-sample-type), [ground truth](#ground-truth-type), and [inference](#inference-type) defined,
we can now use [`define_workflow`][kolena.workflow.define_workflow.define_workflow] to declare our workflow:

```python
from kolena.workflow import define_workflow

# use these TestCase, TestSuite, and Model definitions to create and run tests
_, TestCase, TestSuite, Model = define_workflow(
    "Keypoint Detection", TestSample, GroundTruth, Inference
)
```

### Step 2: Defining Metrics

With our core data types defined, the next step is to lay out our evaluation criteria: our metrics.

#### Test Sample Metrics

Test Sample Metrics ([`MetricsTestSample`][kolena.workflow.MetricsTestSample]) are metrics computed from a single test
sample and its associated ground truths and inferences.

For the keypoint detection workflow, an example metric may be **normalized mean error** (NME), the normalized distance
between the ground truth and inference keypoints.

```python
from kolena.workflow import MetricsTestSample

@dataclass(frozen=True)
class TestSampleMetrics(MetricsTestSample):
    normalized_mean_error: float

    # If the normalized mean error is above some configured threshold, this test
    # sample is considered an "alignment failure".
    alignment_failure: bool
```

#### Test Case Metrics

Test case metrics ([`MetricsTestCase`][kolena.workflow.MetricsTestCase]) are aggregate metrics computed across a
population. All of your standard evaluation metrics should go here — things like accuracy, precision, recall, or any
other aggregate metrics that apply to your problem.

For keypoint detection, we care about the **mean NME** and **alignment failure rate** across the different test samples
in a test case:

```python
from kolena.workflow import MetricsTestCase

@dataclass(frozen=True)
class TestCaseMetrics(MetricsTestCase):
    mean_nme: float
    alignment_failure_rate: float
```

!!! tip "Tip: Plots"

    Evaluators can also compute test-case-level plots using the [`Plot`][kolena.workflow.Plot] API. These plots are
    visualized on the [:kolena-results-16: Results](https://app.kolena.com/redirect/results) dashboard alongside the
    metrics reported for each test case.

!!! tip "Tip: Test Suite Metrics"

    Metrics can also be computed per test suite by extending [`MetricsTestSuite`][kolena.workflow.MetricsTestSuite].

    Test suite metrics typically measure variance in performance across different test cases, being used e.g. to measure
    fairness across demographics for a test suite with test cases stratifying by demographic.

### Step 3: Creating Tests

With our data already in an S3 bucket and metadata loaded into memory, we can start creating test cases!

Let's create a simple test case containing the entire dataset:

```python
import json

test_samples = [TestSample(locator) for locator in df["locator"]]
ground_truths = [
    GroundTruth(
        keypoints=Keypoints(points=json.loads(record.points)),
        normalization_factor=record.normalization_factor,
    )
    for record in df.itertuples()
]
ts_with_gt = list(zip(test_samples, ground_truths))
test_case = TestCase(f"{DATASET} :: basic", test_samples=ts_with_gt)
```

!!! note "Note: Creating test cases"

    In this tutorial we created only a single simple test case, but more advanced test cases can be generated in a
    variety of fast and scalable ways, either programmatically with the `kolena` Python client or visually in the
    [:kolena-studio-16: Studio](https://app.kolena.com/redirect/studio).

Now that we have a basic test case for our entire dataset let's create a test suite for it:

```python
test_suite = TestSuite(f"{DATASET} :: basic", test_cases=[test_case])
```

### Step 4: Running Tests

With basic tests defined for the 300-W dataset, we're almost ready to start testing our models.

#### Implementing an Evaluator

Core to the testing process is the [`Evaluator`][kolena.workflow.Evaluator] implementation to compute the metrics
defined in [step 2](#step-2-defining-metrics). Usually, an evaluator simply plugs your existing metrics computation
logic into the [class-based][kolena.workflow.Evaluator] or [function-based][kolena.workflow.BasicEvaluatorFunction]
evaluator interface.

Evaluators can have arbitrary configuration ([`EvaluatorConfiguration`][kolena.workflow.EvaluatorConfiguration]),
allowing you to evaluate model performance under a variety of conditions.
For this keypoint detection example, perhaps we want to compute performance at a few different NME threshold values, as
this threshold drives the `alignment_failure` metric.

```python
from kolena.workflow import EvaluatorConfiguration

@dataclass(frozen=True)
class NmeThreshold(EvaluatorConfiguration):
    # threshold for NME above which an image is considered an "alignment failure"
    threshold: float

    def display_name(self):
        return f"NME threshold: {self.threshold}"
```

Here, we'll mock out an evaluator implementation using the [function-based][kolena.workflow.BasicEvaluatorFunction]
interface:

```python
from random import random
from typing import List

from kolena.workflow import EvaluationResults, TestCases

def evaluate_keypoint_detection(
    test_samples: List[TestSample],
    ground_truths: List[GroundTruth],
    inferences: List[Inference],
    test_cases: TestCases,
    configuration: NmeThreshold,  # uncomment when configuration is used
) -> EvaluationResults:
    # compute per-sample metrics for each test sample
    per_sample_metrics = [
        TestSampleMetrics(normalized_mean_error=random(), alignment_failure=bool(random() > 0.5))
        for gt, inf in zip(ground_truths, inferences)
    ]

    # compute aggregate metrics across all test cases using `test_cases.iter(...)`
    aggregate_metrics = []
    for test_case, *s in test_cases.iter(test_samples, ground_truths, inferences, per_sample_metrics):
        test_case_metrics = TestCaseMetrics(mean_nme=random(), alignment_failure_rate=random())
        aggregate_metrics.append((test_case, test_case_metrics))

    return EvaluationResults(
        metrics_test_sample=list(zip(test_samples, per_sample_metrics)),
        metrics_test_case=aggregate_metrics,
    )
```

#### Running tests

To test our models, we can define an [`infer`][kolena.workflow.Model.infer] function that maps the `TestSample` object
we defined above into an `Inference`:

```python
from random import randint

def infer(test_sample: TestSample) -> Inference:
    """
    1. load the image pointed to at `test_sample.locator`
    2. pass the image to our model and transform its output into an `Inference` object
    """

    # Generate the dummy inference for the demo purpose.
    return Inference(Keypoints([(randint(100, 400), randint(100, 400)) for _ in range(5)]), random())

model = Model("example-model-name", infer=infer, metadata=dict(
    description="Any freeform metadata can go here",
    hint="It may be helpful to include information about the model's framework, training methodology, dataset, etc.",
))
```

We now have the pieces in place to run tests on our new workflow using [`test`][kolena.workflow.test]:

```python
from kolena.workflow import test

test(
    model,
    test_suite,
    evaluate_keypoint_detection,
    configurations=[NmeThreshold(0.01), NmeThreshold(0.05), NmeThreshold(0.1)],
)
```

That wraps up the testing process! We can now visit [:kolena-results-16: Results](https://app.kolena.com/redirect/results)
to analyze and debug our model's performance on this test suite.

### Conclusion

In this tutorial we learned how to build a workflow for an arbitrary ML problem, using a facial keypoint
detection model as an example. We created new tests, tested our models on Kolena, and learned how to customize
evaluation to fit our exact expectations.

This tutorial just scratches the surface of what's possible with Kolena and covered a fraction of the
`kolena` API — now that we're up and running, we can think about ways to create more detailed tests, improve
existing tests, and dive deep into model behaviors.
