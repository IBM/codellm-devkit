---
description: How to calculate and interpret f1 score to evaluate ML model performance
---

# F<sub>1</sub>-score

The **F<sub>1</sub>-score**, also known as **balanced F-score** or **F-measure**, is a metric that combines two
competing metrics, [precision](./precision.md) and [recall](./recall.md), with an equal weight. F<sub>1</sub>-score is
the harmonic mean between precision and recall, and symmetrically represents both in one metric.

!!! info inline end "Guides: Precision and Recall"

    Read the [precision](./precision.md) and the [recall](./recall.md) guides if you're not familiar with those metrics.

Precision and recall offer a trade-off: increasing precision often reduces recall, and vice versa. This is called
the **precision/recall trade-off**.

Ideally, we want to maximize both precision and recall to obtain the perfect model. This is where the
F<sub>1</sub>-score comes in play. Because the F<sub>1</sub>-score is the
[**harmonic mean**](https://en.wikipedia.org/wiki/Harmonic_mean) of precision and recall, maximizing the
F<sub>1</sub>-score implies simultaneously maximizing both precision and recall.
Thus, the F<sub>1</sub>-score has become a popular metric for the evaluation of many workflows, such as classification,
object detection, semantic segmentation, and information retrieval.

<div class="grid cards" markdown>
- :kolena-manual-16: API Reference: [`f1_score` ↗][kolena.metrics.f1_score]
</div>

!!!example
    To see an example of of the F1 Score, checkout the
    [Object Detection (COCO 2014) on app.kolena.com/try.](https://app.kolena.io/try/dataset/standards?datasetId=14&models=N4IglgJiBcBMCsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIYAWAX3qA&models=N4IglgJiBcBMAsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIZ4BfOoA&metricGroupVisibilities=N4IgbglgzhBGA2BTEAuALgJwK6IDQgFtFMIBjKVAbVEhgWXW0QF9cbo4lVMdX26ujXm3Ad63JswC6zIA)

## Implementation Details

Using [TP / FP / FN / TN](./tp-fp-fn-tn.md), we can define [precision](./precision.md) and [recall](./recall.md).
The F<sub>1</sub>-score is computed by taking the **harmonic mean** of **precision** and **recall**.

The F<sub>1</sub>-score is defined:

$$
\begin{align}
\text{F}_1 &= \frac {2} {\frac {1} {\text{Precision}} + \frac {1} {\text{Recall}}} \\[1em]
&= \frac {2 \times \text{Precision} \times \text{Recall}} {\text{Precision} + \text{Recall}}
\end{align}
$$

It can also be calculated directly from **true positive** (TP) / **false positive** (FP) / **false negative** (FN)
counts:

$$
\text{F}_1 = \frac {\text{TP}} {\text{TP} + \frac 1 2 \left( \text{FP} + \text{FN} \right)}
$$

### Examples

Perfect inferences:

<div class="grid" markdown>
| Metric | Value |
| --- | --- |
| TP | 20 |
| FP | 0 |
| FN | 0 |

$$
\begin{align}
\text{Precision} = \frac{20}{20 + 0} &= 1.0 \\[1em]
\text{Recall} = \frac{20}{20 + 0} &= 1.0 \\[1em]
\text{F}_1 = \frac{20}{20 + \frac 1 2 \left( 0 + 0 \right)} &= 1.0
\end{align}
$$
</div>

Partially correct inferences, where every ground truth is recalled by an inference:

<div class="grid" markdown>
| Metric | Value |
| --- | --- |
| TP | 25 |
| FP | 75 |
| FN | 0 |

$$
\begin{align}
\text{Precision} = \frac{25}{25 + 75} &= 0.25 \\[1em]
\text{Recall} = \frac{25}{25 + 0} &= 1.0 \\[1em]
\text{F}_1 = \frac{25}{25 + \frac 1 2 \left( 75 + 0 \right)} &= 0.4
\end{align}
$$
</div>

Perfect inferences but some ground truths are missed:
<div class="grid" markdown>
| Metric | Value |
| --- | --- |
| TP | 25 |
| FP | 0 |
| FN | 75 |

$$
\begin{align}
\text{Precision} = \frac{25}{25 + 0} &= 1.0 \\[1em]
\text{Recall} = \frac{25}{25 + 75} &= 0.25 \\[1em]
\text{F}_1 = \frac{25}{25 + \frac 1 2 \left( 0 + 75 \right)} &= 0.4
\end{align}
$$
</div>

Zero correct inferences with non-zero false positive and false negative:
<div class="grid" markdown>
| Metric | Value |
| --- | --- |
| TP | 0 |
| FP | 15 |
| FN | 10 |

$$
\begin{align}
\text{Precision} = \frac{0}{0 + 15} &= 0.0 \\[1em]
\text{Recall} = \frac{0}{0 + 10} &= 0.0 \\[1em]
\text{F}_1 = \frac{0}{0 + \frac 1 2 \left( 15 + 10\right)} &= 0.0
\end{align}
$$
</div>

Zero correct inferences with zero false positive and false negative:
<div class="grid" markdown>
<div markdown>
| Metric | Value |
| --- | --- |
| TP | 0 |
| FP | 0 |
| FN | 0 |

!!! warning "Undefined F<sub>1</sub>"

    This example shows an edge case where both precision and recall are `undefined`. When either metric is `undefined`,
    F<sub>1</sub> is also `undefind`. In such cases, it's often interpreted as `0.0` instead.

</div>
$$
\begin{align}
\text{Precision} &= \frac{0}{0 + 0} \\[1em]
&= \text{undefined} \\[1em]
\text{Recall} &= \frac{0}{0 + 0} \\[1em]
&= \text{undefined} \\[1em]
\text{F}_1 &= \frac{0}{0 + \frac 1 2 \left( 0 + 0\right)} \\[1em]
&= \text{undefined} \\[1em]
\end{align}
$$
</div>

### Multiple Classes

In workflows with multiple classes, the F<sub>1</sub>-score can be computed per class. In the [TP / FP / FN / TN](./tp-fp-fn-tn.md)
guide, we learned how to compute per-class metrics when there are multiple classes, using the [one-vs-rest](./tp-fp-fn-tn.md#multiclass)
(OvR) strategy. Once you have TP, FP, and FN counts computed for each class, you can compute precision, recall, and
F<sub>1</sub>-score for each class by treating each as a single-class problem.

### Aggregating Per-class Metrics

If you are looking for a **single** F<sub>1</sub>-score that summarizes model performance across all classes, there are
different ways to aggregate per-class F<sub>1</sub>-scores: **macro**, **micro**, and **weighted**. Read more about
these methods in the [Averaging Methods](./averaging-methods.md) guide.

## F$_\beta$-score

The **F$_\beta$-score** is a generic form of the F<sub>1</sub>-score with a weight parameter, $\beta$, where
recall is considered $\beta$ times more important than precision:

<!-- markdownlint-disable MD013 -->

$$
\text{F}_{\beta} = \frac {(1 + \beta^2) \times \text{precision} \times \text{recall}} {(\beta^2 \times \text{precision}) + \text{recall}}
$$

<!-- markdownlint-enable MD013 -->

The three most common values for the beta parameter are as follows:

- **F<sub>0.5</sub>-score** $\left(\beta = 0.5\right)$,
  where precision is more important than recall, it focuses more on minimizing FPs than minimizing FNs
- **F<sub>1</sub>-score** $\left(\beta = 1\right)$,
  the true harmonic mean of precision and recall
- **F<sub>2</sub>-score** $\left(\beta = 2\right)$,
  where recall is more important than precision, it focuses more on minimizing FNs than minimizing FPs

## Limitations and Biases

While the F<sub>1</sub>-score can be used to evaluate classification/object detection models with a single metric,
this metric is not adequate to use for all applications. In some applications, such as identifying pedestrians from an
autonomous vehicle, any false negatives can be life-threatening. In these scenarios, having a few more false positives
as a trade-off for reducing the chance of any life-threatening events happening is preferred. Here, recall should be
weighted much more than the precision as it minimizes false negatives. To address the significance of recall,
**$\text{F}_\beta$ score** can be used as an alternative.

### Threshold-Dependence

Precision, recall, and F<sub>1</sub>-score are all **threshold-dependent** metrics. Threshold-dependent means that,
before computing these metrics, a confidence score threshold must be applied to inferences to decide which should be
used for metrics computation and which should be ignored.

A small change to this confidence score threshold can have a large impact on threshold-dependent metrics. To evaluate
a model across _all_ thresholds, rather than at a single-threshold, use threshold-independent metrics, like
**average precision**.
