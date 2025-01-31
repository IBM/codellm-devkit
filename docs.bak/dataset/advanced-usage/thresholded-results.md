---
icon: kolena/diagram-tree-16

---

# :kolena-diagram-tree-20: Thresholded Results

## When to Use Thresholded Results

Thresholded results give you the flexibility to enable threshold dependent metrics and plots.
Below are the supported use cases:

- When you want to calculate and compare metrics at different thresholds on Kolena. A common task for
semantic segmentation or object detection tasks.
- Plotting the [Precision-Recall (PR) Curve](../../metrics/pr-curve.md) dynamically based on your test cases.
- Computing [Average Precision](../../metrics/average-precision.md) dynamically based on your test cases.

## Format and upload thresholded results

A thresholded result consist of a list of dictionaries. Each dictionary contains:

- A required numeric field named `threshold` (number).
- An optional string field `label` (string).
- A true positive field
- A false positive field
- A false negative field

For instance, in a semantic segmentation problem:

```py
[
    {
        "threshold": 0.3,
        "label": "train",
        "tp": 30,  # True Positives
        "fp": 20,  # False Positives
        "fn": 10,   # False Negatives
    },
    {
        "threshold": 0.4,
        "label": "train",
        "tp": 40,
        "fp": 20,
        "fn": 0,
    },
    ...
]
```

!!! note
    The thresholded object should be added to a model result dataframe with at least one column other
    than the `thresholded` column that contains the thresholded object.

### Uploading Thresholded Results

After generating your thresholded object, you can upload it to Kolena using the `upload_results` method:

```py
from kolena.dataset import upload_results

def generate_thresholded_object():
    # Generate thresholded object based on your raw evaluation data and logic
    ...

df_results["thresholded"] = generate_thresholded_object()
upload_results("dataset_name", "model_name", df_results, thresholded_fields=["thresholded"])
```
Once you have uploaded your thresholded results, you can setup [Thresholded Object Metrics](../../dataset/advanced-usage/task-metrics.md#thresholded-object-metrics).

!!! note
    You can upload multiple thresholded object fields if needed.

## Plots for Thresholded Object metrics

You can find Kolena's plotting tools under the Debugger tab.

### Task Plots

Once on the Debugger tab, navigate to the "Task Plots" tab to view out-of-the-box plots.
You are able to configure each plot based on the format of your Thresholded Results and your objectives.

<figure markdown>
![Thresholded Plots](../../assets/images/thresholded-plots-light.gif#only-light)
![Thresholded Plots](../../assets/images/thresholded-plots-dark.gif#only-dark)
<figcaption>Precision vs Recall and F1 vs Threshold Plots</figcaption>
</figure>
