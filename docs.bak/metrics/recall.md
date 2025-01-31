---
description: How to calculate and interpret recall to evaluate ML model performance
---

# Recall (TPR, Sensitivity)

<div class="grid" markdown>
<div markdown>
Recall, also known as **true positive rate** (TPR) and **sensitivity**, measures the proportion of all positive
ground truths that a model correctly predicts, ranging from 0 to 1 (where 1 is best).

As shown in this diagram, recall is the fraction of all positive ground truths that are correctly predicted:

$$\text{Recall} = \frac{\text{TP}}{\text{TP} + \text{FN}}$$

In the above formula, $\text{TP}$ is the number of true positive inferences and $\text{FN}$ is the number of false
negative ground truths.

!!! info "Guide: True Positive / False Negative"

    Read the [TP / FP / FN / TN](./tp-fp-fn-tn.md) guide if you're not familiar with "TP" and "FN" terminology.

</div>
![Recall Image](../assets/images/metrics-recall-light.svg#only-light)
![Recall Image](../assets/images/metrics-recall-dark.svg#only-dark)
</div>

<div class="grid cards" markdown>
- :kolena-manual-16: API Reference: [`recall` ↗][kolena.metrics.recall]
</div>

## Implementation Details

Recall is used across a wide range of workflows, including classification, object detection, instance segmentation,
semantic segmentation, and information retrieval. It is especially useful when the objective is to measure and reduce
**false negative** ground truths, i.e. model misses.

For most tasks, recall is the ratio of the number of correct positive inferences to the total number of positive
ground truths.

$$
\text{Recall} = \frac {\text{# True Positives}} {\text{# True Positives} + \text{# False Negatives}}
$$

For workflows with a localization component, such as object detection and instance segmentation, see the
[Geometry Matching](./geometry-matching.md) guide to learn how to compute true positive and false negative counts.

### Examples

Perfect model inferences, where every ground truth is recalled by an inference:

<div class="grid" markdown>
| Metric | Value |
| --- | --- |
| TP | 20 |
| FN | 0 |

$$
\begin{align}
\text{Recall} &= \frac{20}{20 + 0} \\[1em]
&= 1.0
\end{align}
$$
</div>

Partially correct inferences, where some ground truths are correctly recalled (TP) and others are missed (FN):

<div class="grid" markdown>
| Metric | Value |
| --- | --- |
| TP | 85 |
| FN | 15 |

$$
\begin{align}
\text{Recall} &= \frac{85}{85 + 15} \\[1em]
&= 0.85
\end{align}
$$
</div>

Zero correct inferences — no positive ground truths are recalled:

<div class="grid" markdown>
| Metric | Value |
| --- | --- |
| TP | 0 |
| FN | 20 |

$$
\begin{align}
\text{Recall} &= \frac{0}{0 + 20} \\[1em]
&= 0.0
\end{align}
$$
</div>

### Multiple Classes

So far, we have only looked at **binary** classification/object detection cases, but in **multiclass** or
**multi-label** cases, recall is computed per class. In the [TP / FP / FN / TN](./tp-fp-fn-tn.md) guide,
we went over multiple-class cases and how these metrics are computed. Once you have these four metrics computed per
class, you can compute recall for each class by treating each as a single-class problem.

### Aggregating Per-class Metrics

If you are looking for a **single** recall score that summarizes model performance across all classes, there are
different ways to aggregate per-class recall scores: **macro**, **micro**, and **weighted**. Read more about these
methods in the [Averaging Methods](./averaging-methods.md) guide.

## Limitations and Biases

As seen in its formula, recall only takes **positive** ground truths (TP and FN) into account; negative ground truths
(TN and FP) are not considered. Thus, recall only provides one half of the picture, and should always be used in
tandem with [precision](./precision.md): precision penalizes false positives (FP), whereas recall does not.

For a single metric that takes both precision and recall into account,
use F<sub>1</sub>-score, which is the harmonic mean between precision and recall.
