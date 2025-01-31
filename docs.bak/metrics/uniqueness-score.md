---
description: How to learn more about your data using the embedding space visualization
---

# Uniqueness Score

The `uniqueness_score` measures how distinctive a data point is relative to the rest of the dataset in the original
high-dimensional embedding space. It is calculated by determining the average distance from each data point from its
nearest neighbors (top 50 nearest). The average of these distances serves as the uniqueness_score, where a larger average
distance indicates that the data point is farther from its neighbors and therefore more unique.

!!! note
    To further assist users with data curation tasks, Kolena automatically calculated a number of metrics
    based on the embedding space details. Enable automatic embedding extractions or
    [upload your own embeddings](../dataset/advanced-usage/upload-embeddings.md) to
    utilize these scores.

## Interpretation

The `uniqueness_score` identifies data points that are unique or underrepresented within your dataset. A higher
`uniqueness_score` suggests that a data point resembles fewer points in the dataset, highlighting rare features. This can
be particularly significant in outlier detection or help enhance the diversity of a dataset. By analyzing data points
with high `uniqueness_score`, you can gain ensure that unique patterns are not overlooked, and make informed decisions
regarding special cases.
