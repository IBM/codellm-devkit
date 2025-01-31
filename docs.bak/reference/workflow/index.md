---
hide:
  - toc
search:
  boost: -0.5
---

# `kolena.workflow`

<div class="grid cards" markdown>
- :kolena-layers-16: Developer Guide: [Building a Workflow ↗](../../workflow/building-a-workflow.md)
- :kolena-developer-16: Examples: [`kolena/examples` ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow)
</div>

`kolena.workflow` contains the definitions to build a [workflow](../../workflow/core-concepts/workflow.md):

1. Design data types, including any [`annotations`](annotation.md) or [`assets`](asset.md):

    !!! info inline end "Defining a workflow"

        [`TestSample`][kolena.workflow.TestSample], [`GroundTruth`][kolena.workflow.GroundTruth], and
        [`Inference`][kolena.workflow.Inference] can be thought of as the data model, or schema, for a workflow.

        [:kolena-layers-16: Learn more ↗](../../workflow/core-concepts/workflow.md)

    - [`TestSample`][kolena.workflow.TestSample]: model inputs, e.g. images, videos, documents
    - [`GroundTruth`][kolena.workflow.GroundTruth]: expected model outputs
    - [`Inference`][kolena.workflow.Inference]: real model outputs

2. Define metrics and how they are computed:

    - [`Evaluator`][kolena.workflow.Evaluator]: metrics computation engine

3. Create tests:

    !!! info inline end "Managing tests"

        See the [test case and test suite](../../workflow/core-concepts/test-suite.md) developer guide for an
        introduction to the test case and test suite concept.

    - [`TestCase`][kolena.workflow.TestCase]: a test dataset, or a slice thereof
    - [`TestSuite`][kolena.workflow.TestSuite]: a collection of test cases

4. Test models:

    - [`Model`][kolena.workflow.Model]: descriptor for a model
    - [`test`][kolena.workflow.test]: interface to run tests
