---
description: How to calculate and interpret accuracy to evaluate ML model performance
---

# Accuracy

Accuracy is one of the most well-known metrics in machine learning model evaluation because it is simple to understand
and straightforward to calculate.

Accuracy measures how often a model correctly predicts something (ranging from 0 to 1, with 1 being perfect
inferences). It reports the ratio of the number of correct inferences to the total number of inferences,
making it a good metric for assessing model performance in simple cases with balanced data. However, accuracy is much
less meaningful with imbalanced datasets (e.g. far more negative ground truths than positive ground truths)
and should be used with caution.

<div class="grid cards" markdown>
- :kolena-manual-16: API Reference: [`accuracy` ↗][kolena.metrics.accuracy]
</div>

!!!example
    To see an example of Accuracy metric in use, checkout the
    [MMLU dataset on app.kolena.com/try](https://app.kolena.io/try/dataset/standards?datasetId=32&models=N4IglgJiBcDsDMAaEBjA9gOwGZgOYFcAnAQwBcxMZRIYBGAFgF9Gg&models=N4IglgJiBcBsCcAaEBjA9gOwGZgOYFcAnAQwBcxMZRIYBGAFgF9Gg&models=N4IglgJiBcDsBMAaEBjA9gOwGZgOYFcAnAQwBcxMZRIYBGAFgF9Gg&metricGroupVisibilities=N4IgbglgzhBGA2BTEAuALgJwK6IDQgFtFMIBjKVAbVEhgWXW0QF9cbo4lVMdX26ujXgF1mQA)

## Implementation Details

Accuracy is generally used to evaluate classification models. Aside from classification, accuracy is also often
used to evaluate semantic segmentation models by measuring the percent of correctly classified pixels in an image.

In a classification workflow, accuracy is the ratio of the number of correct inferences to the total number of inferences.

With [TP / FP / FN / TN counts](./tp-fp-fn-tn.md) computed, accuracy is defined:

$$
\text{Accuracy} =  \frac {\text{TP} + \text{TN}} {\text{TP} + \text{FP} + \text{FN} + \text{TN}}
$$

### Examples

Perfect inferences:

<div class="grid" markdown>
| Metric | Value |
| --- | --- |
| TP | 10 |
| FP | 0 |
| FN | 0 |
| TN | 10 |

$$
\begin{align}
\text{Accuracy} &= \frac{10 + 10}{10 + 0 + 0 + 10} \\[1em]
&= 1.0
\end{align}
$$
</div>

Partially correct inferences:

<div class="grid" markdown>
| Metric | Value |
| --- | --- |
| TP | 8 |
| FP | 4 |
| FN | 2 |
| TN | 6 |

$$
\begin{align}
\text{Accuracy} &= \frac{8 + 6}{8 + 4 + 2 + 6} \\[1em]
&= 0.7
\end{align}
$$
</div>

Highly imbalanced data, with 990 negative ground truths and 10 positive ground truths, with _no_ positive inferences:

<div class="grid" markdown>
| Metric | Value |
| --- | --- |
| TP | 0 |
| FP | 0 |
| FN | 10 |
| TN | 990 |

<div markdown>
$$
\begin{align}
\text{Accuracy} &= \frac{0 + 990}{0 + 0 + 10 + 990} \\[1em]
&= 0.99
\end{align}
$$

!!! warning "Be careful with imbalanced datasets!"

    This example describes a trivial model that only ever returns negative inferences, yet it has the high accuracy
    score of 99%.
</div>
</div>

## Limitations and Biases

While accuracy generally describes a classifier’s performance, it is important to note that the metric can be deceptive,
especially when [the data is imbalanced](https://stephenallwright.com/imbalanced-data/).

For example, let’s say there
are a total of 500 ground truths, with 450 belonging to the positive class and 50 to the negative.
If the model correctly predicts all the positive ground truths but misses all the negative ones,
its accuracy is `450 / 500 = 0.9`. An accuracy score of 90% indicates a pretty good model —
but is a model that fails 100% of the time on negative ground truths useful?
Using the accuracy metric alone can hide a model’s true performance,
so we recommend other metrics that are better suited for
imbalanced data, such as:

- [Balanced accuracy](https://stephenallwright.com/balanced-accuracy/)
- [Precision](./precision.md)
- [Recall](./recall.md)
- [F<sub>1</sub>-score](./f1-score.md)
