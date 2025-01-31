---
search:
  boost: -0.5
---

# :kolena-classification-20: Classification

!!! example "Experimental Feature"

    This pre-built workflow is an experimental feature. Experimental features are under active development and may
    occasionally undergo API-breaking changes.

Classification is a machine learning task aiming to group objects and ideas into preset categories.
Classification models used in machine learning predict the likelihood or probability that the data will fall into
one of the predetermined categories.

There are different types of classification models:

| Classification Type | Description                                                                                                                                     |
| ------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------- |
| **Binary**          | Classification model predicts a single class, using a threshold on prediction confidence to bisect the test set                                 |
| **Multiclass**      | Classification model predicts a single class from more than two classes, with highest prediction confidence                                     |
| **Multi-label**     | Classification model predicts multiple classes, with each prediction over a threshold considered positive (i.e. ensemble of binary classifiers) |

This pre-built workflow is work in progress; however, you can refer to the workflow implementation for **binary**
and **multiclass** types from the examples below:

<div class="grid cards" markdown>
- [:kolena-classification-20: Example: Binary Classification](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/classification#binary-classification-on-dogs-vs-cats)

    !["Dog" classification example from the Dogs vs. Cats dataset.](../../assets/images/classification-dog.jpg)

    ---

    Binary Classification of class "Dog" using the [Dogs vs. Cats](https://www.kaggle.com/c/dogs-vs-cats) dataset

- [:kolena-classification-20: Example: Multiclass Classification](https://github.com/kolenaIO/kolena/tree/trunk/examples/workflow/classification#multiclass-classification-on-cifar-10)

    ![Example images from CIFAR-10 dataset.](../../assets/images/CIFAR-10.jpg)

    ---

    Multiclass Classification using the [CIFAR-10](https://www.cs.toronto.edu/~kriz/cifar.html) dataset

</div>

## Utility Methods

::: kolena._experimental.classification.utils
    options:
      members_order: alphabetical
