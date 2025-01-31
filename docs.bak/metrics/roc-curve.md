---
subtitle: AUC ROC
description: How to create and interpret ROC curves to evaluate ML model performance
---

# ROC Curve

A receiver operating characteristic (ROC) curve is a plot that is used to evaluate the performance of binary
classification models by using the [true positive rate (TPR)](./recall.md) and the
[false positive rate (FPR)](./fpr.md). The curve is built with the TPR on the y-axis and the FPR on the x-axis computed
across many thresholds, showing a trade-off of how TPR and FPR values change when a classification threshold changes.

!!!example
    To see an example of the ROC cureve, chekcout the
    [CIFAR-10 on app.kolena.com/try.](https://app.kolena.io/try/dataset/debugger?datasetId=20&aggregations=N4IgNghgRgpmIC4QEEDGqCuAnCqCeIANCAM4D22qMiIWMJGYALkSBAObt3sRNlaJQHLjB58BSXJhz5WEAG4wc7AJYA7dgFkYTABZkAJjQC2uLGRABfYgAcIOYyUEguFNQYAqWDHoBiKuCMkVwx3AH0mbz0AOkhYeGJ1ADMlGDUqf0CaVEgSEhUklVReFTI1WOg4Vj06En0wILVGMET0sAwDGDCIDFREJIgwEhhrECT%2BUxYkGyUqNSYOamJdFXZdMFXdKdAVtaUASRIAIR0mJURIjBHiEJsAOQhjaiRtSKKAAgBxcwwbKyA&aggregations=N4IgNghgRgpmIC4QAUBOMDGBLAzlg9gHYAEAFALIQar4CUIANCDvgK6oYyIjo6tgAXRiAgBzUelEQB%2BVIlBiJMKTLlIADumx4iwiADcYqMVkKjyMAQAt8AE24BbKjRABfJuojGHOeSAlshLYAKqis1gBiWHD2SAGsQQD6AmHWAHSQsPBMpgBmRjCEnFEx3BiQOHi5WBjSBIQZ0HDC1rw2YLGE-GA5RWCstjCJEKwYiLkQYDgw7iC5sk5CGkachAJiXExWWKJWYDtWS6Dbu0YAkjgAQpYCRogprDNM8eoAchAOXEgWKTXEAOI0VjqNxAA&aggregations=N4IgNghgRgpmIC4QCUYGMJjAAgBQFkI0AnAewEoQAaEAZ1IFdi0ZERiZaGwAXakCAHNBHQRB6liiUEJEwxEqUg4Ys-CADcYxIQEsAdoPwweAC1IATNgFsiZEAF8aABwg7rtaSBGN9FgCrEDGYAYrpwVkg%2BDH4A%2BjxBZgB0kLDwNAYAZtow%2BixhEWxokLS0upm6GDy6pPop0HD8Zhy05mCR%2BtxgGXlgDBYwsRAMaIiZmLQwTiCZkrZ8SM7aLPo8Qqw0prqCpmDbpgugWzvaAJK0AEImPNqICQxTNNHOAHIQ1qxIxgmV2ADiZAYzkcQA&aggregations=N4IgNghgRgpmIC4QDECMACAygYwPYCcZ0AKAWQm31wEoQAaEAZ1wFd9sZERDGWwAXeiAgBzEYRER%2BBRKFHiYk6fi4AzVAH1GeQkIgA3GPlEBLAHYjSMfgAtcAEy4BbClRABfBgAcIxp41kQcVYzewAVfBZbZBM4RyRgllCNfkjbADpIWHgGc1UjGDMOGLiubEhGRhNVE2wpE1wzTOg4IVseOzB4sz4wXKKwFnsYDQgWbERVCDBGGE8QVQIXQSQvIw4zflFOBhsTERswfZsV0D2DowBJRgAha34jRFSWOYZErwA5CCdOJCtU2roADiVBYXg8QA&models=N4IglgJiBcAsA0IDGB7AdgMzAcwK4CcBDAFzHRlEhgFYBfWoA&models=N4IglgJiBcDMA0IDGB7AdgMzAcwK4CcBDAFzHRlEhgFYBfWoA)

!!! info "Guides: TPR and FPR"

    The TPR is also known as sensitivity or recall, and it represents the proportion of true positive inferences
    (correctly predicted positive instances) among all actual positive instances. The FPR is the proportion of false
    positive inferences (incorrectly predicted positive instances) among all actual negative instances. Read the
    [true positive rate (TPR)](./recall.md) and the [false positive rate (FPR)](./fpr.md) guides to learn more about
    these metrics.

<div class="grid cards" markdown>
- :kolena-manual-16: API Reference: [`CurvePlot` ↗][kolena.workflow.plot.CurvePlot]
</div>

## Implementation Details

The curve’s points (TPRs and FPRs) are calculated with a varying threshold, and made into points (TPR values on the
y-axis and FPR values on the x-axis). TPR and FPR metrics are threshold-dependent where a threshold value must be
defined to compute them, and by computing and plotting these two metrics across many thresholds we can check how these
metrics change depending on the threshold.

??? info "Thresholds Selection"

    Threshold ranges are very customizable. Typically, a uniformly spaced range of values from 0 to 1 can model a ROC
    curve, where users pick the number of thresholds to include. Another common approach to picking thresholds is
    collecting and sorting the unique confidences of every prediction.

### Example: Binary Classification

Let's consider a simple binary classification example and plot a ROC curve at a uniformly spaced range of thresholds.
The table below shows eight samples (four positive and four negative) sorted by their confidence score. Each inference
is evaluated at each threshold: 0.25, 0.5, and 0.75. It's a negative prediction if its confidence score is below the
evaluating threshold; otherwise, it's positive.

<center>

| Sample | <nobr>Confidence ↓</nobr> | Inference @ 0.25 | Inference @ 0.5 | Inference @ 0.75 |
| --- | --- | --- | --- | --- |
| <span class="mg-cell-color-positive">Positive</span> | 0.9 | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-positive">Positive</span> |
| <span class="mg-cell-color-positive">Positive</span> | 0.8 | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-positive">Positive</span> |
| <span class="mg-cell-color-negative">Negative</span> | 0.75 | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-positive">Positive</span> |
| <span class="mg-cell-color-positive">Positive</span> | 0.7 | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-negative">Negative</span> |
| <span class="mg-cell-color-negative">Negative</span> | 0.5 | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-negative">Negative</span> |
| <span class="mg-cell-color-positive">Positive</span> | 0.35 | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-negative">Negative</span> | <span class="mg-cell-color-negative">Negative</span> |
| <span class="mg-cell-color-negative">Negative</span> | 0.3 | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-negative">Negative</span> | <span class="mg-cell-color-negative">Negative</span> |
| <span class="mg-cell-color-negative">Negative</span> | 0.2 | <span class="mg-cell-color-negative">Negative</span> | <span class="mg-cell-color-negative">Negative</span> | <span class="mg-cell-color-negative">Negative</span> |

</center>

As the threshold increases, there are fewer true positives and fewer false positives, most likely yielding lower TPR
and FPR. Conversely, decreasing the threshold may increase both TPR and FPR. Let's compute the TPR and FPR values at
each threhold using the following formulas:

$$\text{TPR} = \frac{\text{TP}}{\text{TP} + \text{FN}}$$

$$\text{FPR} = \frac{\text{FP}}{\text{TN} + \text{FP}}$$

<center>

| Threshold | TP | FP | FN | TN | TPR | FPR |
| --- | --- | --- | --- | --- | --- | --- |
| **0.25** | 4 | 3 | 0 | 1 | $\frac{4}{4}$ | $\frac{3}{4}$ |
| **0.5** | 3 | 2 | 1 | 2 | $\frac{3}{4}$ | $\frac{2}{4}$ |
| **0.75** | 2 | 1 | 2 | 3 | $\frac{2}{4}$ | $\frac{1}{4}$ |

</center>

Using these TPR and FPR values, a ROC curve can be plotted:

<center>
![roc.png](../assets/images/metrics-roccurve-example-light.png#only-light)
![roc.png](../assets/images/metrics-roccurve-example-dark.png#only-dark)
</center>

### Example: Multiclass Classification

For multiple classes, a curve is plotted **per class** by treating each class as a binary classification problem. This
technique is known as [**one-vs-rest**](./tp-fp-fn-tn.md#multiclass) (OvR). With this strategy, we can have `n` ROC
curves for `n` unique classes.

Let's take a look at a multiclass classification example and plot **per class** ROC curves for the same three
thresholds that we used in the example above: 0.25, 0.5, and 0.75. In this example, we have three classes:
`Airplane`, `Boat`, and `Car`. The multiclass classifier outputs a confidence score for each class:

<center>

| Sample # | Label | `Airplane` Confidence | `Boat` Confidence | `Car` Confidence |
| --- | --- | --- | --- | --- |
| 1 | `Airplane` | 0.9 | 0.05 | 0.05 |
| 2 | `Airplane` | 0.7 | 0.05 | 0.25 |
| 3 | `Airplane` | 0.25 | 0.25 | 0.5 |
| 4 | `Boat` | 0.6 | 0.25 | 0.15 |
| 5 | `Boat` | 0.4 | 0.5 | 0.1 |
| 6 | `Car` | 0.25 | 0.25 | 0.5 |
| 7 | `Car` | 0.05 | 0.7 | 0.25 |

</center>

Just like the binary classification example, we are going to determine whether each inference is positive or negative
depending on the evaluating threshold, so for class `Airplane`:

<center>

| Sample # | Sample | `Airplane` Confidence ↓ | Inference @ 0.25 | Inference @ 0.5 | Inference @ 0.75 |
| --- | --- | --- | --- | --- | --- |
| 1 | <span class="mg-cell-color-positive">Positive</span> | 0.9 | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-positive">Positive</span> |
| 2 | <span class="mg-cell-color-positive">Positive</span> | 0.7 | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-negative">Negative</span> |
| 4 | <span class="mg-cell-color-negative">Negative</span> | 0.6 | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-negative">Negative</span> |
| 5 | <span class="mg-cell-color-negative">Negative</span> | 0.4 | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-negative">Negative</span> | <span class="mg-cell-color-negative">Negative</span> |
| 3 | <span class="mg-cell-color-positive">Positive</span> | 0.25 | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-negative">Negative</span> | <span class="mg-cell-color-negative">Negative</span> |
| 6 | <span class="mg-cell-color-negative">Negative</span> | 0.25 | <span class="mg-cell-color-positive">Positive</span> | <span class="mg-cell-color-negative">Negative</span> | <span class="mg-cell-color-negative">Negative</span> |
| 7 | <span class="mg-cell-color-negative">Negative</span> | 0.05 | <span class="mg-cell-color-negative">Negative</span> | <span class="mg-cell-color-negative">Negative</span> | <span class="mg-cell-color-negative">Negative</span> |

</center>

And the TPR and FPR values for class `Airplane` can be computed:
<center>

| Threshold | TP | FP | FN | TN | <nobr>`Airplane` TPR</nobr> | <nobr>`Airplane` FPR</nobr> |
| --- | --- | --- | --- | --- | --- | --- |
| **0.25** | 3 | 3 | 0 | 1 | $\frac{3}{3}$ | $\frac{3}{4}$ |
| **0.5** | 2 | 1 | 1 | 3 | $\frac{2}{3}$ | $\frac{1}{4}$ |
| **0.75** | 1 | 0 | 2 | 4 | $\frac{1}{3}$ | $\frac{0}{4}$ |

</center>

We are going to repeat this step to compute TPR and FPR for class `Boat` and `Car`.

<center>

| Threshold | <nobr>`Airplane` TPR</nobr> | <nobr>`Airplane` FPR</nobr> | `Boat` TPR | `Boat` FPR | `Car` TPR | `Car` FPR |
| --- | --- | --- | --- | --- | --- | --- |
| **0.25** | $\frac{3}{3}$ | $\frac{3}{4}$ | $\frac{2}{2}$ | $\frac{3}{5}$ | $\frac{2}{2}$ | $\frac{2}{5}$ |
| **0.5** | $\frac{2}{3}$ | $\frac{1}{4}$ | $\frac{1}{2}$ | $\frac{1}{5}$ | $\frac{1}{2}$ | $\frac{1}{5}$ |
| **0.75** | $\frac{1}{3}$ | $\frac{0}{4}$ | $\frac{0}{2}$ | $\frac{0}{5}$ | $\frac{0}{2}$ | $\frac{0}{5}$ |

</center>

Using these TPR and FPR values, per class ROC curves can be plotted:

<center>
![roc.png](../assets/images/metrics-roccurve-example-multiclass-light.png#only-light)
![roc.png](../assets/images/metrics-roccurve-example-multiclass-dark.png#only-dark)
</center>

## Area Under the ROC Curve (AUC ROC)

The area under the ROC curve (AUC ROC) is a **threshold-independent** metric that summarizes the performance of a model
depicted by a ROC curve. A perfect classifier would have an AUC ROC value of 1. The greater the area, the better a
model performs at classifying the positive and negative instances. Using AUC ROC metric alongside other evaluation
metrics, we can assess and compare the performance of different models and choose the one that best suits their
specific problem.

Let's take a look at the binary classification example again. Given the following TPR and FPR values, the AUC ROC can be
computed:

<center>

| Threshold | TP | FP | FN | TN | TPR | FPR |
| --- | --- | --- | --- | --- | --- | --- |
| **0.25** | 4 | 3 | 0 | 1 | $\frac{4}{4}$ | $\frac{3}{4}$ |
| **0.5** | 3 | 2 | 1 | 2 | $\frac{3}{4}$ | $\frac{2}{4}$ |
| **0.75** | 2 | 1 | 2 | 3 | $\frac{2}{4}$ | $\frac{1}{4}$ |

</center>

The area under the curve can computed by taking an integral along the x-axis using the composite trapezoidal rule:

$$
\begin{align}
\text{AUC ROC} &= \int y(x) \, dx \\[1em]
&\approx \sum_{k=1}^{N} \frac{y(x_{k-1}) + y(x_{k})}{2} \Delta x_{k} \\[1em]
&= \frac{\frac{1}{4}(\frac{4}{4} + \frac{3}{4})}{2} + \frac{\frac{1}{4}(\frac{3}{4} + \frac{2}{4})}{2} \\[1em]
&= \frac{7}{32} + \frac{5}{32} \\[1em]
&= \frac{3}{8} \\[1em]
\end{align}
$$

For Python implementation, we recommend using NumPy's
[`np.trapz(y, x)`](https://numpy.org/doc/stable/reference/generated/numpy.trapz.html) or
scikit-learn's [`sklearn.metrics.auc`](https://scikit-learn.org/stable/modules/generated/sklearn.metrics.auc.html).

## Limitations and Biases

While ROC curves and AUC ROC are widely used in machine learning for evaluating classification models, they do have
limitations and potential biases that should be considered:

1. **Sensitive to Class Imbalance**: Classes with too few data points may have ROC curves that are poor representations
of actual performance or overall performance. The performance of minority classes may be less accurate compared to a
majority class.
2. **Partial Insight to Model Performance**: ROC curves only gauge TPR and FPR based on classifications, they do not
surface misclassification patterns or reasons for different types of errors. AUC ROC treats false positives and
false negatives equally which may not be appropriate in situations where one type of error is more costly or impactful
than the other. In such cases, other evaluation metrics like [precision](./precision.md) or [PR curve](./pr-curve.md)
can be used.
3. **Dependence on Threshold**: The values of the thresholds affect the shape of ROC curves, which can affect how they
are interpreted. Having a different number of thresholds, or having different threshold values, make ROC curve
comparisons difficult.
