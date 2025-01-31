---
description: Understanding false negative rate (FNR) to evaluate ML model performance
---


# False Negative Rate (FNR)

False negative rate (FNR) measures the proportion of actual positive instances that are incorrectly classified as negative,
ranging from 0 to 1. A low false negative rate indicates that the model is
good at identifying positive cases, where a high false positive rate suggests that the model is
frequently missing positive cases, which can be critical in applications like medical diagnoses.
FNR is equivalent to 1 - [TPR (recall)](./recall.md).

As shown in this formula, false positive rate is the fraction of all negative ground truths that are incorrectly predicted:

$$\text{FNR} = \frac{\text{FN}}{\text{FN} + \text{TP}}$$

In the above formula, $\text{FN}$ is the number of false negative inferences and $\text{TP}$ is the number of true
positive inferences.

!!! info "Guide: True Negative / False Positive"

    Read the [TP / FP / FN / TN](./tp-fp-fn-tn.md) guide if you're not familiar with "FN" and "TP" terminology.

## Implementation Details

FNR is used to evaluate the performance of a classification model, particularly in tasks like binary
classification, where the goal is to classify data into one of two possible classes.

Here is how FPR is calculated:

$$
\text{FPR} = \frac {\text{# False Negatives}} {\text{# False Negatives} + \text{# True Positives}}
$$

### Multiple Classes

So far, we have only looked at **binary** classification cases, but in **multiclass** or **multi-label** cases,
FNR is computed per class. In the [TP / FP / FN / TN](./tp-fp-fn-tn.md) guide,
we went over multiple-class cases and how these metrics are computed. Once you have these four metrics computed per
class, you can compute FNR for each class by treating each as a single-class problem.

### Aggregating Per-class Metrics

If you are looking for a **single** FNR score that summarizes model performance across all classes, there are
different ways to aggregate per-class FNR scores: **macro**, **micro**, and **weighted**. Read more about these
methods in the [Averaging Methods](./averaging-methods.md) guide.

## Limitations and Biases

While FNR is a valuable metric for evaluating the performance of classification models, it does have limitations
and potential biases that should be considered when interpreting results:

1. **Sensitivity to Class Imbalance**: similar to other metrics, FNR can be skewed when there is a significant
imbalance between classes. In datasets where the positive class is underrepresented, FNR may appear low simply because
the total number of false negatives is small relative to the overall dataset size,
even if the model frequently misclassifies positives.

2. **Ignores True Negatives:**: FNR focuses exclusively on the true positives (correctly classified positive cases)
and false negatives (positive cases incorrectly classified as negative). It does not account for true negatives
(correctly classified negative cases) or false positives (negative cases misclassified as positive),
potentially neglecting the model’s performance on the negative class.

3. **Incomplete Performance Context:** relying solely on FNR provides an incomplete picture of the model’s effectiveness.
For example, a model with a low FNR might still have a high false positive rate (FPR), indicating it incorrectly flags
many negatives as positives. Metrics like precision, recall, and the F1-score should be used alongside FNR for a more
holistic evaluation.
4. **Threshold Dependence:** FNR is influenced by the decision threshold of the classification model. Adjusting the
threshold can significantly alter the FNR, but this does not always reflect actual improvements in the model’s underlying
ability to separate classes. Threshold-independent metrics, such as the area under the precision-recall curve (AUC-PR),
can provide additional insights.
5. **Bias Against Minority Classes:** in applications where the minority class represents the positive cases (e.g., fraud
detection or disease diagnosis), a model biased toward the majority class can artificially achieve a low FNR by rarely or
never predicting positives. This creates an illusion of effectiveness while failing the minority class entirely.

6. **Limited Use in Cost-Sensitive Scenarios:** while FNR highlights the proportion of missed positives, it does not
directly account for the costs associated with these errors. In scenarios where false negatives have severe consequences
(e.g., misdiagnosing a disease), additional metrics like cost-sensitive measures or domain-specific impact analysis should
be considered.

7. **No Assessment of Prediction Confidence:** FNR does not consider the confidence of predictions. A model that
misclassifies positive cases with high confidence is treated the same as one that does so with low confidence, which may
overlook critical differences in model behavior.
