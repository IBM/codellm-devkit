---
icon: kolena/metrics-glossary-16

---

# :kolena-metrics-glossary-20: Task Metrics

Kolena supports automatic calculation of a number of metrics commonly used in machine learning tasks.

## Regression

For regression problems, Kolena web application currently supports [`mean_absolute_error`](../../metrics/mean-absolute-error.md),
[`mean_squared_error`](../../metrics/mean-squared-error.md),
[`root_mean_squared_error`](../../metrics/root-mean-squared-error.md), [`r^2`](../../metrics/coefficient-of-determination.md),
`pearson_correlation` and `spearman_correlation`

Follow the steps below to setup Regression metrics:
<figure markdown>
![Defining Metrics](../../assets/images/task-metrics-regression.gif#only-dark)
![Defining Metrics](../../assets/images/task-metrics-regression-light.gif#only-light)
<figcaption>Task Metrics: Regression</figcaption>
</figure>

## Binary Classification

For binary classification problems, Kolena web application currently supports [`accuracy`](../../metrics/accuracy.md),[`precision`](../../metrics/precision.md),
[`recall`](../../metrics/recall.md) and [`f1_score`](../../metrics/f1-score.md).

Follow the steps below to setup Binary Classification metrics:
<figure markdown>
![Defining Metrics](../../assets/images/task-metrics-binary-classification.gif#only-dark)
![Defining Metrics](../../assets/images/task-metrics-binary-classification-light.gif#only-light)
<figcaption>Task Metrics: Binary classification</figcaption>
</figure>

## Multiclass Classification

For multiclass classification problems, Kolena web application currently supports [`accuracy`](../../metrics/accuracy.md),[`precision`](../../metrics/precision.md),
[`recall`](../../metrics/recall.md) and [`f1_score`](../../metrics/f1-score.md).

You are able to apply [`macro`](../../metrics/averaging-methods.md#macro-average),
[`micro`](../../metrics/averaging-methods.md#micro-average) and
[`weighted`](../../metrics/averaging-methods.md#weighted-average) averaging methods to above metrics.

<figure markdown>
![Defining Metrics](../../assets/images/task-metrics-multiclass-classification.gif#only-dark)
![Defining Metrics](../../assets/images/task-metrics-multiclass-classification-light.gif#only-light)
<figcaption>Task Metrics: Multiclass classification</figcaption>
</figure>

## Object detection

For object detection problems, Kolena web application currently
supports [`average precision`](../../metrics/average-precision.md), [`precision`](../../metrics/precision.md),
[`recall`](../../metrics/recall.md) and [`f1_score`](../../metrics/f1-score.md).

??? "Required fields"
    Kolena attempts to automatically detect fields for true
    positive, false positive and false negative counts. For more information on how to prepare your object
    detection data, please visit [Formatting results for Object Detection](../advanced-usage/dataset-formatting/computer-vision.md#2d-object-detection)

<figure markdown>
![Object detection](../../assets/images/task-metrics-object-detection.gif#only-dark)
![Object detection](../../assets/images/task-metrics-object-detection-light.gif#only-light)
<figcaption>Task Metrics: Object detection</figcaption>
</figure>

## Thresholded Object Metrics

Kolena provides the flexibility to calculate and use metrics that are threshold dependent. For more information about
how to setup thresholded results checkout the [Thresholded Results](../advanced-usage/thresholded-results.md) page.

Once your can uploaded thresholded results, the option to setup thresholded metrics will be available.

<figure markdown>
![Thresholded Metrics](../../assets/images/thresholded-metrics-dark.gif#only-dark)
![Thresholded Metrics](../../assets/images/thresholded-metrics-light.gif#only-light)

<figcaption>Task Metrics: Object detection</figcaption>
</figure>

## Custom Metrics

Kolena provides out-of-the-box aggregation options for your datapoint level evaluations that
correspond with your desired metrics. For numeric evaluations you are able to
select from `count`, `mean`, `median`, `min`, `max`, `stddev` and `sum` aggregations options.
For categorical evaluations (class label, boolean, etc) `rate` and `count` aggregation options are available.

The Kolena web application currently supports [`precision`](../../metrics/precision.md),
[`recall`](../../metrics/recall.md), [`f1_score`](../../metrics/f1-score.md),
[`accuracy`](../../metrics/accuracy.md), [`false_positive_rate`](../../metrics/fpr.md),
and [`true_negative_rate`](../../metrics/recall.md).

To leverage these, add the following columns to your model result file: `count_TP`, `count_FP`, `count_FN`, `count_TN`.

<figure markdown>
![Custom Metrics](../../assets/images/custom-metrics-dark.png#only-dark)
![Custom MetricsCus](../../assets/images/custom-metrics-light.png#only-light)
<figcaption>Custom metrics</figcaption>
</figure>
