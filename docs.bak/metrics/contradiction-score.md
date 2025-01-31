---
description: How to calculate and interpret contradiction scores for NLP
---

# Contradiction Score

The [Cross-Encoder for Natural Language Inference](https://huggingface.co/cross-encoder/nli-deberta-v3-base) (NLI) is a
text classification model that takes a pair of text and assigns a label: `'contradiction'`, `'entailment'` or
`'neutral'`. Additionally, it assigns a probability ranging from 0 to 1 for each label. The higher the
score, the more confident the model is to assign that label. So, it assigns the label with the highest score. This is
useful for hallucination detection, as factual consistency implies the absence of contradictions.

## Implementation Details

This cross-encoder for NLI classification is based on Microsoft's
[deberta-v3-base](https://huggingface.co/microsoft/deberta-v3-base), which was trained on NLI data, using
[`sentence_transformers.cross_encoder`](https://www.sbert.net/docs/package_reference/cross_encoder.html).

Further details for development and usage can be found on Hugging Face:
[cross-encoder/nli-deberta-v3-base](https://huggingface.co/vectara/hallucination_evaluation_model).
Below is a quick example of how it can be used:

```py
# Installation:
# pip install -U sentence-transformers

from sentence_transformers import CrossEncoder
nli_model = CrossEncoder('cross-encoder/nli-deberta-v3-base')

def compute_metric(ground_truth: str, inference: str) -> float:
    scores = nli_model.predict([ground_truth, inference], apply_softmax=True)
    label = ['contradiction', 'entailment', 'neutral'][scores.argmax()]
    return {
        'label': label,
        'contradiction': scores[0],
        'entailment': scores[1],
        'neutral': scores[2],
    }

print(compute_metric("The duck crossed the road", "The duck did not cross the road"))
# Outputs:
# {'label': 'contradiction', 'contradiction': 0.999961, 'entailment': 0.000010, 'neutral': 0.000028}
```

## Examples

| Ground Truth | Inference | <nobr>Classification</nobr> | Contradiction | Entailment | Neutral |
| --- | --- | --- | --- | --- | --- |
| `The duck crossed the road` | `The duck did not cross the road` | `contradiction` | `0.999` | `0.000` | `0.000` |
| `The duck crossed the road` | `The animal crossed the road` | `entailment` | `0.000` | `0.971` | `0.029` |
| `The duck crossed the road` | `The duck crossed my path` | `neutral` | `0.011` | `0.096` | `0.893` |

## Limitations and Biases

1. The model provided on Hugging Face suggests that expected input is simply pairs of text. This means that lengthy
context cannot be provided. If relevant, context can be added to the front of both texts within the pair. For example:
`("What did the duck do? The duck crossed the road.", "What did the duck do? The animal crossed the road.")`.

2. Explainability is a challenge for these scores. This model is a black-box that is not able to explain the reasons
for assigning a contradiction score of `0.5` or `0.7`. Users should examine this classifier's behavior on their own
data, and see how extra details within a ground truth or inference might impact the decision of assigning
`'contradiction'`, `'entailment'` or `'neutral'`.

Overall, this NLI classifier is quick and easy to use, and provides insightful metrics relevant to a hallucination
detection system.
