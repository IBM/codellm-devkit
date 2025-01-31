---
icon: kolena/take-action-16

---

# :kolena-take-action-20: Programmatically Compare Models

!!! example "Experimental Feature"

    Experimental features are under active development and may occasionally undergo API-breaking changes.

## Download Quality Standard Result

The **Quality Standard** should contain the key performance metrics a team uses to evaluate a model's performance on a
dataset.

The SDK provides a
function, [`download_quality_standard_result`](../../reference/experimental/index.md#kolena._experimental.quality_standard.download_quality_standard_result),
to download a dataset's quality standard result. This enables users to automate processes surrounding a Quality
Standard's result.

The return value is a multi-index DataFrame with indices `(stratification, test_case)` and columns `(model, eval_config,
metric_group, metric)`.

<figure markdown>
![Quality Standard Result](../../assets/images/quality_standard_diagram_light.jpeg#only-light)
![Quality Standard Result](../../assets/images/quality_standard_diagram_dark.jpeg#only-dark)
<figcaption>Quality Standard Result</figcaption>
</figure>

### Use Case: Continuous Integration

In order to automate deployment decisions with Kolena a team could:

1. Define the metric requirements a model must meet in order to be considered for deployment.
2. [Upload model results](../../reference/dataset/index.md#kolena.dataset.evaluation.upload_results) as part of a CI/CD
   pipeline.
3. [Download the dataset's quality standard
   results](../../reference/experimental/index.md#kolena._experimental.quality_standard.download_quality_standard_result)
   and programmatically compare against the defined criteria.
4. Proceed to the next stage of the CI/CD pipeline based on the outcome of the assessment. For instance:
    * if a model surpasses all the defined thresholds, it is promoted.
    * if a model partially surpasses the defined thresholds, it is a promotion candidate.
    * if a model surpasses none of the defined thresholds, it is not promoted.
