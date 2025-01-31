---
description: How to find common model regressions and failures
---

# Difficulty Score

Difficulty scores are automatically computed within [Kolena](https://www.kolena.com/) to **surface
[datapoints](../dataset/core-concepts/index.md#datapoints) that commonly contribute to poor
model performance**. Difficulty scores consider a user's custom
[Quality Standard](../dataset/core-concepts/index.md#quality-standard) configuration to make
an informed assessment of which datapoints lead to the greatest recurring problems across all models using multiple
performance indicators. Difficulty scores range from 0 to 1, where a lower difficulty score
indicates that models produce the ideal datapoint-level metrics (e.g. lower inference time, higher accuracy),
and a higher difficulty score indicates that models consistently face problems or "difficulty"
(e.g. longer inference time, lower [BLEU scores](./bleu.md), and/or lower [recall](./recall.md)).

!!!example
    To see an example of the Difficulty Score in action, checkout the
    [Object Detection (COCO 2014) on app.kolena.com/try.](https://app.kolena.io/try/dataset/studio?datasetId=14&models=N4IglgJiBcBMCsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIYAWAX3qA&models=N4IglgJiBcBMAsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIZ4BfOoA&modelResultNullFilters=N4IglgJiBcBMCsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIYAWAX2TAGcB9DfAG05i2M%2BYBTekA&modelResultNullFilters=N4IglgJiBcBMAsAaEBjA9gOwGZgOYFcAnAQwBcxMZRIZ4BfZMAZwH0N8AbDmLYjpgKZ0gA&sortByType=datapoint&sortBy=_kolena.extracted.difficulty_score&filters=datapoint._kolena.extracted.difficulty_score%3AN4IgdgrgtgRgpgJwPoIIZgOZxALlFASzFwAYA6ATgBoQpUAPXARgF8aB7ABwBcD2wAzrlAI4WekjrcAxgAtcAM1QAbAXBqjxSBctQYhOECBpEAbojWKValiyA%3Anr)

!!! note
    For Kolena to calculate the `datapoint.difficulty_score` you must have:

    * at least one [Model Result](../dataset/quickstart.md#step-2-upload-model-results) uploaded
    * at least one metric defined in your [Quality Standard](../dataset/core-concepts/index.md#quality-standard)
    * set the direction of the [metric](../dataset/core-concepts/index.md#metrics)
    (`Lower is better` or `Higher is better`)

When one model is selected in [<nobr>:kolena-studio-16: Studio</nobr>](https://app.kolena.com/redirect/studio), the
difficulty score of a datapoint is the difficulty score derived from that model's results. When more than one model is
considered, the overall difficulty score of a datapoint (`datapoint.difficulty_score`) is the average value from
each difficulty score from each model's results.

??? "Using Difficulty Scores for Regression Testing"
    When two models called `A` and `B` are selected in Studio, users can
    see two model-level difficulty scores, and one overall difficulty score for any datapoint. With a filter for
    `resultB.difficulty_score > resultA.difficulty_score`, we find all the datapoints that performed worse
    for model `B`, which highlights the regressions.

    With a filter for `datapoint.difficulty_score > 0.9`, we see all the datapoints that significantly struggle
    across both models, which are common failures that persist over different model iterations.

## Implementation Details

Suppose we have defined quality standards composed of various metrics and performance indicators.
Some of these will be metrics like [ROUGE](./rouge-n.md) or [accuracy](./accuracy.md) where higher values are better
(`HIB`, higher is better), but it is desirable for [word_error_rate](./wer-cer-mer.md) or cost to be minimized
(`LIB`, lower is better). Using the results of some model `A`, we can compute model-level difficulty scores for `A`,
denoted as `resultA.difficulty_score`.

To describe the computation of difficulty scores at a high level:

$$
\text{resultA.difficulty_score} = \sum_{i=1}^{QS} w_i \cdot \text{norm}(q_i)
$$

$$
\text{datapoint.difficulty_score} = \frac{1}{len(M)} \sum_{m}^{M} \text{result\{m\}.difficulty_score}
$$

where:

- \( QS \) is the set of quality standards
- \( w_i \) represents the weight for each quality standard \( i \)
- \( q_i \) represents the value of the quality standard \( i \), inverted if necessary
- \( \text{norm}(q_i) \) indicates the normalized value of the quality standard \( q_i \)
- \( M \) is the set of chosen models, such as models `A`, `B`, and `C`

!!! info "Important Note"

    Note that `datapoint.difficulty_score` is the average of all relevant
    model-level `resultX.difficulty_score` values.

Below is a detailed example of how `resultA.difficulty_score` is computed
using cost, recall, and accuracy:

```py
import pandas as pd

# names of quality standards where "lower is better"
LIB = ['cost']

# names of quality standards where "higher is better"
HIB = ['recall', 'accuracy']

# the name of the column for unique identifiers to a datapoint
id_column = 'id'


quality_standards = LIB + HIB
weights = [1/len(quality_standards)] * len(quality_standards) # weighting can be customized


model_results_csv = "path-to-first-model-results.csv"
df = pd.read_csv(model_results_csv, usecols=[id_column] + quality_standards)


def add_model_difficulty_score(df, lib, hib, weights):
    """
    Adds difficulty scores to datapoints in dataframe provided a quality standard configuration.
    """

    qs = lib + hib
    for col in qs:
        df[col] = df[col].astype(float)

        # invert HIB values such that higher values yield lower difficulty scores
        if col in hib:
            df[col] = df[col] * -1.0

        # normalize the column
        min_val = df[col].min()
        max_val = df[col].max()
        df[col] = (df[col] - min_val) / (max_val - min_val)

    df["difficulty_score"] = df[qs].dot(weights) # weighted sum
    return df


df = add_model_difficulty_score(df, LIB, HIB, weights)
```

## Example

1. Begin with a CSV of model results.

    | id | recall (HIB) | cost (LIB) | accuracy (HIB) |
    | --- | --- | --- | --- |
    | `1` | `0.10` | `3.14` | `0.50` |
    | `2` | `0.50` | `0.90` | `0.80` |
    | `3` | `0.90` | `0.01` | `0.99` |
    | `4` | `0.60` | `0.50` | `0.55` |

2. Invert every `HIB` column.

    | id | recall (HIB) | cost (LIB) | accuracy (HIB) |
    | --- | --- | --- | --- |
    | `1` | `-0.10` | `3.14` | `-0.50` |
    | `2` | `-0.50` | `0.90` | `-0.80` |
    | `3` | `-0.90` | `0.01` | `-0.99` |
    | `4` | `-0.60` | `0.50` | `-0.55` |

3. Normalize every column.

    | id | recall (HIB) | cost (LIB) | accuracy (HIB) |
    | --- | --- | --- | --- |
    | `1` | `1.00` | `1.00` | `1.00` |
    | `2` | `0.50` | `0.284` | `0.387` |
    | `3` | `0.00` | `0.00` | `0.00` |
    | `4` | `0.375` | `0.156` | `0.898` |

4. Compute difficulty scores using weighted sums for each datapoint.

    | id | recall (HIB) | cost (LIB) | accuracy (HIB) | difficulty_score |
    | --- | --- | --- | --- | --- |
    | `1` | `1.00` | `1.00` | `1.00` | `1.00` |
    | `2` | `0.50` | `0.284` | `0.387` | `0.390` |
    | `3` | `0.00` | `0.00` | `0.00` | `0.00` |
    | `4` | `0.375` | `0.156` | `0.898` | `0.476` |

    Below is the math behind the 2nd datapoint's (`id == 2`) difficulty score assuming equal weighting:

    $$
    \begin{align}
    \text{difficulty_score} &= \frac{1}{3} * 0.5 + \frac{1}{3} * 0.284 + \frac{1}{3} * 0.387 \\[1em]
    &= 0.390
    \end{align}
    $$

We have computed a new column of difficulty scores for each datapoint based on the quality standards set by the user.
If we were to add a new model, then the overall difficulty score would be the average of difficulty scores
across each model.

| id | resultA.difficulty_score | resultB.difficulty_score | resultC.difficulty_score | datapoint.difficulty_score |
| --- | --- | --- | --- | --- |
| `1` | `0.3` | `0.3` | `0.3` | `0.30` |
| `2` | `0.1` | **`0.9`** | `0.1` | **`0.37`** |
| `3` | `0.4` | **`0.2`** | `0.6` | **`0.4`** |

From the table above, we see that the 2nd datapoint performs very poorly on the 2nd model (`resultB`) with a difficulty
score of `0.9`, while the 3rd datapoint has `0.2` just underneath. However, the computed `difficulty_score` values
indicate that the 3rd datapoint is repeatedly the most challenging datapoint for the models based on the defined
quality standard.

### Difficulty Scores for Task Metrics

Difficulty scores for [task metrics](../dataset/advanced-usage/task-metrics.md) are aggregate metrics that do not offer
datapoint-level details of performance. However, the information provided to an aggregate metric is sufficient in
establishing difficulty scores at the datapoint level, similar to datapoint-level `inference_time` or `cost`.

#### Binary Classification and Regression

The difficulty score is the absolute error between the model result and the ground truth value.

Suppose in a binary classification problem, a model's inference is binarized by the threshold of `0.5`,
so the positive class would be defined by values `0.5` to `1.0`, and values of the negative class would
be from `0.0` to `0.5`.

| id | ground_truth | inference | Δ | norm(Δ) |
| --- | --- | --- | --- | --- |
| `1` | `1`| `0.01` | `0.99` | `1.00` |
| `2` | `1`| `0.49` | `0.51` | `0.51` |
| `3` | `1`| `0.50` | `0.50` | `0.50` |
| `4` | `1`| `0.80` | `0.20` | `0.19` |
| `5` | `0`| `0.01` | `0.01` | `0.00` |
| `6` | `0`| `0.49` | `0.49` | `0.49` |
| `7` | `0`| `0.50` | `0.50` | `0.50` |
| `8` | `0`| `0.80` | `0.80` | `0.81` |

In the case of regression problems, difficulty can be measured in a similar way using the magnitude of the
difference between the ground truth and the inference.

| id | ground_truth | inference | Δ | norm(Δ) |
| --- | --- | --- | --- | --- |
| `1` | `1`| `1` | `0` | `0.00` |
| `2` | `2`| `1` | `1` | `0.08` |
| `3` | `3`| `2` | `1` | `0.08` |
| `4` | `4`| `3` | `1` | `0.08` |
| `5` | `5`| `5` | `0` | `0.00` |
| `6` | `6`| `8` | `2` | `0.15` |
| `7` | `7`| `13` | `6` | `0.46` |
| `8` | `8`| `21` | `13` | `1.00` |

The greater the distance the inference is from the ground truth, the greater the difficulty of that
datapoint. These normalized `Δ` column, called a `norm(Δ)` column, becomes another column in step 4 of the example
above which is parallel to `cost` or `recall`. Then, it can be involved in the computation of the overall
`datapoint.difficulty_score`.

#### Multiclass Classification

The `Δ` column for a datapoint of a multiclass classification task is the count of misclassifications for the
datapoint. For example, with three models the best case is a count of zero mistakes and the worst case
sums to three mistakes. Like the binary classification and regression task, the `norm(Δ)` column normalizes `Δ` to be
used in computing the overall `datapoint.difficulty_score`.

#### Object Detection

The `Δ` column for an object detection task is the [F<sub>1</sub>-score](./f1-score.md) computed using the total number
of [TP / FP / FN counts](./tp-fp-fn-tn.md). If [recall](./recall.md) is more important, this can become the default
signal instead of F<sub>1</sub>-scores at the datapoint level.
Like the other tasks, the `norm(Δ)` column normalizes `Δ` to be used in computing the overall
`datapoint.difficulty_score`.

## Limitations and Biases

Difficulty scores require well-configured quality standards and perform better with multiple metrics and
indicators involved. Difficulty scores are not as useful unless multiple models are involved.

It is hard to interpret the value of a difficulty score directly, as it is an aggregate
signal reflecting quality standards relative to all models of interest with all their results.
