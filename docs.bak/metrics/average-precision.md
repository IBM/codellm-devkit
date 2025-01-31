---
description: How to calculate and interpret average precision for ML tasks
---

# Average Precision

Average precision (AP) summarizes a [precision-recall (PR) curve](./pr-curve.md)
into a single value representing the average of all precisions.
It is generally understood as the approximation of the area under the PR curve. AP ranges between 0 and 1,
where a perfect model has [precision](./precision.md), [recall](./recall.md), and AP scores of 1. The larger the metric,
the better a model performs across different thresholds.

!!! info inline end "Guides: Precision and Recall"

    Read the [precision](./precision.md) and the [recall](./recall.md) guides if you're not familiar with those metrics.

Unlike metrics like [precision](./precision.md), [recall](./recall.md), and [F<sub>1</sub>-score](./f1-score.md), which
are threshold-dependent where a confidence threshold value must be defined to compute them, AP is a key performance
**threshold-independent** metric that removes the dependency of selecting one confidence threshold value and measures a
model's performance across all thresholds.

AP is commonly used to evaluate the performance of object detection and information retrieval workflows. This metric
(or an aggregated version of it called [mean average precision (mAP)](#mean-average-precision-map)) is the primary
metric used across popular object detection benchmarks such as [PASCAL VOC 2012](http://host.robots.ox.ac.uk/pascal/VOC/),
[COCO](https://cocodataset.org/#detection-2015), and
[Open Images V7](https://storage.googleapis.com/openimages/web/evaluation.html).

!!!example
    To see an example use of Average Precision, checkout the
    [KITTI Vision Benchmark Suite on app.kolena.com/try.](https://app.kolena.io/try/dataset/standards?datasetId=44&models=N4IglgJiBcAcCsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIYBGAX3qA&models=N4IglgJiBcAcAsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIYBGAX3qA&metricGroupVisibilities=N4IgbglgzhBGA2BTEAuALgJwK6IDQgFtFMIBjKVAbVEhgWXW0QF9cbo4lVMdX26ujXm3Ad63JswC6zIA)

## Implementation Details

The general definition of AP is finding the approximation of the area under the [PR curve](./pr-curve.md). The actual
area under the curve, where $p(r)$ is the precision at recall $r$, can be defined:

$$
\text{AP} = \int_{0}^{1} p(r)dr
$$

The integral above is in practice replaced with a **finite sum** over every **unique recall** value (or over a set of
evenly spaced recall values) — different interpolation methods are discussed in the section below. The average precision
over a set of recall values or over a range of thresholds at which we are evaluating the model can be defined:

$$
AP = \sum_{k=0}^{k=n-1}[r(k) - r(k+1)] * p(k)
$$

where

- $p(k)$ is the precision at threshold $k$
- $r(k)$ is the recall at threshold $k$
- $n$ is the number of thresholds

Let’s take a closer look at different implementations of the AP metric. Two primary machine learning workflows that
use AP as a main evaluation metric are **object detection** and **information retrieval**, but AP is implemented
slightly differently for both.

### Object Detection

Let’s consider the following simple example:

![example legends](../assets/images/metrics-bbox-legend-gt-light.svg#only-light)
![example legends](../assets/images/metrics-bbox-legend-gt-dark.svg#only-dark)
![object detection example](../assets/images/metrics-ap-od-example-light.svg#only-light)
![object detection example](../assets/images/metrics-ap-od-example-dark.svg#only-dark)

The above three images show a total of four ground truth objects, all of which are matched with an inference bounding box
based on the [Intersection over Union (IoU)](./iou.md) scores. Let’s look at each inference bounding box and sort them
by their confidence score in descending order.

| Inference | <nobr>Confidence ↓</nobr> | TP/FP | cumsum(TP) | cumsum(FP) | Precision | Recall |
| --- | --- | --- | --- | --- | --- | --- |
| H | 0.99 | TP | 1 | 0 | 1.0 | 0.2 |
| B | 0.88 | TP | 2 | 0 | 1.0 | 0.4 |
| E | 0.72 | FP | 2 | 1 | 0.667 | 0.4 |
| A | 0.70 | FP | 2 | 2 | 0.5 | 0.4 |
| J | 0.54 | FP | 2 | 3 | 0.4 | 0.4 |
| D | 0.54 | TP | 3 | 3 | 0.5 | 0.6 |
| I | 0.38 | TP | 4 | 3 | 0.571 | 0.8 |
| C | 0.2 | FP | 4 | 4 | 0.5 | 0.8 |
| F | 0.2 | FP | 4 | 5 | 0.444 | 0.8 |
| G | 0.1 | TP | 5 | 5 | 0.5 | 1.0 |

!!! info inline end "Guides: TP/FP Counts in Object Detection"

    Read the [Intersection over Union (IoU)](./iou.md), the [Geometry Matching](./geometry-matching.md), and the
    [TP / FP / FN / TN](./tp-fp-fn-tn.md) guides if you're not familiar with those terminologies.

In order to compute AP, we first need to define precision and recall at each threshold. In this example, we are going
to use every unique confidence score as threshold to calculate precision and recall metrics, so we have the
complete list of unique recall values. Starting from the top, each inference is assigned to be either a [true
positive (TP) or false positive (FP)](./tp-fp-fn-tn.md) depending on the [matching](./geometry-matching.md) results —
if the inference is matched with a ground truth, then it's a TP; otherwise, a FP. Notice that all inferences in this
table are considered to be positive (either a TP or FP) because we are evaluating them at the thresholds equal to
their confidence scores. Then, the cumulative sum of TP and FP counts respectively from the previous
rows are computed at each row. Using these accumulated TP and FP counts, the precision and recall metrics
can be defined at each threshold. We are using the cumulative sum because once again each row is evaluated at the
threshold equal to its confidence score, so only the upper rows and the current row (i.e., inference with score
greater than or equal to the threshold) count as positive inferences.

Now that we have the precision and recall defined at each threshold, let’s plot the PR curve:

![object detection example - PR curve](../assets/images/metrics-ap-od-pr-light.svg#only-light)
![object detection example - PR curve](../assets/images/metrics-ap-od-pr-dark.svg#only-dark)

Notice the zigzag pattern, often referred to as “**wiggles**” —
the precision goes down with FPs and goes up again with TPs as the
recall increases. It is a common practice to first smooth out the wiggles before calculating the AP metric by taking the
maximum precision value to its graphical right side of each recall value. This is why AP is called the **approximation**
of the area under the PR curve. The interpolated precision at each recall is defined:

$$
p_{interp}(r) = \max_{\hat{r} \geq r}p(\hat{r})
$$

The PR curve is re-plotted using the interpolated precisions (see orange line in the plot below).

![object detection example - PR curve with interpolation](../assets/images/metrics-ap-od-pr-interpolation-light.svg#only-light)
![object detection example - PR curve with interpolation](../assets/images/metrics-ap-od-pr-interpolation-dark.svg#only-dark)

<div class="grid" markdown>
The precisions (y-values) of the smoothed out curve, the orange line on the plot above, are
**monotonically decreasing**. We’re now ready to calculate AP, which is simply the **area under the smoothed out curve**:

!!! info "The start and the end of PR curve"

    Notice the above PR curve doesn't start at zero recall. It is because there is no valid threshold that will result
    in zero recall. In order to ensure that the graph starts on the y-axis, the first point on the curve extends all the
    way to the y-axis. Similarly, the end of the PR curve doesn't always extend all the way to the recall value of 1.
    This is because not all the ground truths are matched. Unlike the start of the curve,
    the tail of the curve doesn't get extended when calculating AP.
</div>

$$
\begin{align}
AP &= ((0.4-0.0) \times 1.0) + ((0.8 - 0.4) \times 0.571) + ((1.0 - 0.8) \times 0.5) \\[1em]
&= 0.7284
\end{align}
$$

!!! info "Smoothing"

    Although smoothing is considered as the standard implementation of average precision,
    scikit-learn's [average precision](https://scikit-learn.org/stable/modules/generated/sklearn.metrics.average_precision_score.html)
    implementation does not smooth out the precisions as mentioned above.

The example above computes AP at **all unique recall** values whenever the maximum precision value drops. This is the
most precise implementation of the metric, used in popular benchmarks like the
[PASCAL VOC challenge since 2010](http://host.robots.ox.ac.uk:8080/pascal/VOC/voc2010/htmldoc/devkit_doc.html#SECTION00044000000000000000).
Prior to 2010, the PASCAL VOC challenge had a different implementation for the AP calculation where the **11 linearly
spaced recall values** from 0.0 to 1.0 were used instead of all unique recall values.

#### 11-point Interpolation

The 11-point interpolated AP was used in the PASCAL VOC until a new AP calculation, what's considered as the standard
now, which was adopted in 2010. This interpolation uses the average of the maximum precision values for 11 linearly spaced
recall values from 0.0 to 1.0:

![object detection example — 11 interpolation](../assets/images/metrics-ap-od-pr-11-light.svg#only-light)
![object detection example — 11 interpolation](../assets/images/metrics-ap-od-pr-11-dark.svg#only-dark)

When the precisions at certain recall values become extremely small, they are exempted from the AP calculation.
The intention of using this 11-point interpolation, according to the
[original paper](http://host.robots.ox.ac.uk/pascal/VOC/pubs/everingham15.pdf), is as follows:

> The intention in interpolating the precision/recall curve in this way is to reduce the impact of the “wiggles” in the
> precision/recall curve, caused by small variations in the ranking of examples.

However, this linearly interpolated method suffers from being less precise and not being able to measure the difference
with low APs due to the approximation mentioned above. The [COCO benchmark](https://cocodataset.org/#detection-eval)
uses a linear interpolation method but with **101** recall values.

### Information Retrieval

Information retrieval is a machine learning workflow where the user provides a query, and the model returns a score
that measures how similar each data is to the query to find the most relevant information from the database.

Average precision is one of the popular metrics used in information retrieval workflow along with object detection
workflow. However, the implementation used in information retrieval workflow is slightly different than the one from
the section above. The formula of the metric is defined:

$$
\text{AP}(n) = \frac 1 {\text{GTP}} \sum_k^{n}p(k) \times rel(k)
$$

where

- $n$ is the total number of data that you are interested in
- $\text{GTP}$ is the total number of positive ground truths
- $p(k)$ is the precision at rank $k$ data
- $rel(k)$ is the relevance at rank $k$ data (1 if the data is relevant, 0 otherwise)

Let’s consider the following example of retrieving similar images to the query from a database of images with different
shapes and colors:

![information retrieval example](../assets/images/metrics-ap-ir-example-light.svg#only-light)
![information retrieval example](../assets/images/metrics-ap-ir-example-dark.svg#only-dark)

The retrieved images are the complete list of images from the database that are ranked by their similarity scores,
which are predicted from the model, where the left-most image is the most similar to the query image.

From the retrieved images, the ones with a circle are the TPs, where $rel(k) = 1$, and any other shapes are labeled as
FPs, where $rel(k) = 0$. Then by simply accumulating all the counts of the TPs in each rank, $p(k) \times rel(k)$ can be
calculated at each rank.

AP is the sum of all the relevant precisions over the total number of positive samples in the database, so in this
example, AP becomes:

$$
\text{AP} = \frac {(\frac 1 1 + \frac 0 2 + \frac 0 3 + \frac 2 4 + \frac 3 5 + \frac 0 6 + ... + 0)} 3 = 0.7
$$

### Mean Average Precision (mAP)

The mean average precision (mAP) is simply the [macro-average](./averaging-methods.md) of the AP calculated across
different classes for object detection workflow or across different queries for information retrieval workflow. It is
important to note that some papers use AP and mAP interchangeably.

## Limitations and Biases

AP is a great metric that summarizes the PR curve into a single value. Instead of comparing models with a single value
of precision and recall at one specific threshold, it lets you compare model performance at every threshold. Although
this metric is very popular and commonly used in object detection and information retrieval workflows, it has some
limitations. Let's make sure to understand these limitations before using the metric to compare your models.

??? info "AP is often overestimated."

    To approximate the area under the curve, it is standard practice to take the maximum precision from the right side
    of the plot. By doing so, it overestimates the area under the curve.

??? info "AP cannot distinguish between very different-looking PR curves."

    Consider the following three plots:

    ![limitation #2 example](../assets/images/metrics-ap-limitation2-light.svg#only-light)
    ![limitation #2 example](../assets/images/metrics-ap-limitation2-dark.svg#only-dark)

    These three plots show very different characteristic, but their APs are exactly the same for all of them.
    Thus, relying solely on the AP metric is not enough. We recommend plotting the PR curve along with the AP metric
    to better understand the behavior of your model.

??? info "AP is not confidence score sensitive."

    AP uses confidence score to sort inferences, and as long as the sorted order is preserved, the distribution of
    confidence scores does not change the AP score. Therefore, predictions that have confidence scores within a very
    small range versus ones with scores that are nicely distributed from 0 to 1 can have the same AP as long as the
    order is preserved.

??? info "AP uses the interpolated PR curve."

    As mentioned in the section above, there are many different ways of interpolating the PR curve. Depending on the
    granularity of the plot, the AP value can be different, so when comparing models using AP, we need to ensure that
    it is calculated using the same interpolation method.

??? info "AP is not a fair comparison for thresholded models where the tail part of the PR curve is missing."

    It is pretty common for object detectors to filter out predictions with very small confidence scores. In such a
    scenario, the curve will be missing the tail part, but because the metric considers the entire recall domain, any
    curves that end early will result in a lower average precision score.

    ![limitation #5 example](../assets/images/metrics-ap-limitation5-light.svg#only-light)
    ![limitation #5 example](../assets/images/metrics-ap-limitation5-dark.svg#only-dark)

    The plot above shows PR curves of two models: one extending to the recall value of `1.0` and the other one extending
    only to `0.6`. Since a large portion of the area under the curve corresponds to the tail of the curve, model 2
    scores a higher AP than model 1.
