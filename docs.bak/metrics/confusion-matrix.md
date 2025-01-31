---
description: How to create and interpret confusion matrices to evaluate ML model performance
---

# Confusion Matrix

!!! info inline end "Guide: True Positive / False Positive / False Negative / True Negative"

    You can find more info on true positive, false positive, false negative, and true negative in the
    [TP / FP / FN / TN](./tp-fp-fn-tn.md) guide.

A confusion matrix is a structured plot describing classification model performance as a table that highlights counts
of objects with predicted classes (columns) against the actual classes (rows). Each cell has a count of the number of
objects that have its correct class and predicted class, which indicates how confused a model is. A model is confused
when a predicted class does not match the actual class. When they do match, this is considered a true positive (TP). In
general, a model resulting in more true positives (TPs) / true negatives (TNs) with fewer false positives (FPs) /
false negatives (FNs) is better.

![Confusion Matrix Image](../assets/images/metrics-confusion-matrix-light.png#only-light)
![Confusion Matrix Image](../assets/images/metrics-confusion-matrix-dark.png#only-dark)

Confusion matrices are used in classification workflows with only one class or with multiple classes, which extends to
object detection workflows, too. They help evaluate models by counting classification errors and visualizing class
imbalances.

!!!example
    To see an example of confusion matrix, checkout the
    [CIFAR-10 on app.kolena.com/try.](https://app.kolena.io/try/dataset/debugger?datasetId=20&aggregations=N4IgNghgRgpmIC4QEEDGqCuAnCqCeIANCAM4D22qMiIWMJGYALkSBAObt3sRNlaJQHLjB58BSXJhz5WEAG4wc7AJYA7dgFkYTABZkAJjQC2uLGRABfYgAcIOYyUEguFNQYAqWDHoBiKuCMkVwx3AH0mbz0AOkhYeGJ1ADMlGDUqf0CaVEgSEhUklVReFTI1WOg4Vj06En0wILVGMET0sAwDGDCIDFREJIgwEhhrECT%2BUxYkGyUqNSYOamJdFXZdMFXdKdAVtaUASRIAIR0mJURIjBHiEJsAOQhjaiRtSKKAAgBxcwwbKyA&aggregations=N4IgNghgRgpmIC4QAUBOMDGBLAzlg9gHYAEAFALIQar4CUIANCDvgK6oYyIjo6tgAXRiAgBzUelEQB%2BVIlBiJMKTLlIADumx4iwiADcYqMVkKjyMAQAt8AE24BbKjRABfJuojGHOeSAlshLYAKqis1gBiWHD2SAGsQQD6AmHWAHSQsPBMpgBmRjCEnFEx3BiQOHi5WBjSBIQZ0HDC1rw2YLGE-GA5RWCstjCJEKwYiLkQYDgw7iC5sk5CGkachAJiXExWWKJWYDtWS6Dbu0YAkjgAQpYCRogprDNM8eoAchAOXEgWKTXEAOI0VjqNxAA&aggregations=N4IgNghgRgpmIC4QCUYGMJjAAgBQFkI0AnAewEoQAaEAZ1IFdi0ZERiZaGwAXakCAHNBHQRB6liiUEJEwxEqUg4Ys-CADcYxIQEsAdoPwweAC1IATNgFsiZEAF8aABwg7rtaSBGN9FgCrEDGYAYrpwVkg%2BDH4A%2BjxBZgB0kLDwNAYAZtow%2BixhEWxokLS0upm6GDy6pPop0HD8Zhy05mCR%2BtxgGXlgDBYwsRAMaIiZmLQwTiCZkrZ8SM7aLPo8Qqw0prqCpmDbpgugWzvaAJK0AEImPNqICQxTNNHOAHIQ1qxIxgmV2ADiZAYzkcQA&aggregations=N4IgNghgRgpmIC4QDECMACAygYwPYCcZ0AKAWQm31wEoQAaEAZ1wFd9sZERDGWwAXeiAgBzEYRER%2BBRKFHiYk6fi4AzVAH1GeQkIgA3GPlEBLAHYjSMfgAtcAEy4BbClRABfBgAcIxp41kQcVYzewAVfBZbZBM4RyRgllCNfkjbADpIWHgGc1UjGDMOGLiubEhGRhNVE2wpE1wzTOg4IVseOzB4sz4wXKKwFnsYDQgWbERVCDBGGE8QVQIXQSQvIw4zflFOBhsTERswfZsV0D2DowBJRgAha34jRFSWOYZErwA5CCdOJCtU2roADiVBYXg8QA&models=N4IglgJiBcDMA0IDGB7AdgMzAcwK4CcBDAFzHRlEhgFYBfWoA&plots=N4IgHiBcoM4PYFcBOBjAplES0wQGwBcQAaEAazQE9MU8BDGGASwDMmU6Cm4A7AOnoAjNHhIghIzNlyE%2BtBszYcuvAXWGjSAEyYx1eNFqgs6eGGgC%2BpatBDxk6TFs50ADnCY8ipCjZABzJEQeLQB9AiQEAgALNQ0xCVFIEGcCNw8vPkDgsIio2MSxHT1BAyNIEzNLCyA)

## Implementation Details

The implementation of a confusion matrix depends on whether the workflow concerns one or more classes.

!!!info
    The Confusion Matrix feature in Kolena supports visualizing comparisons
    between a single predicted value and a single ground truth value. If you're
    having trouble generating or customizing plots for these visualizations,
    feel free to reach out to the Kolena team for support.

![Steps To Generate Plot](../assets/images/metrics-confusion-matrix-steps.gif)

**Steps to Generate a Custom Confusion Matrix Plot**:

1. Go to the Debugger page.

2. Add model results with predicted values.

3. Scroll down to Custom Plots.

4. Select the predicted results for the X-Axis and the ground truth for the Y-Axis.

## Single-Class

Single-class confusion matrices are used for binary classification problems. After computing the number of TPs, FPs,
FNs, and TNs, a confusion matrix would look like this:

<center>

|  | Predicted Positive | Predicted Negative |
| --- | --- | --- |
| **Actual Positive** | <span class="mg-cell-color-positive">True Positive (TP)</span> | <span class="mg-cell-color-negative">False Negative (FN)</span> |
| **Actual Negative** | <span class="mg-cell-color-negative">False Positive (FP)</span> | <span class="mg-cell-color-positive">True Negative (TN)</span> |

</center>

### Example: Single-Class

Let's consider a simple binary classification example and plot a confusion matrix. The table below shows five samples'
(three positive and two negative) ground truth labels and inference labels.

<center>

|  | Sample 1 | Sample 2 | Sample 3 | Sample 4 | Sample 5 |
| --- | --- | --- | --- | --- | --- |
| **Ground Truth** | `Cat` | `Cat` | `Cat` | `No Cat` | `No Cat` |
| **Inference** | `Cat` | `No Cat` | `No Cat` | `No Cat` | `Cat` |

</center>

A confusion matrix for this example can be plotted:

<center>

|  | Predicted `Cat` | Predicted `No Cat` |
| --- | --- | --- |
| **`Cat`** | 1 | 2 |
| **`No Cat`** | 1 | 1 |

</center>

## Multiclass

Multiclass confusion matrices, used for multiclass classification problems, outline counts of TPs, FPs, FNs, and TNs for
every unique pair of actual and predicted labels. A multiclass classification confusion matrix with three classes
would have the following format:

|  | Predicted `Airplane` | Predicted `Boat` | Predicted `Car` |
| --- | --- | --- | --- |
| **Actual `Airplane`** | <span class="mg-cell-color-positive">Correct Prediction</span> | <span class="mg-cell-color-negative">Incorrect Prediction</span> | <span class="mg-cell-color-negative">Incorrect Prediction</span> |
| **Actual `Boat`** | <span class="mg-cell-color-negative">Incorrect Prediction</span> | <span class="mg-cell-color-positive">Correct Prediction</span> | <span class="mg-cell-color-negative">Incorrect Prediction</span> |
| **Actual `Car`** | <span class="mg-cell-color-negative">Incorrect Prediction</span> | <span class="mg-cell-color-negative">Incorrect Prediction</span> | <span class="mg-cell-color-positive">Correct Prediction</span> |

And for example, if we are trying to calculate the counts of TP, FP, FN, and TN for class `Boat`:

|  | Predicted `Airplane` | Predicted `Boat` | Predicted `Car` |
| --- | --- | --- | --- |
| **Actual `Airplane`** | <span class="mg-cell-color-positive">True Negative</span> | <span class="mg-cell-color-negative">False Positive</span> | <span class="mg-cell-color-positive">True Negative</span> |
| **Actual `Boat`** | <span class="mg-cell-color-negative">False Negative</span> | <span class="mg-cell-color-positive">True Positive</span> | <span class="mg-cell-color-negative">False Negative</span> |
| **Actual `Car`** | <span class="mg-cell-color-positive">True Negative</span> | <span class="mg-cell-color-negative">False Positive</span> | <span class="mg-cell-color-positive">True Negative</span> |

### Example: Multiclass

Let's take a look at a multiclass classification example and plot a confusion matrix. In this example, we have three
classes: `Airplane`, `Boat`, and `Car`. The multiclass classifier outputs the following inferences:

<center>

|  | Sample 1 | Sample 2 | Sample 3 | Sample 4 | Sample 5 | Sample 6 |
| --- | --- | --- | --- | --- | --- | --- |
| **Ground Truth** | `Airplane` | `Boat` | `Car` | `Airplane` | `Boat` | `Boat` |
| **Inference** | `Airplane` | `Boat` | `Airplane` | `Airplane` | `Boat` | `Car` |

</center>

A confusion matrix for this example can be plotted:

<center>

|  | Predicted `Airplane` | Predicted `Boat` | Predicted `Car` |
| --- | --- | --- | --- |
| `Airplane` | 2 | 0 | 0 |
| `Boat` | 0 | 2 | 1 |
| `Car` | 1 | 0 | 0 |

</center>

In a different case, these counts may be much higher:

<center>

|  | Predicted `Airplane` | Predicted `Boat` | Predicted `Car` |
| --- | --- | --- | --- |
| `Airplane` | 200 | 0 | 0 |
| `Boat` | 100 | 8,800 | 600 |
| `Car` | 100 | 0 | 1,000 |

</center>

This confusion matrix reveals that a model is very good at identifying the `Boat` class: 8,800 of 9,500 `Boat`s were correctly
predicted. Of the 700 incorrect `Boat` predictions, 600 were predicted as `Car`, and 100 were predicted as `Airplane`.

This confusion matrix indicates that when a model makes a `Airplane` inference, the model is correct half the time. If
it is incorrect, it has labeled the `Airplane` as a `Boat` or a `Car`.

Whenever there is an actual `Airplane` class, the model never predicts that there is a different transportation object.

## Normalization

Sometimes it is easier to focus on **class-level behavior** if you are using a normalized confusion matrix. If confusion
matrices are color-coded, normalizing can also create a better visual representation:

![Example of normalized and colored confusion matrix](../assets/images/metrics-confusion-matrix-normalized-light.png#only-light)
![Example of normalized and colored confusion matrix](../assets/images/metrics-confusion-matrix-normalized-dark.png#only-dark)

You can normalize a confusion matrix by `row` (actual classes), `column` (predicted classes), or `all` (entire matrix).
Each type of normalization surfaces a view sharing different information, which is outlined below.

??? info "Normalizing by `row`"

    For an actual class, this normalization allows us to see the proportion of correctly or incorrectly predicted
    objects for each predicted class. Notice that the diagonal values from this normalization match the
    [recall](./recall.md) per class. To normalize by `row`, divide each entry in that `row` by the sum of values within
    it. If we normalize the multiclass example by `row`, we get:

    <center>

    |  | Predicted `Airplane` | Predicted `Boat` | Predicted `Car` |
    | --- | --- | --- | --- |
    | `Airplane` | 1 | 0 | 0 |
    | `Boat` | 0.01 | 0.93 | 0.06 |
    | `Car` | 0.09 | 0 | 0.91 |

    </center>

??? info "Normalizing by `column`"

    For a predicted class, this normalization allows us to see the proportion of instances predicted as a certain class
    that actually belong to each true class. Notice that the diagonal values from this normalization match the
    [precision](./precision.md) per class. To normalize by `column`, divide each entry in a `column` by the sum of
    values within that `column`. If we normalize the multiclass example by `column`, we get:

    <center>

    |  | Predicted `Airplane` | Predicted `Boat` | Predicted `Car` |
    | --- | --- | --- | --- |
    | `Airplane` | 0.5 | 0 | 0 |
    | `Boat` | 0.25 | 1 | 0.375 |
    | `Car` | 0.25 | 0 | 0.625 |

    </center>

??? info "Normalizing by `all`"

    For each entry, this normalization allows us to see the overall proportion of instances that fall into a combination
    of an actual and predicted class. To normalize by `all`, divide each entry by the total sum of all the values in
    the matrix. If we normalize the multiclass example by `all`, we get:

    <center>

    |  | Predicted `Airplane` | Predicted `Boat` | Predicted `Car` |
    | --- | --- | --- | --- |
    | `Airplane` | 0.02 | 0 | 0 |
    | `Boat` | 0.01 | 0.81 | 0.06 |
    | `Car` | 0.01 | 0 | 0.09 |

    </center>

## Limitations and Biases

Confusion matrices are great for the evaluation of models that deal with multiple classes. They are structured tables of
numbers, which is its strength and weakness.

1. **Class imbalance**: Confusion matrices can appear biased when dealing with imbalanced numbers of instances per class,
leading to skewed numbers. This can be addressed by [normalizing](#normalization) the matrix.
2. **Categorical evaluation**: Confusion matrices have categorical outputs and do not surface any details for
misclassifications. All misclassifications are treated equally, so there may be cases where classes are similar or
hierarchically related, but confusion matrices will not account for these details.
