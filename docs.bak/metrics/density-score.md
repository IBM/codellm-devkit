---
description: How to learn more about your data using the embedding space visualization
---

# Density Score

The `density_score` quantifies the local data density surrounding each data point within the 2D UMAP embedding space.
It is calculated by applying a Kernel Density Estimation (KDE) using a Gaussian kernel on the two-dimensional embeddings
of the dataset. The bandwidth parameter of the KDE (defaulted to 0.1 but tunable) controls the smoothness of
the density estimation, influencing how local or global the density estimation is. The resulting `density_score` reflects
the density distribution of data points in the 2D space.

!!! note
    To further assist users with data curation tasks, Kolena automatically calculated a number of metrics
    based on the embedding space details. Enable automatic embedding extractions or
    [upload your own embeddings](../dataset/advanced-usage/upload-embeddings.md) to
    utilize these scores.

## Interpretation

A higher `density_score` indicates that a data point is located in a region with many neighboring points,
suggesting that it
represents a common pattern or feature within the dataset. Conversely, a lower `density_score` suggests that the data point
resides in a sparsely populated region, potentially highlighting outliers or rare patterns. This metric is particularly
useful for visual data exploration, enabling you to identify clusters, anomalies, and understand the overall structure of
your dataset in the embeddings visualizer.
