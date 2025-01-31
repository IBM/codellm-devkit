---
description: How to measure ML model agreement using Cohen's Kappa
---

# Cohen's Kappa

Cohen's Kappa is a statistical metric that evaluates the reliability of an algorithm's predictions with similar goals
as [accuracy](./accuracy.md) or [F<sub>1</sub>-score](./f1-score.md). However, the Cohen's Kappa score magnifies the
advantage a classifier has over a random classifier based on class frequencies. The Kappa statistic is a robust metric,
particularly when there is significant class imbalance for binary and multiclass classification problems, which
extends to more complicated ML tasks such as object detection.

Like statistical correlation coefficients, Cohen's Kappa ranges from -1 to +1, but typically ranges from 0 to 1. A
value of 0 means that a model agrees with a completely random model, with the same level of performance.

## Implementation Details

Cohen's Kappa coefficient (\(\kappa\)) is calculated by comparing the observed agreement
([true positives and true negatives](./tp-fp-fn-tn.md)) between the model inferences and the ground truths against
expected classifications by random chance based on the marginal frequencies of each class.

The formula for Cohen's Kappa is:

$$
\kappa = \frac{P_o - P_e}{1 - P_e}
$$

where:

- \(P_o\) is the `observed agreement`
- \(P_e\) is the hypothetical probability of `chance agreement`

!!! info "Understanding \(P_o\) and \(P_e\)"
    [Accuracy](./accuracy.md) measures the overall correctness of predictions. Cohen's Kappa adjusts for any agreement
    that may happen by chance, providing a more detailed understanding of the classifier's performance.

    **\(P_o\) (Observed Agreement)**:
    The proportion of times where the model and the ground truth agree. This is the number of instances
    where the predicted labels match the true labels, divided by the total number of instances
    (i.e. [accuracy](./accuracy.md) of the model).


    **\(P_e\) (Chance Agreement)**: We can simplify the chance agreement calculation into three steps.

    1. **Calculate class proportions**:
    For each class \(c\), calculate the proportion of instances predicted as class \(c\) by the model, denoted
    as \(p_{c,\text{pred}}\). Do the same for instances among ground truths, denoted as \(p_{c,\text{true}}\).

    2. **Compute chance agreement for each class**:
    For each class \(c\), calculate the chance agreement by multiplying the model's proportion for class \(c\) with
    the ground truth's proportion for class \(c\):

        $$
        \text{chance}_{c} = p_{c,\text{pred}} \times p_{c,\text{true}}
        $$

    3. **Sum chance agreements across all classes**:
    Sum the calculated chance agreements for all classes to get the total probability of chance agreement. This is
    the overall probability that the model and the ground truths would agree on the classification of instances by
    chance alone, across all classes:

        $$
        P_e = \sum_{c} \text{chance}_{c}
        $$

There is an alternative method using [TP, FP, FN, and TN](./tp-fp-fn-tn.md) when only two classes are involved:

$$
\kappa = \frac{2 \cdot (TP \cdot TN - FP \cdot FN)}{(TP + FP) \cdot (FP + TN) + (TP + FN) \cdot (FN + TN)}
$$

### Interpretation

Cohen's Kappa ranges from \(-1\) to \(1\). A value of \(1\) indicates perfect agreement between the model predictions
and the ground truths, while a value of \(0\) indicates no agreement better than random chance. Negative values
indicate a model that is less performant than a random guesser.

| Kappa Coefficient | Agreement Level       |
| ----------------- | --------------------- |
| <= 0              | Poor                  |
| > 0               | Slight                |
| > 0.2             | Fair                  |
| > 0.4             | Moderate              |
| > 0.6             | Substantial           |
| > 0.8             | Almost perfect        |
| = 1.0             | Perfect               |

### Multiple Classes

In workflows with multiple classes, Kappa can be computed per class. In the [TP / FP / FN / TN](./tp-fp-fn-tn.md)
guide, we learned how to compute per-class metrics when there are multiple classes, using the
[one-vs-rest](./tp-fp-fn-tn.md#multiclass) (OvR) strategy. Once you have TP, FP, and FN counts computed for each
class, you can compute Cohen's Kappa for each class by treating each as a single-class problem.

### Aggregating Per-class Metrics

If you are looking for a **single** Kappa score that summarizes model performance across all classes, there are
different ways to aggregate the scores: **macro**, **micro**, and **weighted**. Read more about
these methods in the [Averaging Methods](./averaging-methods.md) guide.

## Example

Suppose `Doctor A` claims 30 of 100 patients are sick, but `Doctor B` claims 42 patients are sick. However,
they agree that 20 of the 100 were certainly sick. This scenario can be visualized below in a
[confusion matrix](./confusion-matrix.md):

|  | sick | not sick |
| --- | --- | --- |
| **sick** | 20 | 22 |
| **not sick** | 10 | 48 |

The \(P_o\) (observed agreement) is: \((20+48)/100=0.68\)

The \(P_e\) (chance agreement) is: \((30/100) *(42/100) + (70/100)* (58/100)\)
\(=0.126+0.406=0.532\)

Now for the Kappa coefficient: \(\frac{P_o - P_e}{1 - P_e}=\frac{0.68 - 0.532}{1 - 0.532}=0.148/0.468=0.316\)

So, `Doctor A` and `Doctor B` are in `Fair Agreement`.

## Limitations and Biases

While Cohen's Kappa can be a powerful metric that provides a more accurate picture of a model's performance on
datasets with class imbalance, it is not without its limitations:

- **Dependence on Marginal Probabilities**: Evaluations can sometimes lead to unintuitive results,
especially if minority classes are highly imbalanced.
- **Ambiguous Interpretation**: Cohen's Kappa values can be somewhat subjective and context-dependent.
What constitutes an agreement level of "substantial" or "almost perfect" can vary by domain or person.
- **Threshold-Dependence**: For probabilistic models, the calculations depend on the threshold used to convert
probabilities into class labels. Different thresholds will lead to different Kappa values.

Cohen's Kappa remains useful as a model performance metric, offering insight while considering the distribution
of classes within a dataset.
