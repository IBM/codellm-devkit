---
search:
  boost: -0.5
---

# :kolena-test-suite-20: Test Case & Test Suite

Test cases and test suites are used to organize test data in Kolena.

A **test case** is a collection of [test samples](workflow.md#test-sample) and their associated
[ground truths](workflow.md#ground-truth). Test cases can be thought of as benchmark datasets, or slices of
a benchmark dataset.

A **test suite** is a collection of test cases. Models are tested on test suites.

Test cases and test suites are found on the [:kolena-test-suite-16: Test Suites](https://app.kolena.com/redirect/testing)
page on Kolena.

## Managing Test Cases & Test Suites

The [`TestCase`][kolena.workflow.TestCase] and [`TestSuite`][kolena.workflow.TestSuite] classes are used to
programmatically create test cases and test suites. Rather than importing these classes from `kolena.workflow` directly,
Use the definitions returned from [`define_workflow`](workflow.md#defining-a-workflow) bound to the test
sample and ground truth types for your workflow:

```python
from kolena.workflow import define_workflow

from my_workflow import MyTestSample, MyGroundTruth, MyInference

_, TestCase, TestSuite, _ = define_workflow(
    "My Workflow",
    MyTestSample,
    MyGroundTruth,
    MyInference,
)
```

These classes can then be used to create, load, and edit test cases and test suites:

=== "Test Case"

    Create using [`TestCase.create`][kolena.workflow.TestCase.create]:

    ```python
    # throws if a test case with name 'example-test-case' already exists
    test_case = TestCase.create(
        "example-test-case",
        # optionally include list of test samples and ground truths to populate the new test case
        # test_samples=[(ts0, gt0), (ts1, gt1), (ts2, gt2)],
    )
    ```

    Load using [`TestCase.load`][kolena.workflow.TestCase.load]:

    ```python
    # throws if a test case with name 'example-test-case' does not exist
    test_case = TestCase.load("example-test-case")
    ```

    Use the [`TestCase`][kolena.workflow.TestCase] constructor for idempotent create/load behavior:

    ```python
    # loads 'example-test-case' or creates it if it does not already exist
    test_case = TestCase("example-test-case")
    ```

    Use [`TestCase.init_many`][kolena.workflow.TestCase.init_many] to initialize multiple test cases at once:

    ```python
    # loads test cases or creates them if they do not already exist
    test_cases = TestCase.init_many([
        ("test case 1", [(test_sample_0, ground_truth_0), (test_sample_1, ground_truth_1)]),
        ("test case 2", [(test_sample_2, ground_truth_2), (test_sample_3, ground_truth_3)])
    ])

    # With 'reset=True', test cases that already exist would be updated with the new test_samples and ground_truths
    test_cases = TestCase.init_many([
        ("test case 1", [(test_sample_0, ground_truth_0), (test_sample_1, ground_truth_1)]),
        ("test case 2", [(test_sample_2, ground_truth_2), (test_sample_3, ground_truth_3)])
    ], reset=True)
    ```

    Test cases can be edited using the context-managed [`Editor`][kolena.workflow.TestCase.Editor] interface:

    ```python
    with TestCase("example-test-case").edit(reset=True) as editor:
        # perform desired editing actions within context
        editor.add(ts0, gt0)
    ```

=== "Test Suite"

    Create using [`TestSuite.create`][kolena.workflow.TestSuite.create]:

    ```python
    # throws if a test suite with name 'example-test-suite' already exists
    test_suite = TestSuite.create(
        "example-test-suite",
        # optionally include list of test cases to populate the new test suite
        # test_cases=[test_case0, test_case1, test_case2],
    )
    ```

    Load using [`TestSuite.load`][kolena.workflow.TestSuite.load]:

    ```python
    # throws if a test suite with name 'example-test-suite' does not exist
    test_suite = TestSuite.load("example-test-suite")
    ```

    Use the [`TestSuite`][kolena.workflow.TestSuite] constructor for idempotent create/load behavior:

    ```python
    # loads 'example-test-suite' or creates it if it does not already exist
    test_suite = TestSuite("example-test-suite")
    ```

    Test suites be edited using the context-managed [`Editor`][kolena.workflow.TestSuite.Editor] interface:

    ```python
    with TestSuite("example-test-suite").edit() as editor:
        editor.add(test_case_a)
        editor.remove(test_case_b)
        # perform desired editing actions within context
    ```

## Versioning

All test data on Kolena is versioned and immutable[^1]. Previous versions of test cases and test suites are always
available and can be visualized on the web and loaded programmatically by specifying a version.

[^1]: Immutability caveat: test suites, along with any test cases and test samples they hold, can be deleted on the
    [:kolena-test-suite-16: Test Suites](https://app.kolena.com/redirect/testing) page.

```python
# load a specific version of a test suite
test_suite_v2 = TestSuite.load("example-name", version=2)
```

## FAQ & Best Practices

??? faq "How should I map my existing benchmark into test cases and test suites?"

    To start, create a test suite containing a single test case for the complete benchmark. This single-test-case test
    suite represents standard, aggregate evaluation on a benchmark dataset.

    Once this test suite has been created, you can start creating test cases! Use the Studio, the Stratifier, or the
    Python client to create test cases slicing through (stratifying) this benchmark.

??? faq "How many test cases should a test suite include?"

    While test suites can hold anywhere from one to thousands of test cases, the sweet spot for the signal-to-noise
    ratio is in the dozens or low hundreds of test cases per test suite.

    Note that the relationship between benchmark dataset and test suite doesn't need to be 1:1. Often it can be useful
    to create different test suites for different stratification strategies applied to the same benchmark.

??? faq "How many samples should be included in a test case?"

    While there's no one-size-fits-all answer, we usually recommend including at least 100 samples in each test case.
    Smaller test cases can be used to provide a very rough signal about the presence or absence of a model beahvior, but
    shouldn't be relied upon for much more than a directional indication of performance.

    The multi-model Results comparison view in Kolena takes the number of test samples within a test case into account
    when highlighting improvements and regressions. The larger the test case, the smaller the ∆ required to consider a
    change from one model to another as "significant."

??? faq "How many negative samples should a test case include?"

    Many workflows, such as object detection or binary classification, have a concept of "negative" samples. In object
    detection, a "negative sample" is a sample (i.e. image) that does not include any objects to be detected.

    Negative samples can have a large impact on certain metrics. To continue with the object detection example, the
    **precision** metric depends on the number of false positive detections:

    $$
    \text{Precision} := \dfrac{\text{# True Positives}}{\text{# True Positives} + \text{# False Positives}}
    $$

    Therefore, since each negative sample has some likelihood of yielding false positive detections but no likelihood of
    yielding true positive detections, adding negative samples to a test case may decrease aggregate precision values
    computed across the test case.

    As a general rule of thumb, we recommend including **an even balance of positive and negative samples in each test
    case.** This composition minimizes the likelihood of different metrics being heavily skewed in one direction or
    another.
