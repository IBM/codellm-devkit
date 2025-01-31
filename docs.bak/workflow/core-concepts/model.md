---
search:
  boost: -0.5
---

# :kolena-model-20: Model

In Kolena, a model is a deterministic transformation from [test samples](workflow.md#test-sample) to
[inferences](workflow.md#inference).

Kolena only stores metadata associated with your model in its [:kolena-model-16: Models](https://app.kolena.com/redirect/models)
registry. Models themselves — their code or their weights — are never uploaded to Kolena, only the inferences from models.

Models are considered black boxes, which makes Kolena agnostic to the underlying framework
and architecture. It's possible to test any sort of model, from deep learning to rules-based, on Kolena.

## Creating Models

The [`Model`][kolena.workflow.Model] class is used to programmatically create models for testing. Rather than importing
the class from `kolena.workflow` directly, use the `Model` definition returned from
[`define_workflow`](workflow.md#defining-a-workflow) bound to the test sample and inference types for your
workflow:

```python
from kolena.workflow import define_workflow

from my_workflow import MyTestSample, MyGroundTruth, MyInference

*_, Model = define_workflow("My Workflow", MyTestSample, MyGroundTruth, MyInference)
```

With this class, models can be created, loaded, and updated:

```python
my_model = Model("example-model")
```

## Implementing `infer`

To test a model using the [`test`][kolena.workflow.test] method, a [`Model.infer`][kolena.workflow.Model.infer]
implementation must be provided. `infer` is where the model itself — the deterministic transformation from test sample
to inference — lives.

```python
# in practice, use TestSample and Inference types from your workflow
from kolena.workflow import TestSample, Inference

def infer(test_sample: TestSample) -> Inference:
    ...
```

When running a model live, this function usually involves loading the image/document/etc. from the `TestSample`, passing
it to your model, and constructing an `Inference` object from the model outputs. When loading results from e.g. a CSV,
this function is often just a lookup.

??? example "Example: Loading inferences from CSV"

    This example considers a classification workflow using the [`Image`][kolena.workflow.Image] test sample type and
    the following inference type:

    ```python
    from dataclasses import dataclass
    from typing import Optional

    from kolena.workflow import Inference
    from kolena.workflow.annotation import ScoredClassificationLabel

    @dataclass(frozen=True)
    class MyInference(Inference):
        # use Optional to accommodate missing inferences
        prediction: Optional[ScoredClassificationLabel] = None
    ```

    With inferences stored in an `inferences.csv` with the `locator`, `label` and `score` columns, implementing `infer`
    as a lookup is straightforward:

    ```python
    import pandas as pd
    from kolena.workflow import Image

    from my_workflow import MyInference

    inference_by_locator = {
        record.locator: MyInference(prediction=ScoredClassificationLabel(
            label=record.label,
            score=record.score,
        )) for record in pd.read_csv("inferences.csv").itertuples()
    }

    def infer(test_sample: Image) -> MyInference:
        return inference_by_locator.get(test_sample.locator, MyInference())
    ```

!!! note "Note: Ensure that models are deterministic"

    To preserve reproducibility, ensure that models tested in Kolena are deterministic.

    This is particularly important for generative models. If your model has a random seed parameter, consider including
    the random seed value used for testing as a piece of metadata attached to the model.

## Metadata

When creating a model, you have the option to specify free-form [`metadata`][kolena.workflow.Model.metadata] to
associate with the model. This metadata can be useful to track relevant information about the model, such as:

- Framework (e.g. [PyTorch](https://pytorch.org/), [TensorFlow](https://www.tensorflow.org/), custom, etc.) and version used
- Person who trained the model, e.g. `name@company.ai`
- GitHub branch, file, or commit hash used to run the model
- Links to your experimentation tracking system
- Free-form notes about methodology or observations
- Location in e.g. S3 where the model's weights are stored
- Training dataset specifier or URL
- Hyperparameters applied during training

Metadata can be specified on the command line or edited on the web on the
[:kolena-model-16: Models](https://app.kolena.com/redirect/models) page.

## FAQ & Best Practices

??? faq "How should models be named?"

    Two factors influence model naming:

    1. A model's name is unique, and
    2. A model is deterministic.

    This means that anything that may change your model's outputs, such as environment or packaging, should be tracked
    as a new model! We recommend storing a variety of information in the model name, for example:

    - Model architecture, e.g. `YOLOR-D6`
    - Input size, e.g. `1280x1280`
    - Framework, e.g. `pytorch-1.7`
    - Additional tracking information, such as its name in Weights & Biases, e.g. `helpful-meadow-5`

    An example model name may therefore be:

    ```
    helpful-meadow-5 (YOLOR-D6, 1280x1280, pytorch-1.7)
    ```

    Model names can be edited on the web on the [:kolena-model-16: Models](https://app.kolena.com/redirect/models) page.
