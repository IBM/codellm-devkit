---
description: How to learn more about your data using the embedding space visualization
---

# Representation Score

The `representation_score` measures how well each data point in an unlabelled or training dataset is represented within the
validation or test dataset. This score is calculated by fitting a Kernel Density Estimation (KDE) model on the embeddings
of the validation/test data in the 2D UMAP space. The KDE model estimates the probability density function of the
validation/test data distribution. For each data point in the training dataset, the representation_score is obtained by
evaluating the KDE (of the validation/test data) at that point, yielding the log-probability density. This score
effectively quantifies the extent to which each training data point lies in regions of the embedding space populated by
validation/test data.

!!! note
    To utilize this score, provide the appropriate metadata that indicate if a datapoint is in the training set or not.
    For example, you can use a `split` property to pass `train` or `test` values and use for this score.

!!! note
    To further assist users with data curation tasks, Kolena automatically calculated a number of metrics
    based on the embedding space details. Enable automatic embedding extractions or
    [upload your own embeddings](../dataset/advanced-usage/upload-embeddings.md) to
    utilize these scores.

## Interpretation

The representation_score is a valuable indicator of the alignment between the unlabelled/training data and the
validation/test data distributions. A higher representation_score for a training data point implies that it occupies a
region in the embedding space that is well-represented in the validation/test set, suggesting that its features are
relevant for the model's performance on unseen data. Conversely, a lower score may highlight areas where the training data
lacks coverage of the validation/test data distribution. Its goal is to identify gaps in the training data that could
impact model training. This metric is useful for guiding data collection, labelling, and augmentation tasks, allowing you
to address underrepresented areas in the training dataset, improving model robustness and accuracy on validation and test
data.
