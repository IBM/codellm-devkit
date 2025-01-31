---
description: How to detect hallucinations in generated text
---

# Hughes Hallucination Evaluation Model (HHEM) Score

[Hughes Hallucination Evaluation Model (HHEM)](https://huggingface.co/vectara/hallucination_evaluation_model) is an
open-source model that can be used to compute scores for hallucination detection. The scores are probabilities that
range from 0 to 1 — 0 means that there is a hallucination and 1 means that there is no hallucination (factually
consistent). According to Vectara, an appropriate threshold for this metric is 0.5 to predict whether a text is
consistent with another.

## Implementation Details

HEM is based on Microsoft's [deberta-v3-base](https://huggingface.co/microsoft/deberta-v3-base), which was trained on
NLI (Natural Language Inference) data, and then fine-tuned on text summarization datasets.

Further details for development and usage can be found on Hugging Face:
[vectara/hallucination_evaluation_model](https://huggingface.co/vectara/hallucination_evaluation_model).
Below is a quick example of how it can be used:

```py
# Installation:
# pip install -U sentence-transformers

from sentence_transformers import CrossEncoder
hallucination_model = CrossEncoder('vectara/hallucination_evaluation_model')

def compute_metric(ground_truth: str, inference: str) -> float:
    hallucination_score = hallucination_model.predict([ground_truth, inference])
    return hallucination_score

print(compute_metric("The duck crossed the road", "The duck did not cross the road"))
# Outputs:
# 0.0004
```

## Examples

| Ground Truth | Inference | Metric |
| --- | --- | --- |
| `The duck crossed the road` | `The duck did not cross the road` | `0.0004` |
| `The duck crossed the road` | `The animal crossed the road` | `0.9817` |
| `The duck crossed the road` | `The duck crossed my path` | `0.5381` |

## Limitations and Biases

1. The model provided on Hugging Face suggests that expected input is simply pairs of text. This means that lengthy
context cannot be provided. If relevant, context can be added to the front of both texts within the pair. For example:
`("What did the duck do? The duck crossed the road.", "What did the duck do? The animal crossed the road.")`.

2. Following the point above, in the absence of context, Vectara's HEM might produce probabilities that a human, who
has the complete context at hand, might disagree with. Certain extra details within an inference might influence a
score to soar from near 0 to almost 1. For example: `("Canada and Mexico", "Canada and Mexico, but not the USA")`.

3. Explainability is a challenge in this space, as the difference between a score of 0.1 and 0.3, or 0.7 and 0.9, is
very hard to subjectively define. This model is a black-box when it comes to numbers, so users who want to consider
multiple thresholds should learn the behaviors of Vectara's HEM on their own data rather than naively defining more
thresholds such as `0.25` and `0.75`.

Overall, Vectara's HEM is open-source, quick and easy to use, and is a very strong starting point in any hallucination
detection system.
