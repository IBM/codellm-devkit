---
hide:
  - toc
search:
  boost: -0.5
---

# :kolena-rocket-20: Advanced Usage

This section contains tutorial documentation for advanced features for Kolena Workflows.

<div class="grid cards" markdown>
- [:octicons-container-24: Packaging for Automated Evaluation](packaging-for-automated-evaluation.md)

    ---

    Package [metrics evaluation logic](../../reference/workflow/evaluator.md) in a Docker container image to dynamically
    compute metrics on relevant subsets of your test data.

- [:kolena-diagram-tree-16: Nesting Test Case Metrics](nesting-test-case-metrics.md)

    ---

    Report class-level metrics within a test case and test ensembles and pipelines of models by nesting aggregate
    metrics within your [`MetricsTestCase`][kolena.workflow.MetricsTestCase].

- [:kolena-heatmap-16: Uploading Activation Maps](uploading-activation-maps.md)

    ---

    Upload and visualize your activation map for each [`TestSample`][kolena.workflow.TestSample] along with your model
    results on the [<nobr>:kolena-studio-16: Studio</nobr>](https://app.kolena.com/redirect/studio).

</div>
