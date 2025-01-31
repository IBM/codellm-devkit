---
subtitle: Macro, Micro, Weighted
description: When to use macro, micro, or weighted average for metrics
---

# Averaging Methods

For multiclass workflows like classification or object detection, metrics such as [precision](./precision.md),
[recall](./recall.md), and [F<sub>1</sub>-score](./f1-score.md) are computed **per class**.
To compute a single value that represents model performance across all classes,
these per-class scores need to be aggregated. There are a few different averaging methods for doing this,
most notably:

- [**Macro**](#macro-average): unweighted mean of all per-class scores
- [**Micro**](#micro-average): global average of per-sample TP, FP, FN scores
- [**Weighted**](#weighted-average): mean of all per-class scores, weighted by sample sizes for each class

!!!example
    To see an example of Macro Averaging Method, checkout the
    [KITTI Vision Benchmark Suite on app.kolena.com/try.](https://app.kolena.io/try/dataset/standards?datasetId=44&models=N4IglgJiBcAcCsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIYBGAX3qA&models=N4IglgJiBcAcAsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIYBGAX3qA&metricGroupVisibilities=N4IgbglgzhBGA2BTEAuALgJwK6IDQgFtFMIBjKVAbVEhgWXW0QF9cbo4lVMdX26ujXm3Ad63JswC6zIA)

## Example: Multiclass Classification

Let’s consider the following multiclass classification metrics, computed across a total of 10 samples:

| Class | # Samples | # True Positives  | # False Positives | # False Negatives | Precision | Recall | F1-score |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `Airplane` | 3 | 2 | 1 | 1 | 0.67 | 0.67 | 0.67 |
| `Boat` | 1 | 1 | 3 | 0 | 0.25 | 1.0 | 0.4 |
| `Car` | 6 | 3 | 0 | 3 | 1.0 | 0.5 | 0.67 |
| Total | 10 | 6 | 4 | 4 | - | - | - |

### Macro Average

**Macro average** is perhaps the most straightforward among the numerous options and is computed by taking an
**unweighted** mean of all the per-class scores:

<!-- markdownlint-disable MD013 -->

$$
\begin{align}
\text{F}_{1 \, \text{macro}} &= \frac{\text{F}_{1 \, \texttt{Airplane}} + \text{F}_{1 \, \texttt{Boat}} + \text{F}_{1 \, \texttt{Car}}}{3} \\[1em]
&= \frac{0.67 + 0.4 + 0.67}{3} \\[1em]
&= 0.58
\end{align}
$$

<!-- markdownlint-enable MD013 -->

### Micro Average

In contrast to macro, **micro average** computes a global average by counting the sums of [true positive (TP), false
negative (FN) and false positive (FP)](./tp-fp-fn-tn.md).

**Micro precision** and **micro recall** are computed with the standard precision and recall formulas, using the total
TP/FP/FN counts across all classes:

<div class="grid" markdown>
$$
\begin{align}
\text{Precision}_\text{micro} &= \frac{\text{TP}_\text{Total}}{\text{TP}_\text{Total} + \text{FP}_\text{Total}} \\[1em]
&= \frac{6}{6 + 4} \\[1em]
&= 0.6
\end{align}
$$

$$
\begin{align}
\text{Recall}_\text{micro} &= \frac{\text{TP}_\text{Total}}{\text{TP}_\text{Total} + \text{FN}_\text{Total}} \\[1em]
&= \frac{6}{6 + 4} \\[1em]
&= 0.6
\end{align}
$$
</div>

What about **micro F<sub>1</sub>**?
Plug the micro-averaged values for precision and recall into the standard formula for F<sub>1</sub>-score:

<!-- markdownlint-disable MD013 -->

$$
\begin{align}
\text{F}_{1 \, \text{micro}} &= 2 \times \frac{\text{Precision}_\text{micro} \times \text{Recall}_\text{micro}}{\text{Precision}_\text{micro} + \text{Recall}_\text{micro}} \\[1em]
&= 2 \times \frac{0.6 \times 0.6}{0.6 + 0.6} \\[1em]
&= 0.6
\end{align}
$$

<!-- markdownlint-enable MD013 -->

Note that precision, recall, and F<sub>1</sub>-score all have the same value: $0.6$. This is because micro-averaging essentially
computes the proportion of correctly classified instances out of all instances, which is the definition of overall
**accuracy**.

In the multiclass classification cases where each sample has a single label, we get the following:

$$
\text{F}_{1 \, \text{micro}} = \text{Precision}_\text{micro} = \text{Recall}_\text{micro} = \text{Accuracy}
$$

### Weighted Average

**Weighted average** computes the mean of all per-class scores while considering each class’s **support**. In this case,
support is the number of actual instances of the class in the dataset.

For example, if there are 3 samples of class `Airplane`, then the support value of class `Airplane` is 3. In other
words, support is the sum of true positive (TP) and false negative (FN) counts. The weight is the proportion of each
class’s support relative to the sum of all support values:

<!-- markdownlint-disable MD013 -->

$$
\begin{align}
\text{F}_{1 \, \text{weighted}} &= \left( \text{F}_{1 \, \texttt{Airplane}} \times \tfrac{\text{#}\ \texttt{Airplane}}{\text{# Total}} \right) +
\left( \text{F}_{1 \, \texttt{Boat}} \times \tfrac{\text{#}\ \texttt{Boat}}{\text{# Total}} \right) +
\left( \text{F}_{1 \, \texttt{Car}} \times \tfrac{\text{#}\ \texttt{Car}}{\text{# Total}} \right) \\[1em]
&= \left( 0.67 \times \tfrac{3}{10} \right) + \left( 0.4 \times \tfrac{1}{10} \right) + \left( 0.67 \times \tfrac{6}{10} \right) \\[1em]
&= 0.64
\end{align}
$$

<!-- markdownlint-enable MD013 -->

## Which Method Should I Use?

You would generally use these three methods to aggregate the metrics computed per class. Averaging is most commonly used
in multiclass/multi-label classification and object detection tasks.

So which average should you use?

If you’re looking for an easily understandable metric for overall model performance regardless of class,
**micro average** is probably best.

If you want to treat all classes equally, then using **macro average** would be a good choice.

If you have an imbalanced dataset but want to assign more weight to classes with more samples, consider using
**weighted average** instead of macro average.
