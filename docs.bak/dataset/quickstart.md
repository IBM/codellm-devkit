---
icon: kolena/flame-16
---

# :kolena-flame-20: Quickstart

Set up rigorous and repeatable model testing in minutes.

In this quickstart guide, we will use the [300 Faces In the Wild (300-W)](https://ibug.doc.ic.ac.uk/resources/300-W/)
example dataset to demonstrate test data curation and model evaluation in Kolena, using both the web app and
the `kolena` Python SDK.

??? note "Using the SDK"
    If you would like to use the SDK for this quickstart follow these instructions to set up the
    SDK and set up the code example locally.

    First, install the `kolena` python SDK:

    === "`pip`"

        ```shell
        pip install kolena
        ```

    === "`uv`"

        ```shell
        uv add kolena
        ```

    Then, clone the example code:

    ```shell
    git clone https://github.com/kolenaIO/kolena.git
    ```

    The [kolenaIO/kolena](https://github.com/kolenaIO/kolena) repository contains a number of
    example integrations to clone
    and run directly. Set up the keypoint detection example for this quickstart:

    ```shell
    cd kolena/examples/dataset/keypoint_detection
    uv venv && source .venv/bin/activate
    uv pip install --all-extras -r pyproject.toml
    ```

## Step 1: Upload Dataset

Model evaluations on Kolena starts with datasets. Datasets are structured assembly of datapoints you wish to use for
model testing and evaluation.

=== "Web App"
    To get started, navigate to [kolena.com](https://app.kolena.com/redirect/datasets) and
    click `Import Dataset` then `Select From Cloud Storage`.
    Using the explorer, navigate to `s3://kolena-public-examples/300-W/` and select `300-W.csv`.

    ??? note "Generating Datasets"
        See the [`keypoint_detection/upload_dataset.py`](https://github.com/kolenaIO/kolena/blob/trunk/examples/dataset/keypoint_detection/keypoint_detection/upload_dataset.py)
        script in the code example for details on how the dataset was generated.

        The keypoints in `300-W.csv` have been defined using `kolena.annotation.Keypoints` from
        the `kolena` SDK.
        See the above script for example usage.

    You will now see a preview of how the information is going to be consumed by Kolena.

    Give your dataset a name and select `locator` as the ID field. The ID field uniquely identifies a datapoint
    and is used when uploading model results to associate results with datapoints.

    Click `Import` to create the dataset. Once the import has completed,
    you can add descriptions and tags to organize your datasets.

    <figure markdown>
        ![Example Dataset Upload](../assets/images/quickstart-upload-dataset-workflow-dark.gif#only-dark)
        ![Example Dataset Upload](../assets/images/quickstart-upload-dataset-workflow-light.gif#only-light)
        <figcaption>Example Dataset Upload</figcaption>
    </figure>

=== "SDK"

    The example code contains a script [`keypoint_detection/upload_dataset.py`](https://github.com/kolenaIO/kolena/blob/trunk/examples/dataset/keypoint_detection/keypoint_detection/upload_dataset.py)
    which will process the CSV file `s3://kolena-public-examples/300-W/raw/300-W.csv`
    and upload it to Kolena using the `upload_dataset` function.

    First, let's first configure our environment by populating the `KOLENA_TOKEN`
    environment variable. Visit the [:kolena-developer-16: Developer](https://app.kolena.com/redirect/developer) page to
    generate an API token and copy and paste the code snippet into your environment:

    ```shell
    export KOLENA_TOKEN="********"
    ```

    We can now register a new dataset using the provided script:

    ```shell
    uv run python3 keypoint_detection/upload_dataset.py
    ```

    After this script has completed, a new dataset named `300-W` will be created, which you can
    see in [:kolena-dataset-20: Datasets](https://app.kolena.com/redirect/datasets).

## Step 2: Upload Model Results

Model results are supplied as tables containing the ID field chosen when uploading the dataset. For
this example, we will upload the results for the open-source
[retinaface](https://github.com/serengil/retinaface) (`retinaface` option)
keypoint detection model and a random keypoint model.

=== "Web App"

    To upload new model results, from the `Details` tab of the dataset, click on `Upload Model Results` in the upper right.
    Then, select `Upload From Cloud Storage`. Using the explorer, navigate to `s3://kolena-public-examples/300-W/results/`
    and select `random.csv`. This CSV file contains model results for the random keypoints
    model for each of the datapoints we uploaded to the dataset.

    ??? note "Generating Model Results"
        See the [`keypoint_detection/upload_results.py`](https://github.com/kolenaIO/kolena/blob/trunk/examples/dataset/keypoint_detection/keypoint_detection/upload_results.py)
        script in the code example for details on how results were generated.

    You will now see a preview of how Kolena will ingest the model results. Click `Import` to
    upload the model results.

    <figure markdown>
        ![Example Results Upload](../assets/images/quickstart-upload-results-workflow.gif#only-dark)
        ![Example Results Upload](../assets/images/quickstart-upload-results-workflow-light.gif#only-light)
        <figcaption>Example Results Upload</figcaption>
    </figure>

    Repeat the above steps with the file `retinaface.csv`.

=== "SDK"

    The example code contains a script [`keypoint_detection/upload_results.py`](https://github.com/kolenaIO/kolena/blob/trunk/examples/dataset/keypoint_detection/keypoint_detection/upload_results.py)
    which will compute metrics on a CSV of model results and upload them to Kolena.

    ```shell
    uv run python3 keypoint_detection/upload_results.py retinaface
    uv run python3 keypoint_detection/upload_results.py random
    ```

    Results for two models named `retinaface` and `random` should now appear <somewhere>

## Step 3: Explore Data and Results

Once you have uploaded your dataset and model results, you can visualize the data using Kolena’s plotting tools.

You can quickly see the distribution of any datapoint or model results field in the `Distributions` tab.

<figure markdown>
![Distribution Plots](../assets/images/quickstart-distribution.jpg#only-dark)
![Distribution Plots](../assets/images/quickstart-distribution-light.png#only-light)
<figcaption>Distribution Plots</figcaption>
</figure>

Additionally, you can create custom plots in the `Debugger` tab. For example, click `Add Model` in the top left and
select the random model. In the plotting widget at the bottom, select `datapoint.normalization_factor` as the x-axis
and `result.mse > mean` as the y-axis to plot these two fields against each other.

<figure markdown>
![Custom Plots](../assets/images/quickstart-otf-plotting.gif#only-dark)
![Custom Plots](../assets/images/quickstart-otf-plotting-light.gif#only-light)
<figcaption>Custom Plots</figcaption>
</figure>

## Step 4: Define Quality Standards

Quality Standards define the criteria by which models are evaluated on each dataset. A Quality Standard consists of
Test Cases, which organize your data into key scenarios, and Metrics, which define key performance indicators. Metrics
can be configured in groups depending on your needs.

### Define Test Cases

To configure test cases, navigate to the `Quality Standards` tab and click on `Create Test Cases`. Select
`datapoint.condition` to create test cases based on the condition field. Select `Save Test Cases` button to save your
test cases to your Quality Standard.

You will now see that your dataset has been organized into test cases based on the `condition` field.
Any Metrics you define will be calculated on each test case.

<figure markdown>
![Creating Test Cases](../assets/images/add-test-cases.gif#only-dark)
![Creating Test Cases](../assets/images/add-test-cases-light.gif#only-light)
<figcaption>Creating Test Cases</figcaption>
</figure>

### Define Metrics

To configure Metrics, from the Quality Standards tab, click `Define Metrics`, select `Add Metric` button and
select `result.mse > mean` option.
Rename the metric to `Average MSE`, and select `Lower is better` as the highlight.
Repeat these steps for `result.nmse > mean`.

<figure markdown>
![Defining Metrics](../assets/images/quickstart-configure-metrics.gif#only-dark)
![Defining Metrics](../assets/images/quickstart-configure-metrics-light.gif#only-light)
<figcaption>Defining Metrics</figcaption>
</figure>

## Step 5: Compare Models

Once you have configured a Quality Standard, the Metrics you define will be calculated across all Test Cases.
To compare the results across models, navigate to the `Quality Standards` tab and add `random` model to the
table using the `Add Models` button in the top right. Then, add the `retinaface` model to compare its performance
to the `random` model.

You will now see all metrics in the Quality Standard computed on every test case in the Quality Standard.
Metrics in this view will also be highlighted according to how much they improve/worsen compared to the
reference model. In this example, retinaface performs better than the random model over the entire dataset
as well as over all test cases we defined earlier.

!!! tip
    Use the `Filter Untested Datapoints (or Filter to Intersection)` option to narrow down your metrics to
    only include datapoints that all selected
    models have tested on. This allows for an apple to apple comparison of metrics.

<figure markdown>
![Comparing Models](../assets/images/quickstart-compare-models.gif#only-dark)
![Comparing Models](../assets/images/quickstart-compare-models-light.gif#only-light)
<figcaption>Comparing Models</figcaption>
</figure>

## Conclusion

In this quickstart, we used an example integration from [kolenaIO/kolena](https://github.com/kolenaIO/kolena) to
integrate data from the [300 Faces In the Wild (300-W)](https://ibug.doc.ic.ac.uk/resources/300-W/) dataset,
created test cases, and tested the open-source [retinaface](https://github.com/serengil/retinaface)
keypoint detection model.

This example shows us how to quickly and easily integrate test data and evaluate model performance in Kolena,
and can be arbitrarily extended with additional metrics, plots, visualizations, and data.

For more information tailored to your specific use case, explore our [Use Cases](../dataset/use-cases.md).
