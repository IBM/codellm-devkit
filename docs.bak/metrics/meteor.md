---
description: Measuring textual similarity with N-gram precision and recall
---

# METEOR

METEOR (**M**etric for **E**valuation of **T**ranslation with **E**xplicit **OR**dering) is a widely recognized and
vital metric used in natural language processing. Originally developed for machine translation workflows, it is used to
measure the quality of candidate texts against reference texts for many different workflows. Though it is an n-gram
based metric, it goes beyond traditional methods by factoring in elements such as precision, recall, and order to
provide a comprehensive measure of text quality. For an in-depth justification behind METEOR's design choices, feel free
to check out the [original paper](https://www.cs.cmu.edu/~alavie/METEOR/pdf/Banerjee-Lavie-2005-METEOR.pdf).

## Implementation Details

We define METEOR as the product of two components - the Unigram Precision / Recall Harmonic Mean, and Word Order
Penalty. That is,

<!-- markdownlint-disable MD013 -->

$$
\text{METEOR} = \underbrace{\text{FMean}}_{\text{Harmonic Mean of Unigram Precision/Recall}} * \underbrace{(1 - \text{Penalty})}_{\text{Word Order Penalty}}
$$

<!-- markdownlint-enable MD013 -->

To understand the formula, let's break down each component into their respective parts.

??? info "FMean: Harmonic Mean of the Unigram Precision / Recall"
    This is defined as

    $$
    FMean = \frac{10PR}{R + 9P}
    $$

    where P represents the unigram precision, and R represents the unigram recall. Here's a recap of
    [precision](precision.md) and [recall](recall.md). Notice that most of the weight is placed on the recall component
    by design – this allows METEOR to prioritize the coverage of essential keywords in the candidate text.

??? info "Penalty: Word Order Penalty"
    Since the FMean is based on unigram precision and recall, to take into account longer sequences, METEOR has a
    penalty factor to alleviate this weakness and enforce an order on the candidate sentence.

    First, the  unigrams in the candidate text that are mapped to unigrams in the reference text are grouped in such a
    way that there exists the fewest number of *chunks*, where each chunk consists of adjacent unigrams. Our penalty
    factor is then defined as:

    $$
    Penalty = 0.5 \times \frac{\text{# of Chunks}}{\text{# of Unigrams Matched}}
    $$

    For example, if our candidate sentence was "the president spoke to the audience" and our reference sentence was
    "the president *then* spoke to the audience", there would be two chunks – "the president" and "spoke to the
    audience" – and 6 unigrams matched. Notice that as the chunks decrease, so does the penalty, which results in a
    higher METEOR score. This is quite intuitive as a lower number of chunks translates to an enforced order and better
    alignment with the reference text.

## Examples

| Candidate | Reference |
| --- | --- |
| `Under the starry night, we danced with glee.` | `We danced with joy under the starry night.` |

??? example "Step 1: Calculate FMean"
    Upon analysis, our precision is $\frac{7}{8} = 0.875$ and our recall is also $\frac{7}{8} = 0.875$.
    As a result, our FMean is

    $$
    \frac{10 \times 0.875 \times 0.875}{0.875 + 9 \times 0.875} = 0.875
    $$

??? example "Step 2: Calculate Word Order Penalty"
    We can break up our candidate sentence into two chunks to map it to our reference sentence.

    <!-- markdownlint-disable MD013 -->

    Candidate: $\underbrace{\text{Under the starry night}}_{\text{Chunk 2}} \space \underbrace{\text{we danced with}}_{\text{Chunk 1}} \space\text{glee}$ <br>
    Reference: $\underbrace{\text{We danced with}}_{\text{Chunk 1}} \space\text{joy}\space \underbrace{\text{under the starry night}}_{\text{Chunk 2}}$

    <!-- markdownlint-enable MD013 -->

    Between the two chunks, we have matched 7 unigrams. This gives us a penalty score of $0.5 \times \frac{2}{7} = 0.143$.

??? example "Step 3: Calculate METEOR"
    With our Penalty and FMean calculated, we can proceed with calculating the METEOR score.

    $$
    \text{METEOR} = 0.875 * (1 - 0.143) = 0.750.
    $$

    Not bad! We have a pretty high score for two sentences that are semantically very similar but have different orders.

Lets try the same reference example with a slightly different candidate.

| Candidate | Reference |
| --- | --- |
| `Danced we with under joy the night starry.` | `We danced with joy under the starry night.` |

??? example "Step 1: Calculate FMean"
    Our first step is trivial. Since both sentences contain the same words, our unigram precision and recall are both
    1.0.
    As a result, our FMean is $\frac{10 \times 1.0 \times 1.0}{1.0 + 9 \times 1.0} = 1$

??? example "Step 2: Calculate Word Order Penalty"
    Our penalty is different from the first example, due to the jumbled up order. We split our candidate sentence into
    8 chunks, since no adjacent words can be mapped to the reference sentence.

    <!-- markdownlint-disable MD013 -->

    Candidate: $\underbrace{\text{Danced}}_\text{Chunk 2}\space\underbrace{\text{we}}_\text{Chunk 1}\space\underbrace{\text{with}}_\text{Chunk 3}\space\underbrace{\text{under}}_\text{Chunk 5}\space\underbrace{\text{joy}}_\text{Chunk 4}\space\underbrace{\text{the}}_\text{Chunk 6}\space\underbrace{\text{night}}_\text{Chunk 8}\space\underbrace{\text{starry}}_\text{Chunk 7}\space$

    Reference: $\underbrace{\text{We}}_\text{Chunk 1}\space\underbrace{\text{danced}}_\text{Chunk 2}\space\underbrace{\text{with}}_\text{Chunk 3}\space\underbrace{\text{joy}}_\text{Chunk 4}\space\underbrace{\text{under}}_\text{Chunk 5}\space\underbrace{\text{the}}_\text{Chunk 6}\space\underbrace{\text{starry}}_\text{Chunk 7}\space\underbrace{\text{night}}_\text{Chunk 8}\space$

    <!-- markdownlint-enable MD013 -->

    Between the eight chunks, we have matched 8 unigrams. This gives us a penalty score of $0.5 \times \frac{8}{8} = 0.5$.

??? example "Step 3: Calculate METEOR"
    With our Penalty and FMean calculated, we can proceed with calculating the METEOR score.

    $$
    \text{METEOR} = 1 * (1 - 0.5) = 0.5.
    $$

    Despite having all the keywords of the reference sentence, our candidate had the wrong order and meaning! This is a
    massive improvement over something like [ROUGE-1](rouge-n.md) which would not have considered the orders of the
    sentences, and given a perfect score of 1.0.

## Limitations and Biases

Although METEOR was created to address some of the major limitations of [BLEU](bleu.md), it still comes with its own
limitations.

1. METEOR does not consider synonyms. Unlike embeddings-based metrics like [BERTScore](bertscore.md), it does not have
a mechanism to quantify the similarity of words within the candidate and reference sentences. Thus, having two sentences
like "She looked extremely happy at the surprise party." and "She appeared exceptionally joyful during the unexpected
celebration." would yield a suboptimal score despite being very similar in meaning. That being said, METEOR has shown to
have a higher correlation with human judgement than both BLEU and ROUGE, making it *generally* better than the two.

2. METEOR can fail on context. If we have two sentences "I am a big fan of Taylor Swift" (Reference) and "Fan of Taylor
Swift I am big" (Candidate), METEOR would yield a good score. However, the candidate sentence makes little sense and
intuitively shouldn't be given a good score. This is a limitation with all n-gram metrics, and not specific to METEOR.

Limitations aside, METEOR is still a great metric to include in NLP workflows for measuring text similarity. Like other
n-gram metrics, it is easy to compute and doesn't require extra hardware for inference. Furthermore, it is a noticeable
improvement over BLEU, and even ROUGE, in many ways – it places weight on *both* precision and recall, factors in word
order, and generally does a better job at filtering out bad candidate texts, as seen in example 2. It is also a better
judge of global coherence than BLEU and ROUGE, since it greedily looks for the largest chunks to calculate its penalty
factor, rather than using a sliding context window of n-grams. METEOR is a powerful metric, and should be included in
every NLP toolbox to give a holistic view of model performance.
