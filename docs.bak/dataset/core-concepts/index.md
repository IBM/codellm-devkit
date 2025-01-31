---
icon: kolena/area-of-interest-16
---

# :kolena-area-of-interest-20: Core Concepts

In this section, we explore Kolena's core concepts, focusing on the key features that facilitate model evaluation and
testing. For a quick overview, refer to the [Quickstart Guide](../quickstart.md).

## :kolena-dataset-20: Dataset

A **dataset** is a structured assembly of datapoints, designed for model evaluation.
This structure is immutable, meaning once a datapoint is added,
it cannot be altered without creating a new version of the dataset.
This immutability ensures the integrity and traceability of the data used in testing models.

### Datapoints

Datapoints are versatile and immutable objects. A datapoint is a set of inputs that you would want to
test on your models and has the following key characteristics:

- **Unified Object Structure**:
  Datapoints are singular, grab-bag objects that can embody various types of data,
  including images, as indicated by the presence of a data_type field.

- **Immunity to Change**: Once a datapoint is added to a dataset, it cannot be altered.
  Any update to a datapoint results in the creation of a new datapoint, and this action consequently versions the dataset.

- **Exclusive Association with Datasets**:
  Datapoints are exclusive to the dataset they belong to and are not shared across different datasets.
  This exclusivity ensures clear demarcation and management of data within specific datasets.

- **Role in Data Ingestion**: Datapoints play a central role in the data ingestion process.
  They are represented in a DataFrame structure with special treatment for certain columns like `locator` and `text`.

- **Extension of Data Classes**: Datapoints extend data classes, allowing for flexibility and customization.
  For instance, they can include annotation objects like [`BoundingBox`][kolena.annotation.BoundingBox],
  and these objects can be further extended as needed.

Consider a single row within the [:kolena-widget-16: Classification (CIFAR-10) ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/dataset/classification)
dataset with the following columns:

| locator                    | ground_truth | image_brightness |   image_contrast |
|---------------------------------------------------------------|--------------|----------|-----|
| `s3://kolena-public-examples/cifar10/data/horse0000.png`        | horse        |     153.994     |    84.126  |

This datapoint points to an image `horse0000.png` which has the ground_truth classification of `horse`,
and has brightness and contrast data.

#### Datapoint Components

**Unique Identifier**: each datapoint should have a hashable unique identifier.

You are able to select one or more fields as your ID field during the import process via the
Web App [:kolena-dataset-16: Datasets](https://app.kolena.com/redirect/datasets) or the
SDK by using the [`upload_dataset`](../../reference/dataset/index.md#kolena.dataset.dataset.upload_dataset) function.

**Meta data**: you can add additional information about your
datapoint simply by adding columns to the dataset with the meta data name and values in each row.

**Referenced Files**: each datapoint can contain a primary reference to a file stored on your cloud storage.
Kolena automatically renders referenced files with column name `locator`. Other column names result in references to appear
as text. Below table outlines what extensions are supported for optimal visualization.

| Data Type      | Supported file formats                                                                |
|----------------|---------------------------------------------------------------------------------------|
| Image          | `jpg`, `jpeg`, `png`, `gif`, `bmp` and other web browser supported image types.       |
| Audio          | `flac`, `mp3`, `wav`, `acc`, `ogg`, `ra` and other web browser supported audio types. |
| Video          | `mov`, `mp4`, `mpeg` and other web browser supported video types.                     |
| Document       | `txt`, `pdf`, `log`, `md` files.                                                      |
| Point Cloud    | `pcd` files.                                                                          |

**Assets**: allow you to connect multiple referenced files to each datapoint for visualization and analysis.
Multiple assets can be attached to a single datapoint.

| Asset Type                                                                 | Description                                                    | Supported Extensions          |
|----------------------------------------------------------------------------|----------------------------------------------------------------|-------------------------------|
| [`ImageAsset`](../../reference/asset.md#kolena.asset.ImageAsset)           | Useful if your data is modeled as multiple related images.     | Same as above reference files |
| [`BinaryAsset`](../../reference/asset.md#kolena.asset.BinaryAsset)         | Useful if you want to attach any segmentation or bitmap masks. | Any, including `.bin` files   |
| [`AudioAsset`](../../reference/asset.md#kolena.asset.AudioAsset)           | Useful if you want to attach an audio file.                    | Same as above reference files |
| [`VideoAsset`](../../reference/asset.md#kolena.asset.VideoAsset)           | Useful if you want to attach a video file.                     | Same as above reference files |
| [`PointCloudAsset`](../../reference/asset.md#kolena.asset.PointCloudAsset) | Useful for attaching 3D point cloud data.                      | `.pcd`, `.npy`, `.npz`        |
| [`MeshAsset`](../../reference/asset.md#kolena.asset.MeshAsset)             | Useful for attaching and visualizing 3D mesh files.            | `.ply`                        |
| [`DocumentAsset`](../../reference/asset.md#kolena.asset.DocumentAsset)     | Useful if you want to attach a document file.                  | `.pdf`, `.txt`, `.log`, `.md` |

**Annotations**: allow you to visualize overlays on top of datapoints through the use of[`annotation`](../../reference/annotation.md).
We currently support 10 different types of annotations each enabling a specific modality.

??? "How to generate datapoints"
    You can structure your dataset as a CSV file. Each row in the file should represent a distinct datapoint.
    For complete information on creating datasets, visit [formatting your datasets](../advanced-usage/dataset-formatting/index.md).

## :kolena-quality-standard-20: Quality Standard

A **Quality Standard** tracks a standardized process for how a team evaluates a model's performance on a dataset.
Users may define and manage quality standards for a dataset in the Kolena web application using the
`Quality Standards` tab.

A Quality Standard is composed of [Test Cases](#test-cases) and [Metrics](#metrics).

### Test Cases

Test cases allow users to evaluate their datasets at various levels of division, providing visibility into how models
perform at differing subsets of the full dataset, and mitigating failures caused by
[hidden stratifications](https://www.kolena.com/blog/best-practices-for-ml-model-testing).

Kolena supports easy test case creation through dividing a dataset along categorical or numeric datapoint properties.
For example, if you have a dataset with images of faces of individuals, you may wish to create a set of test cases that
divides your dataset by `datapoint.race` (categorical) or `datapoint.age` (numeric).

The quickstart guide provides a more hands-on example of
[defining test cases](../quickstart.md/#define-test-cases).

### Metrics

Metrics describe the criteria used to evaluate the performance of a model and compare it with other models over a given
dataset and its test cases.

Kolena supports defining metrics by applying standard aggregations over datapoint level results or by leveraging
common machine learning aggregations, such as [Precision](../../metrics/precision.md) or
[F1 Score](../../metrics/f1-score.md). Once defined, users may also specify highlighting for metrics, indicating if
`Higher is better`, or if `Lower is better`.

The datasets quickstart provides a more hands-on example of
[defining metrics](../quickstart.md/#define-metrics). For more details on out-of-the-box and custom metrics visit [Task Metrics](../advanced-usage/task-metrics.md)

### Model Comparison

Once you've defined your test cases and metrics, you can view and compare model results in the `Quality Standards` tab,
which provides a quick and standardized high level overview of which models perform best over your different test cases.

For step-by-step instructions, take a look at the quickstart for
[model comparison](../quickstart.md/#step-5-compare-models).

!!! tip
    Use the `Filter Untested Datapoints (or Filter to Intersection)` option to narrow down your metrics
    to only include datapoints that all selected
    models have tested on. This allows for an apple to apple comparison of metrics.

### Debugging

The `Debugger` tab of a dataset allows users to experiment with test cases and metrics without saving them off to the
team level quality standards. This allows users to search for meaningful test cases and experiment with different
metrics with the confidence that they can safely save these updated values to their quality standards when comfortable,
without the risk of accidentally replacing what the team has previously defined. This also provides a view for
visualizing results and relations in plots.

For step-by-step instructions, take a look at the quickstart for
[results exploration](../quickstart.md/#step-3-explore-data-and-results).

## Model Leaderboard

Model Leaderboard allows you to identify the best performing models at a glance. The leaderboard organizes all
uploaded model results in order of their rank.

!!! requirements
    A functional Model Leaderboard depends on well defined **metrics** and uploaded **model results**.
    To enable this feature, make sure that you have at least
    one metric defined in your Quality Standards and ensure that the direction of that metric is set (Higher is better or
    Lower is better).

    Metrics without direction are not used in the ranking algorithm.

**Rank**: ranking leverages standardized scoring (z-score) to compare metrics
from different distributions. Kolena uses the range of each metric to
estimate its standard deviation which is used in calculating the z-score.

**Filter Untested Datapoint (or Filter to Intersection)**: this option allows you to rank your models only on
datapoints that they were all tested on. Use this feature if its important to you to
standardize the comparison set of datapoints.

**Metric Selection**: Kolena's model leaderboard allows you see the model rank based on specific
metric groups. For instance you can group cost related metrics (such as `inference cost` or
`inference time`) under a metric group and chose to include or exclude those metrics in your
model ranking

<figure markdown>
![Select Metric Groups](../../assets/images/metric-groups-leaderboard-light.gif#only-light)
![Select Metric Groups](../../assets/images/metric-groups-leaderboard-dark.gif#only-dark)
<figcaption>Select Metric Groups to see relative ranks</figcaption>
</figure>

!!! tip
    Using the `Compare Top 3 Models` you can dive deeper into model performance and evaluate them top 3 models
    based on your defined quality standards. You can also select specific models to review in detail from the leaderboard.
