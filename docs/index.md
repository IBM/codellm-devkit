---
icon: cldk/layers-20
hide:
  - toc
---

![CLDK](./assets/images/cldk-light.png#only-light)
![CLDK](./assets/images/cldk-dark.png#only-dark)

<p align='center'>
  <a href="https://arxiv.org/abs/2410.13007">
    <img src="https://img.shields.io/badge/arXiv-2410.13007-b31b1b.svg" />
  </a>
  <a href="https://www.python.org/downloads/release/python-3110/">
    <img src="https://img.shields.io/badge/python-3.11-blue.svg" />
  </a>
  <a href="https://opensource.org/licenses/Apache-2.0">
    <img src="https://img.shields.io/badge/License-Apache%202.0-green.svg" />
  </a>
  <a href="https://ibm.github.io/codellm-devkit/">
    <img src="https://img.shields.io/badge/GitHub%20Pages-Docs-blue" />
  </a>
  <a href="https://badge.fury.io/py/cldk">
    <img src="https://badge.fury.io/py/cldk.svg" />
  </a>
</p>

---

[Codellm-devkit](https://cldk.info) (aka. CLDK) is a multilingual program analysis framework that bridges the gap between traditional static analysis tools and Large Language Models (LLMs) specialized for code (CodeLLMs). CLDK simplifies multi-language code analysis by providing a unified Python library that integrates outputs from various analysis tools and prepares them for effective use by CodeLLMs.

CLDK streamlines the process of transforming raw code into actionable insights, enabling robust analysis pipelines and seamless integration with tools like WALA, Tree-sitter, LLVM, and CodeQL.

---

# :cldk-layers-20: Developer Guide

Learn how to use Codellm-devkit to analyze your code effectively:

<div class="grid cards" markdown>

- [:cldk-flame-16: Quickstart](quickstart.md)

    ---

    Run through an example to quickly set up CLDK and perform multilingual code analysis.

- [:cldk-developer-16: Installing `cldk`](installing.md)

    ---

    Install and initialize the `cldk` Python package to start analyzing your codebases.

- [:cldk-area-of-interest-16: Core Concepts](core-concepts/index.md)

    ---

    Explore the key components of CLDK—including data models and analysis backends—that simplify code analysis workflows.

- [:cldk-manual-16: API Reference](reference/index.md)

    ---

    Developer-focused, detailed API reference documentation for `cldk`.

</div>
---

## Why Codellm-devkit?

!!! tip inline "TL;DR"

    CLDK unifies traditional program analysis tools with CodeLLMs, streamlining multi-language code analysis into a single, cohesive framework.

Current code analysis often involves juggling multiple disjointed tools and workflows. With Codellm-devkit, you can:

- **Unified**: Integrate various analysis tools and CodeLLMs into one cohesive framework.
- **Extensible**: Easily add support for new tools and evolving LLM platforms.
- **Streamlined**: Simplify the transformation of raw code into structured, actionable insights.

By providing a consistent and extensible interface, CLDK reduces friction and accelerates the development of robust analysis pipelines.

---

## Contact

For any questions, feedback, or suggestions, please contact the authors:

| Name           | Email                                    |
| -------------- | ---------------------------------------- |
| Rahul Krishna  | [i.m.ralk@gmail.com](mailto:imralk+oss@gmail.com) |
| Rangeet Pan    | [rangeet.pan@ibm.com](mailto:rangeet.pan@gmail.com) |
| Saurabh Sihna  | [sinhas@us.ibm.com](mailto:sinhas@us.ibm.com) |