---
search:
  boost: -0.5
---

# :kolena-flame-20: Quickstart

!!! note
    Kolena Workflows are simplified in Kolena Datasets.
    If you are setting up your Kolena environments for the first time, please refer to the
    [Datasets Quickstart](../dataset/quickstart.md).

Install Kolena to set up rigorous and repeatable model testing in minutes.

In this quickstart guide, we'll use the
[`object_detection_2d`](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/object_detection_2d)
example integration to demonstrate the how to curate test data and test models in Kolena.

## Install `kolena`

Install the `kolena` Python package to programmatically interact with Kolena:

=== "`pip`"

    ```shell
    pip install kolena
    ```

=== "`uv`"

    ```shell
    uv add kolena
    ```

## Clone the Examples

The [kolenaIO/kolena](https://github.com/kolenaIO/kolena) repository contains a number of example integrations to clone
and run directly:

<div class="grid cards" markdown>

- [:kolena-fr-20: Example: Face Recognition 1:1 ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/face_recognition_11)

    ![Example image from Face Recognition 1:1 Workflow.](../assets/images/fr11.jpg)

    ---

    End-to-end face recognition 1:1 using the [Labeled Faces in the Wild (LFW)](http://vis-www.cs.umass.edu/lfw/) dataset.

- [:kolena-age-estimation-20: Example: Age Estimation ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/age_estimation)

    ![Example images from the Labeled Faces in the Wild dataset.](../assets/images/LFW.jpg)

    ---

    Age Estimation using the [Labeled Faces in the Wild (LFW)](http://vis-www.cs.umass.edu/lfw/) dataset

- [:kolena-keypoint-detection-20: Example: Keypoint Detection ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/keypoint_detection)

    ![Example image and five-point facial keypoints array from the 300 Faces in the Wild dataset.](../assets/images/300-W.jpg)

    ---

    Facial Keypoint Detection using the [300 Faces in the Wild (300-W)](https://ibug.doc.ic.ac.uk/resources/300-W/)
    dataset

- [:kolena-text-summarization-20: Example: Text Summarization ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/text_summarization)

    ![Example articles from the CNN-DailyMail dataset.](../assets/images/CNN-DailyMail.jpg)

    ---

    Text Summarization using [OpenAI GPT-family models](https://platform.openai.com/docs/guides/gpt) and the
    [CNN-DailyMail](https://paperswithcode.com/dataset/cnn-daily-mail-1) dataset

- [:kolena-widget-20: Example: Object Detection (2D) ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/object_detection_2d)

    ![Example 2D bounding boxes from the COCO dataset.](../assets/images/COCO-transportation.jpeg)

    ---

    2D Object Detection using the [COCO](https://cocodataset.org/#overview) dataset

- [:kolena-drive-time-20: Example: Object Detection (3D) ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/object_detection_3d)

    ![Example pointcloud and 3D object bounding boxes from the KITTI dataset.](../assets/images/KITTI-pointcloud.png)

    ---

    3D Object Detection using the [KITTI](https://www.cvlibs.net/datasets/kitti/eval_object.php?obj_benchmark=3d) dataset

- [:kolena-classification-20: Example: Binary Classification ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/classification#binary-classification-on-dogs-vs-cats)

    !["Dog" classification example from the Dogs vs. Cats dataset.](../assets/images/classification-dog.jpg)

    ---

    Binary Classification of class "Dog" using the [Dogs vs. Cats](https://www.kaggle.com/c/dogs-vs-cats) dataset

- [:kolena-classification-20: Example: Multiclass Classification ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/classification#multiclass-classification-on-cifar-10)

    ![Example images from CIFAR-10 dataset.](../assets/images/CIFAR-10.jpg)

    ---

    Multiclass Classification using the [CIFAR-10](https://www.cs.toronto.edu/~kriz/cifar.html) dataset

- [:kolena-comparison-20: Example: Semantic Textual Similarity ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/semantic_textual_similarity)

    ![Example sentence pairs from STS benchmark dataset.](../assets/images/STS-benchmark.jpg)

    ---

    Semantic Textual Similarity using the [STS benchmark](http://ixa2.si.ehu.eus/stswiki/index.php/STSbenchmark) dataset

- [:kolena-chat-20: Example: Question Answering ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/question_answering)

    ![Example questions from CoQA dataset.](../assets/images/CoQA.jpg)

    ---

    Question Answering using the
    [Conversational Question Answering (CoQA)](https://stanfordnlp.github.io/coqa/) dataset

- [:kolena-polygon-20: Example: Semantic Segmentation ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/semantic_segmentation)

    ![Example image from COCO-Stuff 10K dataset.](../assets/images/coco-stuff-10k.jpg)

    ---

    Semantic Segmentation on class `Person` using the
    [COCO-Stuff 10K](https://github.com/nightrome/cocostuff10k) dataset

- [:kolena-audio-workflow-20: Example: Automatic Speech Recognition ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/automatic_speech_recognition)

    ![Example image from the automatic speech recognition workflow.](../assets/images/librispeech-workflow-example.png)

    ---

    Automatic speech recognition using the
    [LibriSpeech](https://www.openslr.org/12) dataset

- [:kolena-diarization-workflow-20: Example: Speaker Diarization ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/speaker_diarization)

    ![Example image from the speaker diarization workflow.](../assets/images/speaker-diarization-example.png)

    ---

    Speaker Diarization using the
    [ICSI-Corpus](https://groups.inf.ed.ac.uk/ami/icsi/) dataset

</div>

To get started, clone the `kolena` repository:

```shell
git clone https://github.com/kolenaIO/kolena.git
```

With the repository cloned, let's set up the
[`object_detection_2d`](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/object_detection_2d) example:

```shell
cd kolena/examples/workflow/object_detection_2d
poetry update && poetry install
```

Now we're up and running and can start [creating test suites](#create-test-suites) and
[testing models](#test-a-model).

## Create Test Suites

Each of the example integrations comes with scripts for two flows:

1. `seed_test_suite.py`: Create test cases and test suite(s) from a source dataset
2. `seed_test_run.py`: Test model(s) on the created test suites

Before running [`seed_test_suite.py`](https://github.com/kolenaIO/kolena/blob/trunk/examples/workflow/object_detection_2d/object_detection_2d/seed_test_suite.py),
let's first configure our environment by populating the `KOLENA_TOKEN`
environment variable. Visit the [:kolena-developer-16: Developer](https://app.kolena.com/redirect/developer) page to
generate an API token and copy and paste the code snippet into your environment:

```shell
export KOLENA_TOKEN="********"
```

We can now create test suites using the provided seeding script:

```shell
poetry run python3 object_detection_2d/seed_test_suite.py
```

After this script has completed, we can visit the [:kolena-test-suite-16: Test Suites](https://app.kolena.com/redirect/testing)
page to view our newly created test suites.

In this `object_detection_2d` example,
we've created test suites stratifying the [COCO](https://cocodataset.org/#overview) 2014 validation set
(which is stored as a CSV in S3) into test cases by brightness and bounding box size.
In this example will be looking at the following labels:

`["bicycle", "car", "motorcycle", "bus", "train", "truck", "traffic light", "fire hydrant", "stop sign"]`

## Test a Model

After we've created test suites, the final step is to test models on these test suites. The `object_detection_2d` example
provides the following models to choose from `{yolo_r, yolo_x, mask_rcnn, faster_rcnn, yolo_v4s, yolo_v3}` for this step:

```shell
poetry run python3 object_detection_2d/seed_test_run.py "yolo_v4s"
```

!!! note "Note: Testing additional models"
    In this example, model results have already been extracted and are stored in CSV files in S3. To run a new model,
    plug it into the `infer` method in [`seed_test_run.py`](https://github.com/kolenaIO/kolena/blob/trunk/examples/workflow/object_detection_2d/object_detection_2d/seed_test_run.py).

Once this script has completed, click the results link in your console or visit
[:kolena-results-16: Results](https://app.kolena.com/redirect/results) to view the test results for this newly tested model.

## Conclusion

In this quickstart, we used an example integration from [kolenaIO/kolena](https://github.com/kolenaIO/kolena) to create
test suites from the [COCO](https://cocodataset.org/#overview) dataset and test the
open-source `yolo_v4s` model on these test suites.

This example shows us how to define an ML problem as a workflow for testing in Kolena, and can be arbitrarily extended
with additional metrics, plots, visualizations, and data.
