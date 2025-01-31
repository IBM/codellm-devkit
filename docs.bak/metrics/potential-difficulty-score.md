---
description: Leverage difficulty score and embedding space to find data for labeling or training
---

# Potential Difficulty Score

The `potential_difficulty_score` estimates the likelihood that each training data point will be challenging for models, based
on its proximity to difficult validation/test data points. This score is computed by fitting a weighted Kernel Density
Estimation (KDE) model on the embeddings of validation/test data points in the 2D UMAP space, where the weights are derived
from the [`difficulty_score`](./difficulty-score.md) associated with each validation/test data point.
The KDE model generates a probability density
function that emphasizes regions with higher difficulty scores. The `potential_difficulty_score` for each training data point
is then obtained by evaluating the weighted KDE at that point to highlight where difficult validation/test data points are
concentrated.

!!! note
    To further assist users with data curation tasks, Kolena automatically calculated a number of metrics
    based on the embedding space details. Enable automatic embedding extractions or
    [upload your own embeddings](../dataset/advanced-usage/upload-embeddings.md) to
    utilize these scores.

!!! note
    You must have at least one metric defined in your quality standard to utilize this score. Metrics are used
    to calculate the difficulty score which is then used in conjunction with the embedding space to calculate the
    potential difficulty score.

## Interpretation

The `potential_difficulty_score` identifies training data points that are in regions associated with higher difficulty from
the validation/test set. A higher score hints that the training data point is close to areas where the model has struggled,
potentially highlighting features that are challenging to learn. By focusing on these data points, you can prioritize
efforts for parts of the dataset towards model improvement such as labelling, data collection, etc. This metric aims to
improve model generalization and robustness by ensuring that challenging patterns are adequately represented in model
training.
