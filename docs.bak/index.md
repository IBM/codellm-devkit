---
icon: kolena/layers-16
hide:
  - toc
---

<figure markdown>
  ![Kolena](assets/images/wordmark-violet.svg#only-light){ width="400" }
  ![Kolena](assets/images/wordmark-white.svg#only-dark){ width="400" }
</figure>

<p align='center'>
  <a href="https://pypi.python.org/pypi/kolena">
    <img
      src="https://img.shields.io/pypi/v/kolena?logo=python&logoColor=white&style=flat-square"
    />
  </a>
  <a href="https://www.apache.org/licenses/LICENSE-2.0">
    <img
      src="https://img.shields.io/pypi/l/kolena?style=flat-square"
    />
  </a>
  <a href="https://github.com/kolenaIO/kolena/actions">
    <img
      src="https://img.shields.io/github/checks-status/kolenaIO/kolena/trunk?logo=circleci&logoColor=white&style=flat-square"
    />
  </a>
  <a href="https://codecov.io/gh/kolenaIO/kolena" >
    <img
      src="https://img.shields.io/codecov/c/github/kolenaIO/kolena?logo=codecov&logoColor=white&style=flat-square&token=8WOY5I8SF1"
    />
  </a>
</p>

---

[Kolena](https://www.kolena.com) is a comprehensive machine learning testing and debugging platform to surface hidden
model behaviors and take the mystery out of model development. Kolena helps you:

- Perform high-resolution model evaluation
- Understand and track behavioral improvements and regressions
- Meaningfully communicate model capabilities
- Automate model testing and deployment workflows

Kolena organizes your test data, stores and visualizes your model evaluations, and provides tooling to craft better
tests. You interface with it through the web at [app.kolena.com](https://app.kolena.com) and programmatically via the
[`kolena`](installing-kolena.md) SDK.

---

# :kolena-layers-20: Developer Guide

Learn how to use Kolena to test your models effectively:

<div class="grid cards" markdown>

- [:kolena-flame-16: Quickstart](dataset/quickstart.md)

    ---

    Run through an example using Kolena to set up rigorous and repeatable model testing in minutes.

- [:kolena-developer-16: Installing `kolena`](installing-kolena.md)

    ---

    Install and initialize the `kolena` Python package, the programmatic interface to Kolena.

- [:kolena-area-of-interest-16: Core Concepts](dataset/core-concepts/index.md)

    ---

    Core concepts for testing in Kolena.

- [:kolena-manual-16: API Reference](reference/index.md)

    ---

    Developer-focused detailed API reference documentation for `kolena`.

</div>
---

## Why Kolena?

!!! tip inline "TL;DR"

    Kolena helps you test your ML models more effectively.

    **Jump right in with the [<nobr>:kolena-flame-16: Quickstart</nobr>](dataset/quickstart.md) guide**.

Current ML evaluation techniques are falling short. Engineers run inference on arbitrarily split benchmark datasets,
spend weeks staring at error graphs to evaluate their models, and ultimately produce a global metric that fails to
capture the true behavior of the model.

Models exhibit highly variable performance across different subsets of a domain. A global metric gives you a high-level
picture of performance but doesn't tell you what you really want to know:
_what sort of behavior can I expect from my model in production?_

To answer this question you need a higher-resolution picture of model performance. Not "how well does my model perform
on class X," but "in what scenarios does my model perform well for class X?"

![Looking at global metric, Model A seems far inferior to Model B.](assets/images/test-case-diff-light.png#only-light)
![Looking at global metric, Model A seems far inferior to Model B.](assets/images/test-case-diff-dark.png#only-dark)

In the above example, looking only at global metric (e.g. F1 score), we'd almost certainly choose to deploy Model B.

But what if the "High Blur" scenario isn't important for our product? Most of Model A's failures are from that
scenario, and it outperforms Model B in more important scenarios like "Front View." Meanwhile, Model B's
underperformance in "Front View," a highly important scenario, is masked by improved performance in
the unimportant "High Blur" scenario.

!!! tip "Test data is more important than training data!"

    Everything you know about a new model's behavior is learned from your tests.

    Fine-grained tests teach you what you need to learn before a model hits production.

Now... why Kolena? Two reasons:

1. **Managing fine-grained tests is a tedious data engineering task**, especially under changing data circumstances as
   your dataset grows and your understanding of your domain develops
2. **Creating fine-grained tests is labor-intensive** and typically involves manual annotation of countless images, a
   costly and time-consuming process

We built Kolena to solve these two problems.

## Read More

- Best Practices for ML Model Testing ([Kolena Blog](https://www.kolena.com/blog/best-practices-for-ml-model-testing))
- Hidden Stratification Causes Clinically Meaningful Failures in Machine Learning for Medical Imaging ([arXiv:1909.12475](https://arxiv.org/abs/1909.12475))
- No Subclass Left Behind: Fine-Grained Robustness in Coarse-Grained Classification Problems ([arXiv:2011.12945](https://arxiv.org/abs/2011.12945))

---
