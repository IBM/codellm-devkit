---
description: Understanding true negative rate (specificity) to evaluate ML model performance
---

# Specificity (TNR)

<div class="grid" markdown>
<div markdown>
Specificity, also known as **true negative rate** (TNR), measures the proportion of negative ground truths that a
model correctly predicts, ranging from 0 to 1. A high specificity indicates that the model is effective at correctly
identifying negative cases, where a low specificity suggests that the model is misclassifying many negative cases as
positive.

As shown in this diagram, specificty is the fraction of all negative ground truths that are correctly predicted:

$$\text{Specificity} = \frac{\text{TN}}{\text{TN} + \text{FP}}$$

In the above formula, $\text{TN}$ is the number of true negative inferences and $\text{FP}$ is the number of false
positive inferences.

!!! info "Guide: True Negative / False Positive"

    Read the [TP / FP / FN / TN](./tp-fp-fn-tn.md) guide if you're not familiar with "TN" and "FP" terminology.

</div>
![Specificity Image](../assets/images/metrics-specificity-light.svg#only-light)
![Specificity Image](../assets/images/metrics-specificity-dark.svg#only-dark)
</div>

Specificity is often used in conjunction with [sensitivity (recall)](./recall.md), also known **true positive rate**
(TPR). By measuring both specificity and sensitivity, a more complete picture of a model's performance can be drawn.

<div class="grid cards" markdown>
- :kolena-manual-16: API Reference: [`Specificity` ↗][kolena.metrics.specificity]
</div>

## Implementation Details

Specificity is used to evaluate the performance of a classification model, particularly in tasks like binary
classification, where the goal is to classify data into one of two possible classes. It is especially useful when the
objective is to measure the model's ability to correctly identify the negative class instances. For example, in medical
diagnostics, a high specificity is essential to avoid unnecessary treatments or alarm.

Here is how specificity is calculated:

$$
\text{Specificity} = \frac {\text{# True Negatives}} {\text{# True Negatives} + \text{# False Positives}}
$$

### Examples

Perfect model inferences, where every negative ground truth is recalled by an inference:

<div class="grid" markdown>
| Metric | Value |
| --- | --- |
| TN | 20 |
| FP | 0 |

$$
\begin{align}
\text{Specificity} &= \frac{20}{20 + 0} \\[1em]
&= 1.0
\end{align}
$$
</div>

Partially correct inferences, where some negative ground truths are correctly recalled (TN) and others are missed (FP):

<div class="grid" markdown>
| Metric | Value |
| --- | --- |
| TN | 85 |
| FP | 15 |

$$
\begin{align}
\text{Specificity} &= \frac{85}{85 + 15} \\[1em]
&= 0.85
\end{align}
$$
</div>

Zero correct inferences — no negative ground truths are recalled:

<div class="grid" markdown>
| Metric | Value |
| --- | --- |
| TN | 0 |
| FP | 20 |

$$
\begin{align}
\text{Specificity} &= \frac{0}{0 + 20} \\[1em]
&= 0.0
\end{align}
$$
</div>

### Multiple Classes

So far, we have only looked at **binary** classification cases, but in **multiclass** or **multi-label** cases,
specificity is computed per class. In the [TP / FP / FN / TN](./tp-fp-fn-tn.md) guide,
we went over multiple-class cases and how these metrics are computed. Once you have these four metrics computed per
class, you can compute specificity for each class by treating each as a single-class problem.

### Aggregating Per-class Metrics

If you are looking for a **single** specificity score that summarizes model performance across all classes, there are
different ways to aggregate per-class specificity scores: **macro**, **micro**, and **weighted**. Read more about these
methods in the [Averaging Methods](./averaging-methods.md) guide.

## Limitations and Biases

While specificity is a valuable metric for evaluating the performance of classification models, it does have limitations
and potential biases that should be considered when interpreting results:

1. **Sensitivity to Class Imbalance**: Specificity is sensitive to class imbalance in dataset. If one class significantly
outnumbers the other, a high specificity can be achieved simply by predicting the majority class most of the time. This
can lead to a misleadingly high specificity score while neglecting the model's ability to correctly classify the
minority class.

2. **Ignoring False Negatives**: Specificity focuses exclusively on the true negatives (correctly classified negative cases)
and false positives (negative cases incorrectly classified as positive), but it doesn't account for false negatives
(positive cases incorrectly classified as negative). Ignoring false negatives can be problematic in applications where
missing positive cases is costly or has severe consequences.

3. **Incomplete Context**: Specificity alone does not provide a complete picture of a model's performance. It is often
used in conjunction with other metrics like sensitivity (recall), precision, and F<sub>1</sub>-score to provide a more
comprehensive assessment. Depending solely on specificity might hide issues related to other aspects of classification,
such as the models' ability to identify true positives.

4. **Threshold Dependence**: Specificity is a binary metric that doesn't take into account the probability
or confidence levels associated with predictions. Models with different probability distributions might achieve the
same specificity score, but their operational characteristics can vary significantly. To address this limitation,
consider using threshold-independent metrics like the area under the receiver operating characteristic curve (AUC-ROC)
which can provide a more comprehensive understanding of model performance.
