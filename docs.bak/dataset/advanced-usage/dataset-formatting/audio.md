---
icon: kolena/music-16
---

# :kolena-music-20: Audio

In this document we will review best practices when setting up Kolena datasets for Audio
problems.

## Basics

### Supported File Data Formats

The Kolena SDK supports uploading of data in the Pandas
[`DataFrame`](https://pandas.pydata.org/docs/reference/frame.html) format.

The Kolena web app supports the following file formats.

| Format    | Description                                              |
|-----------|----------------------------------------------------------|
| `.csv`     | Comma-separated values file, ideal for tabular data.     |
| `.parquet` | Apache Parquet format, efficient for columnar storage.   |
| `.jsonl`   | JSON Lines format, suitable for handling nested data.    |

### Using the `locator`

Kolena uses references to files stored in your cloud storage to render them.
Refer to ["Connecting Cloud Storage"](../../../connecting-cloud-storage/index.md)
for details on how to configure this.

Audio samples can be visualized on Kolena in one of two ways.

**Gallery mode**: visualizes each audio as a tile.
To enable the Gallery view store references to audio files in a column named `locator`. `locator` can be used as
the unique identifier of the datapoint which is also referenced by your model results.

!!! example
    The [:kolena-widget-16: Speaker Diarization ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/dataset/speaker_diarization)
    example showcases how audio files can be uploaded in Gallery mode.

<figure markdown>
![Audio Gallery View](../../../assets/images/gallery-audio-dark.png#only-dark)
![Audio Gallery View](../../../assets/images/gallery-audio-light.png#only-light)
<figcaption>Gallery View</figcaption>
</figure>

!!! tip
    You can use [`TimeSegment`](../../../reference/annotation.md#kolena.annotation.TimeSegment)'s
    or [`LabeledTimeSegment`](../../../reference/annotation.md#kolena.annotation.LabeledTimeSegment)
    to highlight segments of your audio file that are of interest to you.

**Tabular**: visualizes each datapoint as a row on a table with the audio file as an asset available for replay. To enable
this view, please use [`AudioAsset`](../../../reference/asset.md#kolena.asset.AudioAsset) to link your audio files
in your dataset.

!!! example
    The [:kolena-widget-16: Automatic Speech Recognition ↗](https://github.com/kolenaIO/kolena/tree/trunk/examples/dataset/automatic_speech_recognition)
    example showcases how `AudioAsset`s can be attached to datapoints.

<figure markdown>
![Audio Tabular View](../../../assets/images/tabular-audio-dark.png#only-dark)
![Audio Tabular View](../../../assets/images/tabular-audio-light.png#only-light)
<figcaption>Tabular View</figcaption>
</figure>

Kolena supports `flac`, `mp3`, `wav`, `acc`, `ogg`, `ra` and other web browser supported audio types.

### Using fields

You can add additional information about your audio files or other features of your data by
adding columns to the `.CSV` file or `DataFrame` with the meta-data name and values in each row.
Below is an example datapoint:

| Locator                                                | Num Speakers | Average Amplitude | audio_length |
|--------------------------------------------------------|--------------|------------------|--------------|
| `s3://kolena-public-examples/ICSI-corpus/data/audio/Bdb001/interval0.mp3` | 5            | 0.08951   | 448.454 |

## Uploading Model Results

Model results contain your model inferences as well as any custom metrics that you wish to monitor on Kolena.
The data structure of model results is very similar to the structure of a dataset.

* make sure to link your inferences to the dataset using the same unique ID (for example the `locator`)
you used when uploading the dataset.
* use [`ScoredTimeSegment`](../../../reference/annotation.md#kolena.annotation.ScoredTimeSegment) or [`ScoredLabeledTimeSegment`](../../../reference/annotation.md#kolena.annotation.ScoredLabeledTimeSegment)
 annotations to indicate the inference confidence score.
