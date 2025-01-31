---
icon: kolena/paragraph-16
---
# :kolena-paragraph-20: Natural Language

In this document we will review best practices when setting up Kolena datasets for NLP or LLM
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

### Using the `text` column

Text samples can be visualized on Kolena one of two ways.

**Gallery mode**: visualizes each text value as a tile.

To enable this view, include your primary text values in your `.CSV` in a column named `text`.

<figure markdown>
![Gallery View](../../../assets/images/gallery-text-dark.png#only-dark)
![Gallery View](../../../assets/images/gallery-text-light.png#only-light)
<figcaption>Gallery View</figcaption>
</figure>

!!! example
    The [:kolena-widget-16: Text Summarization ↗](https://github.com/kolenaIO/kolena/tree/3b97541ad4b6b1fb7721d754aa0d0092cd752cca/examples/dataset/text_summarization)
    example showcases how texts can be uploaded in Gallery mode.

!!! tip
    Use the [`TextSegment`](../../../reference/annotation.md#kolena.annotation.TextSegment) or
    [`LabeledTextSegment`](../../../reference/annotation.md#kolena.annotation.LabeledTextSegment) annotations
    to highlight parts of your text that is of interest to you.

**Tabular mode**: visualizes each text field with its corresponding meta-data in a table with common table functionalities.

To use this view, simply provide the text values in your `.CSV` with any column names other than `text`.
<figure markdown>
![Tabular View](../../../assets/images/tabular-text-dark.png#only-dark)
![Tabular View](../../../assets/images/tabular-text-light.png#only-light)
<figcaption>Tabular View</figcaption>
</figure>

### Using fields

You can add additional information about your text by adding columns to the `.CSV` file with the meta-data name and
values in each row.

!!! tip
    Kolena is able to automatically extract multiple properties from your text values by [Extracting Metadata from Text Fields](../../../automations/extract-text-metadata.md).
    You can use these values to create test cases and better understand your data.

## Uploading Model Results

Model results contain your model inferences as well as any custom metrics that you wish to monitor on Kolena.
The data structure of model results is very similar to the structure of a dataset.

* make sure to link your inferences to the dataset using the same unique ID (for example the `text` field)
you used when uploading the dataset.
* use [`ScoredTextSegment`](../../../reference/annotation.md#kolena.annotation.ScoredTextSegment) or [`ScoredLabeledTextSegment`](../../../reference/annotation.md#kolena.annotation.ScoredLabeledTextSegment)
 annotations to indicate the inference confidence score.
