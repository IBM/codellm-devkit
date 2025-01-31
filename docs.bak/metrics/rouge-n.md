---
description: Measuring textual similarity with N-gram overlap
---

# ROUGE-N

!!! info inline end "ROUGE vs. Recall"

    Complimentary to [BLEU](bleu.md), ROUGE-N can be thought of as an analog to [recall](recall.md) for text
    comparisons.

ROUGE-N (**R**ecall-**O**riented **U**nderstudy for **G**isting **E**valuation), a metric within the broader ROUGE
metric collection, is a vital metric in the field of natural language processing and text evaluation. It assesses the
quality of a candidate text by measuring the overlap of n-grams between the candidate text and reference texts, and
ranges between 0 and 1. A score of 0 indicates no overlap between candidate and reference texts, whereas a perfect
score of 1 indicates perfect overlap. ROUGE-N provides insights into the ability of a system to capture essential
content and linguistic nuances, making it an important and versatile tool used in many NLP workflows. As the name
implies, it is a recall-based metric — a complement to the precision-based BLEU score.

## Implementation Details

Formally, ROUGE-N is an n-gram recall between a candidate and a set of reference texts. That is, ROUGE-N calculates the
number of overlapping n-grams between the generated and reference texts divided by the total number of n-grams in the
reference texts.

Mathematically, we define ROUGE-N as follows:

<!-- markdownlint-disable MD013 -->

$$
\text{ROUGE-N} = \frac{\sum_{S \in \text{Reference Texts}} \sum_{\text{n-gram} \in S} \text{Match}(\text{n-gram})}{\sum_{S \in \text{Reference Texts}} \sum_{\text{n-gram} \in S} \text{Count}(\text{n-gram})}
$$

<!-- markdownlint-enable MD013 -->

where $\text{Match(n-gram)}$ is the maximum number of n-grams co-occuring in a candidate text and set of reference
texts.

It is clear that ROUGE-N is analogous to a recall-based measure, since the denominator of the equation is the sum of the
number of n-grams on the reference-side.

### Multiple References - Taking the Max

We usually want to compare a candidate text against multiple reference texts, as there is no single correct reference
text that can capture all semantic nuances. Though the above formula is sufficient for calculating ROUGE-N across
multiple references, another proposed way of calculating ROUGE-N across multiple references, as highlighted in the
[original ROUGE paper](https://aclanthology.org/W04-1013.pdf), is as follows:

We compute pairwise ROUGE-N between a candidate text s, and every reference, $r_i$, in the reference set, then take the
maximum of pairwise ROUGE-N scores as the final multiple reference ROUGE-N score. That is,

$$
\text{ROUGE-N}_\text{multi} = argmax_i \space \text{ROUGE-N}(r_i, s)
$$

The decision to use classic $\text{ROUGE-N}$ or $\text{ROUGE-N}_\text{multi}$ is up to the user.

### Interpretation

When using ROUGE-N, it is important to consider the metric for multiple values of N (n-gram length). For smaller values
of N, like ROUGE-1, it places more focus on capturing the presence of keywords or content terms in the candidate text.
For larger values of N, like ROUGE-3, it places focus on syntactic structure and replicating linguistic patterns. In
other words, ROUGE-1 and small values of N are suitable for tasks where the primary concern is to assess whether the
candidate text contains essential vocabulary, while ROUGE-3 and larger values of N are used in tasks where sentence
structure and fluency are important.

Generally speaking, a higher ROUGE-N score is desirable. However, the score varies among different tasks and values of
N, so it is a good idea to benchmark your model's ROUGE-N score against other models trained on the same data, or
previous iterations of your model.

## Examples

### ROUGE-1 (Unigrams)

Assume we have the following candidate and reference texts:

| | |
| --- | --- |
| **Reference #1** | `A fast brown dog jumps over a sleeping fox` |
| **Reference #2** | `A quick brown dog jumps over the fox` |
| **Candidate** | `The quick brown fox jumps over the lazy dog` |

??? example "Step 1: Tokenization & n-Grams"
    Splitting our candidate and reference texts into unigrams, we get the following:

    | | |
    | --- | --- |
    | **Reference #1** | [`A`, `fast`, `brown`, `dog`, `jumps`, `over`, `a`, `sleeping`, `fox`] |
    | **Reference #2** | [`A`, `quick`, `brown`, `dog`, `jumps`, `over`, `the`, `fox`] |
    | **Candidate** | [`The`, `quick`, `brown`, `fox`, `jumps`, `over`, `the`, `lazy`, `dog`] |

??? example "Step 2: Calculate ROUGE"
    Recall that our ROUGE-N formula is: $\frac{\text{# of overlapping n-grams}}{\text{# of unigrams in reference}}$

    There are 5 overlapping unigrams in the first reference and 7 in the second reference, and 9 total unigrams in the
    first reference and 8 in the second.
    Thus our calculated ROUGE-1 score is $\frac{12}{17} \approx 0.706$

### ROUGE-2 (Bigrams)

Assume we have the same following candidate and reference texts:

| | |
| --- | --- |
| **Reference #1** | `A fast brown dog jumps over a sleeping fox` |
| **Reference #2** | `A quick brown dog jumps over the fox` |
| **Candidate** | `The quick brown fox jumps over the lazy dog` |

??? example "Step 1: Tokenization & n-Grams"
    Splitting our candidate and reference texts into bigrams, we get the following:

    <!-- markdownlint-disable MD013 -->

    | | |
    | --- | --- |
    | <nobr>**Reference #1**</nobr> | [`A fast`, `fast brown`, `brown dog`, `dog jumps`, `jumps over`, `over a`, `a sleeping`, `sleeping fox`] |
    | <nobr>**Reference #2**</nobr> | [`A quick`, `quick brown`, `brown dog`, `dog jumps`, `jumps over`, `over the`, `the fox`] |
    | **Candidate** | [`The quick`, `quick brown`, `brown fox`, `fox jumps`, `jumps over`, `over the`, `the lazy`, `lazy dog`] |

    <!-- markdownlint-enable MD013 -->

??? example "Step 2: Calculate ROUGE"
    Recall that our ROUGE-N formula is: $\frac{\text{# of overlapping n-grams}}{\text{# of unigrams in reference}}$

    There is 1 overlapping bigram in the first reference and 3 in the second reference, and 8 total bigrams in the first
    reference and 7 in the second.
    Thus our calculated ROUGE-2 score is $\frac{4}{15} = 0.267$

    Note that our ROUGE-2 score is significantly lower than our ROUGE-1 score on the given candidate and reference
    texts. It is always important to consider multiple n-grams when using ROUGE-N, as one value of N does not give a
    holistic view of candidate text quality.

## Limitations and Biases

ROUGE-N, like any other n-gram based metric, suffers from the following limitations:

1. Unlike [BERTScore](bertscore.md), ROUGE-N is not able to consider order, context, or semantics when calculating a
score. Since it only relies on overlapping n-grams, it can not tell when a synonym is being used or if the placement of
two matching n-grams have any meaning on the overall sentence. As a result, the metric may not be a perfect
representation of the quality of the text, but rather the "likeness" of the n-grams in two sentences. Take for example,
the ROUGE-2 score of "This is an example of text" and "Is an example of text this". Both ROUGE-1 and ROUGE-2 would give
this a (nearly) perfect score, but the second sentence makes absolutely no sense!

2. ROUGE-N can not capture global coherence. Given a long paragraph, realistically, having too large of a value for N
would not return a meaningful score for two sentences, but having a reasonable number like N = 3 wouldn't be able to
capture the flow of the text. The score might yield good results, but the entire paragraph might not flow smoothly at
all. This is a weakness of n-gram based metrics, as they are limited to short context windows.

That being said, ROUGE-N has some advantages over embeddings-based metrics. First of all, it is very simple and easy to
compute — it is able to calculate scores for large corpuses efficiently with no specialized hardware. ROUGE-N is also
relatively easy to interpret. The N value can be adjusted to measure the granularity of measurements, and higher scores
indicate greater overlap with the reference text. In fact, ROUGE is very widely used in NLP which allows engineers to
benchmark their models against others on most open-source NLP datasets. Lastly, it can be used in complement with other
n-gram based metrics like BLEU to provide a more holistic view of test results — since BLEU provides a precision-related
score, and ROUGE provides a recall-related score, it makes it easier to pinpoint potential failure cases.
